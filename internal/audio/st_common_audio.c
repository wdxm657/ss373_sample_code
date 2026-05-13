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
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "st_common_audio.h"
#include "st_common.h"
#include "platform.h"

// AI

MI_S32 ST_Common_GetAiDefaultDevAttr(MI_AI_Attr_t *pstAiDevAttr)
{
    ST_CHECK_POINTER(pstAiDevAttr);
    pstAiDevAttr->enFormat      = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    pstAiDevAttr->enSoundMode   = E_MI_AUDIO_SOUND_MODE_MONO;
    pstAiDevAttr->enSampleRate  = E_MI_AUDIO_SAMPLE_RATE_8000;
    pstAiDevAttr->u32PeriodSize = 1024;
    pstAiDevAttr->bInterleaved  = TRUE;

    return MI_SUCCESS;
}

MI_S32 ST_Common_AiOpenDev(MI_AUDIO_DEV AiDevId, MI_AI_Attr_t *pstAiDevAttr)
{
    ST_CHECK_POINTER(pstAiDevAttr);
    STCHECKRESULT(MI_AI_Open(AiDevId, pstAiDevAttr));

    return MI_SUCCESS;
}

MI_S32 ST_Common_AiAttachIf(MI_AUDIO_DEV AiDevId, MI_U8 ChnGrpId, MI_AI_If_e *aenAiIfs, MI_U8 u8AiIfSize)
{
    ST_CHECK_POINTER(aenAiIfs);
    MI_S16 s8dpgaGain[] = {-10};
    STCHECKRESULT(MI_AI_AttachIf(AiDevId, aenAiIfs, u8AiIfSize));
    STCHECKRESULT(MI_AI_SetIfGain(aenAiIfs[0], 10, 10));
    STCHECKRESULT(MI_AI_SetGain(AiDevId, ChnGrpId, s8dpgaGain, sizeof(s8dpgaGain) / sizeof(s8dpgaGain[0])));

    return MI_SUCCESS;
}

MI_S32 ST_Common_AiEnableChnGroup(MI_AUDIO_DEV AiDevId, MI_U8 ChnGrpId)
{
    STCHECKRESULT(MI_AI_EnableChnGroup(AiDevId, ChnGrpId));

    return MI_SUCCESS;
}

MI_S32 ST_Common_AiDisableChnGroup(MI_AUDIO_DEV AiDevId, MI_U8 ChnGrpId)
{
    STCHECKRESULT(MI_AI_DisableChnGroup(AiDevId, ChnGrpId));

    return MI_SUCCESS;
}

MI_S32 ST_Common_AiCloseDev(MI_AUDIO_DEV AiDevId)
{
    STCHECKRESULT(MI_AI_Close(AiDevId));

    return MI_SUCCESS;
}

// AO

MI_S32 ST_Common_GetAoDefaultDevAttr(MI_AO_Attr_t *pstAoDevAttr)
{
    ST_CHECK_POINTER(pstAoDevAttr);
    pstAoDevAttr->enFormat      = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    pstAoDevAttr->enSoundMode   = E_MI_AUDIO_SOUND_MODE_MONO;
    pstAoDevAttr->enSampleRate  = E_MI_AUDIO_SAMPLE_RATE_8000;
    pstAoDevAttr->u32PeriodSize = 1024;
    pstAoDevAttr->enChannelMode = E_MI_AO_CHANNEL_MODE_DOUBLE_MONO;

    return MI_SUCCESS;
}

MI_S32 ST_Common_AoOpenDev(MI_AUDIO_DEV AoDevId, MI_AO_Attr_t *pstAoDevAttr)
{
    ST_CHECK_POINTER(pstAoDevAttr);
    STCHECKRESULT(MI_AO_Open(AoDevId, pstAoDevAttr));

    return MI_SUCCESS;
}

MI_S32 ST_Common_AoAttachIf(MI_AUDIO_DEV AoDevId, MI_AO_If_e *aenAoIfs)
{
    ST_CHECK_POINTER(aenAoIfs);
    STCHECKRESULT(MI_AO_AttachIf(AoDevId, aenAoIfs[0], 0));
    STCHECKRESULT(MI_AO_SetIfVolume(aenAoIfs[0], 0, 0));
    STCHECKRESULT(MI_AO_SetVolume(AoDevId, 0, 0, 0));

    return MI_SUCCESS;
}

MI_S32 ST_Common_AoCloseDev(MI_AUDIO_DEV AoDevId)
{
    STCHECKRESULT(MI_AO_Close(AoDevId));

    return MI_SUCCESS;
}
