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

typedef struct ST_VifInputParam_s
{
    MI_U8 u8SensorIndex;
    MI_U8 u8CmdIndex;
    char  IqBinPath[128];
} ST_VifInputParam_t;

typedef struct ST_SensorSize_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
} ST_SensorSize_t;

static ST_VifInputParam_t gstVifInputParm;
static ST_SensorSize_t    gstSensorSize[2];

#ifdef _FASTBOOT_
static MI_BOOL g_bExit = TRUE;
#endif

void ST_Common_Pause_Vif(void)
{
#ifdef _FASTBOOT_
    g_bExit = FALSE;
    while (!g_bExit)
    {
        usleep(1 * 1000 * 1000);
    }
#else
    ST_Common_Pause();
#endif
}

#define SNRPADMAXID 2
static MI_S32 STUB_BaseModuleInit(MI_U8 u8Sensornum)
{
    MI_SNR_PlaneInfo_t stPlaneInfo;
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;
    MI_U32             u32SrcFrmrate;
    MI_U32             u32DstFrmrate;
    MI_SYS_BindType_e  eBindType;
    MI_U32             u32BindParam;
    int                i                          = 0;
    MI_U32             u32SnrPadId[SNRPADMAXID]   = {0, 2};
    MI_U8              u8SensorRes                = gstVifInputParm.u8SensorIndex;
    MI_U32             u32VifGroupId[SNRPADMAXID] = {0, 1};
    MI_U32             u32VifDevId[SNRPADMAXID]   = {0, 4};
    MI_U32             u32VifPortId               = 0;

    MI_U32 u32IspDevId              = 0;
    MI_U32 u32IspChnId[SNRPADMAXID] = {0, 1};
    MI_U32 u32IspPortId             = 0;

    MI_U32 u32SclDevId              = 0;
    MI_U32 u32SclChnId[SNRPADMAXID] = {0, 1};
    MI_U32 u32SclPortId             = 0;

    MI_U32 u32VencDevId              = 0;
    MI_U32 u32VencChnId[SNRPADMAXID] = {0, 1};

    /************************************************
    step1 :init SYS
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Init());

    if (u8Sensornum > SNRPADMAXID)
    {
        printf("u8Sensornum:%d larger than SNRPADMAXID:%d\n", u8Sensornum, SNRPADMAXID);
        u8Sensornum = SNRPADMAXID;
    }

    MI_VIF_OutputPortAttr_t stVifPortAttr;
    for (i = 0; i < u8Sensornum; i++)
    {
        MI_VIF_GroupAttr_t stVifGroupAttr;
        MI_VIF_DevAttr_t   stVifDevAttr;
        /************************************************
        step2 :init sensor/vif
        *************************************************/
        STCHECKRESULT(ST_Common_SensorInit(u32SnrPadId[i], FALSE, u8SensorRes, 0xFF));

        memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));
        STCHECKRESULT(MI_SNR_GetPlaneInfo(u32SnrPadId[i], 0, &stPlaneInfo)); // get sensor size
        gstSensorSize[i].u16Width  = stPlaneInfo.stCapRect.u16Width;
        gstSensorSize[i].u16Height = stPlaneInfo.stCapRect.u16Height;

        ST_Common_GetVifDefaultGrouptAttr(&stVifGroupAttr);
        STCHECKRESULT(ST_Common_VifCreateDevGroup(u32VifGroupId[i], &stVifGroupAttr));

        ST_Common_GetVifDefaultDevAttr(&stVifDevAttr);
        STCHECKRESULT(ST_Common_VifEnableDev(u32VifDevId[i], &stVifDevAttr));

        ST_Common_GetVifDefaultPortAttr(&stVifPortAttr);
    }
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
    for (i = 0; i < u8Sensornum; i++)
    {
        ST_Common_GetIspDefaultChnAttr(&stIspChnAttr, &stIspInputCrop, &stIspChnParam);

        if (i == 0)
        {
            stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR0;
        }
        else
        {
            stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR2;
        }

        STCHECKRESULT(
            ST_Common_IspStartChn(u32IspDevId, u32IspChnId[i], &stIspChnAttr, &stIspInputCrop, &stIspChnParam));

        ST_Common_GetIspDefaultPortAttr(&stIspOutPortParam);
        STCHECKRESULT(ST_Common_IspEnablePort(u32IspDevId, u32IspChnId[i], u32IspPortId, &stIspOutPortParam));

        /************************************************
        step4 :bind vif->isp
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
        stSrcChnPort.u32DevId  = u32VifDevId[i];
        stSrcChnPort.u32ChnId  = 0;
        stSrcChnPort.u32PortId = u32VifPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stDstChnPort.u32DevId  = u32IspDevId;
        stDstChnPort.u32ChnId  = u32IspChnId[i];
        stDstChnPort.u32PortId = u32IspPortId;
        u32SrcFrmrate          = 30;
        u32DstFrmrate          = 30;
        if(u8Sensornum == 1)
        {
             eBindType              = E_MI_SYS_BIND_TYPE_REALTIME;
        }
        if(u8Sensornum == 2)
        {
             eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
        }
        u32BindParam = 0;

        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                          u32BindParam));

        stDstChnPort.u32PortId = u32IspPortId + 1;
        STCHECKRESULT(ST_Common_IspEnablePort(u32IspDevId, u32IspChnId[i], stDstChnPort.u32PortId, &stIspOutPortParam));
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 1, 4));
    }
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
    for (i = 0; i < u8Sensornum; i++)
    {
        ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
        STCHECKRESULT(
            ST_Common_SclStartChn(u32SclDevId, u32SclChnId[i], &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

        /************************************************
        step6 :bind isp->scl
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId  = u32IspDevId;
        stSrcChnPort.u32ChnId  = u32IspChnId[i];
        stSrcChnPort.u32PortId = u32IspPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId  = u32SclDevId;
        stDstChnPort.u32ChnId  = u32SclChnId[i];
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
        STCHECKRESULT(ST_Common_VifEnablePort(u32VifDevId[i], u32VifPortId, &stVifPortAttr));
    }

    /************************************************
    step7 :init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;

    ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));
    ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);
    for (i = 0; i < u8Sensornum; i++)
    {
        MI_VENC_ModType_e eType = E_MI_VENC_MODTYPE_H265E;
        memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));
        STCHECKRESULT(MI_SNR_GetPlaneInfo(u32SnrPadId[i], 0, &stPlaneInfo)); // get sensor size

        ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr);
        stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = gstSensorSize[i].u16Width;
        stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = gstSensorSize[i].u16Height;
        STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId[i], &stVencChnAttr));

        /************************************************
        step8 :bind scl->venc
        *************************************************/
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId  = u32SclDevId;
        stSrcChnPort.u32ChnId  = u32SclChnId[i];
        stSrcChnPort.u32PortId = u32SclPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = u32VencDevId;
        stDstChnPort.u32ChnId  = u32VencChnId[i];
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate          = 30;
        u32DstFrmrate          = 30;
        eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
        u32BindParam           = 0;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                          u32BindParam));

        stSclOutPortParam.stSCLOutputSize.u16Width  = gstSensorSize[i].u16Width;
        stSclOutPortParam.stSCLOutputSize.u16Height = gstSensorSize[i].u16Height;
        STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId[i], u32SclPortId, &stSclOutPortParam));
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 0, 4));
    }

    return MI_SUCCESS;
}

static MI_S32 STUB_BaseModuleDeInit(MI_U8 u8Sensornum)
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    int              i                = 0;
    MI_U32           u32SnrPadId[2]   = {0, 2};
    MI_U32           u32VifGroupId[2] = {0, 1};
    MI_U32           u32VifDevId[2]   = {0, 4};
    MI_U32           u32VifPortId     = 0;

    MI_U32 u32IspDevId    = 0;
    MI_U32 u32IspChnId[2] = {0, 1};
    MI_U32 u32IspPortId   = 0;

    MI_U32 u32SclDevId    = 0;
    MI_U32 u32SclChnId[2] = {0, 1};
    MI_U32 u32SclPortId   = 0;

    MI_U32 u32VencDevId    = 0;
    MI_U32 u32VencChnId[2] = {0, 1};

    /************************************************
    step1 :unbind scl->venc
    *************************************************/
    for (i = 0; i < u8Sensornum; i++)
    {
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId  = u32SclDevId;
        stSrcChnPort.u32ChnId  = u32SclChnId[i];
        stSrcChnPort.u32PortId = u32SclPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = u32VencDevId;
        stDstChnPort.u32ChnId  = u32VencChnId[i];
        stDstChnPort.u32PortId = 0;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        /************************************************
        step2 :deinit venc
        *************************************************/
        STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId[i]));
    }
    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    step3 :unbind isp->scl
    *************************************************/
    for (i = 0; i < u8Sensornum; i++)
    {
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId  = u32IspDevId;
        stSrcChnPort.u32ChnId  = u32IspChnId[i];
        stSrcChnPort.u32PortId = u32IspPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId  = u32SclDevId;
        stDstChnPort.u32ChnId  = u32SclChnId[i];
        stDstChnPort.u32PortId = u32SclPortId;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        /************************************************
        step4 :deinit scl
        *************************************************/
        STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId[i], u32SclPortId));
        STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId[i]));
        STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId[i], u32IspPortId + 1));
    }
    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    step5 :unbind vif->isp
    *************************************************/
    for (i = 0; i < u8Sensornum; i++)
    {
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
        stSrcChnPort.u32DevId  = u32VifDevId[i];
        stSrcChnPort.u32ChnId  = 0;
        stSrcChnPort.u32PortId = u32VifPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
        stDstChnPort.u32DevId  = u32IspDevId;
        stDstChnPort.u32ChnId  = u32IspChnId[i];
        stDstChnPort.u32PortId = u32IspPortId;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        /************************************************
        step6 :deinit isp
        *************************************************/
        STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId[i], u32IspPortId));
        STCHECKRESULT(ST_Common_IspStopChn(u32IspDevId, u32IspChnId[i]));
    }
    STCHECKRESULT(ST_Common_IspDestroyDevice(u32IspDevId));

    /************************************************
    step7 :deinit vif/sensor
    *************************************************/
    for (i = 0; i < u8Sensornum; i++)
    {
        STCHECKRESULT(ST_Common_VifDisablePort(u32VifDevId[i], u32VifPortId));
        STCHECKRESULT(ST_Common_VifDisableDev(u32VifDevId[i]));
        STCHECKRESULT(ST_Common_VifDestroyDevGroup(u32VifGroupId[i]));
        STCHECKRESULT(ST_Common_SensorDeInit(u32SnrPadId[i]));
    }

    /************************************************
    step8 :sys exit
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Exit());

    return MI_SUCCESS;
}

static MI_S32 ST_SignleSensorPipeline_Preview()
{
    MI_U8                u8SensorNum  = 1;
    MI_U32               u32VencDevId = 0;
    MI_U32               u32VencChnId = 0;
    MI_U32               u32IspDevId  = 0;
    MI_U32               u32IspChnId  = 0;
    MI_U32               u32SensorPad = 0;
    char                 IqApiBinFilePath[128];
    MI_SNR_PlaneInfo_t   stPlaneInfo;
    ST_VideoStreamInfo_t stStreamInfo;

    STCHECKRESULT(STUB_BaseModuleInit(u8SensorNum));

    if (strlen(gstVifInputParm.IqBinPath) == 0)
    {
        MI_SNR_GetPlaneInfo(u32SensorPad, 0, &stPlaneInfo);
        sprintf(IqApiBinFilePath, "/config/iqfile/%s_api.bin", stPlaneInfo.s8SensorName);
    }
    else
    {
        strcpy(IqApiBinFilePath, gstVifInputParm.IqBinPath);
    }
    ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId, IqApiBinFilePath);

    memset(&stStreamInfo, 0x00, sizeof(ST_VideoStreamInfo_t));
    stStreamInfo.eType        = E_MI_VENC_MODTYPE_H265E;
    stStreamInfo.VencDev      = u32VencDevId;
    stStreamInfo.VencChn      = u32VencChnId;
    stStreamInfo.u32Width     = gstSensorSize[0].u16Width;
    stStreamInfo.u32Height    = gstSensorSize[0].u16Height;
    stStreamInfo.u32FrameRate = 30;
    stStreamInfo.rtspIndex    = 0;

    // start rtsp
    ST_Common_RtspServerStartVideo(&stStreamInfo);

    ST_Common_Pause_Vif();

    // stop rtsp
    ST_Common_RtspServerStopVideo(&stStreamInfo);

    STCHECKRESULT(STUB_BaseModuleDeInit(u8SensorNum));

    return MI_SUCCESS;
}

static MI_S32 ST_DualSensorPipline_Preview()
{
    MI_U8                u8SensorNum     = 2;
    MI_U32               u32VencDevId    = 0;
    MI_U32               u32VencChnId[2] = {0, 1};
    ST_VideoStreamInfo_t stStreamInfo[2];
    int                  i;
    STCHECKRESULT(STUB_BaseModuleInit(u8SensorNum));

    // start rtsp
    for (i = 0; i < u8SensorNum; i++)
    {
        memset(&stStreamInfo[i], 0x00, sizeof(ST_VideoStreamInfo_t));
        stStreamInfo[i].eType        = E_MI_VENC_MODTYPE_H265E;
        stStreamInfo[i].VencDev      = u32VencDevId;
        stStreamInfo[i].VencChn      = u32VencChnId[i];
        stStreamInfo[i].u32Width     = gstSensorSize[i].u16Width;
        stStreamInfo[i].u32Height    = gstSensorSize[i].u16Height;
        stStreamInfo[i].u32FrameRate = 30;
        stStreamInfo[i].rtspIndex    = i;
        // start rtsp
        STCHECKRESULT(ST_Common_RtspServerStartVideo(&stStreamInfo[i]));
    }

    ST_Common_Pause_Vif();

    // stop rtsp
    for (i = 0; i < u8SensorNum; i++)
    {
        STCHECKRESULT(ST_Common_RtspServerStopVideo(&stStreamInfo[i]));
    }

    STCHECKRESULT(STUB_BaseModuleDeInit(u8SensorNum));

    return MI_SUCCESS;
}

void ST_Vif_Usage(void)
{
    printf("Usage:./prog_vif_sesnor_demo 0) single sensor realtime vif->isp->scl->venc->rtsp\n");
    printf("Usage:./prog_vif_sesnor_demo 1) dual sensor framemode vif->isp->scl->venc->rtsp\n");
    printf("Usage:./prog_vif_sesnor_demo x index x iqbin xxx.bin)set sensor Resindex and iqbin\n");
}

MI_S32 ST_Vif_GetCmdlineParam(int argc, char **argv)
{
    gstVifInputParm.u8SensorIndex = 0xFF; // default user input
    gstVifInputParm.u8CmdIndex    = atoi(argv[1]);
    memset(gstVifInputParm.IqBinPath, 0, 128);

    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "index"))
        {
            gstVifInputParm.u8SensorIndex = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "iqbin"))
        {
            strcpy(gstVifInputParm.IqBinPath, argv[i + 1]);
        }
    }

    return MI_SUCCESS;
}

#ifdef _FASTBOOT_
MI_S32 ngx_daemon()
{
    int fd;
    switch (fork())
    {
        case -1:
            printf("fork failed\n");
            return -1;
        case 0:
            break;
        default:
            exit(0);
            break;
    }

    if (setsid() == -1)
    {
        printf("setsid failed\n");
        return -1;
    }

    umask(0);
    // _FASTBOOT_ if need debug log please remove the following code
    // app need run after "mdev -s", or there is no /dev/null
    fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    return 1;
}

void ST_HandleSig(MI_S32 signo)
{
    if (signo == SIGINT)
    {
        printf("catch Ctrl + C, exit normally\n");
    }
    else if (signo == SIGTERM)
    {
        printf("catch SIGTERM, exit normally\n");
    }
    g_bExit = TRUE;
}
#endif

MI_S32 main(int argc, char **argv)
{
    if (argc < 2)
    {
        ST_Vif_Usage();
        return -1;
    }

#ifdef _FASTBOOT_
    if (ngx_daemon() != 1)
    {
        printf("%d\n", __LINE__);
        return -1;
    }
    else
    {
        struct sigaction sigAction;
        sigAction.sa_handler = ST_HandleSig;
        sigemptyset(&sigAction.sa_mask);
        sigAction.sa_flags = 0;
        sigaction(SIGINT, &sigAction, NULL);  //-2
        sigaction(SIGKILL, &sigAction, NULL); //-9
        sigaction(SIGTERM, &sigAction, NULL); //-15
#else
    {
#endif
        ST_Vif_GetCmdlineParam(argc, argv);

        switch (gstVifInputParm.u8CmdIndex)
        {
            //./prog_vif 0 single sensor preview
            //./prog_vif 1 dual sensor preview
            case 0:
                STCHECKRESULT(ST_SignleSensorPipeline_Preview());
                break;
            case 1:
                STCHECKRESULT(ST_DualSensorPipline_Preview());
                break;
            default:
                printf("the index is invaild!\n");
                ST_Vif_Usage();
                return -1;
        }

        memset(&gstVifInputParm, 0x00, sizeof(ST_VifInputParam_t));
        memset(&gstSensorSize, 0x00, sizeof(ST_SensorSize_t) * 2);
    }

    return 0;
}
