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
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <sys/prctl.h>
#include <errno.h>
#include <assert.h>
#include "mi_common_datatype.h"
#include "mi_scl.h"
#include "mi_vdf.h"
#include "mi_vdf_datatype.h"
#include "mi_sys_datatype.h"
#include "mi_sys.h"
#include "mi_md.h"
#include "mi_od.h"
#include "mi_vg.h"
#include "mi_rgn.h"
#include "st_common.h"
#include "st_common_vdf.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_rgn.h"
#include "st_common_venc.h"
#include "st_common_font.h"

#define WINDOW_INDEX_CNT_MAX        6
#define RGN_FRAME_TOTAL_WIDTH       8192
#define RGN_FRAME_TOTAL_HEIGHT      8192
#define INPUT_WIDTH                 640
#define INPUT_HEIGHT                360
#define INPUT_PATH                  "./resource/input/vg_test_640x360_422yuyv.yuv"
#define OUTPUT_CNT                  60

extern int pthread_setname_np(pthread_t thread, const char *name);
static ST_Common_InputFile_Attr_t g_stIspInputFileAttr;
static ST_Common_OutputFile_Attr_t g_stVencOutputFileAttr;

static MI_U8            g_u8CmdIndex = 0;
static MI_RGN_HANDLE    g_hRgnHandleCover[WINDOW_INDEX_CNT_MAX];
static MI_RGN_ChnPort_t g_stRgnChnPort;


MI_VDF_ChnAttr_t        g_stVdfChnAttr;
static MI_VDF_CHANNEL   g_VdfVgChn = VDF_VG_CHN_NO;
static MI_BOOL          g_bVgExit = FALSE;
static MI_S32           g_pThreadVgState = -1;
static pthread_t        g_pThreadVg;

static MI_U16           g_u16ImageWidth = INPUT_WIDTH;
static MI_U16           g_u16ImageHeight = INPUT_HEIGHT;

static FILE             *g_pOutFile;
char                    g_strOdResult[50];
MI_U32                  g_u32DumpLogCnt = 0;

MI_S32 ST_Common_RgnInitFrameWindows_Max(MI_RGN_ChnPort_t* stRgnChnPort)
{
    int i;
    MI_S32 s32Ret;
    MI_U8 u8FrameHeight;
    MI_RGN_Attr_t stRegionAttr;
    ST_Common_GetRgnDefaultCreateAttr(&stRegionAttr);
    stRegionAttr.eType = E_MI_RGN_TYPE_FRAME;

    MI_U8 u8RgnFrameWidth     =   100;
    MI_U8 u8RgnFrameHeight    =   100;
    MI_U8 u8RgnFrameRectX     =   0;
    MI_U8 u8RgnFrameRectY     =   0;

    MI_RGN_ChnPortParam_t stRgnChnPortParam;
    stRgnChnPortParam.bShow = 0;
    stRgnChnPortParam.u32Layer = 0;
    stRgnChnPortParam.stFrameChnPort.u8Thickness = 3;
    stRgnChnPortParam.stFrameChnPort.u32Color = 0XFF0000;

    stRgnChnPortParam.stFrameChnPort.stRect.u32Width  = u8RgnFrameWidth * RGN_FRAME_TOTAL_WIDTH / g_u16ImageWidth;
    stRgnChnPortParam.stFrameChnPort.stRect.u32Height = u8RgnFrameHeight * RGN_FRAME_TOTAL_HEIGHT / g_u16ImageHeight;

    for(i = 0; i < WINDOW_INDEX_CNT_MAX; i++)
    {
        g_hRgnHandleCover[i] = i;
        s32Ret = ST_Common_RgnCreate(0, g_hRgnHandleCover[i], &stRegionAttr);
        if(s32Ret != MI_SUCCESS)
        {
            ST_ERR("Create Rgn failed \n");
            return -1;
        }
        stRgnChnPortParam.stFrameChnPort.stRect.s32X = u8RgnFrameRectX * RGN_FRAME_TOTAL_WIDTH / g_u16ImageWidth;
        stRgnChnPortParam.stFrameChnPort.stRect.s32Y = u8RgnFrameRectY * RGN_FRAME_TOTAL_HEIGHT / g_u16ImageHeight;
        STCHECKRESULT(ST_Common_RgnAttachChn(0, g_hRgnHandleCover[i], stRgnChnPort, &stRgnChnPortParam));
    }

    s32Ret = ST_Common_RgnCreate(0, 6, &stRegionAttr);
    if(s32Ret != MI_SUCCESS)
    {
        ST_ERR("Create Rgn failed \n");
        return -1;
    }
    s32Ret = ST_Common_RgnCreate(0, 7, &stRegionAttr);
    if(s32Ret != MI_SUCCESS)
    {
        ST_ERR("Create Rgn failed \n");
        return -1;
    }
    stRgnChnPortParam.bShow = 1;
    stRgnChnPortParam.u32Layer = 0;
    stRgnChnPortParam.stFrameChnPort.u8Thickness = 1;
    stRgnChnPortParam.stFrameChnPort.u32Color = 0X00FF00;

    MI_U8 u8VgWarnLineWidth = 2;

    stRgnChnPortParam.stFrameChnPort.stRect.u32Width  = u8VgWarnLineWidth * RGN_FRAME_TOTAL_WIDTH / g_u16ImageWidth;

    u8FrameHeight = g_stVdfChnAttr.stVgAttr.line[0].py.y - g_stVdfChnAttr.stVgAttr.line[0].px.y;
    stRgnChnPortParam.stFrameChnPort.stRect.u32Height = u8FrameHeight * RGN_FRAME_TOTAL_HEIGHT / g_u16ImageHeight;
    stRgnChnPortParam.stFrameChnPort.stRect.s32X = g_stVdfChnAttr.stVgAttr.line[0].px.x * RGN_FRAME_TOTAL_WIDTH / g_u16ImageWidth;
    stRgnChnPortParam.stFrameChnPort.stRect.s32Y = g_stVdfChnAttr.stVgAttr.line[0].px.y * RGN_FRAME_TOTAL_HEIGHT / g_u16ImageHeight;
    STCHECKRESULT(ST_Common_RgnAttachChn(0, 6, stRgnChnPort, &stRgnChnPortParam));

    u8FrameHeight = g_stVdfChnAttr.stVgAttr.line[1].py.y - g_stVdfChnAttr.stVgAttr.line[1].px.y;
    stRgnChnPortParam.stFrameChnPort.stRect.u32Height = u8FrameHeight * RGN_FRAME_TOTAL_HEIGHT / g_u16ImageHeight;
    stRgnChnPortParam.stFrameChnPort.stRect.s32X = g_stVdfChnAttr.stVgAttr.line[1].px.x * RGN_FRAME_TOTAL_WIDTH / g_u16ImageWidth;
    stRgnChnPortParam.stFrameChnPort.stRect.s32Y = g_stVdfChnAttr.stVgAttr.line[1].px.y * RGN_FRAME_TOTAL_HEIGHT / g_u16ImageHeight;
    STCHECKRESULT(ST_Common_RgnAttachChn(0, 7, stRgnChnPort, &stRgnChnPortParam));

    return MI_SUCCESS;
}

MI_S32 VGtoRECT(MI_U8 chn, MI_VDF_VgAttr_t *pVgAttr, MI_VG_Result_t *pVgResult)
{
    int i;
    MI_U8 u8FrameWidth, u8FrameHeight;
    MI_S32 s32Ret;
    MI_RGN_ChnPortParam_t stTmpRgnChnPortParam;
    MI_RGN_ChnPort_t stTmpRgnChnPort;
    memset(&stTmpRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stTmpRgnChnPort.eModId = E_MI_MODULE_ID_VENC;
    stTmpRgnChnPort.s32DevId = 0;
    stTmpRgnChnPort.s32ChnId = 0;
    stTmpRgnChnPort.s32PortId = 0;
    int bprint = 0;

    if (pVgAttr && pVgResult)
    {
        printf("chn:%d ", chn);

        printf("pVgResult:{");
        printf("alarm:{");
        for (i = 0; i < MAX_NUMBER; i++)
        {
            printf("%d, ", pVgResult->alarm[i]);
        }
        printf("}");
        printf("alarm_cnt:%d,", pVgResult->alarm_cnt);

        printf("bounding_box:{");
        for (i = 0; i < WINDOW_INDEX_CNT_MAX; i++)
        {
            if( (pVgResult->bounding_box[i].up == 0)   && (pVgResult->bounding_box[i].down == 0) &&   //up.down -> y
                (pVgResult->bounding_box[i].left == 0) && (pVgResult->bounding_box[i].right == 0))  //left.right -> x
            {
                s32Ret = MI_RGN_GetDisplayAttr(0, g_hRgnHandleCover[i], &stTmpRgnChnPort, &stTmpRgnChnPortParam);
                if(s32Ret < 0)
                {
                    printf("MI_RGN_GetDisplayAttr err!!!\n");
                    return -1;
                }
                stTmpRgnChnPortParam.bShow = 0;
                s32Ret = MI_RGN_SetDisplayAttr(0, g_hRgnHandleCover[i], &stTmpRgnChnPort, &stTmpRgnChnPortParam);
                if(s32Ret < 0)
                {
                    printf("MI_RGN_SetDisplayAttr err!!!\n");
                    return -1;
                }
            }
            else
            {
                u8FrameWidth = pVgResult->bounding_box[i].right - pVgResult->bounding_box[i].left;
                u8FrameHeight = pVgResult->bounding_box[i].down - pVgResult->bounding_box[i].up;

                s32Ret = MI_RGN_GetDisplayAttr(0, g_hRgnHandleCover[i], &stTmpRgnChnPort, &stTmpRgnChnPortParam);
                if(s32Ret < 0)
                {
                    printf("MI_RGN_GetDisplayAttr err!!!\n");
                    return -1;
                }
                stTmpRgnChnPortParam.bShow = 1;
                stTmpRgnChnPortParam.stFrameChnPort.stRect.s32X = pVgResult->bounding_box[i].left * 8192 / g_u16ImageWidth;
                stTmpRgnChnPortParam.stFrameChnPort.stRect.s32Y = pVgResult->bounding_box[i].up * 8192 / g_u16ImageHeight;
                stTmpRgnChnPortParam.stFrameChnPort.stRect.u32Width  = u8FrameWidth * 8192 / g_u16ImageWidth;
                stTmpRgnChnPortParam.stFrameChnPort.stRect.u32Height = u8FrameHeight * 8192 / g_u16ImageHeight;
                s32Ret = MI_RGN_SetDisplayAttr(0, g_hRgnHandleCover[i], &stTmpRgnChnPort, &stTmpRgnChnPortParam);
                if(s32Ret < 0)
                {
                    printf("MI_RGN_SetDisplayAttr err!!!\n");
                    return -1;
                }
                bprint = 1;
            }
        }
        printf("}");
        printf("}\n");
    }
    if(g_u32DumpLogCnt < OUTPUT_CNT)
    {
        if(bprint)
        {
            sprintf(g_strOdResult, "Alarm%d:{%d, %d} cnt:%d Rect:",g_u32DumpLogCnt+1, pVgResult->alarm[0], pVgResult->alarm[1], pVgResult->alarm_cnt);
            fprintf(g_pOutFile, "%s", g_strOdResult);
            for(i = 0; i < pVgResult->alarm_cnt; i++)
            {
                sprintf(g_strOdResult, "{%d, %d, %d, %d}\n",  pVgResult->bounding_box[i].up, pVgResult->bounding_box[i].down,
                                                              pVgResult->bounding_box[i].left, pVgResult->bounding_box[i].right);
                fprintf(g_pOutFile, "%s", g_strOdResult);
            }
        }
    }
    g_u32DumpLogCnt++;
    return 0;
}

void *ST_VG_Task(void *argu)
{
    int chn = 0;
    MI_S32 s32Ret = 0;
    MI_VDF_Result_t stVdfResult = {0};

    MI_VG_Result_t stVgResult;

    MI_BOOL blEnter = FALSE;

    printf("enter vg task loop\n");

    while (!g_bVgExit && s32Ret++ < 30);
    memset(&stVgResult, 0x00, sizeof(MI_VG_Result_t));
    stVdfResult.stVgResult.alarm[0] = 1;
    stVdfResult.stVgResult.alarm[1] = 1;
    s32Ret = VGtoRECT(chn, &g_stVdfChnAttr.stVgAttr, &stVdfResult.stVgResult);
    if(s32Ret != 0)
    {
        ST_ERR("Rgn DisplayAttr err, VGtoRECT fail!\n");
    }
    while (!g_bVgExit)
    {
        blEnter = FALSE;

        memset(&stVdfResult, 0x00, sizeof(MI_VDF_Result_t));
        stVdfResult.enWorkMode = E_MI_VDF_WORK_MODE_VG;
        s32Ret = MI_VDF_GetResult(g_VdfVgChn, &stVdfResult, 0);
        if (MI_SUCCESS == s32Ret)
        {
            for (int i = 0; i < g_stVdfChnAttr.stVgAttr.line_number; i++)
            {
                if (1 == stVdfResult.stVgResult.alarm[i])
                    blEnter = TRUE;
            }
            memcpy(&stVgResult, &stVdfResult.stVgResult, sizeof(stVgResult));
            MI_VDF_PutResult(g_VdfVgChn, &stVdfResult);
            if (blEnter)
            {
                s32Ret = VGtoRECT(chn, &g_stVdfChnAttr.stVgAttr, &stVgResult);
                if(s32Ret != 0)
                {
                    ST_ERR("Rgn DisplayAttr err, VGtoRECT fail!\n");
                    continue;
                }
            }
            else
                g_u32DumpLogCnt++;
        }
    }
    return NULL;
}

static MI_S32 STUB_BaseModuleInit(void)
{
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;
    MI_U32             u32SrcFrmrate;
    MI_U32             u32DstFrmrate;
    MI_SYS_BindType_e  eBindType;
    MI_U32             u32BindParam;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 0;

    MI_U32 u32SclDevId = 0;
    MI_U32 u32SclChnId = 0;
    MI_U32 u32SclPortId0 = 0;
    MI_U32 u32SclPortId1 = 1;
    /************************************************
    step1 :init SYS
    *************************************************/
    STCHECKRESULT(ST_Common_Sys_Init());
    char *output_file = "./out/vdf/vg_result.txt";
    ST_Common_CheckMkdirOutFile(output_file);
    g_pOutFile = fopen(output_file, "w");
    if (g_pOutFile == NULL)
    {
        printf("Error: Can't open ./out/vdf/vg_result.txt\n");
        return -1;
    }
    /************************************************
    step2 :init isp
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
    stIspOutPortParam.stCropRect.u16Width  = INPUT_WIDTH;
    stIspOutPortParam.stCropRect.u16Height = INPUT_HEIGHT;
    STCHECKRESULT(ST_Common_IspEnablePort(u32IspDevId, u32IspChnId, u32IspPortId, &stIspOutPortParam));

    /************************************************
    step3 :init SCL
    *************************************************/
    MI_SCL_DevAttr_t stSclDevAttr;
    MI_SCL_ChannelAttr_t stSclChnAttr;
    MI_SCL_ChnParam_t stSclChnParam;
    MI_SYS_WindowRect_t stSclInputCrop;
    MI_SCL_OutPortParam_t  stSclOutPortParam;

    ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
    STCHECKRESULT(ST_Common_SclCreateDevice(u32SclDevId, &stSclDevAttr));

    ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
    STCHECKRESULT(ST_Common_SclStartChn(u32SclDevId, u32SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

    /************************************************
    step4 :bind isp->scl
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
    stDstChnPort.u32PortId = u32SclPortId0;
    u32SrcFrmrate          = 15;
    u32DstFrmrate          = 15;
    eBindType              = E_MI_SYS_BIND_TYPE_REALTIME;
    u32BindParam           = 0;
    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                        u32BindParam));

    /************************************************
    step5 :init VENC
    *************************************************/
    MI_VENC_DEV                 s32VencDevId = 0;
    MI_VENC_CHN                 s32VencChnId = 0;
    MI_VENC_ModType_e           eType    = E_MI_VENC_MODTYPE_H264E;
    MI_VENC_InitParam_t         VencParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;

    STCHECKRESULT(ST_Common_Sys_Init());
    ST_Common_GetVencDefaultDevAttr(&VencParam);
    STCHECKRESULT(ST_Common_VencCreateDev(s32VencDevId, &VencParam));
    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr);

    stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth         = INPUT_WIDTH;
    stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight        = INPUT_HEIGHT;

    stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 15;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 12;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 12;

    g_u16ImageWidth = stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth;
    g_u16ImageHeight = stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight;

    STCHECKRESULT(ST_Common_VencStartChn(s32VencDevId, s32VencChnId, &stVencChnAttr));

    /************************************************
    step6 :bind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId0;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = s32VencDevId;
    stDstChnPort.u32ChnId  = s32VencChnId;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 1;
    u32DstFrmrate          = 15;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));
    ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);        //E_MI_SCL_HWSCL0 | E_MI_SCL_HWSCL1;

    stSclOutPortParam.stSCLOutputSize.u16Width  = INPUT_WIDTH;
    stSclOutPortParam.stSCLOutputSize.u16Height = INPUT_HEIGHT;

    /**********scl port0*********/
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId0, &stSclOutPortParam));

    /**********scl port1*********/
    stSclOutPortParam.ePixelFormat  = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId, u32SclPortId1, &stSclOutPortParam));


    /************************************************
    step7 :init RGN
    *************************************************/
    MI_U16 u16SocId = 0;
    MI_RGN_PaletteTable_t stPaletteTable;
    g_u16ImageWidth = stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth;
    g_u16ImageHeight = stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight;

    memset(&g_stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    g_stRgnChnPort.eModId = E_MI_MODULE_ID_VENC;
    g_stRgnChnPort.s32DevId = s32VencDevId;
    g_stRgnChnPort.s32ChnId = s32VencChnId;
    g_stRgnChnPort.s32PortId = 0;

    ST_Common_Rgn_SetImageSize(g_u16ImageWidth, g_u16ImageHeight);

    STCHECKRESULT(ST_Common_GetRgnDefaultInitAttr(&stPaletteTable));
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    /************************************************
    step8 :init VDF_VG
    *************************************************/
    STCHECKRESULT(MI_VDF_Init());

    ST_Common_GetVdfVgDefaultChnAttr(&g_stVdfChnAttr, g_u16ImageWidth, g_u16ImageHeight);

    if(MI_SUCCESS != ST_Common_RgnInitFrameWindows_Max(&g_stRgnChnPort))
    {
        ST_ERR("Init Rgn Frame Windows failed \n");
        return -1;
    }
    if (0 != ST_Common_VdfCheckAlign((MI_U16)MDMB_MODE_BUTT, g_stVdfChnAttr.stVgAttr.stride))
    {
        ST_ERR("Vdf Check Align Err!\n");
        return -1;
    }

    /************************************************
    step9 :bind scl->vdf
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId1;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VDF;
    stDstChnPort.u32DevId  = 0;
    stDstChnPort.u32ChnId  = 0;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 15;
    u32DstFrmrate          = 15;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    STCHECKRESULT(MI_VDF_CreateChn(g_VdfVgChn, &g_stVdfChnAttr));
    STCHECKRESULT(MI_VDF_Run(E_MI_VDF_WORK_MODE_VG));
    STCHECKRESULT(MI_VDF_EnableSubWindow(g_VdfVgChn, 0, 0, 1));

    /************************************************
    step10 :get venc input port Yuv420sp
    *************************************************/
    g_stVencOutputFileAttr.stModuleInfo.eModId = E_MI_MODULE_ID_VENC;
    g_stVencOutputFileAttr.stModuleInfo.u32DevId = s32VencDevId;
    g_stVencOutputFileAttr.stModuleInfo.u32ChnId = s32VencChnId;
    g_stVencOutputFileAttr.stModuleInfo.u32PortId = 0;
    g_stVencOutputFileAttr.bThreadExit = FALSE;
    sprintf(g_stVencOutputFileAttr.FilePath, "./out/vdf/vg_demo_case%d", g_u8CmdIndex);
    g_stVencOutputFileAttr.u16DumpBuffNum = 60;

    ST_Common_CheckResult(&g_stVencOutputFileAttr);

    g_stIspInputFileAttr.stModuleInfo.eModId = E_MI_MODULE_ID_ISP;
    g_stIspInputFileAttr.stModuleInfo.u32DevId = u32IspDevId;
    g_stIspInputFileAttr.stModuleInfo.u32ChnId = u32IspChnId;
    g_stIspInputFileAttr.stModuleInfo.u32PortId = 0;
    g_stIspInputFileAttr.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
    g_stIspInputFileAttr.u32Width = INPUT_WIDTH;
    g_stIspInputFileAttr.u32Height = INPUT_HEIGHT;
    g_stIspInputFileAttr.bThreadExit = FALSE;
    g_stIspInputFileAttr.bInputOnce = TRUE;
    g_stIspInputFileAttr.u32Fps  = 15;
    sprintf(g_stIspInputFileAttr.InputFilePath, INPUT_PATH);
    pthread_create(&g_stIspInputFileAttr.pPutDatathread, NULL, ST_Common_PutInputDataThread, (void *)&g_stIspInputFileAttr);

    g_pThreadVgState = pthread_create(&g_pThreadVg, NULL, ST_VG_Task, NULL);
    if (0 == g_pThreadVgState)
    {
        pthread_setname_np(g_pThreadVg, "ST_VG_Task");
    }
    else
    {
#ifndef __MUSL__
        g_pThreadVg = -1;
        printf("ST_VG_Initial, create g_pthread_vg=%ld is err[%d]!\n %s\n", g_pThreadVg, g_pThreadVgState,
                  strerror(g_pThreadVgState));
#else
        g_pThreadVg = NULL;
#endif
        return -1;
    }

    ST_Common_WaitDumpFinsh(&g_stVencOutputFileAttr);

    return MI_SUCCESS;
}

static MI_S32 STUB_BaseModuleDeInit(void)
{
    MI_SYS_ChnPort_t   stSrcChnPort;
    MI_SYS_ChnPort_t   stDstChnPort;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 0;

    MI_U32 u32SclDevId = 0;
    MI_U32 u32SclChnId = 0;
    MI_U32 u32SclPortId0 = 0;
    MI_U32 u32SclPortId1 = 1;

    MI_U32 u32VencDevId = 0;
    MI_U32 u32VencChnId = 0;

    /************************************************
    step1 :deinit Yuv thread
    *************************************************/
    g_stIspInputFileAttr.bThreadExit = TRUE;
    pthread_join(g_stIspInputFileAttr.pPutDatathread, NULL);
    g_bVgExit = TRUE;
    pthread_join(g_pThreadVg, NULL);

    /************************************************
    step2 :deinit rgn
    *************************************************/
    for(int Index = 0 ; Index < WINDOW_INDEX_CNT_MAX; Index++ )
    {
        STCHECKRESULT(ST_Common_RgnDetachChn(0, g_hRgnHandleCover[Index], &g_stRgnChnPort));
        STCHECKRESULT(ST_Common_RgnDestroy(0, g_hRgnHandleCover[Index]));
    }
    STCHECKRESULT(ST_Common_RgnDetachChn(0, 6, &g_stRgnChnPort));
    STCHECKRESULT(ST_Common_RgnDestroy(0, 6));

    STCHECKRESULT(ST_Common_RgnDetachChn(0, 7, &g_stRgnChnPort));
    STCHECKRESULT(ST_Common_RgnDestroy(0, 7));

    STCHECKRESULT(ST_Common_RgnDeInit(0));

    /************************************************
    step3 :unbind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId0;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32VencChnId;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step4 :deinit venc
    *************************************************/
    STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId));
    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    step5 :unbind scl->vdf
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnId;
    stSrcChnPort.u32PortId = u32SclPortId1;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VDF;
    stDstChnPort.u32DevId  = 0;
    stDstChnPort.u32ChnId  = 0;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step6 :deinit vdf
    *************************************************/
    STCHECKRESULT(MI_VDF_EnableSubWindow(g_VdfVgChn, 0, 0, 0));
    STCHECKRESULT(MI_VDF_Stop(E_MI_VDF_WORK_MODE_VG));
    STCHECKRESULT(MI_VDF_DestroyChn(g_VdfVgChn));
    STCHECKRESULT(MI_VDF_Uninit());

    /************************************************
    step7 :unbind isp->scl
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = 0;
    stSrcChnPort.u32ChnId  = 0;
    stSrcChnPort.u32PortId = 0;
    stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId  = u32SclDevId;
    stDstChnPort.u32ChnId  = u32SclChnId;
    stDstChnPort.u32PortId = u32SclPortId0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    step8 :deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId0));
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId, u32SclPortId1));
    STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId));
    STCHECKRESULT(ST_Common_SclDestroyDevice(u32SclDevId));

    /************************************************
    step9 :deinit isp
    *************************************************/
    STCHECKRESULT(ST_Common_IspDisablePort(u32IspDevId, u32IspChnId, u32IspPortId));
    STCHECKRESULT(ST_Common_IspStopChn(u32IspDevId, u32IspChnId));
    STCHECKRESULT(ST_Common_IspDestroyDevice(u32IspDevId));

    /************************************************
    step10 :sys exit
    *************************************************/
    fclose(g_pOutFile);
    STCHECKRESULT(ST_Common_Sys_Exit());

    return MI_SUCCESS;
}

void ST_Vdf_Usage(void)
{
    printf("Usage:./prog_vdf_vg_demo) file -> isp -> scl -> venc(rgn attach cover Frame from vdf_vg) -> file\n");
}

MI_S32 main(int argc, char **argv)
{
    ST_Vdf_Usage();
    STCHECKRESULT(STUB_BaseModuleInit());
    STCHECKRESULT(STUB_BaseModuleDeInit());

    return 0;
}
