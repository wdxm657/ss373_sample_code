/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized diSclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _ST_COMMON_AUDIO_H_
#define _ST_COMMON_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "mi_sys.h"
#include "mi_ai.h"
#include "mi_ao.h"

typedef struct WAVE_FORMAT
{
    signed short wFormatTag;
    signed short wChannels;
    unsigned int dwSamplesPerSec;
    unsigned int dwAvgBytesPerSec;
    signed short wBlockAlign;
    signed short wBitsPerSample;
} WaveFormat_t;

typedef struct WAVEFILEHEADER
{
    char         chRIFF[4];
    unsigned int dwRIFFLen;
    char         chWAVE[4];
    char         chFMT[4];
    unsigned int dwFMTLen;
    WaveFormat_t wave;
    char         chDATA[4];
    unsigned int dwDATALen;
} WaveFileHeader_t;

typedef enum {
    E_SOUND_MODE_MONO   = 0, /* mono */
    E_SOUND_MODE_STEREO = 1, /* stereo */
} SoundMode_e;

typedef enum {
    E_SAMPLE_RATE_8000  = 8000,  /* 8kHz sampling rate */
    E_SAMPLE_RATE_16000 = 16000, /* 16kHz sampling rate */
    E_SAMPLE_RATE_32000 = 32000, /* 32kHz sampling rate */
    E_SAMPLE_RATE_48000 = 48000, /* 48kHz sampling rate */
} SampleRate_e;

MI_S32 ST_Common_GetAiDefaultDevAttr(MI_AI_Attr_t *pstAiDevAttr);
MI_S32 ST_Common_AiOpenDev(MI_AUDIO_DEV AiDevId, MI_AI_Attr_t *pstAiDevAttr);
MI_S32 ST_Common_AiAttachIf(MI_AUDIO_DEV AiDevId, MI_U8 ChnGrpId, MI_AI_If_e *aenAiIfs, MI_U8 u8AiIfSize);
MI_S32 ST_Common_AiEnableChnGroup(MI_AUDIO_DEV AiDevId, MI_U8 ChnGrpId);
MI_S32 ST_Common_AiDisableChnGroup(MI_AUDIO_DEV AiDevId, MI_U8 ChnGrpId);
MI_S32 ST_Common_AiCloseDev(MI_AUDIO_DEV AiDevId);

MI_S32 ST_Common_GetAoDefaultDevAttr(MI_AO_Attr_t *pstAoDevAttr);
MI_S32 ST_Common_AoOpenDev(MI_AUDIO_DEV AoDevId, MI_AO_Attr_t *pstAoDevAttr);
MI_S32 ST_Common_AoAttachIf(MI_AUDIO_DEV AoDevId, MI_AO_If_e *aenAoIfs);
MI_S32 ST_Common_AoCloseDev(MI_AUDIO_DEV AoDevId);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_AUDIO_H_
