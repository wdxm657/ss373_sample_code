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
#include <sys/time.h>

#include "mi_sys.h"
#include "mi_isp_cus3a_api.h"

#include "st_common_font.h"
#include "st_common.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_rgn.h"
#include "st_common_venc.h"
#include "st_common_dla_cls.h"
#include "st_common_rtsp_video.h"

typedef struct ST_InputParam_s
{
    MI_U8       u8SensorIndex;
    InputAttr_t stClsModelAttr;
    char        au8CLSModelPath[MAX_CLS_STRLEN];
} ST_InputParam_t;

typedef struct ST_SensorSize_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
} ST_SensorSize_t;

typedef struct ST_ClsThread_Attr_s
{
    pthread_t pGetCLSThread;
    MI_BOOL   bThreadExit;
    MI_S32    s32ThreadRetval;
} ST_ClsThread_Attr_t;

typedef struct ST_ClsResult_s
{
    MI_BOOL bPerson;
    MI_BOOL bCat;
    MI_BOOL bDog;
} ST_ClsResult_t;

static ST_InputParam_t     gstClsInputParm;
static ST_ClsThread_Attr_t gstClsThread_Attr;
static ST_SensorSize_t     gstSensorSize;
static ST_ClsResult_t      gstClsResult;

static MI_RGN_ChnPortParam_t        gstRgnOsdChnPortParam;
static ST_Common_OsdDrawText_Attr_t gstDrawTextAttr;

MI_RGN_HANDLE hOsd = 0;
void *        pClsHandle;

void *ST_WaitQThread(void *args)
{
    MI_U8 u8Key;
    printf("press q to exit\n");
    while ((u8Key = getchar()) != 'q')
    {
        printf("press q to exit\n");
        usleep(1 * 1000 * 1000);
    }
    gstClsThread_Attr.bThreadExit = TRUE;
    return NULL;
}

MI_S32 ST_Show_ClsResult(ST_ClsResult_t *pstClsResult)
{
    MI_S32 s32Ret       = 0;
    MI_U8  au8text[128] = {0};
    memset(gstDrawTextAttr.text, 0x00, sizeof(gstDrawTextAttr.text));

    if (TRUE == pstClsResult->bPerson)
    {
        strcat((char *)au8text, "Person ");
    }

    if (TRUE == pstClsResult->bCat)
    {
        strcat((char *)au8text, "Cat ");
    }

    if (TRUE == pstClsResult->bDog)
    {
        strcat((char *)au8text, "Dog ");
    }

    strcpy(gstDrawTextAttr.text, (char *)au8text);
    s32Ret = OsdDrawTextCanvas(&gstDrawTextAttr);
    if (MI_SUCCESS != s32Ret)
    {
        printf("OsdDrawTextCanvas failed!\n");
        return s32Ret;
    }
    return MI_SUCCESS;
}

void *ST_CLSThread(void *args)
{
    MI_SYS_ChnPort_t  stChnPort;
    MI_SYS_BufInfo_t  stScaledBufInfo;
    MI_SYS_BUF_HANDLE hScaledHandle;
    MI_U32            u32SclScaledPortId = 1;

    MI_RGN_ChnPort_t stRgnChnPort;

    memset(&stChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId   = E_MI_MODULE_ID_SCL;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = 0;
    stRgnChnPort.s32ChnId  = 0;
    stRgnChnPort.s32PortId = 0;

    OutputInfo_t astTargetBoxes[NUM_OUTPUS] = {0};
    MI_S32       s32OutputCount             = 0;

    while (gstClsThread_Attr.bThreadExit != TRUE)
    {
        stChnPort.u32PortId = u32SclScaledPortId;
        CHECK_DLA_RESULT(ST_Common_GetOutputBufInfo(stChnPort, &stScaledBufInfo, &hScaledHandle, NULL),
                         gstClsThread_Attr.s32ThreadRetval, EXIT);

        CHECK_DLA_RESULT(ST_Common_CLS_GetTargetData(&pClsHandle, &stScaledBufInfo, astTargetBoxes, &s32OutputCount),
                         gstClsThread_Attr.s32ThreadRetval, EXIT);

        memset(&gstClsResult, 0x00, sizeof(ST_ClsResult_t));
        for (MI_U16 i = 0; i < s32OutputCount; i++)
        {
            if (0 == astTargetBoxes[i].class_id)
            {
                gstClsResult.bPerson = TRUE;
            }

            if (1 == astTargetBoxes[i].class_id)
            {
                gstClsResult.bCat = TRUE;
            }

            if (2 == astTargetBoxes[i].class_id)
            {
                gstClsResult.bDog = TRUE;
            }
        }
        CHECK_DLA_RESULT(ST_Show_ClsResult(&gstClsResult), gstClsThread_Attr.s32ThreadRetval, EXIT);

        CHECK_DLA_RESULT(ST_Common_PutOutputBufInfo(&hScaledHandle), gstClsThread_Attr.s32ThreadRetval, EXIT);
    }

EXIT:
    gstClsThread_Attr.bThreadExit = TRUE;
    return NULL;
}

static MI_S32 STUB_BaseModuleInit()
{
    MI_SNR_PlaneInfo_t stPlaneInfo;
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;
    MI_U32             u32SrcFrmrate;
    MI_U32             u32DstFrmrate;
    MI_SYS_BindType_e  eBindType;
    MI_U32             u32BindParam;
    MI_U32             u32SnrPadId   = 0;
    MI_U8              u8SensorRes   = gstClsInputParm.u8SensorIndex;
    MI_U32             u32VifGroupId = 0;
    MI_U32             u32VifDevId   = 0;
    MI_U32             u32VifPortId  = 0;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 0;

    MI_U32 u32SclDevId        = 0;
    MI_U32 u32SclChnId        = 0;
    MI_U32 u32SclSourcePortId = 0;
    MI_U32 u32SclScaledPortId = 1;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    /************************************************
    step1 :init SYS
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_VIF_GroupAttr_t      stVifGroupAttr;
    MI_VIF_DevAttr_t        stVifDevAttr;
    MI_VIF_OutputPortAttr_t stVifPortAttr;
    /************************************************
    step2 :init sensor/vif
    *************************************************/
    STCHECKRESULT(ST_Common_SensorInit(u32SnrPadId, FALSE, u8SensorRes, 0xFF));

    memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));
    STCHECKRESULT(MI_SNR_GetPlaneInfo(u32SnrPadId, 0, &stPlaneInfo)); // get sensor size
    gstSensorSize.u16Width  = stPlaneInfo.stCapRect.u16Width;
    gstSensorSize.u16Height = stPlaneInfo.stCapRect.u16Height;

    ST_Common_GetVifDefaultGrouptAttr(&stVifGroupAttr);
    STCHECKRESULT(ST_Common_VifCreateDevGroup(u32VifGroupId, &stVifGroupAttr));

    ST_Common_GetVifDefaultDevAttr(&stVifDevAttr);
    STCHECKRESULT(ST_Common_VifEnableDev(u32VifDevId, &stVifDevAttr));

    ST_Common_GetVifDefaultPortAttr(&stVifPortAttr);
    STCHECKRESULT(ST_Common_VifEnablePort(u32VifDevId, u32VifPortId, &stVifPortAttr));

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
    STCHECKRESULT(ST_Common_IspStartChn(u32IspDevId, u32IspChnId, &stIspChnAttr, &stIspInputCrop, &stIspChnParam));

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

    eBindType = E_MI_SYS_BIND_TYPE_REALTIME;

    u32BindParam = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    step5 :init cls
    *************************************************/
    InitInfo_t stCLSionInfo;

    memset(&stCLSionInfo, 0x0, sizeof(InitInfo_t));
    memcpy(stCLSionInfo.model, gstClsInputParm.au8CLSModelPath, MAX_CLS_STRLEN);
    stCLSionInfo.threshold = 0.5;

    STCHECKRESULT(ST_Common_CLS_Init(&pClsHandle, &stCLSionInfo));
    STCHECKRESULT(ST_Common_CLS_SetDefaultParam(&pClsHandle));
    STCHECKRESULT(ST_Common_CLS_GetModelAttr(&pClsHandle, &(gstClsInputParm.stClsModelAttr)));

    /************************************************
    step6 :init scl
    *************************************************/
    MI_SCL_DevAttr_t      stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t     stSclChnParam;
    MI_SYS_WindowRect_t   stSclInputCrop;
    MI_SCL_OutPortParam_t stSclOutSourcePortParam;
    MI_SCL_OutPortParam_t stSclOutScaledPortParam;

    ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
    STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));

    ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
    STCHECKRESULT(ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

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
    stDstChnPort.u32PortId = u32SclSourcePortId;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_REALTIME;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    /************************************************
    step8 :init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;

    ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));

    MI_VENC_ModType_e eType = E_MI_VENC_MODTYPE_H265E;
    memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));
    STCHECKRESULT(MI_SNR_GetPlaneInfo(u32SnrPadId, 0, &stPlaneInfo)); // get sensor size

    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr);
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth  = gstSensorSize.u16Width;
    stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = gstSensorSize.u16Height;
    stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate =
        stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth * stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight * 1.3;
    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr));

    /************************************************
    step9 :bind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclSourcePortId;
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
    ST_Common_GetSclDefaultPortAttr(&stSclOutSourcePortParam);
    ST_Common_GetSclDefaultPortAttr(&stSclOutScaledPortParam);

    stSclOutSourcePortParam.stSCLOutputSize.u16Width  = gstSensorSize.u16Width;
    stSclOutSourcePortParam.stSCLOutputSize.u16Height = gstSensorSize.u16Height;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclSourcePortId, &stSclOutSourcePortParam));

    stSclOutScaledPortParam.stSCLOutputSize.u16Width  = gstClsInputParm.stClsModelAttr.width;
    stSclOutScaledPortParam.stSCLOutputSize.u16Height = gstClsInputParm.stClsModelAttr.height;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclScaledPortId, &stSclOutScaledPortParam));

    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 2, 4));

    stSrcChnPort.u32PortId = u32SclScaledPortId;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stSrcChnPort, 2, 4));


    /************************************************
    step9 :init rgn
    *************************************************/
    MI_U16                u16SocId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;
    MI_RGN_ChnPort_t      stRgnChnPort;
    MI_RGN_Attr_t         stRgnOsdAttr;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = u32VencDevId;
    stRgnChnPort.s32ChnId  = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    STCHECKRESULT(ST_Common_GetRgnDefaultInitAttr(&stPaletteTable));
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    ST_Common_GetRgnDefaultCreateAttr(&stRgnOsdAttr);
    stRgnOsdAttr.stOsdInitParam.ePixelFmt        = E_MI_RGN_PIXEL_FORMAT_ARGB1555;
    stRgnOsdAttr.stOsdInitParam.stSize.u32Height = gstSensorSize.u16Height;
    stRgnOsdAttr.stOsdInitParam.stSize.u32Width  = gstSensorSize.u16Width;
    STCHECKRESULT(ST_Common_RgnCreate(u16SocId, hOsd, &stRgnOsdAttr));

    /************************************************
    step10 :attch handle to venc
    *************************************************/
    ST_Common_GetRgnDefaultOsdAttr(&gstRgnOsdChnPortParam);
    gstRgnOsdChnPortParam.u32Layer                  = 0;
    gstRgnOsdChnPortParam.stOsdChnPort.u8PaletteIdx = 0;
    gstRgnOsdChnPortParam.stOsdChnPort.stPoint.s32X = 0;
    gstRgnOsdChnPortParam.stOsdChnPort.stPoint.s32Y = 0;
    gstRgnOsdChnPortParam.bShow                     = TRUE;
    STCHECKRESULT(ST_Common_RgnAttachChn(u16SocId, hOsd, &stRgnChnPort, &gstRgnOsdChnPortParam));

    memset(&gstDrawTextAttr, 0x0, sizeof(ST_Common_OsdDrawText_Attr_t));
    gstDrawTextAttr.color     = COLOR_OF_RED_ARGB1555;
    gstDrawTextAttr.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_ARGB1555;
    gstDrawTextAttr.eFontType = SS_FONT_32x32;
    gstDrawTextAttr.rot       = FONT_ROT_NONE;
    gstDrawTextAttr.u32X      = 50;
    gstDrawTextAttr.u32Y      = 50;
    gstDrawTextAttr.handle    = hOsd;

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

    MI_U32 u32SclDevId        = 0;
    MI_U32 u32SclChnId        = 0;
    MI_U32 u32SclSourcePortId = 0;
    MI_U32 u32SclScaledPortId = 1;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    MI_U16           u16SocId = 0;
    MI_RGN_ChnPort_t stRgnChnPort;

    memset(&stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stRgnChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stRgnChnPort.s32DevId  = u32VencDevId;
    stRgnChnPort.s32ChnId  = u32VencChnId;
    stRgnChnPort.s32PortId = 0;

    /************************************************
    step1 :detach rgn->venc
    *************************************************/
    STCHECKRESULT(ST_Common_RgnDetachChn(u16SocId, hOsd, &stRgnChnPort));
    STCHECKRESULT(ST_Common_RgnDestroy(u16SocId, hOsd));

    STCHECKRESULT(ST_Common_RgnDeInit(u16SocId));

    /************************************************
    step2 :unbind scl->venc
    *************************************************/

    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclSourcePortId;
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
    stDstChnPort.u32PortId = u32SclSourcePortId;

    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step5 :deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclSourcePortId));
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclScaledPortId));
    STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));

    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    step6 :deinit cls
    *************************************************/
    STCHECKRESULT(ST_Common_CLS_DeInit(&pClsHandle));

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
    stDstChnPort.u32PortId = u32IspPortId;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step8 :deinit isp
    *************************************************/
    STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId, u32IspPortId));
    STCHECKRESULT(ST_Common_IspStopChn(u32IspDevId, u32IspChnId));

    STCHECKRESULT(ST_Common_IspDestroyDevice(u32IspDevId));

    /************************************************
    step9 :deinit vif/sensor
    *************************************************/
    STCHECKRESULT(ST_Common_VifDisablePort(u32VifDevId, u32VifPortId));
    STCHECKRESULT(ST_Common_VifDisableDev(u32VifDevId));
    STCHECKRESULT(ST_Common_VifDestroyDevGroup(u32VifGroupId));
    STCHECKRESULT(ST_Common_SensorDeInit(u32SnrPadId));

    /************************************************
    step10 :sys exit
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Exit());

    return MI_SUCCESS;
}

static MI_S32 ST_CLS_Preview()
{
    pthread_t            pGetWaitQTheead;
    MI_U32               u32VencDevId = 0;
    MI_U32               u32VencChnId = 0;
    ST_VideoStreamInfo_t stStreamInfo;

    STCHECKRESULT(STUB_BaseModuleInit());

    memset(&gstClsThread_Attr, 0, sizeof(ST_ClsThread_Attr_t));
    gstClsThread_Attr.bThreadExit     = FALSE;
    gstClsThread_Attr.s32ThreadRetval = -1;

    pthread_create(&pGetWaitQTheead, NULL, ST_WaitQThread, NULL);
    pthread_create(&gstClsThread_Attr.pGetCLSThread, NULL, ST_CLSThread, NULL);

    // start rtsp
    memset(&stStreamInfo, 0x00, sizeof(ST_VideoStreamInfo_t));
    stStreamInfo.eType        = E_MI_VENC_MODTYPE_H265E;
    stStreamInfo.VencDev      = u32VencDevId;
    stStreamInfo.VencChn      = u32VencChnId;
    stStreamInfo.u32Width     = gstSensorSize.u16Width;
    stStreamInfo.u32Height    = gstSensorSize.u16Height;
    stStreamInfo.u32FrameRate = 30;
    stStreamInfo.rtspIndex    = 0;
    ST_Common_RtspServerStartVideo(&stStreamInfo);

    while (FALSE == gstClsThread_Attr.bThreadExit)
    {
        usleep(1 * 1000 * 1000);
    }

#ifndef __MUSL__
    pthread_cancel(pGetWaitQTheead);
#endif
    pthread_join(gstClsThread_Attr.pGetCLSThread, NULL);

    // stop rtsp
    ST_Common_RtspServerStopVideo(&stStreamInfo);

    STCHECKRESULT(STUB_BaseModuleDeInit());

    if (MI_SUCCESS != gstClsThread_Attr.s32ThreadRetval)
    {
        return gstClsThread_Attr.s32ThreadRetval;
    }
    return MI_SUCCESS;
}

void ST_CLS_Usage(int argc, char **argv)
{
    printf("Usage : %s model x index x\n", argv[0]);
    printf("model : Specify the ipu clsection model file\n");
    printf("index : Specify the sensor index\n");
}

MI_S32 ST_CLS_GetCmdlineParam(int argc, char **argv)
{
    gstClsInputParm.u8SensorIndex = 0xFF;

    for (MI_S32 i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "index"))
        {
            gstClsInputParm.u8SensorIndex = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "model"))
        {
            strcpy(gstClsInputParm.au8CLSModelPath, argv[i + 1]);
        }
    }

    return MI_SUCCESS;
}

MI_S32 main(int argc, char **argv)
{
    if (argc < 3)
    {
        ST_CLS_Usage(argc, argv);
        return -1;
    }

    ST_CLS_GetCmdlineParam(argc, argv);

    STCHECKRESULT(ST_CLS_Preview());

    memset(&gstClsInputParm, 0x00, sizeof(ST_InputParam_t));
    memset(&gstSensorSize, 0x00, sizeof(ST_SensorSize_t));
    return 0;
}
