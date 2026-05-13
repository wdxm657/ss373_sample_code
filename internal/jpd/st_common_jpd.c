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

#include "st_common_jpd.h"
#include "st_common.h"

MI_S32 ST_Common_GetJpdDefaultDevAttr(MI_JPD_InitParam_t *pstInitParam)
{
    ST_CHECK_POINTER(pstInitParam);
    return 0;
}

MI_S32 ST_Common_GetJpdDefaultChnAttr(MI_JPD_ChnCreatConf_t *pstChnAttr)
{
    ST_CHECK_POINTER(pstChnAttr);
    memset(pstChnAttr, 0, sizeof(MI_JPD_ChnCreatConf_t));
    pstChnAttr->u32StreamBufSize = 1024 * 1024;
    pstChnAttr->ePixelFormat     = 0x0; // Not user
    pstChnAttr->u32MaxPicWidth   = 3840;
    pstChnAttr->u32MaxPicHeight  = 2160;

    return 0;
}

MI_S32 ST_Common_JpdCreateDev(MI_JPD_DEV DevId, MI_JPD_InitParam_t *pstInitParam)
{
    MI_S32 s32Ret = MI_SUCCESS;
    ST_CHECK_POINTER(pstInitParam);
    s32Ret |= MI_JPD_CreateDev(DevId, pstInitParam);

    return s32Ret;
}

MI_S32 ST_Common_JpdDestroyDev(MI_JPD_DEV DevId)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret |= MI_JPD_DestroyDev(DevId);

    return s32Ret;
}

MI_S32 ST_Common_JpdStartChn(MI_JPD_DEV DevId, MI_JPD_CHN Chn, MI_JPD_ChnCreatConf_t *pstChnAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    ST_CHECK_POINTER(pstChnAttr);
    s32Ret |= MI_JPD_CreateChn(DevId, Chn, pstChnAttr);
    s32Ret |= MI_JPD_StartChn(DevId, Chn);
    return s32Ret;
}

MI_S32 ST_Common_JpdStopChn(MI_JPD_DEV DevId, MI_JPD_CHN VdecChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret |= MI_JPD_StopChn(DevId, VdecChn);
    s32Ret |= MI_JPD_DestroyChn(DevId, VdecChn);

    return s32Ret;
}
