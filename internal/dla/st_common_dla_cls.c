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
#include "st_common_dla_cls.h"
#include "st_common.h"

MI_S32 ST_Common_CLS_Init(void** ppHandle, InitInfo_t* pstInit_info)
{
    ST_CHECK_POINTER(pstInit_info);
    STCHECKRESULT(ALGO_CLS_CreateHandle(ppHandle));
    STCHECKRESULT(ALGO_CLS_InitHandle(*ppHandle, pstInit_info));
    return MI_SUCCESS;
}

MI_S32 ST_Common_CLS_DeInit(void** ppHandle)
{
    ST_CHECK_POINTER(ppHandle);
    STCHECKRESULT(ALGO_CLS_DeInitHandle(*ppHandle));
    STCHECKRESULT(ALGO_CLS_ReleaseHandle(*ppHandle));
    return MI_SUCCESS;
}

MI_S32 ST_Common_CLS_SetDefaultParam(void** ppHandle)
{
    ST_CHECK_POINTER(ppHandle);
    MI_FLOAT fThreshold = 0.5;

    STCHECKRESULT(ALGO_CLS_SetThreshold(*ppHandle, fThreshold));
    return MI_SUCCESS;
}

MI_S32 ST_Common_CLS_GetModelAttr(void** ppHandle, InputAttr_t* pstInput_attr)
{
    ST_CHECK_POINTER(ppHandle);
    ST_CHECK_POINTER(pstInput_attr);

    STCHECKRESULT(ALGO_CLS_GetInputAttr(*ppHandle, pstInput_attr));
    return MI_SUCCESS;
}

MI_S32 ST_Common_CLS_GetTargetData(void** ppHandle, MI_SYS_BufInfo_t* pstScaledBufInfo,
                                   OutputInfo_t astResults[NUM_OUTPUS], MI_S32* pOutputCount)
{
    ST_CHECK_POINTER(ppHandle);
    ST_CHECK_POINTER(pstScaledBufInfo);
    ST_CHECK_POINTER(astResults);
    ST_CHECK_POINTER(pOutputCount);

    MI_S32       s32Ret = 0;
    ALGO_Input_t stDetInputBufInfo;
    stDetInputBufInfo.buf_size   = pstScaledBufInfo->stFrameData.u32BufSize;
    stDetInputBufInfo.p_vir_addr = pstScaledBufInfo->stFrameData.pVirAddr[0];
    stDetInputBufInfo.phy_addr   = pstScaledBufInfo->stFrameData.phyAddr[0];

    CHECK_DLA_RESULT(ALGO_CLS_Run(*ppHandle, &stDetInputBufInfo, astResults, pOutputCount), s32Ret, EXIT);

EXIT:
    return s32Ret;
}