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
#include "st_common_dla_hgr.h"
#include "st_common.h"

MI_S32 ST_Common_HGR_Init(MI_HGR_HANDLE* phHgr, AlgoHgrInitParam_t* pstInitParam)
{
    ST_CHECK_POINTER(phHgr);
    ST_CHECK_POINTER(pstInitParam);

    STCHECKRESULT(ALGO_HGR_CreateHandle(phHgr));
    STCHECKRESULT(ALGO_HGR_HandleInit(*phHgr, pstInitParam));

    return MI_SUCCESS;
}

MI_S32 ST_Common_HGR_DeInit(MI_HGR_HANDLE* phHgr)
{
    ST_CHECK_POINTER(phHgr);

    STCHECKRESULT(ALGO_HGR_HandleDeinit(*phHgr));
    STCHECKRESULT(ALGO_HGR_ReleaseHandle(*phHgr));

    return MI_SUCCESS;
}

MI_S32 ST_Common_HGR_DetectHands(MI_HGR_HANDLE* phHgr, MI_SYS_BufInfo_t* pstScaledBufInfo, MI_S32* ps32Count,
                                 HgrBbox_t** ppstOutbox)
{
    ST_CHECK_POINTER(phHgr);
    ST_CHECK_POINTER(pstScaledBufInfo);
    ST_CHECK_POINTER(ps32Count);

    // get hand data
    MI_S32             s32Ret = 0;
    AlgoHgrInputInfo_t stHgrScaledBufInfo;

    memset(&stHgrScaledBufInfo, 0x0, sizeof(AlgoHgrInputInfo_t));

    stHgrScaledBufInfo.bufsize         = pstScaledBufInfo->stFrameData.u32BufSize;
    stHgrScaledBufInfo.pt_tensor_data  = pstScaledBufInfo->stFrameData.pVirAddr[0];
    stHgrScaledBufInfo.phy_tensor_addr = pstScaledBufInfo->stFrameData.phyAddr[0];
    CHECK_DLA_RESULT(ALGO_HGR_HandPersonDetect(*phHgr, &stHgrScaledBufInfo, ppstOutbox, ps32Count), s32Ret, EXIT);

    // Width and height alignment
    for (MI_U32 i = 0; i < *ps32Count; i++)
    {
        (*ppstOutbox)[i].width  = ALIGN_BACK((*ppstOutbox)[i].width, 16);
        (*ppstOutbox)[i].height = ALIGN_BACK((*ppstOutbox)[i].height, 16);
    }

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_HGR_RecognizeHand(MI_HGR_HANDLE* phHgr, MI_SYS_BufInfo_t* pstHandBufInfo, HgrBbox_t* pstOutbox,
                                   HgrPoint_t** ppstOutpoint, MI_S32* ps32Pointcounts, AlgoHgrResult_t* pstHgrResult)
{
    ST_CHECK_POINTER(phHgr);
    ST_CHECK_POINTER(pstHandBufInfo);

    AlgoHgrInputInfo_t stHgrHandBufInfo;
    stHgrHandBufInfo.bufsize         = pstHandBufInfo->stFrameData.u32BufSize;
    stHgrHandBufInfo.pt_tensor_data  = pstHandBufInfo->stFrameData.pVirAddr[0];
    stHgrHandBufInfo.phy_tensor_addr = pstHandBufInfo->stFrameData.phyAddr[0];

    MI_S32 s32Ret = ALGO_HGR_HandPoseRecognition(*phHgr, &stHgrHandBufInfo, *pstOutbox, ppstOutpoint, ps32Pointcounts,
                                                 pstHgrResult);
    return s32Ret;
}
