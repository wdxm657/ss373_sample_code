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

#include "st_common_vdec.h"
#include "st_common.h"

MI_S32 ST_Common_GetVdecDefaultDevAttr(MI_VDEC_InitParam_t *pstInitParam)
{
    ST_CHECK_POINTER(pstInitParam);
    pstInitParam->u16MaxWidth  = 3840;
    pstInitParam->u16MaxHeight = 2160;

    return 0;
}

MI_S32 ST_Common_GetVdecDefaultChnAttr(MI_VDEC_CodecType_e eCodecType, MI_VDEC_ChnAttr_t *pstVdecChnAttr)
{
    ST_CHECK_POINTER(pstVdecChnAttr);
    if (eCodecType == E_MI_VDEC_CODEC_TYPE_H264 || eCodecType == E_MI_VDEC_CODEC_TYPE_H265)
    {
        memset(pstVdecChnAttr, 0, sizeof(MI_VDEC_ChnAttr_t));
        pstVdecChnAttr->eCodecType                                                = eCodecType;
        pstVdecChnAttr->u32BufSize                                                = 1024 * 1024;
        pstVdecChnAttr->u32Priority                                               = 0x0;
        pstVdecChnAttr->u32PicWidth                                               = 3840;
        pstVdecChnAttr->u32PicHeight                                              = 2160;
        pstVdecChnAttr->eVideoMode                                                = E_MI_VDEC_VIDEO_MODE_FRAME;
        pstVdecChnAttr->eDpbBufMode                                               = E_MI_VDEC_DPB_MODE_NORMAL;
        pstVdecChnAttr->stVdecVideoAttr.u32RefFrameNum                            = 2;
        pstVdecChnAttr->stVdecVideoAttr.stErrHandlePolicy.bUseCusPolicy           = FALSE;
        pstVdecChnAttr->stVdecVideoAttr.stErrHandlePolicy.u8ErrMBPercentThreshold = 0;
        pstVdecChnAttr->stVdecVideoAttr.bDisableLowLatency                        = 0;
    }
    else
    {
        ST_ERR("eCodecType %d not support\n", eCodecType);
    }

    return 0;
}
MI_S32 ST_Common_GetVdecDefaultPortAttr(MI_VDEC_OutputPortAttr_t *pstOutPortAttr)
{
    ST_CHECK_POINTER(pstOutPortAttr);
    pstOutPortAttr->u16Width  = 3840;
    pstOutPortAttr->u16Height = 2160;

    return 0;
}

MI_S32 ST_Common_VdecCreateDev(MI_VDEC_DEV DevId, MI_VDEC_InitParam_t *pstInitParam)
{
    MI_S32 s32Ret = MI_SUCCESS;
    ST_CHECK_POINTER(pstInitParam);
    s32Ret |= MI_VDEC_CreateDev(DevId, pstInitParam);

    return s32Ret;
}

MI_S32 ST_Common_VdecDestroyDev(MI_VDEC_DEV DevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_VDEC_DestroyDev(DevId);

    return s32Ret;
}

MI_S32 ST_Common_VdecStartChn(MI_VDEC_DEV DevId, MI_VDEC_CHN VdecChn, MI_VDEC_ChnAttr_t *pstVdecChnAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    ST_CHECK_POINTER(pstVdecChnAttr);
    s32Ret |= MI_VDEC_CreateChn(DevId, VdecChn, pstVdecChnAttr);
    s32Ret |= MI_VDEC_StartChn(DevId, VdecChn);
    return s32Ret;
}

MI_S32 ST_Common_VdecStopChn(MI_VDEC_DEV DevId, MI_VDEC_CHN VdecChn)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret |= MI_VDEC_StopChn(DevId, VdecChn);
    s32Ret |= MI_VDEC_DestroyChn(DevId, VdecChn);

    return s32Ret;
}
MI_S32 ST_Common_VdecSetOutputPortAttr(MI_VDEC_DEV DevId, MI_VDEC_CHN VdecChn, MI_VDEC_OutputPortAttr_t *pstOutPortAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    ST_CHECK_POINTER(pstOutPortAttr);
    s32Ret |= MI_VDEC_SetOutputPortAttr(DevId, VdecChn, pstOutPortAttr);
    return s32Ret;
}
