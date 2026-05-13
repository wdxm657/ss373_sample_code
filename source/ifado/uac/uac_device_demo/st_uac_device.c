/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/poll.h>
#include <fcntl.h>

#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_common.h"
#include "st_common.h"

#include "mi_ao.h"
#include "mi_ai.h"
#include "ss_uac.h"

#define AI_VOLUME_AMIC_MIN (0)
#define AI_VOLUME_AMIC_MAX (19)
#define AI_VOLUME_DMIC_MIN (0)
#define AI_VOLUME_DMIC_MAX (6)
// global value
typedef struct
{
    MI_S32 wavFd;
    char   filePath[64];
} ST_UacFile_t;
static MI_BOOL      g_bExit         = FALSE;
MI_S32              trace_level     = UAC_DBG_ERR;
MI_BOOL             bEnableAI       = FALSE;
MI_BOOL             bEnableAO       = FALSE;
MI_BOOL             g_bEnableFile   = FALSE;
MI_U32              AoIfId          = E_MI_AO_IF_DAC_AB;
MI_U32              AiIfId          = E_MI_AI_IF_ADC_AB;
MI_U32              u32AoSampleRate = E_MI_AUDIO_SAMPLE_RATE_48000;
static ST_UacFile_t g_stUacFile[2]  = {
    [0] = {-1, {0}},
    [1] = {-1, {0}},
};

static void help_message(char **argv)
{
    printf("\n");
    printf("usage: %s \n", argv[0]);
    printf(" -f : use file instead of MI,ex: -f /customer/resource\n");
    printf(" -d : AI Attach Intf Id\n");
    printf(" -D : AO Attach Intf Id\n");
    printf(" -S : AO Samle Rate\n");
    printf("\nExample: %s -D0 -S48000\n", argv[0]);
    printf("\n");
}
void ST_HandleSig(MI_S32 signo)
{
    if (signo == SIGINT)
    {
        printf("catch Ctrl + C, exit normally\n");

        g_bExit = TRUE;
    }
}
static MI_S32 ST_AO_FILE_Init(MI_U32 u32AoSampleRate, MI_U8 chn)
{
    char wav_path[256], wav_name[256];

    memset(wav_name, 0x00, sizeof(wav_name));
    sprintf(wav_name, "%dK_16bit_STERO_30s.wav", u32AoSampleRate / 1000);
    memset(wav_path, 0x00, sizeof(wav_path));
    strcat(wav_path, g_stUacFile[1].filePath);
    strcat(wav_path, "/wav/out/");
    strcat(wav_path, wav_name);

    g_stUacFile[1].wavFd = open(wav_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
    if (g_stUacFile[1].wavFd < 0)
    {
        printf("open %s error\n", wav_path);
        return -1;
    }

    return MI_SUCCESS;
}

static MI_S32 ST_AO_FILE_Deinit(void)
{
    close(g_stUacFile[1].wavFd);
    return MI_SUCCESS;
}

static MI_S32 ST_AI_FILE_Init(MI_U32 u32AiSampleRate, MI_U8 chn)
{
    char wav_path[256], wav_name[256];

    memset(wav_name, 0x00, sizeof(wav_name));
    sprintf(wav_name, "%dK_16bit_Mono.wav", u32AiSampleRate / 1000);
    memset(wav_path, 0x00, sizeof(wav_path));
    strcat(wav_path, g_stUacFile[0].filePath);
    strcat(wav_path, "/wav_in/");
    strcat(wav_path, wav_name);

    g_stUacFile[0].wavFd = open(wav_path, O_RDONLY);
    if (g_stUacFile[0].wavFd < 0)
    {
        printf("open %s error\n", wav_path);
        return -1;
    }

    return MI_SUCCESS;
}

static MI_S32 ST_AI_FILE_Deinit(void)
{
    close(g_stUacFile[0].wavFd);
    return MI_SUCCESS;
}

static MI_S32 ST_AI_FILE_SetVqeVolume(SS_UAC_Volume_t stVolume)
{
    return MI_SUCCESS;
}

static MI_S32 AO_FILE_TakeBuffer(void *uac, SS_UAC_Frame_t *stUacFrame)
{
    MI_S32 ret = MI_SUCCESS;
    MI_S32 len;

    len = write(g_stUacFile[1].wavFd, stUacFrame->data, stUacFrame->length);
    if (len <= 0)
    {
        ret = -1;
    }

    return ret;
}

static MI_S32 AI_FILE_FillBuffer(void *uac, SS_UAC_Frame_t *stUacFrame)
{
    MI_S32 ret = MI_SUCCESS;
    MI_S32 len, s32NeedSize = stUacFrame->length;

    len = read(g_stUacFile[0].wavFd, stUacFrame->data, s32NeedSize);
    if (len > 0)
    {
        stUacFrame->length = len;
    }
    else if (0 == len)
    {
        lseek(g_stUacFile[0].wavFd, 0, SEEK_SET);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

static MI_S32 ST_AO_Init(void *uac, MI_U32 u32AoSampleRate, MI_U8 chn)
{
    MI_AO_Attr_t       stAoSetAttr;
    MI_S8              s8LeftVolume  = 0;
    MI_S8              s8RightVolume = 0;
    MI_AO_GainFading_e eGainFading   = E_MI_AO_GAIN_FADING_OFF;

    memset(&stAoSetAttr, 0, sizeof(MI_AO_Attr_t));
    stAoSetAttr.enChannelMode = E_MI_AO_CHANNEL_MODE_DOUBLE_MONO;
    stAoSetAttr.enFormat      = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    stAoSetAttr.enSampleRate  = (MI_AUDIO_SampleRate_e)u32AoSampleRate;
    stAoSetAttr.enSoundMode   = E_MI_AUDIO_SOUND_MODE_MONO;
    stAoSetAttr.u32PeriodSize = 1024;
    printf("ao enSampleRate %d\n", stAoSetAttr.enSampleRate);

    ExecFunc(MI_AO_Open(0, &stAoSetAttr), MI_SUCCESS);
    ExecFunc(MI_AO_AttachIf(0, (MI_AO_If_e)AoIfId, 0), MI_SUCCESS);

    ExecFunc(MI_AO_SetVolume(0, s8LeftVolume, s8RightVolume, eGainFading), MI_SUCCESS);

    return MI_SUCCESS;
}

static MI_S32 ST_AO_Deinit(void *uac)
{
    ExecFunc(MI_AO_DetachIf(0, (MI_AO_If_e)AoIfId), MI_SUCCESS);
    ExecFunc(MI_AO_Close(0), MI_SUCCESS);
    return MI_SUCCESS;
}

static MI_S32 ST_AI_Init(void *uac, MI_U32 u32AiSampleRate, MI_U8 chn)
{
    MI_AI_Attr_t     stAiSetAttr;
    MI_AI_If_e       enAiIf[] = {(MI_AI_If_e)AiIfId};
    MI_SYS_ChnPort_t stAiChnOutputPort;

    memset(&stAiSetAttr, 0, sizeof(MI_AI_Attr_t));
    stAiSetAttr.bInterleaved  = TRUE;
    stAiSetAttr.enFormat      = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    stAiSetAttr.enSampleRate  = (MI_AUDIO_SampleRate_e)u32AiSampleRate;
    stAiSetAttr.enSoundMode   = E_MI_AUDIO_SOUND_MODE_MONO;
    stAiSetAttr.u32PeriodSize = 1024;
    printf("ai enSampleRate %d\n", stAiSetAttr.enSampleRate);

    ExecFunc(MI_AI_Open(0, &stAiSetAttr), MI_SUCCESS);
    ExecFunc(MI_AI_AttachIf(0, enAiIf, sizeof(enAiIf) / sizeof(enAiIf[0])), MI_SUCCESS);

    memset(&stAiChnOutputPort, 0, sizeof(stAiChnOutputPort));
    stAiChnOutputPort.eModId    = E_MI_MODULE_ID_AI;
    stAiChnOutputPort.u32DevId  = 0;
    stAiChnOutputPort.u32ChnId  = 0;
    stAiChnOutputPort.u32PortId = 0;
    ExecFunc(MI_SYS_SetChnOutputPortDepth(0, &stAiChnOutputPort, 4, 8), MI_SUCCESS);
    ExecFunc(MI_AI_SetIfGain(enAiIf[0], 18, 18), MI_SUCCESS);
    ExecFunc(MI_AI_EnableChnGroup(0, 0), MI_SUCCESS);

    return MI_SUCCESS;
}

static MI_S32 ST_AI_Deinit(void *uac)
{
    ExecFunc(MI_AI_DisableChnGroup(0, 0), MI_SUCCESS);
    ExecFunc(MI_AI_Close(0), MI_SUCCESS);
    return MI_SUCCESS;
}

static inline MI_S32 Vol_to_Db(SS_UAC_Volume_t stVolume)
{
    switch (AiIfId)
    {
        case E_MI_AI_IF_ADC_AB:
        case E_MI_AI_IF_ADC_CD:
            return ((stVolume.s32Volume - stVolume.s32Min) * (AI_VOLUME_AMIC_MAX - AI_VOLUME_AMIC_MIN)
                    / (stVolume.s32Max - stVolume.s32Min))
                   + AI_VOLUME_AMIC_MIN;
            break;
        case E_MI_AI_IF_DMIC_A_01:
        case E_MI_AI_IF_DMIC_A_23:
            return ((stVolume.s32Volume - stVolume.s32Min) * (AI_VOLUME_DMIC_MAX - AI_VOLUME_DMIC_MIN)
                    / (stVolume.s32Max - stVolume.s32Min))
                   + AI_VOLUME_DMIC_MIN;
            break;
        default:
            break;
    }
    return -EINVAL;
}

static MI_S32 ST_AI_SetVqeVolume(SS_UAC_Volume_t stVolume)
{
    MI_S32 ret = MI_SUCCESS, s32AiVolume;

    s32AiVolume = Vol_to_Db(stVolume);
    ret         = MI_AI_SetIfGain((MI_AI_If_e)AiIfId, s32AiVolume, s32AiVolume);
    if (MI_SUCCESS != ret)
    {
        printf("MI_AI_SetIfGain failed: %x.\n", ret);
    }

    return ret;
}

static MI_S32 AO_TakeBuffer(void *uac, SS_UAC_Frame_t *stUacFrame)
{
    MI_S32 ret = MI_SUCCESS;

    do
    {
        ret = MI_AO_Write(0, stUacFrame->data, stUacFrame->length, 0, -1);
    } while ((ret == MI_AO_ERR_NOBUF));

    if (MI_SUCCESS != ret)
    {
        printf("MI_AO_Write failed: %x.\n", ret);
    }

    return ret;
}

static MI_S32 AI_FillBuffer(void *uac, SS_UAC_Frame_t *stUacFrame)
{
    MI_S32       ret = MI_SUCCESS;
    MI_AI_Data_t stAiFrame, stAiFrame2;

    ret = MI_AI_Read(0, 0, &stAiFrame, &stAiFrame2, -1);
    if (MI_SUCCESS != ret)
    {
        printf("MI_AI_Read failed: %x.\n", ret);
        goto exit;
    }

    memcpy(stUacFrame->data, stAiFrame.apvBuffer[0], stAiFrame.u32Byte[0]);
    stUacFrame->length = stAiFrame.u32Byte[0];

    ret = MI_AI_ReleaseData(0, 0, &stAiFrame, &stAiFrame2);
    if (MI_SUCCESS != ret)
    {
        printf("MI_AI_ReleaseData failed: %x.\n", ret);
    }

exit:
    return ret;
}

MI_S32 ST_UacInit(SS_UAC_Handle_h *pstHandle)
{
    MI_S32                  ret       = MI_SUCCESS;
    static SS_UAC_Device_t *pstDevice = NULL;
    SS_UAC_OPS_t            opsAo     = {ST_AO_Init, AO_TakeBuffer, ST_AO_Deinit, NULL};
    SS_UAC_OPS_t            opsAi     = {ST_AI_Init, AI_FillBuffer, ST_AI_Deinit, ST_AI_SetVqeVolume};
    MI_S32                  eMode     = 0;

    if (!bEnableAO && !bEnableAI)
    {
        printf("ai and ao not enable\n");
        return ret;
    }

    if (bEnableAO)
    {
        eMode |= AS_OUT_MODE;
    }
    if (bEnableAI)
    {
        eMode |= AS_IN_MODE;
    }

    if (g_bEnableFile)
    {
        opsAi.UAC_AUDIO_Init    = ST_AI_FILE_Init;
        opsAi.UAC_AUDIO_BufTask = AI_FILE_FillBuffer;
        opsAi.UAC_AUDIO_Deinit  = ST_AI_FILE_Deinit;
        opsAi.UAC_AUDIO_SetVol  = ST_AI_FILE_SetVqeVolume;

        opsAo.UAC_AUDIO_Init    = ST_AO_FILE_Init;
        opsAo.UAC_AUDIO_BufTask = AO_FILE_TakeBuffer;
        opsAo.UAC_AUDIO_Deinit  = ST_AO_FILE_Deinit;
        opsAo.UAC_AUDIO_SetVol  = NULL;
    }

    ret = SS_UAC_AllocStream(NULL, pstHandle);
    if (MI_SUCCESS != ret)
    {
        printf("ST_UAC_AllocStream failed!\n");
        goto exit;
    }

    pstDevice                                 = (SS_UAC_Device_t *)*pstHandle;
    pstDevice->mode                           = eMode;
    pstDevice->opsAo                          = opsAo;
    pstDevice->opsAi                          = opsAi;
    pstDevice->config[1]->pcm_config.rate     = u32AoSampleRate;
    pstDevice->config[1]->pcm_config.channels = 1;
    pstDevice->config[0]->pcm_config.rate     = E_MI_AUDIO_SAMPLE_RATE_48000;
    pstDevice->config[0]->pcm_config.channels = 1;
    pstDevice->config[0]->volume.s32Volume    = -EINVAL;

    SS_UAC_StartDev(*pstHandle);

exit:
    return ret;
}

MI_S32 ST_UacDeinit(SS_UAC_Handle_h stHandle)
{
    MI_S32 ret = MI_SUCCESS;

    if (!bEnableAO && !bEnableAI)
    {
        return ret;
    }

    SS_UAC_StoptDev(stHandle);
    ret = SS_UAC_FreeStream(stHandle);
    if (MI_SUCCESS != ret)
    {
        printf("ST_UAC_FreeStream failed.\n");
    }

    return ret;
}

int main(int argc, char *argv[])
{
    struct sigaction sigAction;
    sigAction.sa_handler = ST_HandleSig;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGINT, &sigAction, NULL);

    SS_UAC_Handle_h stHandle;
    MI_S32 result = 0;

    while((result = getopt(argc, argv, "d:D:S:f:t:")) != -1)
    {
        switch(result)
        {
            case 'f':
                for (int i = 0; i < 2; i++)
                {
                    strcpy(g_stUacFile[i].filePath, optarg);
                }
                ST_DBG("FilePath:%s\n", optarg);
                g_bEnableFile = TRUE;
                break;

            case 'd':
                bEnableAI = TRUE;
                AiIfId = strtol(optarg, NULL, 10);
                break;

            case 'D':
                bEnableAO = TRUE;
                AoIfId = strtol(optarg, NULL, 10);
                break;

            case 'S':
                u32AoSampleRate = strtol(optarg, NULL, 10);
                break;

            case 't':
                trace_level = strtol(optarg, NULL, 10);
                break;

            case 'h':
                help_message(argv);
                break;

            default:
                break;
        }
    }

    SS_UAC_SetTraceLevel(trace_level);
    STCHECKRESULT(ST_Common_Sys_Init());
    STCHECKRESULT(ST_UacInit(&stHandle));
    while (!g_bExit)
    {
        usleep(100 * 1000);
    }
    usleep(100 * 1000);
    STCHECKRESULT(ST_UacDeinit(stHandle));
    STCHECKRESULT(ST_Common_Sys_Exit());
    return 0;
}
