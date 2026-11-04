/** @file audio_ao.c @brief MI AO WAV 播放与音量档位 */
#define LOG_TAG "audio_ao"
#include "log.h"
#include <stdio.h>
#include "audio.h"
#include "audio_ao.h"
#include "audio_sys.h"
#include "app_config.h"
#include "uart_cmd.h"
#include "uart_proto.h"

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
static volatile int g_ao_has_output;
static int g_ao_thread_valid;
static int g_ao_stopped;
static pthread_t g_ao_play_thread;
static pthread_mutex_t g_ao_play_mutex = PTHREAD_MUTEX_INITIALIZER;
static char g_ao_play_path[256];
static int g_ao_notify_calm_done;
static uint8_t g_ao_play_audio_index;

static int audio_ao_stop_locked(void)
{
    int stop_ret = MI_SUCCESS;
    int should_stop = g_ao_thread_valid || g_ao_has_output;

    g_ao_stop_req = 1;
    if (g_ao_ready && should_stop)
    {
        /* Stop also interrupts a blocking Write and discards queued AO samples. */
        stop_ret = MI_AO_Stop(g_ao_dev);
        if (stop_ret != MI_SUCCESS)
        {
            LOG_ERROR("MI_AO_Stop failed 0x%x\n", stop_ret);
        }
    }

    if (g_ao_thread_valid)
    {
        pthread_join(g_ao_play_thread, NULL);
        g_ao_thread_valid = 0;
    }
    if (g_ao_ready && should_stop)
    {
        int flush_ret = MI_AO_Stop(g_ao_dev);
        if (flush_ret == MI_SUCCESS)
        {
            stop_ret = MI_SUCCESS;
        }
        else
        {
            LOG_ERROR("MI_AO_Stop after join failed 0x%x\n", flush_ret);
        }
    }
    if (stop_ret == MI_SUCCESS && should_stop)
    {
        g_ao_stopped = 1;
        g_ao_has_output = 0;
    }
    g_ao_playing = 0;
    return should_stop && stop_ret != MI_SUCCESS ? -1 : 0;
}

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

int audio_ao_set_gain_db(int8_t db) /* 直接设置增益 -60..30 dB */
{
    if (!g_ao_ready)
    {
        return -1;
    }
    if (db < -60) db = -60;
    if (db > 30)  db = 30;
    MI_S32 ret = MI_AO_SetVolume(g_ao_dev, (MI_S16)db, (MI_S16)db, 0);
    return (ret == MI_SUCCESS) ? 0 : -1;
}

/* ========== GPIO 14（音频功放使能） ========== */

#define AUDIO_AO_GPIO_NUM 14

static int audio_ao_gpio14_set(int on)
{
    static int g_gpio_exported = 0;
    char buf[64];

    if (!g_gpio_exported)
    {
        int fd = open("/sys/class/gpio/export", O_WRONLY);
        if (fd >= 0)
        {
            write(fd, "14", 2);
            close(fd);
            usleep(100000);  /* 等待 sysfs 创建 gpio14 目录 */
            g_gpio_exported = 1;
        }
    }

    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio14/direction");
    int fd = open(buf, O_WRONLY);
    if (fd >= 0)
    {
        write(fd, "out", 3);
        close(fd);
    }

    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio14/value");
    fd = open(buf, O_WRONLY);
    if (fd < 0)
    {
        LOG_ERROR("gpio14 open failed\n");
        return -1;
    }
    write(fd, on ? "1" : "0", 1);
    close(fd);
    LOG_INFO("gpio14 set %s\n", on ? "HIGH" : "LOW");
    return 0;
}

int audio_ao_get_volume_level(uint8_t *level_0_30) /* 通过 MI_AO_GetVolume 读取硬件当前音量 */
{
    if (!g_ao_ready || !level_0_30)
    {
        return -1;
    }
    MI_S16 left, right;
    if (MI_AO_GetVolume(g_ao_dev, &left, &right) != MI_SUCCESS)
    {
        return -1;
    }
    /* gain = level + DS_AO_VOLUME_BASE_GAIN => level = gain - DS_AO_VOLUME_BASE_GAIN */
    int level = (int)left - DS_AO_VOLUME_BASE_GAIN;
    if (level < 0)  level = 0;
    if (level > 30) level = 30;
    *level_0_30 = (uint8_t)level;
    return 0;
}

static void *audio_ao_play_thread(void *arg) /* 跳过 WAV 头后循环 MI_AO_Write */
{
    (void)arg;
    int notify_calm_done = g_ao_notify_calm_done;
    uint8_t audio_index = g_ao_play_audio_index;
    int completed = 0;
    const char *path = g_ao_play_path;
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        LOG_ERROR("open play file %s: %s\n", path, strerror(errno));
        goto done;
    }

    WaveFileHeader_t hdr;
    if (read(fd, &hdr, sizeof(hdr)) != (ssize_t)sizeof(hdr))
    {
        LOG_ERROR("read wav header failed\n");
        close(fd);
        fd = -1;
        goto done;
    }

    uint8_t chunk[4096];
    LOG_INFO("ao play start: %s\n", path);

    /* 播放前拉高 GPIO 14（功放使能） */
    audio_ao_gpio14_set(1);
    usleep(1);
    audio_ao_gpio14_set(0);
    usleep(1);
    audio_ao_gpio14_set(1);

    while (!g_ao_stop_req)
    {
        ssize_t n = read(fd, chunk, sizeof(chunk));
        if (n <= 0)
        {
            completed = (n == 0);
            break;
        }
        MI_S32 ret = MI_AO_Write(g_ao_dev, chunk, (MI_U32)n, 0, -1);
        if (ret != MI_SUCCESS)
        {
            LOG_ERROR("MI_AO_Write failed 0x%x\n", ret);
            break;
        }
        g_ao_has_output = 1;
    }

    /* 播放完成或中断后拉低 GPIO 14（功放关闭） */
    audio_ao_gpio14_set(0);

done:
    if (fd >= 0)
    {
        close(fd);
    }
    g_ao_playing = 0;
    if (completed && notify_calm_done && !g_ao_stop_req)
    {
        uart_proto_send_evt(DS_EVT_CALM_MUSIC_DONE, 0, &audio_index, 1);
    }
    LOG_INFO("ao play done/stop\n");
    // 此时需要允许AI继续运行
    resume_ai();
    return NULL;
}

static int audio_ao_start_wav_file(const char *path, int notify_calm_done, uint8_t audio_index)
{
    int ret = 0;

    if (!g_ao_ready || !path || !path[0])
    {
        return -1;
    }

    pthread_mutex_lock(&g_ao_play_mutex);
    if (audio_ao_stop_locked() != 0)
    {
        pthread_mutex_unlock(&g_ao_play_mutex);
        return -2;
    }
    if (g_ao_stopped && MI_AO_Start(g_ao_dev) != MI_SUCCESS)
    {
        LOG_ERROR("MI_AO_Start failed\n");
        pthread_mutex_unlock(&g_ao_play_mutex);
        return -3;
    }
    g_ao_stopped = 0;

    strncpy(g_ao_play_path, path, sizeof(g_ao_play_path) - 1);
    g_ao_play_path[sizeof(g_ao_play_path) - 1] = '\0';
    g_ao_notify_calm_done = notify_calm_done;
    g_ao_play_audio_index = audio_index;

    g_ao_stop_req = 0;
    g_ao_playing = 1;
    if (pthread_create(&g_ao_play_thread, NULL, audio_ao_play_thread, NULL) != 0)
    {
        g_ao_playing = 0;
        ret = -4;
    }
    else
    {
        g_ao_thread_valid = 1;
    }
    pthread_mutex_unlock(&g_ao_play_mutex);
    return ret;
}

int audio_ao_play_wav_file(const char *path)
{
    return audio_ao_start_wav_file(path, 0, 0);
}

int audio_ao_play_calm_wav_file(const char *path, uint8_t audio_index)
{
    if (audio_index > 1)
    {
        return -1;
    }
    return audio_ao_start_wav_file(path, 1, audio_index);
}

void audio_ao_stop(void) /* 同步退出播放线程并清空 AO 队列 */
{
    pthread_mutex_lock(&g_ao_play_mutex);
    audio_ao_stop_locked();
    pthread_mutex_unlock(&g_ao_play_mutex);
}

int audio_ao_is_playing(void) /* 播放线程未结束 */
{
    return g_ao_playing ? 1 : 0;
}
