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

#define SNR_WIDTH  2560
#define SNR_HEIGHT 1440

char  IqBinPath[128];

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
        if((stRes.stCropRect.u16Width == SNR_WIDTH) &&
           (stRes.stCropRect.u16Height == SNR_HEIGHT) &&
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

    MI_U32 u32VifGroupId = 0;
    MI_U32 u32VifDevId   = 0;
    MI_U32 u32VifPortId  = 0;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 1;

    MI_U32 u32SclDevId  = 1;
    MI_U32 u32SclChnId  = 0;
    MI_U32 u32SclPortId = 0;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

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
    STCHECKRESULT(ST_Common_SensorInit(u32SnrPadId, FALSE, u8SensorRes, 0xFF));

    ST_Common_GetVifDefaultGrouptAttr(&stVifGroupAttr);
    STCHECKRESULT(ST_Common_VifCreateDevGroup(u32VifGroupId, &stVifGroupAttr));

    ST_Common_GetVifDefaultDevAttr(&stVifDevAttr);
    STCHECKRESULT(ST_Common_VifEnableDev(u32VifDevId, &stVifDevAttr));

    ST_Common_GetVifDefaultPortAttr(&stVifPortAttr);

    /************************************************
    step3 :config vif private pool
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                                       = E_MI_SYS_PER_DEV_PRIVATE_POOL;
    stConfig.bCreate                                           = TRUE;
    stConfig.uConfig.stPreDevPrivPoolConfig.eModule            = E_MI_MODULE_ID_VIF;
    stConfig.uConfig.stPreDevPrivPoolConfig.u32Devid           = u32VifDevId;
    stConfig.uConfig.stPreDevPrivPoolConfig.u32PrivateHeapSize = SNR_WIDTH * SNR_HEIGHT * 1.5;
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step4 :config isp private pool
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                                       = E_MI_SYS_PER_CHN_PRIVATE_POOL;
    stConfig.bCreate                                           = TRUE;
    stConfig.uConfig.stPreChnPrivPoolConfig.eModule            = E_MI_MODULE_ID_ISP;
    stConfig.uConfig.stPreChnPrivPoolConfig.u32Devid           = u32IspDevId;
    stConfig.uConfig.stPreChnPrivPoolConfig.u32Channel         = u32IspChnId;
    stConfig.uConfig.stPreChnPrivPoolConfig.u32PrivateHeapSize = SNR_WIDTH * SNR_HEIGHT * 1.5;
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step5 :init isp
    *************************************************/

    MI_ISP_DevAttr_t      stIspDevAttr;
    MI_ISP_ChannelAttr_t  stIspChnAttr;
    MI_ISP_ChnParam_t     stIspChnParam;
    MI_SYS_WindowRect_t   stIspInputCrop;
    MI_ISP_OutPortParam_t stIspOutPortParam;

    ST_Common_GetIspDefaultDevAttr(&stIspDevAttr);
    STCHECKRESULT(ST_Common_IspCreateDevice(u32IspDevId, &stIspDevAttr));

    ST_Common_GetIspDefaultChnAttr(&stIspChnAttr, &stIspInputCrop, &stIspChnParam);
    stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR_INVALID;
    STCHECKRESULT(ST_Common_IspStartChn(u32IspDevId, u32IspChnId, &stIspChnAttr, &stIspInputCrop, &stIspChnParam));

    ST_Common_GetIspDefaultPortAttr(&stIspOutPortParam);
    STCHECKRESULT(ST_Common_IspEnablePort(u32IspDevId, u32IspChnId, u32IspPortId, &stIspOutPortParam));

    /************************************************
    step6 :bind vif->isp
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
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    step7 :init scl
    *************************************************/
    MI_SCL_DevAttr_t      stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t     stSclChnParam;
    MI_SYS_WindowRect_t   stSclInputCrop;
    MI_SCL_OutPortParam_t stSclOutPortParam;

    ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
    stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL2;
    STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));

    ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
    STCHECKRESULT(
        ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

    /************************************************
    step8 :config scl private pool
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                                       = E_MI_SYS_PER_DEV_PRIVATE_POOL;
    stConfig.bCreate                                           = TRUE;
    stConfig.uConfig.stPreDevPrivPoolConfig.eModule            = E_MI_MODULE_ID_SCL;
    stConfig.uConfig.stPreDevPrivPoolConfig.u32Devid           = u32SclDevId;
    stConfig.uConfig.stPreDevPrivPoolConfig.u32PrivateHeapSize = stSclOutPortParam.stSCLOutputSize.u16Width *
                                                                 stSclOutPortParam.stSCLOutputSize.u16Height * 1.5;
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step9 :bind isp->scl
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
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    step10 :init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;

    ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));

    MI_VENC_ModType_e eType = E_MI_VENC_MODTYPE_H265E;
    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr);
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = SNR_WIDTH;
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = SNR_HEIGHT;
    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr));

    /************************************************
    step11 :config venc private pool
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                                       = E_MI_SYS_PER_DEV_PRIVATE_POOL;
    stConfig.bCreate                                           = TRUE;
    stConfig.uConfig.stPreDevPrivPoolConfig.eModule            = E_MI_MODULE_ID_VENC;
    stConfig.uConfig.stPreDevPrivPoolConfig.u32Devid           = u32VencDevId;
    stConfig.uConfig.stPreDevPrivPoolConfig.u32PrivateHeapSize = stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth *
                                                                 stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight * 1.5;
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step12 :bind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32VencChnId;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));
    ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);

    stSclOutPortParam.stSCLOutputSize.u16Width  = SNR_WIDTH;
    stSclOutPortParam.stSCLOutputSize.u16Height = SNR_HEIGHT;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId, &stSclOutPortParam));
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 0, 4));

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
    MI_U32 u32IspPortId = 1;

    MI_U32 u32SclDevId  = 1;
    MI_U32 u32SclChnId  = 0;
    MI_U32 u32SclPortId = 0;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    MI_SYS_GlobalPrivPoolConfig_t stConfig;

    /************************************************
    step1 :destroy venc private pool
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                             = E_MI_SYS_PER_DEV_PRIVATE_POOL;
    stConfig.bCreate                                 = FALSE;
    stConfig.uConfig.stPreDevPrivPoolConfig.eModule  = E_MI_MODULE_ID_VENC;
    stConfig.uConfig.stPreDevPrivPoolConfig.u32Devid = u32VencDevId;
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step2 :unbind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32VencChnId;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step3 :deinit venc
    *************************************************/
    STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId));
    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    step4 :unbind isp->scl
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
    step5 :destroy scl private pool
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                             = E_MI_SYS_PER_DEV_PRIVATE_POOL;
    stConfig.bCreate                                 = FALSE;
    stConfig.uConfig.stPreDevPrivPoolConfig.eModule  = E_MI_MODULE_ID_SCL;
    stConfig.uConfig.stPreDevPrivPoolConfig.u32Devid = u32SclDevId;
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step6 :deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId));
    STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));
    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    step7 :unbind vif->isp
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
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step8 :deinit isp
    *************************************************/
    STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId, u32IspPortId));
    STCHECKRESULT(ST_Common_IspStopChn(u32IspDevId, u32IspChnId));
    STCHECKRESULT(ST_Common_IspDestroyDevice(u32IspDevId));

    /************************************************
    step9 :destroy isp private pool
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                               = E_MI_SYS_PER_CHN_PRIVATE_POOL;
    stConfig.bCreate                                   = FALSE;
    stConfig.uConfig.stPreChnPrivPoolConfig.eModule    = E_MI_MODULE_ID_ISP;
    stConfig.uConfig.stPreChnPrivPoolConfig.u32Devid   = u32IspDevId;
    stConfig.uConfig.stPreChnPrivPoolConfig.u32Channel = u32IspChnId;
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step10 :destroy vif private pool
    *************************************************/
    memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

    stConfig.eConfigType                             = E_MI_SYS_PER_DEV_PRIVATE_POOL;
    stConfig.bCreate                                 = FALSE;
    stConfig.uConfig.stPreDevPrivPoolConfig.eModule  = E_MI_MODULE_ID_VIF;
    stConfig.uConfig.stPreDevPrivPoolConfig.u32Devid = u32VifDevId;
    STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));

    /************************************************
    step11 :deinit vif/sensor
    *************************************************/
    STCHECKRESULT(ST_Common_VifDisablePort(u32VifDevId, u32VifPortId));
    STCHECKRESULT(ST_Common_VifDisableDev(u32VifDevId));
    STCHECKRESULT(ST_Common_VifDestroyDevGroup(u32VifGroupId));
    STCHECKRESULT(ST_Common_SensorDeInit(u32SnrPadId));

    /************************************************
    step12 :sys exit
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Exit());

    return MI_SUCCESS;
}

static MI_S32 ST_Start_Preview()
{
    MI_U32               u32VencDevId      = 0;
    MI_U32               u32MainVencChnId  = 0;
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
    stStreamInfo.u32Width     = SNR_WIDTH;
    stStreamInfo.u32Height    = SNR_HEIGHT;
    stStreamInfo.u32FrameRate = 30;
    stStreamInfo.rtspIndex    = 0;

    // start rtsp
    ST_Common_RtspServerStartVideo(&stStreamInfo);

    ST_Common_Pause();

    // stop rtsp
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
    }

    return MI_SUCCESS;
}

MI_S32 main(int argc, char **argv)
{
    ST_GetCmdlineParam(argc, argv);
    STCHECKRESULT(ST_Start_Preview());

    return 0;
}
