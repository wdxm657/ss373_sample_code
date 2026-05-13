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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mi_sys.h"
#include "st_common.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_venc.h"
#include "st_common_rtsp_video.h"

#define MAIN_STREAM_WIDTH  2560
#define MAIN_STREAM_HEIGHT 1440
#define SUB_STREAM_WIDTH   1920
#define SUB_STREAM_HEIGHT  1080

char  IqBinPath[128];

int Snr_Res_Index = -1;

static MI_S32 ST_SetAssignResolution(void)
{
    MI_S32       s32Ret            = 0;
    MI_U32       u32GetSnrResCount = 0;
    MI_U8        u8ResIndex        = 0;
    MI_U32       u32SnrPadId       = 0;
    MI_SNR_Res_t stRes;

    MI_SNR_SetPlaneMode(u32SnrPadId,0);
    MI_SNR_QueryResCount(u32SnrPadId, &u32GetSnrResCount);
    for (u8ResIndex = 0; u8ResIndex < u32GetSnrResCount; u8ResIndex++)
    {
        MI_SNR_GetRes(u32SnrPadId, u8ResIndex, &stRes);
        if((stRes.stCropRect.u16Width == MAIN_STREAM_WIDTH) &&
            (stRes.stCropRect.u16Height == MAIN_STREAM_HEIGHT) &&
            (stRes.u32MaxFps == 30))
        {
            s32Ret = u8ResIndex;
        }
    }
    return s32Ret;
}

static MI_S32 STUB_BaseModuleInit(void)
{
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;
    MI_U32             u32SrcFrmrate;
    MI_U32             u32DstFrmrate;
    MI_SYS_BindType_e  eBindType;
    MI_U32             u32BindParam;
    MI_U32             u32SnrPadId   = 0;
    MI_U8              u8SensorRes   = 0;
    MI_U32             u32VifGroupId = 0;
    MI_U32             u32VifDevId   = 0;
    MI_U32             u32VifPortId  = 0;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 0;

    MI_U32 u32SclDevId  = 0;
    MI_U32 u32SclChnId  = 0;
    MI_U32 u32SclPortId = 0;

    MI_U32 u32VencDevId     = 0;
    MI_U32 u32MainVencChnId = 0;
    MI_U32 u32SubVencChnId  = 1;

    MI_SYS_GlobalPrivPoolConfig_t stConfig;

    /************************************************
    step1 :init SYS
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_VIF_OutputPortAttr_t stVifPortAttr;

    MI_VIF_GroupAttr_t stVifGroupAttr = {0};
    MI_VIF_DevAttr_t   stVifDevAttr   = {0};
    /************************************************
    step2 :init sensor/vif
    *************************************************/
    u8SensorRes = ST_SetAssignResolution();
    if(Snr_Res_Index != -1)
    {
        u8SensorRes = Snr_Res_Index;
    }
    STCHECKRESULT(ST_Common_SensorInit(u32SnrPadId, FALSE, u8SensorRes, 0xFF));

    ST_Common_GetVifDefaultGrouptAttr(&stVifGroupAttr);
    STCHECKRESULT(ST_Common_VifCreateDevGroup(u32VifGroupId, &stVifGroupAttr));

    ST_Common_GetVifDefaultDevAttr(&stVifDevAttr);
    STCHECKRESULT(ST_Common_VifEnableDev(u32VifDevId, &stVifDevAttr));

    ST_Common_GetVifDefaultPortAttr(&stVifPortAttr);

    /************************************************
    step3 :init isp
    *************************************************/

    MI_ISP_DevAttr_t      stIspDevAttr;
    MI_ISP_ChannelAttr_t  stIspChnAttr;
    MI_ISP_ChnParam_t     stIspChnParam;
    MI_SYS_WindowRect_t   stIspInputCrop;
    MI_ISP_OutPortParam_t stIspOutPortParam;

    ST_Common_GetIspDefaultDevAttr(&stIspDevAttr);
    STCHECKRESULT(ST_Common_IspCreateDevice(u32IspDevId, &stIspDevAttr));

    MI_U32 u32SensorBindId = 0;
    ST_Common_GetIspDefaultChnAttr(&stIspChnAttr, &stIspInputCrop, &stIspChnParam);
    ST_Common_GetIspBindSensorIdByPad(0, &u32SensorBindId);
    stIspChnAttr.u32SensorBindId = u32SensorBindId;
    STCHECKRESULT(
        ST_Common_IspStartChn(u32IspDevId, u32IspChnId, &stIspChnAttr, &stIspInputCrop, &stIspChnParam));

    ST_Common_GetIspDefaultPortAttr(&stIspOutPortParam);
    STCHECKRESULT(ST_Common_IspEnablePort(u32IspDevId, u32IspChnId, u32IspPortId, &stIspOutPortParam));

    /************************************************
    step4 :bind vif->isp
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
    stSrcChnPort.u32DevId  = u32VifDevId;
    stSrcChnPort.u32ChnId  = 0;
    stSrcChnPort.u32PortId = u32VifPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stDstChnPort.u32DevId  = u32IspDevId;
    stDstChnPort.u32ChnId  = u32IspChnId;
    stDstChnPort.u32PortId = u32IspPortId;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_REALTIME;
    u32BindParam           = 0;
    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                        u32BindParam));

    stDstChnPort.u32PortId = u32IspPortId + 1;
    STCHECKRESULT(ST_Common_IspEnablePort(u32IspDevId, u32IspChnId, stDstChnPort.u32PortId, &stIspOutPortParam));
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 1, 4));

    /************************************************
    step5 :init scl
    *************************************************/
    MI_SCL_DevAttr_t      stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t     stSclChnParam;
    MI_SYS_WindowRect_t   stSclInputCrop;
    MI_SCL_OutPortParam_t stSclOutPortParam;

    ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
    STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));

    ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
    STCHECKRESULT(
        ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));
    ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);

    stSclOutPortParam.stSCLOutputSize.u16Width  = MAIN_STREAM_WIDTH;
    stSclOutPortParam.stSCLOutputSize.u16Height = MAIN_STREAM_HEIGHT;

    /************************************************
    step6 :config scl ring buf
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                                     = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
    stConfig.bCreate                                         = TRUE;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.eModule      = E_MI_MODULE_ID_SCL;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.u32Devid     = u32SclDevId;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth  = stSclOutPortParam.stSCLOutputSize.u16Width;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = stSclOutPortParam.stSCLOutputSize.u16Height;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.u16RingLine  = stSclOutPortParam.stSCLOutputSize.u16Height + 2;
    strcpy((char*)stConfig.uConfig.stpreDevPrivRingPoolConfig.u8MMAHeapName, "mma_heap_name0");
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step7 :bind isp->scl
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = u32IspDevId;
    stSrcChnPort.u32ChnId  = u32IspChnId;
    stSrcChnPort.u32PortId = u32IspPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId  = u32SclDevId;
    stDstChnPort.u32ChnId  = u32SclChnId;
    stDstChnPort.u32PortId = u32SclPortId;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_REALTIME;
    u32BindParam           = 0;
    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                        u32BindParam));
    // In order to work out the image as soon as the entire pipleline is created, otherwise the first image of vif
    // will be wasted and the next image will have to wait a timeout of 100ms when vif is enabled without bind
    // the next module
    STCHECKRESULT(ST_Common_VifEnablePort(u32VifDevId, u32VifPortId, &stVifPortAttr));

    /************************************************
    step8 :init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;

    ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));

    MI_VENC_ModType_e eType = E_MI_VENC_MODTYPE_H265E;
    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr);
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = MAIN_STREAM_WIDTH;
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = MAIN_STREAM_HEIGHT;
    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32MainVencChnId, &stVencChnAttr));
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = SUB_STREAM_WIDTH;
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = SUB_STREAM_HEIGHT;
    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32SubVencChnId, &stVencChnAttr));

    /************************************************
    step9 :bind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32MainVencChnId;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_HW_RING;
    u32BindParam           = 0;
    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                        u32BindParam));
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId, &stSclOutPortParam));
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 0, 4));


    /************************************************
    step10 :config venc ring buf
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                                     = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
    stConfig.bCreate                                         = TRUE;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.eModule      = E_MI_MODULE_ID_VENC;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.u32Devid     = u32VencDevId;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth  = stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.u16RingLine  = stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight;
    strcpy((char*)stConfig.uConfig.stpreDevPrivRingPoolConfig.u8MMAHeapName, "mma_heap_name0");
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step11 :bind venc->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stSrcChnPort.u32DevId  = u32VencDevId;
    stSrcChnPort.u32ChnId  = u32MainVencChnId;
    stSrcChnPort.u32PortId = 0;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32SubVencChnId;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_HW_RING;
    u32BindParam           = 0;
    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                        u32BindParam));

    return MI_SUCCESS;
}

static MI_S32 STUB_BaseModuleDeInit()
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32           u32SnrPadId   = 0;
    MI_U32           u32VifGroupId = 0;
    MI_U32           u32VifDevId   = 0;
    MI_U32           u32VifPortId  = 0;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 0;

    MI_U32 u32SclDevId  = 0;
    MI_U32 u32SclChnId  = 0;
    MI_U32 u32SclPortId = 0;

    MI_U32 u32VencDevId     = 0;
    MI_U32 u32MainVencChnId = 0;
    MI_U32 u32SubVencChnId  = 1;

    MI_SYS_GlobalPrivPoolConfig_t stConfig;

    /************************************************
    step1 :unbind venc->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stSrcChnPort.u32DevId  = u32VencDevId;
    stSrcChnPort.u32ChnId  = u32MainVencChnId;
    stSrcChnPort.u32PortId = 0;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32SubVencChnId;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step2 :destroy venc ring buf
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                                 = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
    stConfig.bCreate                                     = FALSE;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.eModule  = E_MI_MODULE_ID_VENC;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.u32Devid = u32VencDevId;
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step3 :unbind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32MainVencChnId;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step4 :deinit venc
    *************************************************/
    STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32MainVencChnId));
    STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32SubVencChnId));
    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    step5 :unbind isp->scl
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = u32IspDevId;
    stSrcChnPort.u32ChnId  = u32IspChnId;
    stSrcChnPort.u32PortId = u32IspPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId  = u32SclDevId;
    stDstChnPort.u32ChnId  = u32SclChnId;
    stDstChnPort.u32PortId = u32SclPortId;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step6 :destroy scl ring buf
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                                 = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
    stConfig.bCreate                                     = FALSE;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.eModule  = E_MI_MODULE_ID_SCL;
    stConfig.uConfig.stpreDevPrivRingPoolConfig.u32Devid = u32SclDevId;
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step7 :deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId));
    STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));
    STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId, u32IspPortId + 1));
    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    step8 :unbind vif->isp
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
    stSrcChnPort.u32DevId  = u32VifDevId;
    stSrcChnPort.u32ChnId  = 0;
    stSrcChnPort.u32PortId = u32VifPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stDstChnPort.u32DevId  = u32IspDevId;
    stDstChnPort.u32ChnId  = u32IspChnId;
    stDstChnPort.u32PortId = u32IspPortId;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step9 :deinit isp
    *************************************************/
    STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId, u32IspPortId));
    STCHECKRESULT(ST_Common_IspStopChn(u32IspDevId, u32IspChnId));
    STCHECKRESULT(ST_Common_IspDestroyDevice(u32IspDevId));

    /************************************************
    step10 :deinit vif/sensor
    *************************************************/
    STCHECKRESULT(ST_Common_VifDisablePort(u32VifDevId, u32VifPortId));
    STCHECKRESULT(ST_Common_VifDisableDev(u32VifDevId));
    STCHECKRESULT(ST_Common_VifDestroyDevGroup(u32VifGroupId));
    STCHECKRESULT(ST_Common_SensorDeInit(u32SnrPadId));

    /************************************************
    step11 :sys exit
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Exit());

    return MI_SUCCESS;
}

static MI_S32 ST_Start_Preview()
{
    MI_U32               u32VencDevId      = 0;
    MI_U32               u32MainVencChnId  = 0;
    MI_U32               u32SubVencChnId   = 1;
    MI_U32               u32IspDevId       = 0;
    MI_U32               u32IspChnId       = 0;
    MI_U32               u32SensorPad      = 0;
    char                 IqApiBinFilePath[128];
    MI_SNR_PlaneInfo_t   stPlaneInfo;
    ST_VideoStreamInfo_t stStreamInfo;

    STCHECKRESULT(STUB_BaseModuleInit());

    if (strlen(IqBinPath) == 0)
    {
        MI_SNR_GetPlaneInfo(u32SensorPad, 0, &stPlaneInfo);
        sprintf(IqApiBinFilePath, "/config/iqfile/%s_api.bin", stPlaneInfo.s8SensorName);
    }
    else
    {
        strcpy(IqApiBinFilePath, IqBinPath);
    }
    ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId, IqApiBinFilePath);

    memset(&stStreamInfo, 0x00, sizeof(ST_VideoStreamInfo_t));
    stStreamInfo.eType        = E_MI_VENC_MODTYPE_H265E;
    stStreamInfo.VencDev      = u32VencDevId;
    stStreamInfo.VencChn      = u32MainVencChnId;
    stStreamInfo.u32Width     = MAIN_STREAM_WIDTH;
    stStreamInfo.u32Height    = MAIN_STREAM_HEIGHT;
    stStreamInfo.u32FrameRate = 30;
    stStreamInfo.rtspIndex    = 0;

    // start rtsp
    ST_Common_RtspServerStartVideo(&stStreamInfo);

    stStreamInfo.VencChn      = u32SubVencChnId;
    stStreamInfo.u32Width     = SUB_STREAM_WIDTH;
    stStreamInfo.u32Height    = SUB_STREAM_HEIGHT;
    stStreamInfo.rtspIndex    = 1;
    ST_Common_RtspServerStartVideo(&stStreamInfo);

    ST_Common_Pause();

    // stop rtsp
    ST_Common_RtspServerStopVideo(&stStreamInfo);

    stStreamInfo.VencChn      = u32MainVencChnId;
    stStreamInfo.u32Width     = MAIN_STREAM_WIDTH;
    stStreamInfo.u32Height    = MAIN_STREAM_HEIGHT;
    stStreamInfo.rtspIndex    = 0;
    ST_Common_RtspServerStopVideo(&stStreamInfo);

    STCHECKRESULT(STUB_BaseModuleDeInit());

    return MI_SUCCESS;
}

MI_S32 ST_GetCmdlineParam(int argc, char **argv)
{
    memset(IqBinPath, 0, 128);

    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "iqbin"))
        {
            strcpy(IqBinPath, argv[i + 1]);
        }

        if (0 == strcmp(argv[i], "index"))
        {
            Snr_Res_Index = atoi(argv[i + 1]);
        }
    }

    return MI_SUCCESS;
}

MI_S32 main(int argc, char **argv)
{
    ST_GetCmdlineParam(argc, argv);
    STCHECKRESULT(ST_Start_Preview());

    return 0;
}
