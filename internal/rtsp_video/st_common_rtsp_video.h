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
#ifndef _ST_COMMON_RTSP_VIDEO_H_
#define _ST_COMMON_RTSP_VIDEO_H_

#ifdef __cplusplus
extern "C"{
#endif	// __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "mi_common_datatype.h"
#include "mi_venc_datatype.h"
#include "ss_rtsp_c_wrapper.h"



typedef struct ST_VencStreamInfo_s
{
    MI_VENC_ModType_e eType;
    MI_VENC_DEV VencDev;
    MI_VENC_CHN VencChn;
    MI_U32 u32Width;
    MI_U32 u32Height;
    MI_U32 u32FrameRate;
    unsigned int rtspIndex;
}ST_VideoStreamInfo_t;


MI_S32 ST_Common_RtspServerStartVideo(ST_VideoStreamInfo_t *pstStreamAttr);
MI_S32 ST_Common_RtspServerStopVideo(ST_VideoStreamInfo_t *pstStreamAttr);
char * ST_Common_RtspServerGetUrl(unsigned int rtspIndex);


#ifdef __cplusplus
}
#endif	// __cplusplus

#endif //_ST_COMMON_RTSP_VIDEO_H_

