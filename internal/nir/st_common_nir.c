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

#include "st_common_nir.h"
#include "st_common.h"

MI_S32 ST_Common_GetNirDefaultDevAttr(MI_NIR_DevAttr_t *pstDevAttr)
{
    ST_CHECK_POINTER(pstDevAttr);

    memset(pstDevAttr, 0, sizeof(MI_NIR_DevAttr_t));
    return 0;
}

MI_S32 ST_Common_GetNirDefaultChnAttr(MI_NIR_ChannelAttr_t *pstChnAttr)
{
    ST_CHECK_POINTER(pstChnAttr);

    memset(pstChnAttr, 0x0, sizeof(MI_NIR_ChannelAttr_t));
    pstChnAttr->eMode = E_MI_NIR_MODE_NORMAL;

    return 0;
}

MI_S32 ST_Common_NirCreateDevice(MI_U32 NirDevId, MI_NIR_DevAttr_t *pstDevAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ST_CHECK_POINTER(pstDevAttr);

    s32Ret |= MI_NIR_CreateDevice(NirDevId, pstDevAttr);

    return s32Ret;
}

MI_S32 ST_Common_NirDestroyDevice(MI_U32 NirDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_NIR_DestroyDevice(NirDevId);

    return s32Ret;
}

MI_S32 ST_Common_NirStartChn(MI_U32 NirDevId, MI_U32 NirChnId, MI_NIR_ChannelAttr_t *pstChnAttr,
                             MI_NIR_ChnParam_t *pstChnParam)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ST_CHECK_POINTER(pstChnAttr);
    ST_CHECK_POINTER(pstChnParam);

    s32Ret |= MI_NIR_CreateChannel(NirDevId, NirChnId, pstChnAttr);

    s32Ret |= MI_NIR_SetChnParam(NirDevId, NirChnId, pstChnParam);

    s32Ret |= MI_NIR_StartChannel(NirDevId, NirChnId);

    return s32Ret;
}

MI_S32 ST_Common_NirStopChn(MI_U32 NirDevId, MI_U32 NirChnId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_NIR_StopChannel(NirDevId, NirChnId);
    s32Ret |= MI_NIR_DestroyChannel(NirDevId, NirChnId);

    return s32Ret;
}
