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

#ifndef _ST_COMMON_DLA_HPOSE_H_
#define _ST_COMMON_DLA_HPOSE_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "algo_hpose_api.h"
#include "st_common.h"

#define HPOSE_DETECT_ACCEPT_WIDTH (640)
#define HPOSE_DETECT_ACCEPT_HEIGHT (352)

#define HPOSE_RECOGNIZE_HEAD_ACCEPT_WIDTH (64)
#define HPOSE_RECOGNIZE_HEAD_ACCEPT_HEIGHT (64)

#define HPOSE_RECOGNIZE_BODY_ACCEPT_WIDTH (192)
#define HPOSE_RECOGNIZE_BODY_ACCEPT_HEIGHT (256)

#define ARGB8888_64x64_SIZE (16384)
#define NV12_256X192_SIZE (73728)

#define CHECK_DLA_RESULT(_func_, _ret_, _exit_label_)                                     \
    do                                                                                    \
    {                                                                                     \
        _ret_ = _func_;                                                                   \
        if (_ret_ != MI_SUCCESS)                                                          \
        {                                                                                 \
            printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, _ret_); \
            goto _exit_label_;                                                            \
        }                                                                                 \
    } while (0)

MI_S32 ST_Common_HPOSE_Init(MI_HPOSE_HANDLE* phHpose, AlgoHposeInitParam_t* pstInitParam);
MI_S32 ST_Common_HPOSE_DeInit(MI_HPOSE_HANDLE* phHpose);
MI_S32 ST_Common_HPOSE_DetectHeadPose(MI_HPOSE_HANDLE* phHpose, MI_SYS_BufInfo_t* pstScaledBufInfo,
                                      HposeBbox_t** faceBox, MI_S32* faceCount, HposeBbox_t** bodyBox,
                                      MI_S32* bodyCount);
MI_S32 ST_Common_HPOSE_RecognizeHead(MI_HPOSE_HANDLE* phHpose, MI_SYS_BufInfo_t* pstHeadBufInfo, HposeBbox_t* pstDetBox,
                                     HposeHeadResult_t* result);
MI_S32 ST_Common_HPOSE_RecognizeBody(MI_HPOSE_HANDLE* phHpose, MI_SYS_BufInfo_t* pstBodyBufInfo, HposeBbox_t* pstDetBox,
                                     HposeBodyResult_t* result);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_DLA_HPOSE_H_
