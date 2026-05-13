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
#include "st_common_dla_hpose.h"
#include "st_common.h"

MI_S32 ST_Common_HPOSE_Init(MI_HPOSE_HANDLE* phHpose, AlgoHposeInitParam_t* pstInitParam)
{
    ST_CHECK_POINTER(pstInitParam);

    STCHECKRESULT(ALGO_HPOSE_CreateHandle(phHpose));
    STCHECKRESULT(ALGO_HPOSE_HandleInit(*phHpose, pstInitParam));

    return MI_SUCCESS;
}

MI_S32 ST_Common_HPOSE_DeInit(MI_HPOSE_HANDLE* phHpose)
{
    ST_CHECK_POINTER(phHpose);

    STCHECKRESULT(ALGO_HPOSE_HandleDeinit(*phHpose));
    STCHECKRESULT(ALGO_HPOSE_ReleaseHandle(*phHpose));

    return MI_SUCCESS;
}

MI_S32 ST_Common_HPOSE_DetectHeadPose(MI_HPOSE_HANDLE* phHpose, MI_SYS_BufInfo_t* pstScaledBufInfo,
                                      HposeBbox_t** faceBox, MI_S32* faceCount, HposeBbox_t** bodyBox,
                                      MI_S32* bodyCount)
{
    ST_CHECK_POINTER(phHpose);
    ST_CHECK_POINTER(pstScaledBufInfo);
    ST_CHECK_POINTER(faceCount);
    ST_CHECK_POINTER(bodyCount);

    // get hand data
    MI_S32               s32Ret = 0;
    AlgoHposeInputInfo_t stHposeScaledBufInfo;

    memset(&stHposeScaledBufInfo, 0x0, sizeof(AlgoHposeInputInfo_t));

    stHposeScaledBufInfo.bufsize         = pstScaledBufInfo->stFrameData.u32BufSize;
    stHposeScaledBufInfo.pt_tensor_data  = pstScaledBufInfo->stFrameData.pVirAddr[0];
    stHposeScaledBufInfo.phy_tensor_addr = pstScaledBufInfo->stFrameData.phyAddr[0];
    CHECK_DLA_RESULT(
        ALGO_HPOSE_FacePersonDetect(*phHpose, &stHposeScaledBufInfo, faceBox, faceCount, bodyBox, bodyCount), s32Ret,
        EXIT);

    // Width and height alignment
    for (MI_U32 j = 0; j < *faceCount; j++)
    {
        (*faceBox)[j].width  = ALIGN_BACK((*faceBox)[j].width, 16);
        (*faceBox)[j].height = ALIGN_BACK((*faceBox)[j].height, 16);
    }

    for (MI_U32 k = 0; k < *bodyCount; k++)
    {
        (*bodyBox)[k].width  = ALIGN_BACK((*bodyBox)[k].width, 16);
        (*bodyBox)[k].height = ALIGN_BACK((*bodyBox)[k].height, 16);
    }

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_HPOSE_RecognizeHead(MI_HPOSE_HANDLE* phHpose, MI_SYS_BufInfo_t* pstHeadBufInfo, HposeBbox_t* pstDetBox,
                                     HposeHeadResult_t* result)
{
    ST_CHECK_POINTER(phHpose);
    ST_CHECK_POINTER(pstHeadBufInfo);

    MI_S32               s32Ret;
    AlgoHposeInputInfo_t stAlgHandBufInfo;
    stAlgHandBufInfo.bufsize         = pstHeadBufInfo->stFrameData.u32BufSize;
    stAlgHandBufInfo.pt_tensor_data  = pstHeadBufInfo->stFrameData.pVirAddr[0];
    stAlgHandBufInfo.phy_tensor_addr = pstHeadBufInfo->stFrameData.phyAddr[0];

    s32Ret = ALGO_HPOSE_HeadPoseRecognition(*phHpose, &stAlgHandBufInfo, *pstDetBox, result);
    return s32Ret;
}

MI_S32 ST_Common_HPOSE_RecognizeBody(MI_HPOSE_HANDLE* phHpose, MI_SYS_BufInfo_t* pstBodyBufInfo, HposeBbox_t* pstDetBox,
                                     HposeBodyResult_t* result)
{
    ST_CHECK_POINTER(phHpose);
    ST_CHECK_POINTER(pstBodyBufInfo);

    MI_S32               s32Ret;
    AlgoHposeInputInfo_t stAlgBodyBufInfo;
    stAlgBodyBufInfo.bufsize         = pstBodyBufInfo->stFrameData.u32BufSize;
    stAlgBodyBufInfo.pt_tensor_data  = pstBodyBufInfo->stFrameData.pVirAddr[0];
    stAlgBodyBufInfo.phy_tensor_addr = pstBodyBufInfo->stFrameData.phyAddr[0];

    s32Ret = ALGO_HPOSE_BodyPoseRecognition(*phHpose, &stAlgBodyBufInfo, *pstDetBox, result);
    return s32Ret;
}
