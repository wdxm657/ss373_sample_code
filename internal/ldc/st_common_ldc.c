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

#include "st_common_ldc.h"
#include "st_common.h"
#include "platform.h"

MI_S32 ST_Common_GetLdcDefaultDevAttr(MI_LDC_DevAttr_t *pstLdcDevAttr)
{
    ST_CHECK_POINTER(pstLdcDevAttr);
    memset(pstLdcDevAttr, 0, sizeof(MI_LDC_DevAttr_t));
    return 0;
}

MI_S32 ST_Common_GetLdcDefaultChnAttr(MI_LDC_ChnAttr_t *pstLdcChnAttr)
{
    ST_CHECK_POINTER(pstLdcChnAttr);
    memset(pstLdcChnAttr, 0, sizeof(MI_LDC_ChnAttr_t));
    pstLdcChnAttr->eWorkMode      = MI_LDC_WORKMODE_LDC;
    pstLdcChnAttr->eInputBindType = E_MI_SYS_BIND_TYPE_HW_AUTOSYNC;

    return 0;
}

MI_S32 ST_Common_GetLdcDefaultLdcModeChnAttr(MI_LDC_ChnLDCAttr_t *pstLdcModeChnAttr)
{
    ST_CHECK_POINTER(pstLdcModeChnAttr);

    memset(pstLdcModeChnAttr, 0, sizeof(MI_LDC_ChnLDCAttr_t));

    pstLdcModeChnAttr->bBgColor                     = FALSE;
    pstLdcModeChnAttr->u32BgColor                   = 0;
    pstLdcModeChnAttr->eMountMode                   = MI_LDC_WALL_MOUNT;
    pstLdcModeChnAttr->stCalibInfo.s32CenterXOffset = 0;
    pstLdcModeChnAttr->stCalibInfo.s32CenterYOffset = 0;
    pstLdcModeChnAttr->stCalibInfo.s32FisheyeRadius = 585;

    pstLdcModeChnAttr->u32RegionNum = 1;

    MI_LDC_RegionAttr_t *pstTempRegionAttr = &pstLdcModeChnAttr->stRegionAttr[0];

    pstTempRegionAttr->stOutRect.u16X                  = 0;
    pstTempRegionAttr->stOutRect.u16Y                  = 0;
    pstTempRegionAttr->stOutRect.u16Width              = 1920;
    pstTempRegionAttr->stOutRect.u16Height             = 1080;
    pstTempRegionAttr->eRegionMode                     = MI_LDC_REGION_180_PANORAMA;
    pstTempRegionAttr->stRegionPara.eCropMode          = MI_LDC_REGION_CROP_NONE;
    pstTempRegionAttr->stRegionPara.s32Pan             = 0;
    pstTempRegionAttr->stRegionPara.s32Tilt            = 30;
    pstTempRegionAttr->stRegionPara.s32ZoomH           = 500;
    pstTempRegionAttr->stRegionPara.s32ZoomV           = 500;
    pstTempRegionAttr->stRegionPara.s32InRadius        = 0;
    pstTempRegionAttr->stRegionPara.s32OutRadius       = 0;
    pstTempRegionAttr->stRegionPara.s32FocalRatio      = 0;
    pstTempRegionAttr->stRegionPara.s32DistortionRatio = 0;
    pstTempRegionAttr->stRegionPara.s32OutRotate       = 0;

    return 0;
}

MI_S32 ST_Common_GetLdcDefaultDisModeChnAttr(MI_LDC_ChnDISAttr_t *pstDisModeChnAttr)
{
    ST_CHECK_POINTER(pstDisModeChnAttr);

    memset(pstDisModeChnAttr, 0, sizeof(MI_LDC_ChnDISAttr_t));

    pstDisModeChnAttr->eMode        = MI_LDC_DIS_GME_8DOF;
    pstDisModeChnAttr->eSceneType   = MI_LDC_DIS_FIX_SCENE;
    pstDisModeChnAttr->eMotionLevel = MI_LDC_DIS_MOTION_LEVEL1;
    pstDisModeChnAttr->u8CropRatio  = 50;
    return 0;
}

MI_S32 ST_Common_GetLdcDefaultNirModeChnAttr(MI_LDC_ChnNIRAttr_t *pstNirModeChnAttr)
{
    ST_CHECK_POINTER(pstNirModeChnAttr);

    memset(pstNirModeChnAttr, 0, sizeof(MI_LDC_ChnNIRAttr_t));
    pstNirModeChnAttr->s32Distance = 5000;

    return 0;
}

MI_S32 ST_Common_GetLdcDefaultStitchModeChnAttr(MI_LDC_ChnStitchAttr_t *pstStitchModeChnAttr)
{
    ST_CHECK_POINTER(pstStitchModeChnAttr);

    memset(pstStitchModeChnAttr, 0, sizeof(MI_LDC_ChnStitchAttr_t));

    pstStitchModeChnAttr->eProjType   = MI_LDC_PROJECTION_SPHERICAL;
    pstStitchModeChnAttr->s32Distance = 5000;

    return 0;
}

MI_S32 ST_Common_GetLdcDefaultPmfModeChnAttr(MI_LDC_ChnPMFAttr_t *pstPMFModeChnAttr)
{
    ST_CHECK_POINTER(pstPMFModeChnAttr);

    memset(pstPMFModeChnAttr, 0, sizeof(MI_LDC_ChnPMFAttr_t));

    // Rotate 10 counterclockwise around the origin
    pstPMFModeChnAttr->as64PMFCoef[0] = 33044666;
    pstPMFModeChnAttr->as64PMFCoef[1] = -5826666;
    pstPMFModeChnAttr->as64PMFCoef[2] = 0;
    pstPMFModeChnAttr->as64PMFCoef[3] = 5826666;
    pstPMFModeChnAttr->as64PMFCoef[4] = 33044666;
    pstPMFModeChnAttr->as64PMFCoef[5] = 0;
    pstPMFModeChnAttr->as64PMFCoef[6] = 0;
    pstPMFModeChnAttr->as64PMFCoef[7] = 0;
    pstPMFModeChnAttr->as64PMFCoef[8] = 33554432;

    return 0;
}

MI_S32 ST_Common_GetLdcDefaultLdcInputPortAttr(MI_LDC_InputPortAttr_t *pstInputPortAttr)
{
    ST_CHECK_POINTER(pstInputPortAttr);

    memset(pstInputPortAttr, 0, sizeof(MI_LDC_InputPortAttr_t));

    pstInputPortAttr->u16Width  = 1920;
    pstInputPortAttr->u16Height = 1080;

    return 0;
}

MI_S32 ST_Common_GetLdcDefaultLdcOutputPortAttr(MI_LDC_OutputPortAttr_t *pstOutputPortAttr)
{
    ST_CHECK_POINTER(pstOutputPortAttr);

    memset(pstOutputPortAttr, 0, sizeof(MI_LDC_OutputPortAttr_t));

    pstOutputPortAttr->u16Width  = 1920;
    pstOutputPortAttr->u16Height = 1080;
    pstOutputPortAttr->ePixelFmt = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

    return 0;
}

MI_S32 ST_Common_LdcCreateDevice(MI_U32 devId, MI_LDC_DevAttr_t *pstDevAttr)
{
    MI_S32 ret = 0;

    ST_CHECK_POINTER(pstDevAttr);

    ret = MI_LDC_CreateDevice(devId, pstDevAttr);
    return ret;
}

MI_S32 ST_Common_LdcDestroyDevice(MI_U32 devId)
{
    MI_S32 ret = 0;
    ret        = MI_LDC_DestroyDevice(devId);
    return ret;
}

MI_S32 ST_Common_LdcStartLdcModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                    MI_LDC_ChnLDCAttr_t *pstLdcModeChnAttr, MI_LDC_InputPortAttr_t *pstInputPortAttr,
                                    MI_LDC_OutputPortAttr_t *pstOutputPortAttr)
{
    MI_S32 ret = 0;

    ST_CHECK_POINTER(pstLdcChnAttr);
    ST_CHECK_POINTER(pstLdcModeChnAttr);
    ST_CHECK_POINTER(pstInputPortAttr);
    ST_CHECK_POINTER(pstOutputPortAttr);

    ret = MI_LDC_CreateChannel(devId, chnId, pstLdcChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_CreateChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    ret = MI_LDC_SetOutputPortAttr(devId, chnId, pstOutputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetOutputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetInputPortAttr(devId, chnId, pstInputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetInputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetChnLDCAttr(devId, chnId, pstLdcModeChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetChnLDCAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_StartChannel(devId, chnId);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_StartChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    return ret;
}

MI_S32 ST_Common_LdcStartDisModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                    MI_LDC_ChnDISAttr_t *pstDisModeChnAttr, MI_LDC_InputPortAttr_t *pstInputPortAttr,
                                    MI_LDC_OutputPortAttr_t *pstOutputPortAttr)
{
    MI_S32 ret = 0;

    ST_CHECK_POINTER(pstLdcChnAttr);
    ST_CHECK_POINTER(pstDisModeChnAttr);
    ST_CHECK_POINTER(pstInputPortAttr);
    ST_CHECK_POINTER(pstOutputPortAttr);

    ret = MI_LDC_CreateChannel(devId, chnId, pstLdcChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_CreateChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    ret = MI_LDC_SetOutputPortAttr(devId, chnId, pstOutputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetOutputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    ret = MI_LDC_SetInputPortAttr(devId, chnId, pstInputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetInputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetChnDISAttr(devId, chnId, pstDisModeChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetChnDISAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    ret = MI_LDC_StartChannel(devId, chnId);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_StartChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    return ret;
}

MI_S32 ST_Common_LdcStartStitchModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                       MI_LDC_ChnStitchAttr_t * pstStitchModeChnAttr,
                                       MI_LDC_InputPortAttr_t * pstInputPortAttr,
                                       MI_LDC_OutputPortAttr_t *pstOutputPortAttr)
{
    MI_S32 ret = 0;

    ST_CHECK_POINTER(pstLdcChnAttr);
    ST_CHECK_POINTER(pstStitchModeChnAttr);
    ST_CHECK_POINTER(pstInputPortAttr);
    ST_CHECK_POINTER(pstOutputPortAttr);

    ret = MI_LDC_CreateChannel(devId, chnId, pstLdcChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_CreateChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetOutputPortAttr(devId, chnId, pstOutputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetOutputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetInputPortAttr(devId, chnId, pstInputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetInputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetChnStitchAttr(devId, chnId, pstStitchModeChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetChnStitchAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_StartChannel(devId, chnId);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_StartChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    return ret;
}

MI_S32 ST_Common_LdcStartDpuModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                    MI_LDC_ChnDPUAttr_t *pstDpuModeChnAttr, MI_LDC_InputPortAttr_t *pstInputPortAttr,
                                    MI_LDC_OutputPortAttr_t *pstOutputPortAttr)
{
    MI_S32 ret = 0;

    ST_CHECK_POINTER(pstLdcChnAttr);
    ST_CHECK_POINTER(pstDpuModeChnAttr);
    ST_CHECK_POINTER(pstInputPortAttr);
    ST_CHECK_POINTER(pstOutputPortAttr);

    ret = MI_LDC_CreateChannel(devId, chnId, pstLdcChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_CreateChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetOutputPortAttr(devId, chnId, pstOutputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetOutputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetInputPortAttr(devId, chnId, pstInputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetInputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetChnDPUAttr(devId, chnId, pstDpuModeChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetChnDPUAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_StartChannel(devId, chnId);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_StartChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    return ret;
}

MI_S32 ST_Common_LdcStartNirModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                    MI_LDC_ChnNIRAttr_t *pstNirModeChnAttr, MI_LDC_InputPortAttr_t *pstInputPortAttr,
                                    MI_LDC_OutputPortAttr_t *pstOutputPortAttr)
{
    MI_S32 ret = 0;

    ST_CHECK_POINTER(pstLdcChnAttr);
    ST_CHECK_POINTER(pstNirModeChnAttr);
    ST_CHECK_POINTER(pstInputPortAttr);
    ST_CHECK_POINTER(pstOutputPortAttr);

    ret = MI_LDC_CreateChannel(devId, chnId, pstLdcChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_CreateChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    ret = MI_LDC_SetOutputPortAttr(devId, chnId, pstOutputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetOutputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    ret = MI_LDC_SetInputPortAttr(devId, chnId, pstInputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetInputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetChnNIRAttr(devId, chnId, pstNirModeChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetChnNIRAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    ret = MI_LDC_StartChannel(devId, chnId);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_StartChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    return ret;
}

MI_S32 ST_Common_LdcStartPmfModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                    MI_LDC_ChnPMFAttr_t *pstPMFModeChnAttr, MI_LDC_InputPortAttr_t *pstInputPortAttr,
                                    MI_LDC_OutputPortAttr_t *pstOutputPortAttr)
{
    MI_S32 ret = 0;

    ST_CHECK_POINTER(pstLdcChnAttr);
    ST_CHECK_POINTER(pstPMFModeChnAttr);
    ST_CHECK_POINTER(pstInputPortAttr);
    ST_CHECK_POINTER(pstOutputPortAttr);

    ret = MI_LDC_CreateChannel(devId, chnId, pstLdcChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_CreateChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetOutputPortAttr(devId, chnId, pstOutputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetOutputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetInputPortAttr(devId, chnId, pstInputPortAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetInputPortAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_SetChnPMFAttr(devId, chnId, pstPMFModeChnAttr);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_SetChnPMFAttr failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_StartChannel(devId, chnId);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_StartChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    return ret;
}

MI_S32 ST_Common_LdcStopChn(MI_U32 devId, MI_U32 chnId)
{
    MI_S32 ret = 0;

    ret = MI_LDC_StopChannel(devId, chnId);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_StopChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }

    ret = MI_LDC_DestroyChannel(devId, chnId);
    if (ret != 0)
    {
        printf("[%s]:%d MI_LDC_DestroyChannel failed\n", __FUNCTION__, __LINE__);
        return ret;
    }
    return ret;
}
