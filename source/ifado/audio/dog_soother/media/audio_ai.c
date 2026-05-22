/** @file audio_ai.c @brief MI AI 16kHz mono 采集（对齐 ai_demo） */
#define LOG_TAG "audio_ai"
#include "log.h"

#include "audio_ai.h"
#include "audio_sys.h"
#include "app_config.h"

#include "mi_sys.h"
#include "mi_ai.h"
#include "st_common_audio.h"
#include "st_common.h"

#include <string.h>

static int g_ai_ready;
static int g_ai_chn_enabled; /* ChnGroup 是否已 Enable，避免重复 Disable */
static MI_AUDIO_DEV g_ai_dev = DS_AI_DEV_ID;
static MI_U8 g_ai_chn_grp = DS_AI_CHN_GRP_ID;
static int g_ai_gain_db = DS_AI_GAIN_DB;

static void audio_ai_disable_chn_if_enabled(void)
{
    if (!g_ai_chn_enabled)
    {
        return;
    }
    if (ST_Common_AiDisableChnGroup(g_ai_dev, g_ai_chn_grp) != MI_SUCCESS)
    {
        LOG_ERROR("AiDisableChnGroup failed\n");
    }
    g_ai_chn_enabled = 0;
}

static int audio_ai_setup_port_depth(void) /* 输出端口缓冲深度 3/5，避免 Read 阻塞过久 */
{
    MI_SYS_ChnPort_t port;
    memset(&port, 0, sizeof(port));
    port.eModId = E_MI_MODULE_ID_AI;
    port.u32DevId = g_ai_dev;
    port.u32ChnId = g_ai_chn_grp;
    port.u32PortId = 0;
    return (MI_SYS_SetChnOutputPortDepth(0, &port, 3, 5) == MI_SUCCESS) ? 0 : -1;
}

int audio_ai_init(int gain_db) /* 打开 ADC、16kHz、使能 ChnGroup */
{
    if (g_ai_ready)
    {
        return 0;
    }
    if (audio_sys_init() != 0)
    {
        return -1;
    }

    g_ai_gain_db = gain_db;

    MI_AI_Attr_t attr;
    MI_AI_If_e ai_if[] = {E_MI_AI_IF_ADC_AB, E_MI_AI_IF_ECHO_A};
    MI_S16 s16_gain = (MI_S16)g_ai_gain_db;

    memset(&attr, 0, sizeof(attr));
    ST_Common_GetAiDefaultDevAttr(&attr);
    attr.enSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;

    if (ST_Common_AiOpenDev(g_ai_dev, &attr) != MI_SUCCESS)
    {
        LOG_ERROR("AiOpenDev failed\n");
        return -2;
    }
    if (ST_Common_AiAttachIf(g_ai_dev, g_ai_chn_grp, ai_if, 2) != MI_SUCCESS)
    {
        LOG_ERROR("AiAttachIf failed\n");
        ST_Common_AiCloseDev(g_ai_dev);
        return -3;
    }
    if (MI_AI_SetGain(g_ai_dev, g_ai_chn_grp, &s16_gain, 1) != MI_SUCCESS)
    {
        LOG_ERROR("AI_SetGain failed\n");
    }
    if (ST_Common_AiEnableChnGroup(g_ai_dev, g_ai_chn_grp) != MI_SUCCESS)
    {
        LOG_ERROR("AiEnableChnGroup failed\n");
        ST_Common_AiCloseDev(g_ai_dev);
        return -4;
    }
    g_ai_chn_enabled = 1;
    if (audio_ai_setup_port_depth() != 0)
    {
        LOG_ERROR("SetChnOutputPortDepth failed\n");
        audio_ai_disable_chn_if_enabled();
        ST_Common_AiCloseDev(g_ai_dev);
        return -5;
    }

    g_ai_ready = 1;
    LOG_INFO("audio_ai ready dev=%u grp=%u gain=%d dB\n", g_ai_dev, g_ai_chn_grp, g_ai_gain_db);
    return 0;
}

void audio_ai_stop_capture(void) /* 不关设备，仅 Disable 一次以打断 MI_AI_Read */
{
    if (!g_ai_ready)
    {
        return;
    }
    audio_ai_disable_chn_if_enabled();
    LOG_INFO("audio_ai capture stopped (ChnGroup disabled)\n");
}

void audio_ai_deinit(void) /* Disable（若尚未）+ CloseDev */
{
    if (!g_ai_ready)
    {
        return;
    }
    audio_ai_disable_chn_if_enabled();
    ST_Common_AiCloseDev(g_ai_dev);
    g_ai_ready = 0;
    LOG_INFO("audio_ai closed\n");
}

int audio_ai_is_ready(void) /* 非 0 表示可 Read */
{
    return g_ai_ready;
}

int audio_ai_set_gain(int gain_db) /* 运行时改麦克风增益 dB */
{
    if (!g_ai_ready)
    {
        return -1;
    }
    MI_S16 s16_gain = (MI_S16)gain_db;
    if (MI_AI_SetGain(g_ai_dev, g_ai_chn_grp, &s16_gain, 1) != MI_SUCCESS)
    {
        return -2;
    }
    g_ai_gain_db = gain_db;
    return 0;
}

int audio_ai_read_pcm(int16_t *dst, size_t max_samples, size_t *out_samples) /* MI_AI_Read 一帧 mic */
{
    MI_AI_Data_t mic;
    MI_AI_Data_t echo;
    MI_S32 ret;

    if (out_samples)
    {
        *out_samples = 0;
    }
    if (!g_ai_ready || !dst || max_samples == 0)
    {
        return -1;
    }

    memset(&mic, 0, sizeof(mic));
    memset(&echo, 0, sizeof(echo));
    ret = MI_AI_Read(g_ai_dev, g_ai_chn_grp, &mic, &echo, -1);
    if (ret != MI_SUCCESS)
    {
        return -2;
    }

    size_t frame_samples = mic.u32Byte[0] / sizeof(int16_t);
    if (frame_samples == 0 || !mic.apvBuffer[0])
    {
        MI_AI_ReleaseData(g_ai_dev, g_ai_chn_grp, &mic, &echo);
        return 0;
    }
    if (frame_samples > max_samples)
    {
        frame_samples = max_samples;
    }
    memcpy(dst, mic.apvBuffer[0], frame_samples * sizeof(int16_t));
    if (out_samples)
    {
        *out_samples = frame_samples;
    }
    MI_AI_ReleaseData(g_ai_dev, g_ai_chn_grp, &mic, &echo);
    return 0;
}
