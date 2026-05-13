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
#ifndef _ST_COMMON_RTSP_AUDIO_H_
#define _ST_COMMON_RTSP_AUDIO_H_

#ifdef __cplusplus
extern "C"{
#endif	// __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "mi_common_datatype.h"
#include "mi_ai_datatype.h"
#include "ss_rtsp_c_wrapper.h"



typedef struct ST_AudioStreamInfo_s
{
    MI_AUDIO_Format_e     enFormat;
    MI_AUDIO_SoundMode_e  enSoundMode;
    MI_AUDIO_SampleRate_e enSampleRate;
    unsigned char sample_width;
    MI_AUDIO_DEV AiDevId;
    MI_U8 u8ChnGrpIdx;

    unsigned int rtspIndex;
}ST_AudioStreamInfo_t;


MI_S32 ST_Common_RtspServerStartAudio(ST_AudioStreamInfo_t *pstStreamAttr);
MI_S32 ST_Common_RtspServerStopAudio(ST_AudioStreamInfo_t *pstStreamAttr);


#ifdef __cplusplus
}
#endif	// __cplusplus

#endif //_ST_COMMON_RTSP_AUDIO_H_

