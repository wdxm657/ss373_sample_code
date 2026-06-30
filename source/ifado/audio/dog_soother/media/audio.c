/**
 * @file audio.c
 * @brief 音量/主人录音 UART、录制线程写 WAV、0x84 EVT
 */
#define LOG_TAG "audio"
#include "log.h"

#include "audio.h"
#include "audio_sys.h"
#include "audio_ai.h"
#include "audio_ao.h"
#include "audio_stream.h"
#include "app_config.h"
#include "app_types.h"
#include "uart_cmd.h"
#include "uart_proto.h"

#include "st_common_audio.h"
#include "st_common.h"
#include "comfort_store.h"
#include "bark_detect.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int8_t g_volume = -15;            /* -60..30 dB，默认 -15 dB（与 audio_ao_init 一致） */
static volatile uint8_t g_recording;    /* 1 时采集线程向 rec 队列投递 */
static uint8_t g_owner_duration_sec;    /* 最近一次有效录音时长 */
static FILE *g_rec_fp;              /* 录制中的 WAV，先写 PCM 后回填头 */
static uint32_t g_rec_pcm_bytes;
static time_t g_rec_start_sec;      /* 用于 10s 自动停 */
static time_t g_owner_tmp_create_sec;   /* tmp 文件创建时间戳；0=无待移动的 tmp 文件 */

static volatile int g_rec_thread_run;
static pthread_t g_rec_thread;

static volatile int g_owner_rec_ai_paused;   /* 1=因主人录音暂停了音频识别 */
static volatile int g_owner_play_ai_paused;  /* 1=因主人播放暂停了音频识别 */

static int audio_is_recording(void) /* 供 audio_stream rec_active 回调 */
{
    return g_recording ? 1 : 0;
}

static int owner_file_exists(void) /* DS_OWNER_PCM_PATH 是否可访问 */
{
    return access(DS_OWNER_PCM_PATH, F_OK) == 0;
}

static MI_S32 audio_wav_fill_header(WaveFileHeader_t *hdr, uint32_t pcm_bytes) /* 16kHz mono 16bit */
{
    if (!hdr)
    {
        return -1;
    }
    hdr->chRIFF[0] = 'R';
    hdr->chRIFF[1] = 'I';
    hdr->chRIFF[2] = 'F';
    hdr->chRIFF[3] = 'F';
    hdr->chWAVE[0] = 'W';
    hdr->chWAVE[1] = 'A';
    hdr->chWAVE[2] = 'V';
    hdr->chWAVE[3] = 'E';
    hdr->chFMT[0] = 'f';
    hdr->chFMT[1] = 'm';
    hdr->chFMT[2] = 't';
    hdr->chFMT[3] = ' ';
    hdr->dwFMTLen = 0x10;
    hdr->wave.wChannels = 1;
    hdr->wave.wFormatTag = 1;
    hdr->wave.wBitsPerSample = 16;
    hdr->wave.dwSamplesPerSec = DS_AUDIO_SAMPLE_RATE;
    hdr->wave.dwAvgBytesPerSec = DS_AUDIO_SAMPLE_RATE * 2;
    hdr->wave.wBlockAlign = 2;
    hdr->chDATA[0] = 'd';
    hdr->chDATA[1] = 'a';
    hdr->chDATA[2] = 't';
    hdr->chDATA[3] = 'a';
    hdr->dwDATALen = pcm_bytes;
    hdr->dwRIFFLen = pcm_bytes + sizeof(WaveFileHeader_t) - 8;
    return MI_SUCCESS;
}

static void audio_owner_rec_stop_internal(uint8_t auto_stop) /* 写头、发 0x84、不足最小时长删文件 */
{
    uint8_t duration_sec = 0;

    if (!g_recording)
    {
        return;
    }
    g_recording = 0;
    if (g_owner_rec_ai_paused)
    {
        g_owner_rec_ai_paused = 0;
        bark_detect_set_active(1);
    }

    if (g_rec_fp)
    {
        WaveFileHeader_t hdr;
        audio_wav_fill_header(&hdr, g_rec_pcm_bytes);
        fseek(g_rec_fp, 0, SEEK_SET);
        fwrite(&hdr, sizeof(hdr), 1, g_rec_fp);
        fclose(g_rec_fp);
        g_rec_fp = NULL;
        duration_sec = (uint8_t)(g_rec_pcm_bytes / (DS_AUDIO_SAMPLE_RATE * 2));
        if (duration_sec > DS_OWNER_REC_MAX_SEC)
        {
            duration_sec = DS_OWNER_REC_MAX_SEC;
        }
    }

    g_owner_duration_sec = duration_sec;
    LOG_INFO("owner rec stop (%s) duration=%u s bytes=%u\n",
             auto_stop ? "auto" : "manual",
             duration_sec,
             g_rec_pcm_bytes);

    /* 录制完成 → 文件在 tmp 目录；记录时间戳用于超时清理 */
    if (duration_sec >= DS_OWNER_REC_MIN_SEC)
    {
        g_owner_tmp_create_sec = time(NULL);
        uint8_t evt[3] = {0x01, DS_UART_STATUS_OK, duration_sec};
        uart_proto_send_evt(DS_EVT_OWNER_REC, 0, evt, sizeof(evt));
    }
    else if (duration_sec > 0)
    {
        g_owner_tmp_create_sec = 0;
        uint8_t evt[3] = {0x01, DS_UART_STATUS_PARAM_ERROR, duration_sec};
        uart_proto_send_evt(DS_EVT_OWNER_REC, 0, evt, sizeof(evt));
        unlink(DS_OWNER_REC_TMP_PATH);
        g_owner_duration_sec = 0;
    }
    else
    {
        g_owner_tmp_create_sec = 0;
        unlink(DS_OWNER_REC_TMP_PATH);
        g_owner_duration_sec = 0;
    }
}

static void audio_rec_process_frame(const audio_stream_frame_t *frame) /* 追加 PCM；满 DS_OWNER_REC_MAX_SEC 自动停 */
{
    time_t now;

    if (!g_recording || !frame || frame->num_samples == 0)
    {
        return;
    }

    now = time(NULL);
    if (g_rec_start_sec > 0 && (now - g_rec_start_sec) >= DS_OWNER_REC_MAX_SEC)
    {
        audio_owner_rec_stop_internal(1);
        return;
    }

    if (g_rec_fp)
    {
        size_t bytes = frame->num_samples * sizeof(int16_t);
        fwrite(frame->pcm, 1, bytes, g_rec_fp);
        g_rec_pcm_bytes += (uint32_t)bytes;
    }
}

static void *audio_rec_thread(void *arg) /* audio_stream_rec_pop → 写盘 / 超时停录 */
{
    (void)arg;

    LOG_INFO("audio rec thread start\n");
    while (g_rec_thread_run)
    {
        audio_stream_frame_t frame;
        int ret = audio_stream_rec_pop(&frame);

        if (ret == 0)
        {
            audio_rec_process_frame(&frame);
            continue;
        }
        if (ret == 1) /* 队列已关闭（audio_stream_stop） */
        {
            break;
        }
        if (!g_rec_thread_run)
        {
            break;
        }
        /* 主人播放结束时恢复音频识别 */
        if (g_owner_play_ai_paused && !audio_ao_is_playing())
        {
            g_owner_play_ai_paused = 0;
            bark_detect_set_active(1);
        }

        /* tmp 录音文件超时清理（10 分钟未保存则删除） */
        if (g_owner_tmp_create_sec > 0)
        {
            time_t now = time(NULL);
            if ((now - g_owner_tmp_create_sec) >= DS_OWNER_REC_MOVE_TIMEOUT_SEC)
            {
                LOG_INFO("owner rec tmp file expired, deleting\n");
                unlink(DS_OWNER_REC_TMP_PATH);
                g_owner_tmp_create_sec = 0;
                g_owner_duration_sec = 0;
            }
        }
        usleep(10000);
    }
    LOG_INFO("audio rec thread exit\n");
    return NULL;
}

static int audio_rec_thread_start(void) /* 从 rec 队列取帧写 WAV */
{
    g_rec_thread_run = 1;
    if (pthread_create(&g_rec_thread, NULL, audio_rec_thread, NULL) != 0)
    {
        g_rec_thread_run = 0;
        LOG_ERROR("pthread_create rec failed\n");
        return -1;
    }
    return 0;
}

static void audio_rec_thread_stop(void) /* join 录制线程 */
{
    if (!g_rec_thread_run)
    {
        return;
    }
    g_rec_thread_run = 0;
    pthread_join(g_rec_thread, NULL);
}

static int audio_pick_ai_gain_db(void) /* 默认 DS_AI_GAIN_DB；可用环境变量 DS_AI_GAIN_DB 覆盖 */
{
    const char *env = getenv("DS_AI_GAIN_DB");
    if (env && env[0])
    {
        return atoi(env);
    }
    return DS_AI_GAIN_DB;
}

int audio_init(void) /* AI+AO+采集流+录制线程；失败时回滚已初始化部分 */
{
    int ai_gain = audio_pick_ai_gain_db();

    g_recording = 0;
    g_rec_fp = NULL;
    g_rec_pcm_bytes = 0;
    g_owner_duration_sec = 0;
    g_owner_rec_ai_paused = 0;
    g_owner_play_ai_paused = 0;
    g_owner_tmp_create_sec = 0;

    if (audio_ai_init(ai_gain) != 0)
    {
        LOG_ERROR("audio_ai_init failed\n");
        return -1;
    }
    if (audio_ao_init() != 0)
    {
        LOG_ERROR("audio_ao_init failed\n");
        audio_ai_deinit();
        return -2;
    }
    audio_ao_set_volume_level(g_volume);

    audio_stream_set_rec_active_fn(audio_is_recording);
    if (audio_stream_start() != 0)
    {
        LOG_ERROR("audio_stream_start failed\n");
        audio_ao_deinit();
        audio_ai_deinit();
        return -3;
    }
    if (audio_rec_thread_start() != 0)
    {
        audio_stream_stop();
        audio_ao_deinit();
        audio_ai_deinit();
        return -4;
    }

    LOG_INFO("audio init ok (AI+AO+capture+rec thread)\n");
    return 0;
}

void audio_deinit(void) /* 停录、停流、停播、释放 MI */
{
    audio_owner_rec_stop_internal(1);
    /* 清理未保存的 tmp 录音文件 */
    unlink(DS_OWNER_REC_TMP_PATH);
    g_owner_tmp_create_sec = 0;
    /* 必须先停流并关闭队列，否则 rec/detect 线程阻塞在 pop 上无法 join */
    audio_stream_stop();
    audio_rec_thread_stop();
    audio_ao_stop();
    audio_ao_deinit();
    audio_ai_deinit();
    audio_sys_deinit();
}

void audio_refresh_owner_info(uint8_t *exist, uint8_t *duration_sec) /* 供 STATUS_GET / comfort_store */
{
    if (exist)
    {
        *exist = owner_file_exists() ? 1 : 0;
    }
    if (duration_sec)
    {
        if (owner_file_exists() && g_owner_duration_sec == 0)
        {
            /* 重启后未缓存时长时仅报存在 */
            *duration_sec = 0;
        }
        else
        {
            *duration_sec = g_owner_duration_sec;
        }
    }
}

void audio_delete_owner_rec(void) /* 停录并删除录音文件 */
{
    audio_owner_rec_stop_internal(0);
    unlink(DS_OWNER_REC_TMP_PATH);
    unlink(DS_OWNER_PCM_PATH);
    g_owner_duration_sec = 0;
    g_owner_tmp_create_sec = 0;
}

uint8_t audio_get_volume(void) /* 返回当前 dB 值（缓存），uint8 编码 */
{
    return (uint8_t)g_volume;
}

uint16_t audio_set_volume( /* payload[0]=dB值(-60..30 以 int8 编码)；rsp: status + 当前dB */
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap)
{
    if (!rsp || rsp_cap < 2 || !payload || payload_len < 1)
    {
        return 0;
    }
    g_volume = (int8_t)payload[0];
    if (g_volume < -60) g_volume = -60;
    if (g_volume > 30)  g_volume = 30;
    LOG_INFO("set volume %d dB\n", g_volume);
    if (audio_ao_set_gain_db(g_volume) != 0)
    {
        rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
        return 1;
    }
    rsp[0] = DS_UART_STATUS_OK;
    rsp[1] = (uint8_t)g_volume;
    return 2;
}

uint16_t audio_handle_uart_cmd( /* 0x20~0x25 主人录音；返回 rsp 长度，0 表示参数错误 */
    uint8_t cmd_id,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap)
{
    (void)payload;
    (void)payload_len;
    if (!rsp || rsp_cap < 1)
    {
        return 0;
    }

    switch (cmd_id)
    {
    case DS_CMD_OWNER_REC_START:
        if (g_recording)
        {
            rsp[0] = DS_UART_STATUS_BUSY;
            return 1;
        }
        if (!audio_ai_is_ready())
        {
            rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
            return 1;
        }
        {
            ds_work_state_t ws = comfort_store_get_work_state();
            if (ws != DS_WORK_OFF && ws != DS_WORK_MONITORING)
            {
                rsp[0] = DS_UART_STATUS_STATE_CONFLICT;
                return 1;
            }
            if (ws == DS_WORK_MONITORING)
            {
                bark_detect_set_active(0);
                g_owner_rec_ai_paused = 1;
            }
        }
        /* 录制到 tmp 目录，待 OWNER_REC_SAVE 时移到播放目录 */
        ST_Common_CheckMkdirOutFile((char *)DS_OWNER_REC_TMP_PATH);
        g_rec_fp = fopen(DS_OWNER_REC_TMP_PATH, "wb");
        if (!g_rec_fp)
        {
            rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
            return 1;
        }
        {
            WaveFileHeader_t hdr;
            memset(&hdr, 0, sizeof(hdr));
            fwrite(&hdr, sizeof(hdr), 1, g_rec_fp);
        }
        g_rec_pcm_bytes = 0;
        g_rec_start_sec = time(NULL);
        g_recording = 1;
        LOG_INFO("owner rec start -> %s (tmp)\n", DS_OWNER_REC_TMP_PATH);
        rsp[0] = DS_UART_STATUS_OK;
        return 1;

    case DS_CMD_OWNER_REC_STOP:
        audio_owner_rec_stop_internal(0);
        rsp[0] = DS_UART_STATUS_OK;
        return 1;

    case DS_CMD_OWNER_REC_PLAY:
        {
            /* payload[0]=0 播放已保存音频, =1 播放 tmp 音频；空 payload 等价于 0 */
            uint8_t play_src = (payload_len >= 1) ? payload[0] : 0;
            const char *play_path = (play_src == 1) ? DS_OWNER_REC_TMP_PATH : DS_OWNER_PCM_PATH;

            if (access(play_path, F_OK) != 0)
            {
                rsp[0] = DS_UART_STATUS_NOT_FOUND;
                return 1;
            }
            {
                ds_work_state_t ws = comfort_store_get_work_state();
                if (ws != DS_WORK_OFF && ws != DS_WORK_MONITORING)
                {
                    rsp[0] = DS_UART_STATUS_STATE_CONFLICT;
                    return 1;
                }
                if (ws == DS_WORK_MONITORING)
                {
                    bark_detect_set_active(0);
                    g_owner_play_ai_paused = 1;
                }
            }
            LOG_INFO("owner rec play src=%u path=%s\n", play_src, play_path);
            if (audio_ao_play_wav_file(play_path) != 0)
            {
                rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
            return 1;
        }
        rsp[0] = DS_UART_STATUS_OK;
        return 1;

    case DS_CMD_OWNER_REC_PLAY_STOP:
        audio_ao_stop();
        if (g_owner_play_ai_paused)
        {
            g_owner_play_ai_paused = 0;
            bark_detect_set_active(1);
        }
        rsp[0] = DS_UART_STATUS_OK;
        return 1;

    case DS_CMD_OWNER_REC_DELETE:
        audio_delete_owner_rec();
        rsp[0] = DS_UART_STATUS_OK;
        return 1;

    case DS_CMD_OWNER_REC_SAVE:
        /* 将 tmp/owner.wav 移到播放目录；若不存在则返回 NOT_FOUND */
        if (access(DS_OWNER_REC_TMP_PATH, F_OK) != 0)
        {
            rsp[0] = DS_UART_STATUS_NOT_FOUND;
            return 1;
        }
        /* 确保播放目录存在 */
        ST_Common_CheckMkdirOutFile((char *)DS_OWNER_PCM_PATH);
        if (rename(DS_OWNER_REC_TMP_PATH, DS_OWNER_PCM_PATH) != 0)
        {
            LOG_ERROR("rename %s -> %s failed\n", DS_OWNER_REC_TMP_PATH, DS_OWNER_PCM_PATH);
            rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
            return 1;
        }
        g_owner_tmp_create_sec = 0;  /* 已保存，取消超时清理 */
        g_owner_duration_sec = (uint8_t)(g_rec_pcm_bytes / (DS_AUDIO_SAMPLE_RATE * 2));
        if (g_owner_duration_sec > DS_OWNER_REC_MAX_SEC)
        {
            g_owner_duration_sec = DS_OWNER_REC_MAX_SEC;
        }
        LOG_INFO("owner rec saved -> %s duration=%u\n", DS_OWNER_PCM_PATH, g_owner_duration_sec);
        rsp[0] = DS_UART_STATUS_OK;
        rsp[1] = g_owner_duration_sec;
        return 2;

    case DS_CMD_OWNER_REC_INFO_GET:
        rsp[0] = DS_UART_STATUS_OK;
        rsp[1] = owner_file_exists() ? 1 : 0;
        rsp[2] = g_owner_duration_sec;
        return 3;

    default:
        rsp[0] = DS_UART_STATUS_PARAM_ERROR;
        return 1;
    }
}
