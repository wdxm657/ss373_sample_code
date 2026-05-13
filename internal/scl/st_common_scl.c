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

#include "st_common_scl.h"
#include "st_common.h"

MI_S32 ST_Common_GetSclDefaultDevAttr(MI_SCL_DevAttr_t *pstDevAttr)
{
    ST_CHECK_POINTER(pstDevAttr);

    memset(pstDevAttr, 0x00, sizeof(MI_SCL_DevAttr_t));

    pstDevAttr->u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL0 | E_MI_SCL_HWSCL1;
    return 0;
}

MI_S32 ST_Common_GetSclDefaultChnAttr(MI_SCL_ChannelAttr_t *pstChnAttr,
                MI_SYS_WindowRect_t *pstInputCrop, MI_SCL_ChnParam_t *pstChnParam)
{
    ST_CHECK_POINTER(pstChnAttr);
    ST_CHECK_POINTER(pstInputCrop);
    ST_CHECK_POINTER(pstChnParam);

    memset(pstChnAttr, 0x00, sizeof(MI_SCL_ChannelAttr_t));
    memset(pstInputCrop, 0x00, sizeof(MI_SYS_WindowRect_t));
    memset(pstChnParam, 0x00, sizeof(MI_SCL_ChnParam_t));

    /*not need to set */
    memset(pstChnAttr, 0x0, sizeof(MI_SCL_ChannelAttr_t));

    /*default not use scl inputcrop */
    pstInputCrop->u16X = 0;
    pstInputCrop->u16Y = 0;
    pstInputCrop->u16Width  = 0;
    pstInputCrop->u16Height = 0;

    pstChnParam->eRot = E_MI_SYS_ROTATE_NONE;

    return 0;
}

MI_S32 ST_Common_GetSclDefaultPortAttr(MI_SCL_OutPortParam_t  *pstOutPortParam)
{
    ST_CHECK_POINTER(pstOutPortParam);

    memset(pstOutPortParam, 0x00, sizeof(MI_SCL_OutPortParam_t));

    pstOutPortParam->bFlip = FALSE;
    pstOutPortParam->bMirror = FALSE;
    pstOutPortParam->eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    pstOutPortParam->ePixelFormat  = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

    /*default not use scl portcrop */
    pstOutPortParam->stSCLOutCropRect.u16X = 0;
    pstOutPortParam->stSCLOutCropRect.u16Y = 0;
    pstOutPortParam->stSCLOutCropRect.u16Width  = 0;
    pstOutPortParam->stSCLOutCropRect.u16Height = 0;

    /*1080p sensor size */
    pstOutPortParam->stSCLOutputSize.u16Width  = 1920;
    pstOutPortParam->stSCLOutputSize.u16Height = 1080;

    return 0;
}


MI_S32 ST_Common_SclCreateDevice(MI_U32 SclDevId, MI_SCL_DevAttr_t *pstSclDevAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_SCL_DevAttr_t stDevAttr;

    ST_CHECK_POINTER(pstSclDevAttr);

    memset(&stDevAttr, 0x0, sizeof(MI_SCL_DevAttr_t));

    stDevAttr.u32NeedUseHWOutPortMask = pstSclDevAttr->u32NeedUseHWOutPortMask;
    s32Ret |= MI_SCL_CreateDevice(SclDevId, &stDevAttr);

    return s32Ret;
}

MI_S32 ST_Common_SclDestroyDevice(MI_U32 SclDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_SCL_DestroyDevice(SclDevId);

    return s32Ret;
}

MI_S32 ST_Common_SclStartChn(MI_U32 SclDevId, MI_U32 SclChnId, MI_SCL_ChannelAttr_t *pstSclChnAttr,
                                    MI_SYS_WindowRect_t *pstSclInputCrop, MI_SCL_ChnParam_t *pstSclChnParam)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_SYS_WindowRect_t stInputCropInfo;
    MI_SCL_ChnParam_t stChnParam;

    ST_CHECK_POINTER(pstSclChnAttr);
    ST_CHECK_POINTER(pstSclInputCrop);
    ST_CHECK_POINTER(pstSclChnParam);

    s32Ret |= MI_SCL_CreateChannel(SclDevId, SclChnId, pstSclChnAttr);

    if(pstSclInputCrop->u16Width != 0
        && pstSclInputCrop->u16Height != 0)
    {
        stInputCropInfo.u16X = pstSclInputCrop->u16X;
        stInputCropInfo.u16Y = pstSclInputCrop->u16Y;
        stInputCropInfo.u16Width = pstSclInputCrop->u16Width;
        stInputCropInfo.u16Height = pstSclInputCrop->u16Height;
        STCHECKRESULT(MI_SCL_SetInputPortCrop(SclDevId, SclChnId, &stInputCropInfo));
    }

    stChnParam.eRot = pstSclChnParam->eRot;
    s32Ret |= MI_SCL_SetChnParam(SclDevId, SclChnId, &stChnParam);

    s32Ret |= MI_SCL_StartChannel(SclDevId, SclChnId);

    return s32Ret;
}

MI_S32 ST_Common_SclStopChn(MI_U32 SclDevId, MI_U32 SclChnId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_SCL_StopChannel(SclDevId, SclChnId);
    s32Ret |= MI_SCL_DestroyChannel(SclDevId, SclChnId);

    return s32Ret;
}

MI_S32 ST_Common_SclEnablePort(MI_U32 SclDevId, MI_U32 SclChnId, MI_U32 SclOutPortId,
                                    MI_SCL_OutPortParam_t *pstOutPortParam)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_SCL_OutPortParam_t stOutPortParam;

    ST_CHECK_POINTER(pstOutPortParam);

    memset(&stOutPortParam, 0x0, sizeof(MI_SCL_OutPortParam_t));

    if(pstOutPortParam->stSCLOutCropRect.u16Width != 0
        && pstOutPortParam->stSCLOutCropRect.u16Height != 0)
    {
        stOutPortParam.stSCLOutCropRect.u16X = pstOutPortParam->stSCLOutCropRect.u16X;
        stOutPortParam.stSCLOutCropRect.u16Y = pstOutPortParam->stSCLOutCropRect.u16Y;
        stOutPortParam.stSCLOutCropRect.u16Width = pstOutPortParam->stSCLOutCropRect.u16Width;
        stOutPortParam.stSCLOutCropRect.u16Height = pstOutPortParam->stSCLOutCropRect.u16Height;
    }
    stOutPortParam.stSCLOutputSize.u16Width = ALIGN_UP(pstOutPortParam->stSCLOutputSize.u16Width, 2);
    stOutPortParam.stSCLOutputSize.u16Height = ALIGN_UP(pstOutPortParam->stSCLOutputSize.u16Height, 2);
    stOutPortParam.bMirror = pstOutPortParam->bMirror;
    stOutPortParam.bFlip = pstOutPortParam->bFlip;
    stOutPortParam.ePixelFormat = pstOutPortParam->ePixelFormat;
    stOutPortParam.eCompressMode = pstOutPortParam->eCompressMode;

    s32Ret |= MI_SCL_SetOutputPortParam(SclDevId, SclChnId, SclOutPortId, &stOutPortParam);

    s32Ret |= MI_SCL_EnableOutputPort(SclDevId, SclChnId, SclOutPortId);

    return s32Ret;
}

MI_S32 ST_Common_SclDisablePort(MI_U32 SclDevId, MI_U32 SclChnId, MI_U32 SclOutPortId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_SCL_DisableOutputPort(SclDevId, SclChnId, SclOutPortId);

    return s32Ret;
}

