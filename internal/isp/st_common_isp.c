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

#include "st_common_isp.h"
#include "st_common.h"
#include "platform.h"

MI_S32 ST_Common_GetIspBindSensorIdByPad(MI_U32 u32SnrPadId, MI_ISP_BindSnrId_e *peSensorBindId)
{
    MI_S32 s32Ret = 0;

    ST_CHECK_POINTER(peSensorBindId);

    if (u32SnrPadId > MAX_SENSOR_NUM - 1)
    {
        ST_ERR("sensor pad %d not support!!!\n", u32SnrPadId);
        return -1;
    }

    switch (u32SnrPadId)
    {
        case 0:
            *peSensorBindId = E_MI_ISP_SENSOR0;
            break;
        case 1:
            *peSensorBindId = E_MI_ISP_SENSOR1;
            break;
        case 2:
            *peSensorBindId = E_MI_ISP_SENSOR2;
            break;
        case 3:
            *peSensorBindId = E_MI_ISP_SENSOR3;
            break;
        case 4:
            *peSensorBindId = E_MI_ISP_SENSOR4;
            break;
        case 5:
            *peSensorBindId = E_MI_ISP_SENSOR5;
            break;
        case 6:
            *peSensorBindId = E_MI_ISP_SENSOR6;
            break;
        case 7:
            *peSensorBindId = E_MI_ISP_SENSOR7;
            break;
        default:
            ST_ERR("pad not support!!!\n");
            return -1;
    }

    return s32Ret;
}

MI_S32 ST_Common_GetIspDefaultDevAttr(MI_ISP_DevAttr_t *pstDevAttr)
{
    ST_CHECK_POINTER(pstDevAttr);

    memset(pstDevAttr, 0x00, sizeof(MI_ISP_DevAttr_t));

    pstDevAttr->u32DevStitchMask = 0;

    return 0;
}

MI_S32 ST_Common_GetIspDefaultChnAttr(MI_ISP_ChannelAttr_t *pstChnAttr, MI_SYS_WindowRect_t *pstInputCrop,
                                      MI_ISP_ChnParam_t *pstChnParam)
{
    ST_CHECK_POINTER(pstChnAttr);
    ST_CHECK_POINTER(pstChnParam);
    ST_CHECK_POINTER(pstChnAttr);

    memset(pstChnAttr, 0x00, sizeof(MI_ISP_ChannelAttr_t));
    memset(pstInputCrop, 0x00, sizeof(MI_SYS_WindowRect_t));
    memset(pstChnParam, 0x00, sizeof(MI_ISP_ChnParam_t));

    /*if vif->isp bind,set sensorid,
      if isp inputfile set E_MI_ISP_SENSOR_INVALID*/
    pstChnAttr->u32SensorBindId = E_MI_ISP_SENSOR0;
    pstChnAttr->u32Sync3AType   = 0;
    memset(&pstChnAttr->stIspCustIqParam, 0x00, sizeof(MI_ISP_CustIQParam_t));

    /*default not use isp crop */
    pstInputCrop->u16X      = 0;
    pstInputCrop->u16Y      = 0;
    pstInputCrop->u16Width  = 0;
    pstInputCrop->u16Height = 0;

    pstChnParam->bFlip      = FALSE;
    pstChnParam->bMirror    = FALSE;
    pstChnParam->bY2bEnable = FALSE;
    pstChnParam->eRot       = E_MI_SYS_ROTATE_NONE;
    pstChnParam->e3DNRLevel = E_MI_ISP_3DNR_LEVEL_OFF;
    /*default hdr off */
    pstChnParam->eHDRType           = E_MI_ISP_HDR_TYPE_OFF;
    pstChnParam->eHDRFusionType     = E_MI_ISP_HDR_FUSION_TYPE_NONE;
    pstChnParam->u16HDRExposureMask = E_MI_ISP_HDR_EXPOSURE_TYPE_NONE;

    return 0;
}

MI_S32 ST_Common_GetIspDefaultPortAttr(MI_ISP_OutPortParam_t *pstOutPortParam)
{
    ST_CHECK_POINTER(pstOutPortParam);

    memset(pstOutPortParam, 0x00, sizeof(MI_ISP_OutPortParam_t));

    /*default not use isp portcrop */
    pstOutPortParam->stCropRect.u16X      = 0;
    pstOutPortParam->stCropRect.u16Y      = 0;
    pstOutPortParam->stCropRect.u16Width  = 0;
    pstOutPortParam->stCropRect.u16Height = 0;

    pstOutPortParam->ePixelFormat  = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    pstOutPortParam->eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
    pstOutPortParam->eBufLayout    = E_MI_ISP_BUFFER_LAYOUT_ONE_FRAME;

    return 0;
}

MI_S32 ST_Common_IspCreateDevice(MI_U32 IspDevId, MI_ISP_DevAttr_t *pstIspDevAttr)
{
    MI_S32           s32Ret = MI_SUCCESS;
    MI_ISP_DevAttr_t stIspDevAttr;

    ST_CHECK_POINTER(pstIspDevAttr);

    memset(&stIspDevAttr, 0x0, sizeof(MI_ISP_DevAttr_t));

    stIspDevAttr.u32DevStitchMask = pstIspDevAttr->u32DevStitchMask;
    s32Ret |= MI_ISP_CreateDevice(IspDevId, &stIspDevAttr);

    return s32Ret;
}

MI_S32 ST_Common_IspDestroyDevice(MI_U32 IspDevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_ISP_DestoryDevice(IspDevId);

    return s32Ret;
}

MI_S32 ST_Common_IspStartChn(MI_U32 IspDevId, MI_U32 IspChnId, MI_ISP_ChannelAttr_t *pstIspChnAttr,
                             MI_SYS_WindowRect_t *pstIspInputCrop, MI_ISP_ChnParam_t *pstChnParam)
{
    MI_S32 s32Ret = MI_SUCCESS;

    ST_CHECK_POINTER(pstIspChnAttr);
    ST_CHECK_POINTER(pstIspInputCrop);
    ST_CHECK_POINTER(pstChnParam);

    s32Ret |= MI_ISP_CreateChannel(IspDevId, IspChnId, pstIspChnAttr);

    if (pstIspInputCrop->u16Width != 0 && pstIspInputCrop->u16Height != 0)
    {
        s32Ret |= MI_ISP_SetInputPortCrop(IspDevId, IspChnId, pstIspInputCrop);
    }

    s32Ret |= MI_ISP_SetChnParam(IspDevId, IspChnId, pstChnParam);

    s32Ret |= MI_ISP_StartChannel(IspDevId, IspChnId);

    return s32Ret;
}

MI_S32 ST_Common_IspStartChnEx(MI_U32 IspDevId, MI_U32 IspChnId, MI_ISP_ChannelAttr_t *pstIspChnAttr,
                               MI_SYS_WindowRect_t *pstIspInputCrop, MI_ISP_ChnParam_t *pstChnParam,
                               MI_ISP_ChnParam_t stSubChnParam[E_MI_ISP_SENSOR_MAX])
{
    MI_S32 s32Ret          = MI_SUCCESS;
    MI_U32 u32SensorBindId = 0;
    int    i;

    ST_CHECK_POINTER(pstIspChnAttr);
    ST_CHECK_POINTER(pstIspInputCrop);
    ST_CHECK_POINTER(pstChnParam);

    u32SensorBindId = pstIspChnAttr->u32SensorBindId;

    s32Ret |= MI_ISP_CreateChannel(IspDevId, IspChnId, pstIspChnAttr);

    if (pstIspInputCrop->u16Width != 0 && pstIspInputCrop->u16Height != 0)
    {
        s32Ret |= MI_ISP_SetInputPortCrop(IspDevId, IspChnId, pstIspInputCrop);
    }

    s32Ret |= MI_ISP_SetChnParam(IspDevId, IspChnId, pstChnParam);

    for (i = 0; i < E_MI_ISP_SENSOR_MAX; i++)
    {
        if (u32SensorBindId & (1 << i))
        {
            MI_U32             IspSubChnId;
            MI_ISP_BindSnrId_e eSensorBindId = (MI_ISP_BindSnrId_e)1 << i;
            MI_ISP_GetSubChnId(IspDevId, IspChnId, eSensorBindId, &IspSubChnId);
            s32Ret |= MI_ISP_SetSubChnParam(IspDevId, IspChnId, IspSubChnId, &stSubChnParam[i]);
        }
    }

    s32Ret |= MI_ISP_StartChannel(IspDevId, IspChnId);

    return s32Ret;
}

MI_S32 ST_Common_IspStopChn(MI_U32 IspDevId, MI_U32 IspChnId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_ISP_StopChannel(IspDevId, IspChnId);
    s32Ret |= MI_ISP_DestroyChannel(IspDevId, IspChnId);

    return s32Ret;
}

MI_S32 ST_Common_IspEnablePort(MI_U32 IspDevId, MI_U32 IspChnId, MI_U32 IspOutPortId,
                               MI_ISP_OutPortParam_t *pstIspOutPortParam)
{
    MI_S32                s32Ret = MI_SUCCESS;
    MI_ISP_OutPortParam_t stOutPortParam;

    ST_CHECK_POINTER(pstIspOutPortParam);

    memset(&stOutPortParam, 0x0, sizeof(MI_ISP_OutPortParam_t));

    stOutPortParam.stCropRect.u16X      = pstIspOutPortParam->stCropRect.u16X;
    stOutPortParam.stCropRect.u16Y      = pstIspOutPortParam->stCropRect.u16Y;
    stOutPortParam.stCropRect.u16Width  = pstIspOutPortParam->stCropRect.u16Width;
    stOutPortParam.stCropRect.u16Height = pstIspOutPortParam->stCropRect.u16Height;
    stOutPortParam.ePixelFormat         = pstIspOutPortParam->ePixelFormat;
    stOutPortParam.eCompressMode        = pstIspOutPortParam->eCompressMode;
    stOutPortParam.eBufLayout           = pstIspOutPortParam->eBufLayout;

    s32Ret |= MI_ISP_SetOutputPortParam(IspDevId, IspChnId, IspOutPortId, &stOutPortParam);

    s32Ret |= MI_ISP_EnableOutputPort(IspDevId, IspChnId, IspOutPortId);

    return s32Ret;
}

MI_S32 ST_Common_IspDisablePort(MI_U32 IspDevId, MI_U32 IspChnId, MI_U32 IspOutPortId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_ISP_DisableOutputPort(IspDevId, IspChnId, IspOutPortId);

    return s32Ret;
}

MI_S32 ST_Common_IspSetIqBin(MI_U32 IspDev, MI_U32 IspChn, char *pConfigPath)
{
    MI_S32              s32Ret = MI_SUCCESS;
    CUS3A_ALGO_STATUS_t stCus3aStatus;
    MI_ISP_IQ_ParamInitInfoType_t stIqstatus;
    memset(&stCus3aStatus, 0, sizeof(CUS3A_ALGO_STATUS_t));
    memset(&stIqstatus, 0, sizeof(MI_ISP_IQ_ParamInitInfoType_t));

    MI_U8 u8ispreadycnt = 0;
    if (strlen(pConfigPath) == 0)
    {
        printf("IQ Bin File path NULL!\n");
        return -1;
    }

#ifdef LINUX_FLOW_ON_DUAL_OS
            ST_DBG("Ready to load IQ bin :%s u8ispreadycnt:%d\n", pConfigPath, u8ispreadycnt);
            s32Ret |= MI_ISP_ApiCmdLoadBinFile(IspDev, IspChn, (char *)pConfigPath, 1234);
#else
    while (1)
    {
        if (u8ispreadycnt > 100)
        {
            ST_ERR("ISP ready time out!\n");
            u8ispreadycnt = 0;
            break;
        }

        CUS3A_GetAlgoStatus((CUS3A_ISP_DEV_e)IspDev, (CUS3A_ISP_CH_e)IspChn, &stCus3aStatus);
        MI_ISP_IQ_GetParaInitStatus(IspDev, IspChn, &stIqstatus);

        if ((stCus3aStatus.Ae == E_ALGO_STATUS_RUNNING) && (stCus3aStatus.Awb == E_ALGO_STATUS_RUNNING)
            && (stIqstatus.stParaAPI.bFlag == E_SS_IQ_TRUE))
        {
            ST_DBG("Ready to load IQ bin :%s u8ispreadycnt:%d\n", pConfigPath, u8ispreadycnt);
            s32Ret |= MI_ISP_ApiCmdLoadBinFile(IspDev, IspChn, (char *)pConfigPath, 1234);

            usleep(10 * 1000);

            u8ispreadycnt = 0;
            break;
        }
        else
        {
            usleep(10 * 1000);
            u8ispreadycnt++;
        }
    }
    UNUSED(u8ispreadycnt);
#endif
    return s32Ret;
}

MI_S32 ST_Common_IspSetCaliData(MI_U32 IspDev, MI_U32 IspChn, MI_ISP_IQ_CaliItem_e eCaliItem, char *pConfigPath)
{
    ST_CHECK_POINTER(pConfigPath);

    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_ISP_ApiCmdLoadCaliData(IspDev, IspChn, eCaliItem, (char *)pConfigPath);

    return s32Ret;
}
