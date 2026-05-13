/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

  Unless otherwise stipulated in writing, any and all information contained
 herein regardless in any format shall remain the sole proprietary of
 Sigmastar Technology Corp. and be kept in strict confidence
 (Sigmastar Confidential Information) by the recipient.
 Any unauthorized act including without limitation unauthorized disclosure,
 copying, use, reproduction, sale, distribution, modification, disassembling,
 reverse engineering and compiling of the contents of Sigmastar Confidential
 Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
 rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "mi_rgn.h"

#include "st_common.h"
#include "st_common_rgn.h"
#include "st_common_scl.h"
#include "st_common_venc.h"
#include "st_common_font.h"
#include "st_common_rtsp_video.h"

static ST_Common_InputFile_Attr_t  gstSclInputFileAttr;
static ST_Common_OutputFile_Attr_t gstVencOutputFileAttr;

static MI_U8 gu8CmdIndex = 0;
static MI_RGN_HANDLE gHandle0 = 0;
static MI_RGN_HANDLE gHandle1 = 1;

static MI_S32 STUB_BaseModuleInit(void)
{
    MI_U32 u32SclDevId    = 1;
    MI_U32 u32SclChnId    = 0;
    MI_U32 u32SclPortId   = 0;
    MI_U32 u32VencDevId   = 0;
    MI_U32 u32VencChnId   = 0;
    MI_SYS_ChnPort_t      stSrcChnPort;
    MI_SYS_ChnPort_t      stDstChnPort;
    MI_U32                u32SrcFrmrate;
    MI_U32                u32DstFrmrate;
    MI_SYS_BindType_e     eBindType;
    MI_U32                u32BindParam;

    /************************************************
    step1 :init SYS
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Init());

    /************************************************
    step2 :init scl
    *************************************************/
    MI_SCL_DevAttr_t      stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t     stSclChnParam;
    MI_SYS_WindowRect_t   stSclInputCrop;
    MI_SCL_OutPortParam_t stSclOutPortParam;

    ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
    stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL2;//dev1 must be use SCL2
    STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));

    ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
    STCHECKRESULT(
        ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

    /************************************************
    step3 :init venc
    *************************************************/
    MI_VENC_ModType_e           eType    = E_MI_VENC_MODTYPE_H265E;
    MI_VENC_InitParam_t         VencParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;

    ST_Common_GetVencDefaultDevAttr(&VencParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &VencParam));
    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr);
    stVencChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth  = 1920;
    stVencChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = 1080;
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth     = 1920;
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight    = 1080;
    stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265FIXQP;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 30;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 12;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 12;
    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr));

    /************************************************
    step4 :bind scl->venc
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
    stSclOutPortParam.stSCLOutputSize.u16Width  = 1920;
    stSclOutPortParam.stSCLOutputSize.u16Height = 1080;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId, &stSclOutPortParam));

    gstVencOutputFileAttr.stModuleInfo.eModId = E_MI_MODULE_ID_VENC;
    gstVencOutputFileAttr.stModuleInfo.u32DevId = u32VencDevId;
    gstVencOutputFileAttr.stModuleInfo.u32ChnId = u32VencChnId;
    gstVencOutputFileAttr.stModuleInfo.u32PortId = 0;
    gstVencOutputFileAttr.bThreadExit = FALSE;
    sprintf(gstVencOutputFileAttr.FilePath, "./out/rgn/rgn_demo_case%d", gu8CmdIndex);
    gstVencOutputFileAttr.u16DumpBuffNum = 3;

    ST_Common_CheckResult(&gstVencOutputFileAttr);
    //put input file to scl
    gstSclInputFileAttr.stModuleInfo.eModId = E_MI_MODULE_ID_SCL;
    gstSclInputFileAttr.stModuleInfo.u32DevId = u32SclDevId;
    gstSclInputFileAttr.stModuleInfo.u32ChnId = u32SclChnId;
    gstSclInputFileAttr.stModuleInfo.u32PortId = 0;
    gstSclInputFileAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    gstSclInputFileAttr.u32Width = 1920;
    gstSclInputFileAttr.u32Height = 1080;
    gstSclInputFileAttr.bThreadExit = FALSE;
    gstSclInputFileAttr.u32Fps  = 30;
    sprintf(gstSclInputFileAttr.InputFilePath, "./resource/input/1920_1080_nv12.yuv");
    pthread_create(&gstSclInputFileAttr.pPutDatathread, NULL, ST_Common_PutInputDataThread, (void *)&gstSclInputFileAttr);
    //pthread_create(&gstVencOutputFileAttr.pGetDataThread, NULL, ST_Common_GetVencOutThread, (void *)&gstVencOutputFileAttr);

    return MI_SUCCESS;
}

static MI_S32 STUB_BaseModuleDeInit(void)
{
    MI_U32 u32SclDevId  = 1;
    MI_U32 u32SclChnId  = 0;
    MI_U32 u32SclPortId = 0;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    gstSclInputFileAttr.bThreadExit =   TRUE;
    gstVencOutputFileAttr.bThreadExit = TRUE;
    pthread_join(gstSclInputFileAttr.pPutDatathread, NULL);
    //pthread_join(gstVencOutputFileAttr.pGetDataThread, NULL);

    /************************************************
    step1 :unbind scl->venc
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
    step2 :deinit venc
    *************************************************/
    STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId));
    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    step3 :deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId));
    STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));

    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    step4 :sys exit
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Exit());

    return MI_SUCCESS;
}

static MI_S32 ST_Rgn_Osd_I4_Preview()
{
    MI_U16 u16SocId = 0;
    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;
    MI_RGN_Attr_t         stRegionAttr;
    MI_RGN_ChnPort_t      stRgnChnPort;
    MI_RGN_ChnPortParam_t stRgnChnPortParam;
    MI_VENC_ChnAttr_t     stVencChnAttr;
    ST_OsdFileAttr_t      stOsdFileAttr;
    MI_U16                u16Image_Width  = 0;
    MI_U16                u16Image_Height = 0;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId = u32VencDevId;
    stRgnChnPort.s32ChnId = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    STCHECKRESULT(MI_VENC_GetChnAttr(u32VencDevId, u32VencChnId, &stVencChnAttr));
    u16Image_Width  = stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth;
    u16Image_Height = stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight;
    ST_Common_Rgn_SetImageSize(u16Image_Width, u16Image_Height);

    //init rgn
    ST_Common_GetRgnDefaultInitAttr(&stPaletteTable);
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    //create handle 0 for osd 0
    ST_Common_GetRgnDefaultCreateAttr(&stRegionAttr);
    stRegionAttr.stOsdInitParam.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_I4;
    stRegionAttr.stOsdInitParam.stSize.u32Width = 424;
    stRegionAttr.stOsdInitParam.stSize.u32Height = 224;
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, gHandle0, &stRegionAttr));

    //attch handle 0 to scl
    ST_Common_GetRgnDefaultOsdAttr(&stRgnChnPortParam);
    stRgnChnPortParam.u32Layer = 0;
    stRgnChnPortParam.stOsdChnPort.u8PaletteIdx = 0;
    stRgnChnPortParam.stOsdChnPort.stPoint.s32X = 100;
    stRgnChnPortParam.stOsdChnPort.stPoint.s32Y = 100;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, gHandle0, &stRgnChnPort, &stRgnChnPortParam));

    stOsdFileAttr.u32FileWidth = 424;
    stOsdFileAttr.u32FileHeight = 224;
    stOsdFileAttr.eFileFormat  = stRegionAttr.stOsdInitParam.ePixelFmt;
    sprintf(stOsdFileAttr.FilePath, "./resource/input/424x224.i4.yuv");
    STCHECKRESULT(ST_Common_RgnUpdateCanvas(u16SocId, gHandle0, &stOsdFileAttr));

    ST_Common_WaitDumpFinsh(&gstVencOutputFileAttr);

    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, gHandle0, &stRgnChnPort));

    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, gHandle0));

    STCHECKRESULT(ST_Common_RgnDeInit(u16SocId));

    return MI_SUCCESS;
}

static MI_S32 ST_Rgn_Cover_Preview()
{
    MI_U16 u16SocId = 0;
    MI_U32 u32SclDevId  = 1;
    MI_U32 u32SclChnId  = 0;
    MI_U32 u32SclPortId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;
    MI_RGN_Attr_t         stRegionAttr;
    MI_RGN_ChnPort_t      stRgnChnPort;
    MI_RGN_ChnPortParam_t stRgnChnPortParam;
    MI_SCL_OutPortParam_t stSclOutputPortParam;
    MI_U16                u16Image_Width  = 0;
    MI_U16                u16Image_Height = 0;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stRgnChnPort.s32DevId  = u32SclDevId;
    stRgnChnPort.s32ChnId  = u32SclChnId;
    stRgnChnPort.s32PortId = u32SclPortId;

    STCHECKRESULT(MI_SCL_GetOutputPortParam(u32SclDevId, u32SclChnId, u32SclPortId, &stSclOutputPortParam));
    u16Image_Width  = stSclOutputPortParam.stSCLOutputSize.u16Width;
    u16Image_Height = stSclOutputPortParam.stSCLOutputSize.u16Height;

    ST_Common_Rgn_SetImageSize(u16Image_Width, u16Image_Height);

    //init rgn
    ST_Common_GetRgnDefaultInitAttr(&stPaletteTable);
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    //create handle 0 for cover
    ST_Common_GetRgnDefaultCreateAttr(&stRegionAttr);
    stRegionAttr.eType = E_MI_RGN_TYPE_COVER;
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, gHandle0, &stRegionAttr));

    //attch handle 0 (cover color rect) to scl
    ST_Common_GetRgnDefaultCoverColorAttr(E_MI_RGN_AREA_TYPE_RECT, &stRgnChnPortParam);
    stRgnChnPortParam.u32Layer = 0;
    stRgnChnPortParam.stCoverChnPort.stColorAttr.u32Color = 0XFB80;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, gHandle0, &stRgnChnPort, &stRgnChnPortParam));

    ST_Common_WaitDumpFinsh(&gstVencOutputFileAttr);

    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, gHandle0, &stRgnChnPort));

    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, gHandle0));

    STCHECKRESULT(ST_Common_RgnDeInit(u16SocId));

    return MI_SUCCESS;
}


static MI_S32 ST_Rgn_FaceFrame_Preview()
{
    MI_U16 u16SocId = 0;
    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;
    MI_RGN_Attr_t         stRegionAttr;
    MI_RGN_ChnPort_t      stRgnChnPort;
    MI_RGN_ChnPortParam_t stRgnChnPortParam;
    MI_VENC_ChnAttr_t     stVencChnAttr;
    MI_U16                u16Image_Width  = 0;
    MI_U16                u16Image_Height = 0;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = u32VencDevId;
    stRgnChnPort.s32ChnId  = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    STCHECKRESULT(MI_VENC_GetChnAttr(u32VencDevId, u32VencChnId, &stVencChnAttr));
    u16Image_Width  = stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth;
    u16Image_Height = stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight;
    ST_Common_Rgn_SetImageSize(u16Image_Width, u16Image_Height);

    //init rgn
    ST_Common_GetRgnDefaultInitAttr(&stPaletteTable);
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    //create handle 0 for Frame 0
    ST_Common_GetRgnDefaultCreateAttr(&stRegionAttr);
    stRegionAttr.eType = E_MI_RGN_TYPE_FRAME;
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, gHandle0, &stRegionAttr));

    //create handle 1 for Frame 1
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, gHandle1, &stRegionAttr));

    //attch handle 0 (rect Frame) to scl
    ST_Common_GetRgnDefaultFrameAttr(&stRgnChnPortParam);
    stRgnChnPortParam.u32Layer = 0;
    stRgnChnPortParam.stFrameChnPort.u8Thickness = 4;
    stRgnChnPortParam.stFrameChnPort.u32Color = 0XFB80;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, gHandle0, &stRgnChnPort, &stRgnChnPortParam));

    //attch handle 1 (rect Frame) to scl
    ST_Common_GetRgnDefaultFrameAttr(&stRgnChnPortParam);
    stRgnChnPortParam.u32Layer = 1;
    stRgnChnPortParam.stFrameChnPort.u8Thickness = 16;
    stRgnChnPortParam.stFrameChnPort.u32Color = 0XF420;
    stRgnChnPortParam.stFrameChnPort.stRect.s32X = 1280 * 8192 / u16Image_Width;
    stRgnChnPortParam.stFrameChnPort.stRect.s32Y = 540 * 8192 / u16Image_Height;
    stRgnChnPortParam.stFrameChnPort.stRect.u32Width  = 540 * 8192 / u16Image_Width;
    stRgnChnPortParam.stFrameChnPort.stRect.u32Height = 400 * 8192 / u16Image_Height;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, gHandle1, &stRgnChnPort, &stRgnChnPortParam));

    ST_Common_WaitDumpFinsh(&gstVencOutputFileAttr);

    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, gHandle0, &stRgnChnPort));
    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, gHandle1, &stRgnChnPort));

    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, gHandle0));
    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, gHandle1));

    STCHECKRESULT(ST_Common_RgnDeInit(u16SocId));

    return MI_SUCCESS;
}

static MI_S32 ST_Rgn_OsdDrawText_Preview()
{
    MI_U16 u16SocId = 0;
    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;
    MI_RGN_Attr_t         stRegionAttr;
    MI_RGN_ChnPort_t      stRgnChnPort;
    MI_RGN_ChnPortParam_t stRgnChnPortParam;
    MI_VENC_ChnAttr_t     stVencChnAttr;
    MI_U16                u16Image_Width  = 0;
    MI_U16                u16Image_Height = 0;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = u32VencDevId;
    stRgnChnPort.s32ChnId  = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    STCHECKRESULT(MI_VENC_GetChnAttr(u32VencDevId, u32VencChnId, &stVencChnAttr));
    u16Image_Width  = stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth;
    u16Image_Height = stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight;
    ST_Common_Rgn_SetImageSize(u16Image_Width, u16Image_Height);

    //init rgn
    ST_Common_GetRgnDefaultInitAttr(&stPaletteTable);
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    //create handle 0 for osd 0
    ST_Common_GetRgnDefaultCreateAttr(&stRegionAttr);
    stRegionAttr.stOsdInitParam.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_ARGB1555;
    stRegionAttr.stOsdInitParam.stSize.u32Width  = u16Image_Width;
    stRegionAttr.stOsdInitParam.stSize.u32Height = u16Image_Height;
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, gHandle0, &stRegionAttr));

    //attch handle 0 to scl
    ST_Common_GetRgnDefaultOsdAttr(&stRgnChnPortParam);
    stRgnChnPortParam.u32Layer = 0;
    stRgnChnPortParam.stOsdChnPort.u8PaletteIdx = 0;
    stRgnChnPortParam.stOsdChnPort.stPoint.s32X = 0;
    stRgnChnPortParam.stOsdChnPort.stPoint.s32Y = 0;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, gHandle0, &stRgnChnPort, &stRgnChnPortParam));

    ST_Common_OsdDrawText_Attr_t stDrawTextAttr;
    memset(&stDrawTextAttr, 0x0, sizeof(ST_Common_OsdDrawText_Attr_t));
    stDrawTextAttr.color = COLOR_OF_RED_ARGB4444;
    stDrawTextAttr.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_ARGB4444;
    stDrawTextAttr.eFontType = SS_FONT_32x32;
    stDrawTextAttr.rot = FONT_ROT_NONE;
    stDrawTextAttr.u32X = 100;
    stDrawTextAttr.u32Y = 100;
    stDrawTextAttr.handle = gHandle0;

    int i;
    int idx = 0;
    int CharPerLine = 16;
    char text[128];
    char ch = '0';
    memset(text, 0, 128);
    for (i = 0; i < 5 ; i++)
    {
        int j=0;

        for(j= 0; j < CharPerLine; j++)
        {
            text[idx] = ch + idx;
            idx++;
        }

        CharPerLine--;
        text[idx] = '\n';
        idx++;
    }
    text[idx-2] = '\0';

    strcpy(stDrawTextAttr.text, text);
    printf("text %s\n", stDrawTextAttr.text);

    pthread_mutex_init(&stDrawTextAttr.Osdmutex, NULL);
    STCHECKRESULT(OsdDrawTextCanvas(&stDrawTextAttr));

    ST_Common_WaitDumpFinsh(&gstVencOutputFileAttr);

    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, gHandle0, &stRgnChnPort));

    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, gHandle0));

    STCHECKRESULT(ST_Common_RgnDeInit(u16SocId));

    return MI_SUCCESS;
}

static MI_S32 ST_Rgn_Rotate_Preview()
{
    MI_U16 u16SocId = 0;
    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;
    MI_RGN_Attr_t         stCoverRegionAttr;
    MI_RGN_Attr_t         stFrameRegionAttr;
    MI_RGN_ChnPort_t      stRgnChnPort;
    MI_RGN_ChnPortParam_t stRgnChnPortCoverParam;
    MI_RGN_ChnPortParam_t stRgnChnPortFrameParam;
    MI_VENC_ChnAttr_t     stVencChnAttr;
    MI_U16                u16Image_Width  = 0;
    MI_U16                u16Image_Height = 0;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = u32VencDevId;
    stRgnChnPort.s32ChnId  = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    STCHECKRESULT(MI_VENC_GetChnAttr(u32VencDevId, u32VencChnId, &stVencChnAttr));
    u16Image_Width  = stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth;
    u16Image_Height = stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight;
    ST_Common_Rgn_SetImageSize(u16Image_Width, u16Image_Height);

    ST_Common_Rgn_SetImageSize(u16Image_Width, u16Image_Height);

    //init rgn
    ST_Common_GetRgnDefaultInitAttr(&stPaletteTable);
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    //create handle 0 for cover
    ST_Common_GetRgnDefaultCreateAttr(&stCoverRegionAttr);
    stCoverRegionAttr.eType = E_MI_RGN_TYPE_LINE;
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, gHandle0, &stCoverRegionAttr));

    //create handle 1 for Frame
    ST_Common_GetRgnDefaultCreateAttr(&stFrameRegionAttr);
    stFrameRegionAttr.eType = E_MI_RGN_TYPE_LINE;
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, gHandle1, &stFrameRegionAttr));

    //attch handle 0 (rotate cover color poly) to venc
    ST_Common_GetRgnDefaultRotateAttr(&stRgnChnPortCoverParam);
    stRgnChnPortCoverParam.u32Layer = 0;
    stRgnChnPortCoverParam.stLineChnPort.u32Color = 0XFF4C55;
    stRgnChnPortCoverParam.stLineChnPort.bHollow = FALSE;
    stRgnChnPortCoverParam.stLineChnPort.stPointFrom.s32X = 1920 * 0.5;
    stRgnChnPortCoverParam.stLineChnPort.stPointFrom.s32Y = 1080 * 0.25;
    stRgnChnPortCoverParam.stLineChnPort.stPointTo.s32X   = 1920 * 0.25;
    stRgnChnPortCoverParam.stLineChnPort.stPointTo.s32Y   = 1080 * 0.5;
    stRgnChnPortCoverParam.stLineChnPort.u32LineWidth = 200;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, gHandle0, &stRgnChnPort, &stRgnChnPortCoverParam));

    //attch handle 1 (rotate poly Frame) to venc
    ST_Common_GetRgnDefaultRotateAttr(&stRgnChnPortFrameParam);
    stRgnChnPortFrameParam.u32Layer = 1;
    stRgnChnPortFrameParam.stLineChnPort.u32Color = 0X6BFF1D;
    stRgnChnPortFrameParam.stLineChnPort.stPointFrom.s32X = 1920 * 0.5;
    stRgnChnPortFrameParam.stLineChnPort.stPointFrom.s32Y = 1080 * 0.75;
    stRgnChnPortFrameParam.stLineChnPort.stPointTo.s32X   = 1920 * 0.75;
    stRgnChnPortFrameParam.stLineChnPort.stPointTo.s32Y   = 1080 * 0.5;
    stRgnChnPortFrameParam.stLineChnPort.u32LineWidth = 200;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, gHandle1, &stRgnChnPort, &stRgnChnPortFrameParam));

    ST_Common_WaitDumpFinsh(&gstVencOutputFileAttr);

    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, gHandle0, &stRgnChnPort));
    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, gHandle1, &stRgnChnPort));

    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, gHandle0));
    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, gHandle1));

    STCHECKRESULT(ST_Common_RgnDeInit(u16SocId));

    return MI_SUCCESS;
}


void ST_Rgn_Usage(void)
{
    printf("Usage:./prog_rgn_rgn_demo 0) file -> scl(rgn attach cover color) -> venc -> rtsp\n");
    printf("Usage:./prog_rgn_rgn_demo 1) file -> scl -> venc(rgn attach osd) -> rtsp\n");
    printf("Usage:./prog_rgn_rgn_demo 2) file -> scl -> venc(rgn attach faceframe) -> rtsp\n");
    printf("Usage:./prog_rgn_rgn_demo 3) file -> scl -> venc(rgn attach attach osd draw text) -> rtsp\n");
    printf("Usage:./prog_rgn_rgn_demo 4) file -> scl -> venc(rgn attach rotate cover and frame) -> rtsp\n");
}


MI_S32 main(int argc, char **argv)
{
    if (argc < 2)
    {
        ST_Rgn_Usage();
        return -1;
    }

    gu8CmdIndex = atoi(argv[1]);

    STCHECKRESULT(STUB_BaseModuleInit());

    switch (gu8CmdIndex)
    {
        case 0:
            STCHECKRESULT(ST_Rgn_Cover_Preview());
            break;
        case 1:
            STCHECKRESULT(ST_Rgn_Osd_I4_Preview());
            break;
        case 2:
            STCHECKRESULT(ST_Rgn_FaceFrame_Preview());
            break;
        case 3:
            STCHECKRESULT(ST_Rgn_OsdDrawText_Preview());
            break;
        case 4:
            STCHECKRESULT(ST_Rgn_Rotate_Preview());
            break;
        default:
            printf("the index is invaild!\n");
            STCHECKRESULT(STUB_BaseModuleDeInit());
            ST_Rgn_Usage();
            return -1;
    }

    STCHECKRESULT(STUB_BaseModuleDeInit());

    return 0;
}

