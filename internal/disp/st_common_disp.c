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

#include "st_common.h"
#include "st_common_disp.h"


void ST_Common_DispGetDefaultPubAttr(MI_DISP_PubAttr_t *pstDispPubAttr)
{
    pstDispPubAttr->u32BgColor = YUYV_BLACK;
    pstDispPubAttr->eIntfType = E_MI_DISP_INTF_TTL;
    pstDispPubAttr->eIntfSync = E_MI_DISP_OUTPUT_USER;

    return;
}

void ST_Common_DispGetDefaultLayerAttr(MI_DISP_VideoLayerAttr_t *pstLayerAttr)
{
    /******** Canvas size of the video layer *******/
    pstLayerAttr->stVidLayerSize.u16Width = 1920;
    pstLayerAttr->stVidLayerSize.u16Height = 1080;
    /******** Display resolution *******************/
    pstLayerAttr->stVidLayerDispWin.u16X = 0;
    pstLayerAttr->stVidLayerDispWin.u16Y = 0;
    pstLayerAttr->stVidLayerDispWin.u16Width = 1920;
    pstLayerAttr->stVidLayerDispWin.u16Height = 1080;
    /******** Pixel format of the video layer*******/
    pstLayerAttr->ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

    return;
}

void ST_Common_DispGetDefaultInputPortAttr(MI_DISP_InputPortAttr_t *pstInputPortAttr)
{
    pstInputPortAttr->u16SrcWidth = 1920;
    pstInputPortAttr->u16SrcHeight = 1080;
    pstInputPortAttr->stDispWin.u16X = 0;
    pstInputPortAttr->stDispWin.u16Y = 0;
    pstInputPortAttr->stDispWin.u16Width = 1920;
    pstInputPortAttr->stDispWin.u16Height = 1080;
    pstInputPortAttr->eDecompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    //stWinRect.u16X = 0;
    //stWinRect.u16Y = 0;
    //stWinRect.u16Width = 1920;
    //stWinRect.u16Height = 1080;
    return;
}

MI_S32 ST_Common_DispEnableDev(MI_DISP_DEV DispDev, MI_DISP_PubAttr_t *pstDispPubAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret |= MI_DISP_SetPubAttr(DispDev, pstDispPubAttr);
    s32Ret |= MI_DISP_Enable(DispDev);

    return s32Ret;
}

MI_S32 ST_Common_DispEnableLayer(MI_DISP_DEV DispDev, MI_DISP_LAYER DispLayer, MI_DISP_VideoLayerAttr_t *pstLayerAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret |= MI_DISP_BindVideoLayer(DispLayer, DispDev);
    s32Ret |= MI_DISP_SetVideoLayerAttr(DispLayer, pstLayerAttr);
    s32Ret |= MI_DISP_EnableVideoLayer(DispLayer );

    return s32Ret;
}

MI_S32 ST_Common_DispEnableInputPort( MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort, MI_DISP_InputPortAttr_t *pstInputPortAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret |= MI_DISP_SetInputPortAttr( DispLayer, LayerInputPort, pstInputPortAttr);
    s32Ret |= MI_DISP_EnableInputPort( DispLayer, LayerInputPort);

    return s32Ret;
}

MI_S32 ST_Common_DispDisableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret |= MI_DISP_DisableInputPort(DispLayer, LayerInputPort);

    return s32Ret;
}

MI_S32 ST_Common_DispDisableVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret |= MI_DISP_DisableVideoLayer(DispLayer);
    s32Ret |= MI_DISP_UnBindVideoLayer(DispLayer, DispDev);

    return s32Ret;
}


MI_S32 ST_Common_DispDisableDev(MI_DISP_DEV DispDev)
{
    MI_S32 s32Ret = MI_SUCCESS;
    s32Ret |= MI_DISP_Disable(DispDev);

    return s32Ret;
}
