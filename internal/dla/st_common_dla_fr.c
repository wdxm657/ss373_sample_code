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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "st_common_dla_fr.h"
#include "st_common.h"

MI_S32 ST_Common_FR_Init(MI_S64* phFr, InitFrParam_t* pstInitParam)
{
    ST_CHECK_POINTER(pstInitParam);
    ST_CHECK_POINTER(phFr);
    STCHECKRESULT(ALGO_FR_Init(*pstInitParam));
    STCHECKRESULT(ALGO_FR_CreateHandle(phFr));
    return MI_SUCCESS;
}

MI_S32 ST_Common_FR_DeInit(MI_S64* phFr)
{
    ST_CHECK_POINTER(phFr);
    STCHECKRESULT(ALGO_FR_ReleaseHandle(*phFr));
    STCHECKRESULT(ALGO_FR_Cleanup());
    return MI_SUCCESS;
}

MI_S32 ST_Common_FR_Detect(MI_S64* phFr, MI_SYS_BufInfo_t* pstSourceBufInfo, MI_SYS_BufInfo_t* pstScaledBufInfo,
                           ParamDet_t stParams, DetectBox_t** ppstDetectout, MI_S32* ps32FaceNum)
{
    ST_CHECK_POINTER(phFr);
    ST_CHECK_POINTER(pstSourceBufInfo);
    ST_CHECK_POINTER(pstScaledBufInfo);
    ST_CHECK_POINTER(ppstDetectout);
    ST_CHECK_POINTER(ps32FaceNum);

    MI_S32            s32Ret = 0;
    AlgoFrInputInfo_t stFrBufInfo;

    memset(&stFrBufInfo, 0x0, sizeof(AlgoFrInputInfo_t));

    stFrBufInfo.bufsize         = pstScaledBufInfo->stFrameData.u32BufSize;
    stFrBufInfo.pt_tensor_data  = pstScaledBufInfo->stFrameData.pVirAddr[0];
    stFrBufInfo.phy_tensor_addr = pstScaledBufInfo->stFrameData.phyAddr[0];

    CHECK_DLA_RESULT(ALGO_FR_Detect(*phFr, &stFrBufInfo, pstSourceBufInfo->stFrameData.u16Width,
                                    pstSourceBufInfo->stFrameData.u16Height, &stParams, ppstDetectout, ps32FaceNum),
                     s32Ret, EXIT);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FR_Align(MI_SYS_BufInfo_t* pstSourceBufInfo, MI_S32 s32FormatType, DetectBox_t stFaceBox,
                          MI_U8* pu8OutCropData)
{
    ST_CHECK_POINTER(pstSourceBufInfo);
    ST_CHECK_POINTER(pu8OutCropData);

    MI_S32 s32Ret = 0;

    CHECK_DLA_RESULT(ALGO_FR_Align(pstSourceBufInfo->stFrameData.pVirAddr[0],
                                   pstSourceBufInfo->stFrameData.u16Width, pstSourceBufInfo->stFrameData.u16Height,
                                   s32FormatType, stFaceBox, pu8OutCropData),
                     s32Ret, EXIT);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FR_FeatureExtract(MI_S64* phFr, MI_U8* pu8OutCropData, MI_S16* ps16FaceFeature)
{
    ST_CHECK_POINTER(phFr);
    ST_CHECK_POINTER(pu8OutCropData);

    MI_S32 s32Ret = 0;

    CHECK_DLA_RESULT(ALGO_FR_FeatureExtract(*phFr, pu8OutCropData, ps16FaceFeature), s32Ret, EXIT);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FR_CompareFeature(MI_S16* ps16FaceFeature, MI_S16 (*pps16FaceFeatures)[FR_FEATURE_SIZE],
                                   MI_S32 s32FaceNum, MI_S32* ps32MaxSimilarNum)
{
    ST_CHECK_POINTER(ps16FaceFeature);
    ST_CHECK_POINTER(pps16FaceFeatures);
    ST_CHECK_POINTER(ps32MaxSimilarNum);

    // feature compare
    MI_S32    s32Ret           = 0;
    MI_FLOAT  f32MaxPerSimilar = 0;
    MI_FLOAT  f32PerSimilar    = 0;
    MI_FLOAT* pf32PerSimilar;

    pf32PerSimilar = (MI_FLOAT*)malloc((sizeof(MI_FLOAT) * s32FaceNum));
    if (!pf32PerSimilar)
    {
        printf("Malloc pf32PerSimilar failed\n");
        return -1;
    }

    for (MI_U16 i = 0; i < s32FaceNum; i++)
    {
        CHECK_DLA_RESULT(ALGO_FR_FeatureCompare(ps16FaceFeature, pps16FaceFeatures[i], FR_FEATURE_SIZE, &f32PerSimilar),
                         s32Ret, EXIT);

        pf32PerSimilar[i] = f32PerSimilar;
        if (f32PerSimilar > f32MaxPerSimilar)
        {
            f32MaxPerSimilar   = f32PerSimilar;
            *ps32MaxSimilarNum = i;
        }
    }

    // Based on experimental data
    if (0.45 > f32MaxPerSimilar)
    {
        *ps32MaxSimilarNum = -1;
    }

EXIT:
    free(pf32PerSimilar);
    return s32Ret;
}
