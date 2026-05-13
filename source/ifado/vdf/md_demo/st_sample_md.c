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

#define WINDOW_INDEX_CNT_MAX        8
#define RGN_FRAME_TOTAL_WIDTH       8192
#define RGN_FRAME_TOTAL_HEIGHT      8192
#define INPUT_WIDTH                 640
#define INPUT_HEIGHT                360
#define INPUT_PATH                  "./resource/input/md_test_640x360_422yuyv.yuv"
#define OUTPUT_CNT                  30

extern int pthread_setname_np(pthread_t thread, const char *name);
static ST_Common_InputFile_Attr_t g_stIspInputFileAttr;
static ST_Common_OutputFile_Attr_t g_stVencOutputFileAttr;

static MI_U8            g_u8CmdIndex = 0;
static MI_RGN_HANDLE    g_hRgnHandleCover[WINDOW_INDEX_CNT_MAX];
static MI_RGN_ChnPort_t g_stRgnChnPort;

MI_VDF_ChnAttr_t        g_stVdfChnAttr;
static MI_VDF_CHANNEL   g_VdfMdChn = VDF_MD_CHN_NO;
static MI_BOOL          g_bMdExit = FALSE;
static MI_S32           g_pThreadMdState = -1;
static pthread_t        g_pThreadMd;

static MI_U16           g_u16ImageWidth = INPUT_WIDTH;
static MI_U16           g_u16ImageHeight = INPUT_HEIGHT;

static FILE             *g_pOutFile;
char                    g_strMdResult[50];
MI_U32                  g_u32DumpLogCnt = 0;





MI_S32 ST_Common_RgnInitFrameWindows_Max(MI_RGN_ChnPort_t* stRgnChnPort)
{
    int i;
    MI_S32 s32Ret;
    MI_RGN_Attr_t stRegionAttr;
    ST_Common_GetRgnDefaultCreateAttr(&stRegionAttr);
    stRegionAttr.eType = E_MI_RGN_TYPE_FRAME;

    MI_U8 u8RgnFrameWidth     =   40;
    MI_U8 u8RgnFrameHeight    =   40;
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
    return MI_SUCCESS;
}


MI_S32 MultiMDtoRECT(MI_U8 u8VdfChn, MI_U8 *pu8MdRstData, MI_U8 col, MI_U8 row)
{
    typedef struct {
    MI_U8 value;
    MI_U8 row;
    MI_U8 col;
    } stMulti5x5Rect;

    MI_S32 s32Ret;
    MI_RGN_ChnPortParam_t stTmpRgnChnPortParam;
    MI_RGN_ChnPort_t stTmpRgnChnPort;
    memset(&stTmpRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    stTmpRgnChnPort.eModId = E_MI_MODULE_ID_VENC;
    stTmpRgnChnPort.s32DevId = 0;
    stTmpRgnChnPort.s32ChnId = 0;
    stTmpRgnChnPort.s32PortId = 0;

    MI_U8 u8RgnFrameWidth     =   40;
    MI_U8 u8RgnFrameHeight    =   40;

    printf("chn:%d, row %d, col %d\n", u8VdfChn, row, col);
    int i, j;
    int Cnt_5x5[9][16] = { {0,}, };
    MI_U8 i_5x5, j_5x5;

    for (i = 0; i < 9; i++)           // 45/5 = 9
    {
        for (j = 0; j < 16; j++)       // 80/5 = 16
        {
            for(i_5x5 = 0; i_5x5 < 5; i_5x5++)
            {
                for(j_5x5 = 0; j_5x5 < 5; j_5x5++)
                {
                    if(pu8MdRstData[( i * 5 + i_5x5 ) * col + (j * 5 + j_5x5)] != 0)
                    {
                        Cnt_5x5[i][j]++;
                    }
                }
            }
        }
    }

    //compare to RGN
    stMulti5x5Rect stMaxNumbers[WINDOW_INDEX_CNT_MAX];
    for (i = 0; i < WINDOW_INDEX_CNT_MAX; i++)
    {
        stMaxNumbers[i].value = Cnt_5x5[0][0];
        stMaxNumbers[i].row = 0;
        stMaxNumbers[i].col = 0;
    }
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 16; j++)
        {
            int minValue = stMaxNumbers[0].value;
            int minIndex = 0;
            for (int k = 1; k < WINDOW_INDEX_CNT_MAX; k++)
            {
                if (stMaxNumbers[k].value < minValue)
                {
                    minValue = stMaxNumbers[k].value;
                    minIndex = k;
                }
            }
            if (Cnt_5x5[i][j] > minValue)
            {
                stMaxNumbers[minIndex].value = Cnt_5x5[i][j];
                stMaxNumbers[minIndex].row = i;
                stMaxNumbers[minIndex].col = j;
            }
        }
    }

    //set to RGN
    for (i = 0; i < WINDOW_INDEX_CNT_MAX; i++)
    {
        if(stMaxNumbers[i].value < 1)
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
        if(stMaxNumbers[i].value > 0)
        {
            s32Ret = MI_RGN_GetDisplayAttr(0, g_hRgnHandleCover[i], &stTmpRgnChnPort, &stTmpRgnChnPortParam);
            if(s32Ret < 0)
            {
                printf("MI_RGN_GetDisplayAttr err!!!\n");
                return -1;
            }
            stTmpRgnChnPortParam.bShow = 1;
            stTmpRgnChnPortParam.stFrameChnPort.stRect.s32X = stMaxNumbers[i].col * u8RgnFrameWidth * RGN_FRAME_TOTAL_WIDTH / g_u16ImageWidth;
            stTmpRgnChnPortParam.stFrameChnPort.stRect.s32Y = stMaxNumbers[i].row * u8RgnFrameHeight * RGN_FRAME_TOTAL_HEIGHT / g_u16ImageHeight;
            stTmpRgnChnPortParam.stFrameChnPort.stRect.u32Width  = u8RgnFrameWidth * RGN_FRAME_TOTAL_WIDTH / g_u16ImageWidth;
            stTmpRgnChnPortParam.stFrameChnPort.stRect.u32Height = u8RgnFrameHeight * RGN_FRAME_TOTAL_HEIGHT / g_u16ImageHeight;
            s32Ret = MI_RGN_SetDisplayAttr(0, g_hRgnHandleCover[i], &stTmpRgnChnPort, &stTmpRgnChnPortParam);
            if(s32Ret < 0)
            {
                printf("MI_RGN_SetDisplayAttr err!!!\n");
                return -1;
            }
        }
    }


    sprintf(g_strMdResult, "A large matrix(%d) composed of 5x5 matrices:\n", g_u32DumpLogCnt+1);
    fprintf(g_pOutFile, "%s", g_strMdResult);
    for(i = 0; i < WINDOW_INDEX_CNT_MAX; i++)
    {
        sprintf(g_strMdResult, "([%d,%d]: %d), ", stMaxNumbers[i].col, stMaxNumbers[i].row, stMaxNumbers[i].value);
        fprintf(g_pOutFile, "%s", g_strMdResult);
    }
    sprintf(g_strMdResult, "\n");
    fprintf(g_pOutFile, "%s", g_strMdResult);

    g_u32DumpLogCnt++;

    return 0;
}

void *ST_MD_Task(void *arg)
{
    MI_U32 col = 0;
    MI_U32 row = 0;
    MI_U32 buffer_size = 0;

    MI_U8 u8VdfChn = 0;
    MI_U8 u8VdfCount = 0;
    MI_MD_static_param_t *pstMdStaticParamsIn = &g_stVdfChnAttr.stMdAttr.stMdStaticParamsIn;
    MI_U8 *pstSadDataArry = NULL;
    if (pstMdStaticParamsIn->mb_size == MDMB_MODE_MB_8x8)
    {
        col = INPUT_WIDTH >> 3;    // 45
        row = INPUT_HEIGHT >> 3;    // 80
    }

    buffer_size = col * row;
    if (pstMdStaticParamsIn->sad_out_ctrl == MDSAD_OUT_CTRL_16BIT_SAD)
    {
        buffer_size *= 2;
    }
    pstSadDataArry = (MI_U8 *)malloc(buffer_size + 1);
    if (NULL == pstSadDataArry)
    {
        printf("can not malloc pstSadDataArry buf.\n");
        goto exit;
    }
    memset(pstSadDataArry, 0x0, buffer_size + 1);
    printf("enter md task loop, md model is %d\n", MDALG_MODE_SAD);

    while (!g_bMdExit)
    {
        MI_S32 s32Ret = 0;
        MI_U8 *pu8MdRstData = NULL;
        MI_VDF_Result_t stVdfResult = {0};

        memset(&stVdfResult, 0x00, sizeof(MI_VDF_Result_t));
        stVdfResult.enWorkMode = E_MI_VDF_WORK_MODE_MD;
        s32Ret = MI_VDF_GetResult(g_VdfMdChn, &stVdfResult, 0);
        if (MI_SUCCESS == s32Ret)
        {
            if ((1 == stVdfResult.stMdResult.u8Enable) && (u8VdfCount < OUTPUT_CNT))
            {
                pu8MdRstData = (MI_U8 *)stVdfResult.stMdResult.pstMdResultStatus->paddr;
                memcpy(pstSadDataArry, pu8MdRstData, buffer_size);

                MI_VDF_PutResult(g_VdfMdChn, &stVdfResult);

                s32Ret = MultiMDtoRECT(u8VdfChn, pstSadDataArry, col, row);
                if(s32Ret != 0)
                {
                    ST_ERR("Rgn DisplayAttr err, MDtoRECT fail!\n");
                    continue;
                }
                else
                {
                    u8VdfCount++;
                }
            }
            else
            {
                MI_VDF_PutResult(g_VdfMdChn, &stVdfResult);
            }
        }
    }
exit:
    if (NULL != pstSadDataArry)
    {
        free(pstSadDataArry);
        pstSadDataArry = NULL;
    }
    printf("exit md task loop\n");

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
    char *output_file = "./out/vdf/md_result.txt";
    ST_Common_CheckMkdirOutFile(output_file);
    g_pOutFile = fopen(output_file, "w");
    if (g_pOutFile == NULL)
    {
        printf("Error: Can't open ./out/vdf/md_result.txt\n");
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
    MI_VENC_DEV                 u32VencDevId = 0;
    MI_VENC_CHN                 u32VencChnId = 0;
    MI_VENC_ModType_e           eType    = E_MI_VENC_MODTYPE_H264E;
    MI_VENC_InitParam_t         stVencParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;

    STCHECKRESULT(ST_Common_Sys_Init());
    ST_Common_GetVencDefaultDevAttr(&stVencParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencParam));
    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr);

    stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth         = g_u16ImageWidth;
    stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight        = g_u16ImageHeight;

    stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32Gop = 30;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = 15;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32IQp = 12;
    stVencChnAttr.stRcAttr.stAttrH264FixQp.u32PQp = 12;

    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId, &stVencChnAttr));

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
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32VencChnId;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 15;
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

    memset(&g_stRgnChnPort, 0x00, sizeof(MI_RGN_ChnPort_t));
    g_stRgnChnPort.eModId = E_MI_MODULE_ID_VENC;
    g_stRgnChnPort.s32DevId = u32VencDevId;
    g_stRgnChnPort.s32ChnId = u32VencChnId;
    g_stRgnChnPort.s32PortId = 0;

    ST_Common_Rgn_SetImageSize(g_u16ImageWidth, g_u16ImageHeight);

    STCHECKRESULT(ST_Common_GetRgnDefaultInitAttr(&stPaletteTable));
    STCHECKRESULT(ST_Common_RgnInit(u16SocId, &stPaletteTable));

    if(MI_SUCCESS != ST_Common_RgnInitFrameWindows_Max(&g_stRgnChnPort))
    {
        ST_ERR("Init Rgn Frame Windows failed \n");
        return -1;
    }

    /************************************************
    step8 :init VDF_MD
    *************************************************/
    STCHECKRESULT(MI_VDF_Init());

    ST_Common_GetVdfMdDefaultChnAttr(&g_stVdfChnAttr, g_u16ImageWidth, g_u16ImageHeight);
    if (0 != ST_Common_VdfCheckAlign(g_stVdfChnAttr.stMdAttr.stMdStaticParamsIn.mb_size, g_stVdfChnAttr.stMdAttr.stMdStaticParamsIn.stride))
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

    STCHECKRESULT(MI_VDF_CreateChn(g_VdfMdChn, &g_stVdfChnAttr));
    STCHECKRESULT(MI_VDF_Run(E_MI_VDF_WORK_MODE_MD));

    STCHECKRESULT(MI_VDF_EnableSubWindow(g_VdfMdChn, 0, 0, 1));

    /************************************************
    step10 :get venc input port Yuv420sp
    *************************************************/
    g_stVencOutputFileAttr.stModuleInfo.eModId = E_MI_MODULE_ID_VENC;
    g_stVencOutputFileAttr.stModuleInfo.u32DevId = u32VencDevId;
    g_stVencOutputFileAttr.stModuleInfo.u32ChnId = u32VencChnId;
    g_stVencOutputFileAttr.stModuleInfo.u32PortId = 0;
    g_stVencOutputFileAttr.bThreadExit = FALSE;
    sprintf(g_stVencOutputFileAttr.FilePath, "./out/vdf/md_demo_case%d", g_u8CmdIndex);
    g_stVencOutputFileAttr.u16DumpBuffNum = 30;

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

    g_pThreadMdState = pthread_create(&g_pThreadMd, NULL, ST_MD_Task, NULL);
    if (0 == g_pThreadMdState)
    {
        pthread_setname_np(g_pThreadMd, "ST_MD_Task");
    }
    else
    {
#ifndef __MUSL__
        g_pThreadMd = -1;
        ST_ERR("ST_MD_Initial, create g_pthread_md=%ld is err[%d]!\n %s\n", g_pThreadMd, g_pThreadMdState,
                  strerror(g_pThreadMdState));
#else
        g_pThreadMd = NULL;
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
    g_bMdExit = TRUE;
    pthread_join(g_pThreadMd, NULL);

    /************************************************
    step2 :deinit rgn
    *************************************************/
    MI_U8 WinIndex = WINDOW_INDEX_CNT_MAX;
    for(int Index = 0 ; Index < WinIndex; Index++ )
    {
        STCHECKRESULT(ST_Common_RgnDetachChn(0, g_hRgnHandleCover[Index], &g_stRgnChnPort));
        STCHECKRESULT(ST_Common_RgnDestroy(0, g_hRgnHandleCover[Index]));
    }
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
    STCHECKRESULT(MI_VDF_EnableSubWindow(g_VdfMdChn, 0, 0, 0));
    STCHECKRESULT(MI_VDF_Stop(E_MI_VDF_WORK_MODE_MD));
    STCHECKRESULT(MI_VDF_DestroyChn(g_VdfMdChn));
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
    printf("Usage:./prog_vdf_md_demo) file -> isp -> scl -> venc(rgn attach cover Frame from vdf_md) -> file\n");
}

MI_S32 main(int argc, char **argv)
{
    ST_Vdf_Usage();
    STCHECKRESULT(STUB_BaseModuleInit());
    STCHECKRESULT(STUB_BaseModuleDeInit());

    return 0;
}
