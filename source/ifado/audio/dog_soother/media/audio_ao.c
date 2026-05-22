/** @file audio_ao.c @brief MI AO WAV 播放与音量档位 */
#define LOG_TAG "audio_ao"
#include "log.h"

#include "audio_ao.h"
#include "audio_sys.h"
#include "app_config.h"

#include "mi_sys.h"
#include "mi_ao.h"
#include "st_common_audio.h"
#include "st_common.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_ao_ready;
static MI_AUDIO_DEV g_ao_dev = DS_AO_DEV_ID;
static volatile int g_ao_stop_req;   /* audio_ao_stop 置 1 打断播放循环 */
static volatile int g_ao_playing;
static pthread_t g_ao_play_thread;
static char g_ao_play_path[256];

static int audio_ao_apply_gain_level(uint8_t level) /* level + DS_AO_VOLUME_BASE_GAIN → MI_AO_SetVolume */
{
    int gain = (int)level + DS_AO_VOLUME_BASE_GAIN;
    if (gain < -60)
    {
        gain = -60;
    }
    if (gain > 30)
    {
        gain = 30;
    }
    MI_S32 ret = MI_AO_SetVolume(g_ao_dev, (MI_S16)gain, (MI_S16)gain, 0);
    return (ret == MI_SUCCESS) ? 0 : -1;
}

int audio_ao_init(void) /* DAC 16kHz mono */
{
    if (g_ao_ready)
    {
        return 0;
    }
    if (audio_sys_init() != 0)
    {
        return -1;
    }

    MI_AO_Attr_t attr;
    MI_AO_If_e ao_if[] = {E_MI_AO_IF_DAC_AB};

    memset(&attr, 0, sizeof(attr));
    ST_Common_GetAoDefaultDevAttr(&attr);
    attr.enSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;

    if (ST_Common_AoOpenDev(g_ao_dev, &attr) != MI_SUCCESS)
    {
        LOG_ERROR("AoOpenDev failed\n");
        return -2;
    }
    if (ST_Common_AoAttachIf(g_ao_dev, ao_if) != MI_SUCCESS)
    {
        LOG_ERROR("AoAttachIf failed\n");
        ST_Common_AoCloseDev(g_ao_dev);
        return -3;
    }
    if (audio_ao_apply_gain_level(30) != 0)
    {
        LOG_ERROR("AoSetVolume failed\n");
    }

    g_ao_ready = 1;
    LOG_INFO("audio_ao ready dev=%u 16kHz mono\n", g_ao_dev);
    return 0;
}

void audio_ao_deinit(void) /* 先 stop 再 CloseDev */
{
    audio_ao_stop();
    if (!g_ao_ready)
    {
        return;
    }
    ST_Common_AoCloseDev(g_ao_dev);
    g_ao_ready = 0;
}

int audio_ao_is_ready(void) /* 非 0 表示可播放 */
{
    return g_ao_ready;
}

int audio_ao_set_volume_level(uint8_t level_0_30) /* 钳位到 0~30 */
{
    if (!g_ao_ready)
    {
        return -1;
    }
    if (level_0_30 > 30)
    {
        level_0_30 = 30;
    }
    return audio_ao_apply_gain_level(level_0_30);
}

static void *audio_ao_play_thread(void *arg) /* 跳过 WAV 头后循环 MI_AO_Write */
{
    (void)arg;
    const char *path = g_ao_play_path;
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        LOG_ERROR("open play file %s: %s\n", path, strerror(errno));
        g_ao_playing = 0;
        return NULL;
    }

    WaveFileHeader_t hdr;
    if (read(fd, &hdr, sizeof(hdr)) != (ssize_t)sizeof(hdr))
    {
        LOG_ERROR("read wav header failed\n");
        close(fd);
        g_ao_playing = 0;
        return NULL;
    }

    uint8_t chunk[4096];
    g_ao_stop_req = 0;
    LOG_INFO("ao play start: %s\n", path);

    while (!g_ao_stop_req)
    {
        ssize_t n = read(fd, chunk, sizeof(chunk));
        if (n <= 0)
        {
            break;
        }
        MI_S32 ret = MI_AO_Write(g_ao_dev, chunk, (MI_U32)n, 0, -1);
        if (ret != MI_SUCCESS)
        {
            LOG_ERROR("MI_AO_Write failed 0x%x\n", ret);
            break;
        }
    }

    close(fd);
    g_ao_playing = 0;
    LOG_INFO("ao play done/stop\n");
    return NULL;
}

int audio_ao_play_wav_file(const char *path) /* 新线程播放；会先 stop 上一段 */
{
    if (!g_ao_ready || !path || !path[0])
    {
        return -1;
    }
    audio_ao_stop();
    strncpy(g_ao_play_path, path, sizeof(g_ao_play_path) - 1);
    g_ao_play_path[sizeof(g_ao_play_path) - 1] = '\0';

    g_ao_playing = 1;
    if (pthread_create(&g_ao_play_thread, NULL, audio_ao_play_thread, NULL) != 0)
    {
        g_ao_playing = 0;
        return -2;
    }
    pthread_detach(g_ao_play_thread);
    return 0;
}

void audio_ao_stop(void) /* 置 stop_req，播放线程自行退出 */
{
    if (g_ao_playing)
    {
        g_ao_stop_req = 1;
    }
}

int audio_ao_is_playing(void) /* 播放线程未结束 */
{
    return g_ao_playing ? 1 : 0;
}
