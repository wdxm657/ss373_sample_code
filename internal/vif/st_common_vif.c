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

#include "st_common_vif.h"
#include "st_common.h"
#include "platform.h"

MI_S32 ST_Common_GetVifGroupDevByPad(MI_SNR_PADID u32SnrPadId, MI_VIF_GROUP *pVifGroup, MI_VIF_DEV *pVifDev)
{
    MI_S32 s32Ret = 0;

    ST_CHECK_POINTER(pVifGroup);
    ST_CHECK_POINTER(pVifDev);

    if (u32SnrPadId > MAX_SENSOR_NUM - 1)
    {
        ST_ERR("sensor pad %d not support!!!\n", u32SnrPadId);
        return -1;
    }

    switch (u32SnrPadId)
    {
        case 0:
            *pVifGroup = 0;
            *pVifDev   = 0;
            break;
        case 1:
            *pVifGroup = 2;
            *pVifDev   = 8;
            break;
        case 2:
            *pVifGroup = 1;
            *pVifDev   = 4;
            break;
        case 3:
            *pVifGroup = 3;
            *pVifDev   = 12;
            break;
        case 4:
            *pVifGroup = 4;
            *pVifDev   = 16;
            break;
        case 5:
            *pVifGroup = 6;
            *pVifDev   = 24;
            break;
        case 6:
            *pVifGroup = 5;
            *pVifDev   = 20;
            break;
        case 7:
            *pVifGroup = 7;
            *pVifDev   = 28;
            break;
        default:
            ST_ERR("pad not support!!!\n");
            return -1;
    }

    return s32Ret;
}

MI_S32 ST_Common_GetSensorPadByVifGroup(MI_VIF_GROUP VifGroup, MI_SNR_PADID *pu32SnrPadId)
{
    ST_CHECK_POINTER(pu32SnrPadId);

    if (VifGroup > MAX_VIF_GROUP_NUM - 1)
    {
        ST_ERR("vif group %d not support!!!\n", VifGroup);
        return -1;
    }

    switch (VifGroup)
    {
        case 0:
            *pu32SnrPadId = 0;
            break;
        case 1:
            *pu32SnrPadId = 2;
            break;
        case 2:
            *pu32SnrPadId = 1;
            break;
        case 3:
            *pu32SnrPadId = 3;
            break;
        case 4:
            *pu32SnrPadId = 4;
            break;
        case 5:
            *pu32SnrPadId = 6;
            break;
        case 6:
            *pu32SnrPadId = 5;
            break;
        case 7:
            *pu32SnrPadId = 7;
            break;
        default:
            return -1;
    }

    return MI_SUCCESS;
}

MI_U32 ST_Common_VifGroupStitchMaskBySnrMask(MI_U32 u32SensorBindId)
{
    // get vif group mask by padid mask
    MI_U32 u32Snr2Grp[8] = {0x1, 0x4, 0x2, 0x8, 0x10, 0x40, 0x20, 0x80};

    MI_U32 u32GroupMask = 0, i = 0;
    for (i = 0; i < MAX_SENSOR_NUM; i++)
    {
        if (u32SensorBindId & (1 << i))
        {
            u32GroupMask |= u32Snr2Grp[i];
        }
    }

    return u32GroupMask;
}

MI_S32 ST_Common_GetSensorPadByVifDev(MI_VIF_DEV VifDev, MI_SNR_PADID *pu32SnrPadId)
{
    MI_S32 s32Ret = 0;

    ST_CHECK_POINTER(pu32SnrPadId);

    if (VifDev > MAX_VIF_DEV_NUM - 1)
    {
        ST_ERR("vif dev %d not support!!!\n", VifDev);
        return -1;
    }

    switch (VifDev)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            *pu32SnrPadId = 0;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            *pu32SnrPadId = 2;
            break;
        case 8:
        case 9:
        case 10:
        case 11:
            *pu32SnrPadId = 1;
            break;
        case 12:
        case 13:
        case 14:
        case 15:
            *pu32SnrPadId = 3;
            break;
        case 16:
        case 17:
        case 18:
        case 19:
            *pu32SnrPadId = 4;
            break;
        case 20:
        case 21:
        case 22:
        case 23:
            *pu32SnrPadId = 6;
            break;
        case 24:
        case 25:
        case 26:
        case 27:
            *pu32SnrPadId = 5;
            break;
        case 28:
        case 29:
        case 30:
        case 31:
            *pu32SnrPadId = 7;
            break;
        default:
            ST_ERR("Dev not support!!!\n");
            return -1;
    }

    return s32Ret;
}

MI_S32 ST_Common_SensorInit(MI_SNR_PADID eSnrPad, MI_BOOL bPlaneMode, MI_U8 u8ChoiceIndex, MI_U32 u32Fps)
{
    MI_S32       s32Ret = MI_SUCCESS;
    MI_U32       u32ResCount;
    MI_U8        u8ResIndex;
    MI_U8        u8ChocieRes = u8ChoiceIndex;
    MI_SNR_Res_t stRes;
    MI_S32       s32Input = 0;

    memset(&stRes, 0x00, sizeof(MI_SNR_Res_t));

    s32Ret |= MI_SNR_SetPlaneMode(eSnrPad, bPlaneMode); // HDR Off bPlaneMode = False
    s32Ret |= MI_SNR_QueryResCount(eSnrPad, &u32ResCount);

    for (u8ResIndex = 0; u8ResIndex < u32ResCount; u8ResIndex++)
    {
        s32Ret |= MI_SNR_GetRes(eSnrPad, u8ResIndex, &stRes);
        printf("index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n", u8ResIndex,
               stRes.stCropRect.u16X, stRes.stCropRect.u16Y, stRes.stCropRect.u16Width, stRes.stCropRect.u16Height,
               stRes.stOutputSize.u16Width, stRes.stOutputSize.u16Height, stRes.u32MaxFps, stRes.u32MinFps,
               stRes.strResDesc);
    }

    if (u8ChocieRes >= u32ResCount && u8ChocieRes != 0xff)
    {
        printf("res set err  %d > =cnt %d\n", u8ChocieRes, u32ResCount);
        return -1;
    }
    else if (u8ChocieRes == 0xff)
    {
        printf("choice which resolution use, cnt %d\n", u32ResCount);
        do
        {
            scanf("%d", &s32Input);
            u8ChocieRes = (MI_U8)s32Input;
            ST_Common_Flush();
            s32Ret |= MI_SNR_QueryResCount(eSnrPad, &u32ResCount);
            if (u8ChocieRes >= u32ResCount)
            {
                printf("choice err res %d > =cnt %d\n", u8ChocieRes, u32ResCount);
            }
        } while (u8ChocieRes >= u32ResCount);
        printf("You select %d res\n", u8ChocieRes);
    }
    printf("Res %d\n", u8ChocieRes);

    s32Ret |= MI_SNR_GetRes(eSnrPad, u8ChocieRes, &stRes);

    s32Ret |= MI_SNR_SetRes(eSnrPad, u8ChocieRes);

    if (u32Fps == 0xff)
    {
        if (stRes.u32MaxFps > 30)
        {
            s32Ret |= MI_SNR_SetFps(eSnrPad, 30); // default use 30 fps
        }
    }
    else
    {
        s32Ret |= MI_SNR_SetFps(eSnrPad, u32Fps);
    }

    s32Ret |= MI_SNR_Enable(eSnrPad);

    return s32Ret;
}

MI_S32 ST_Common_SensorDeInit(MI_SNR_PADID eSnrPad)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_SNR_Disable(eSnrPad);

    return s32Ret;
}

MI_S32 ST_Common_GetVifDefaultGrouptAttr(MI_VIF_GroupAttr_t *pstGroupAttr)
{
    ST_CHECK_POINTER(pstGroupAttr);

    memset(pstGroupAttr, 0x00, sizeof(MI_VIF_GroupAttr_t));

    /*set from SnrpadInfo */
    pstGroupAttr->eIntfMode = E_MI_VIF_MODE_MIPI;
    /*set from SnrpadInfo */
    pstGroupAttr->eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;
    // eMclk default not need set */
    pstGroupAttr->eMclk = 0;

    pstGroupAttr->eWorkMode          = E_MI_VIF_WORK_MODE_1MULTIPLEX;
    pstGroupAttr->eHDRType           = E_MI_VIF_HDR_TYPE_OFF;
    pstGroupAttr->eScanMode          = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    pstGroupAttr->u32GroupStitchMask = 0;
    pstGroupAttr->eHDRFusionTpye     = E_MI_VIF_HDR_FUSION_TYPE_NONE;
    pstGroupAttr->u8HDRExposureMask  = E_MI_VIF_HDR_EXPOSURE_TYPE_NONE;

    pstGroupAttr->stMetaDataAttr.u32MetaDataTypeMask  = 0;
    pstGroupAttr->stMetaDataAttr.stCropRect.u16X      = 0;
    pstGroupAttr->stMetaDataAttr.stCropRect.u16Y      = 0;
    pstGroupAttr->stMetaDataAttr.stCropRect.u16Width  = 0;
    pstGroupAttr->stMetaDataAttr.stCropRect.u16Height = 0;

    return 0;
}

MI_S32 ST_Common_GetVifDefaultDevAttr(MI_VIF_DevAttr_t *pstDevAttr)
{
    ST_CHECK_POINTER(pstDevAttr);

    memset(pstDevAttr, 0x00, sizeof(MI_VIF_DevAttr_t));

    /*set from SnrplaneInfo */
    pstDevAttr->eInputPixel = E_MI_SYS_PIXEL_BAYERID_MAX;
    /*set from SnrplaneInfo here not set */
    pstDevAttr->stInputRect.u16X      = 0;
    pstDevAttr->stInputRect.u16X      = 0;
    pstDevAttr->stInputRect.u16Width  = 0;
    pstDevAttr->stInputRect.u16Height = 0;

    pstDevAttr->eField       = E_MI_SYS_FIELDTYPE_NONE;
    pstDevAttr->bEnH2T1PMode = FALSE;

    return 0;
}

MI_S32 ST_Common_GetVifDefaultPortAttr(MI_VIF_OutputPortAttr_t *pstPortAttr)
{
    ST_CHECK_POINTER(pstPortAttr);

    memset(pstPortAttr, 0x00, sizeof(MI_VIF_OutputPortAttr_t));

    /*set from dev attr */
    pstPortAttr->stCapRect.u16X       = 0;
    pstPortAttr->stCapRect.u16Y       = 0;
    pstPortAttr->stCapRect.u16Width   = 0;
    pstPortAttr->stCapRect.u16Height  = 0;
    pstPortAttr->stDestSize.u16Width  = pstPortAttr->stCapRect.u16Width;
    pstPortAttr->stDestSize.u16Height = pstPortAttr->stCapRect.u16Height;
    /*set from dev attr */
    pstPortAttr->ePixFormat = E_MI_SYS_PIXEL_FRAME_FORMAT_MAX;

    pstPortAttr->eFrameRate    = E_MI_VIF_FRAMERATE_FULL;
    pstPortAttr->eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;

    return 0;
}

MI_S32 ST_Common_VifCreateDevGroup(MI_U32 VifGroupId, MI_VIF_GroupAttr_t *pstVifGroupAttr)
{
    MI_U32             u32SnrPad = 0;
    MI_S32             s32Ret    = MI_SUCCESS;
    MI_VIF_GroupAttr_t stGroupAttr;
    MI_SNR_PADInfo_t   stSnrPadInfo;

    ST_CHECK_POINTER(pstVifGroupAttr);

    memset(&stGroupAttr, 0x0, sizeof(MI_VIF_GroupAttr_t));
    memset(&stSnrPadInfo, 0x0, sizeof(MI_SNR_PADInfo_t));

    ST_Common_GetSensorPadByVifGroup(VifGroupId, &u32SnrPad);

    s32Ret |= MI_SNR_GetPadInfo(u32SnrPad, &stSnrPadInfo);

    stGroupAttr.eIntfMode = (MI_VIF_IntfMode_e)stSnrPadInfo.eIntfMode;
    if (E_MI_VIF_MODE_BT656 == stGroupAttr.eIntfMode)
    {
        stGroupAttr.eClkEdge = (MI_VIF_ClkEdge_e)stSnrPadInfo.unIntfAttr.stBt656Attr.eClkEdge;
    }
    else
    {
        stGroupAttr.eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;
    }

    stGroupAttr.eWorkMode          = pstVifGroupAttr->eWorkMode;
    stGroupAttr.eScanMode          = pstVifGroupAttr->eScanMode;
    stGroupAttr.eHDRType           = pstVifGroupAttr->eHDRType;
    stGroupAttr.eHDRFusionTpye     = pstVifGroupAttr->eHDRFusionTpye;
    stGroupAttr.u8HDRExposureMask  = pstVifGroupAttr->u8HDRExposureMask;
    stGroupAttr.u32GroupStitchMask = pstVifGroupAttr->u32GroupStitchMask;
    memcpy(&stGroupAttr.stMetaDataAttr, &pstVifGroupAttr->stMetaDataAttr, sizeof(MI_VIF_MetaDataAttr_t));

    s32Ret |= MI_VIF_CreateDevGroup(VifGroupId, &stGroupAttr);

    return s32Ret;
}

MI_S32 ST_Common_VifDestroyDevGroup(MI_U32 VifGroupId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_VIF_DestroyDevGroup(VifGroupId);

    return s32Ret;
}

MI_S32 ST_Common_VifEnableDev(MI_U32 VifDevId, MI_VIF_DevAttr_t *pstVifDevAttr)
{
    MI_S32             s32Ret    = MI_SUCCESS;
    MI_U32             u32SnrPad = 0;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_VIF_DevAttr_t   stDevAttr;

    ST_CHECK_POINTER(pstVifDevAttr);

    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t));
    memset(&stDevAttr, 0x0, sizeof(MI_VIF_DevAttr_t));

    ST_Common_GetSensorPadByVifDev(VifDevId, &u32SnrPad);

    s32Ret |= MI_SNR_GetPlaneInfo(u32SnrPad, 0, &stSnrPlane0Info);

    stDevAttr.stInputRect.u16X      = stSnrPlane0Info.stCapRect.u16X;
    stDevAttr.stInputRect.u16Y      = stSnrPlane0Info.stCapRect.u16Y;
    stDevAttr.stInputRect.u16Width  = stSnrPlane0Info.stCapRect.u16Width;
    stDevAttr.stInputRect.u16Height = stSnrPlane0Info.stCapRect.u16Height;
    if (stSnrPlane0Info.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
    {
        stDevAttr.eInputPixel = stSnrPlane0Info.ePixel;
    }
    else
    {
        stDevAttr.eInputPixel =
            (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);
    }

    if (pstVifDevAttr->stInputRect.u16Width != 0 && pstVifDevAttr->stInputRect.u16Height != 0)
    {
        stDevAttr.stInputRect.u16X      = pstVifDevAttr->stInputRect.u16X;
        stDevAttr.stInputRect.u16Y      = pstVifDevAttr->stInputRect.u16Y;
        stDevAttr.stInputRect.u16Width  = pstVifDevAttr->stInputRect.u16Width;
        stDevAttr.stInputRect.u16Height = pstVifDevAttr->stInputRect.u16Height;
    }

    stDevAttr.bEnH2T1PMode = pstVifDevAttr->bEnH2T1PMode;
    stDevAttr.eField       = pstVifDevAttr->eField;

    s32Ret |= MI_VIF_SetDevAttr(VifDevId, &stDevAttr);
    s32Ret |= MI_VIF_EnableDev(VifDevId);

    return s32Ret;
}

MI_S32 ST_Common_VifDisableDev(MI_U32 VifDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_VIF_DisableDev(VifDevId);

    return s32Ret;
}

MI_S32 ST_Common_VifEnablePort(MI_U32 VifDevId, MI_U32 VifPortId, MI_VIF_OutputPortAttr_t *pstVifPortAttr)
{
    MI_S32                  s32Ret = MI_SUCCESS;
    MI_VIF_DevAttr_t        stDevAttr;
    MI_VIF_OutputPortAttr_t stPortAttr;

    ST_CHECK_POINTER(pstVifPortAttr);

    memset(&stDevAttr, 0x0, sizeof(MI_VIF_DevAttr_t));
    memset(&stPortAttr, 0x0, sizeof(MI_VIF_OutputPortAttr_t));

    s32Ret |= MI_VIF_GetDevAttr(VifDevId, &stDevAttr);

    stPortAttr.stCapRect.u16X       = 0;
    stPortAttr.stCapRect.u16Y       = 0;
    stPortAttr.stCapRect.u16Width   = stDevAttr.stInputRect.u16Width;
    stPortAttr.stCapRect.u16Height  = stDevAttr.stInputRect.u16Height;
    stPortAttr.stDestSize.u16Width  = stDevAttr.stInputRect.u16Width;
    stPortAttr.stDestSize.u16Height = stDevAttr.stInputRect.u16Height;
    stPortAttr.ePixFormat           = stDevAttr.eInputPixel;

    stPortAttr.eFrameRate    = pstVifPortAttr->eFrameRate;
    stPortAttr.eCompressMode = pstVifPortAttr->eCompressMode;
    s32Ret |= MI_VIF_SetOutputPortAttr(VifDevId, VifPortId, &stPortAttr);

    s32Ret |= MI_VIF_EnableOutputPort(VifDevId, VifPortId);

    return s32Ret;
}

MI_S32 ST_Common_VifDisablePort(MI_U32 VifDevId, MI_U32 VifPortId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_VIF_DisableOutputPort(VifDevId, VifPortId);

    return s32Ret;
}
