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
#ifndef _ST_COMMON_DLA_FR_H_
#define _ST_COMMON_DLA_FR_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "algo_fr_api.h"
#include "st_common.h"

#define FR_ACCEPT_WIDTH (480)
#define FR_ACCEPT_HEIGHT (288)
#define FR_FEATURE_SIZE (514) // base ipu network model

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

    MI_S32 ST_Common_FR_Init(MI_S64* phFr, InitFrParam_t* pstInitParam);
    MI_S32 ST_Common_FR_DeInit(MI_S64* phFr);

    MI_S32 ST_Common_FR_Detect(MI_S64* phFr, MI_SYS_BufInfo_t* pstSourceBufInfo, MI_SYS_BufInfo_t* pstScaledBufInfo,
                               ParamDet_t stParams, DetectBox_t** ppstDetectout, MI_S32* ps32FaceNum);

    MI_S32 ST_Common_FR_Align(MI_SYS_BufInfo_t* pstSourceBufInfo, MI_S32 s32FormatType, DetectBox_t stFaceBox,
                              MI_U8* pu8OutCropData);

    MI_S32 ST_Common_FR_FeatureExtract(MI_S64* phFr, MI_U8* pu8OutCropData, MI_S16* ps16FaceFeature);

    MI_S32 ST_Common_FR_CompareFeature(MI_S16* ps16FaceFeature, MI_S16 (*pps16FaceFeatures)[FR_FEATURE_SIZE],
                                       MI_S32 s32FaceNum, MI_S32* ps32MaxSimilarNum);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_DLA_FR_H_
