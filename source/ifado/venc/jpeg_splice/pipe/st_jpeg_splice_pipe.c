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
#include "st_common.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_venc.h"
#include "st_common_jpeg.h"

#define OUTPUT_PATH             "./out/"
#define PIPELINE_FPS            30

typedef struct ST_SensorSize_s
{
    MI_U16 u16Width;
    MI_U16 u16Height;
} ST_SensorSize_t;

static MI_U8           gu8SensorIndex;
static MI_U16          gu16SaveImgNum;
static ST_SensorSize_t gstSensorSize;
static char            IqBinPath[128];
static MI_BOOL         bVencDumpSmallDone = FALSE;
static MI_BOOL         bVencDumpSpliceDone = FALSE;

pthread_t g_pThreadIsp;
MI_BOOL g_bThreadExitIsp = FALSE;
pthread_t g_pThreadVenc;
MI_BOOL g_bThreadExitVenc = FALSE;
pthread_t g_pThreadSplice;
MI_BOOL g_bThreadExitSplice = FALSE;

pthread_mutex_t ListMutexVenc = PTHREAD_MUTEX_INITIALIZER;

void *ST_ISP_Task()
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U16 u16SclWidth,u16SclHeight;
    MI_U32 u32IspStride;

    MI_U8  u8IspDev = 0x0;
    MI_S32 s32IspChn = 0;

    MI_U32 u32SclDevId          = 1;
    MI_U32 u32SclChnId[4]       = {0,1,2,3};
    MI_U32 u32SclChnSmallId     = 4;
    MI_U32 u32SclPortId         = 0;

    struct timeval tv;
    MI_S32 s32FdIspPort1 = -1;
    fd_set ReadFdsIspPort1;

    //*******isp port1*********
    MI_SYS_ChnPort_t stIspChnPort1;
    MI_SYS_BufInfo_t stIspBufInfo;
    MI_SYS_BUF_HANDLE hIspBufHandle;

    memset(&stIspChnPort1, 0x0, sizeof(MI_SYS_ChnPort_t));

    stIspChnPort1.eModId = E_MI_MODULE_ID_ISP;
    stIspChnPort1.u32DevId = u8IspDev;
    stIspChnPort1.u32ChnId = s32IspChn;
    stIspChnPort1.u32PortId = 0x1;

    MI_SYS_SetChnOutputPortDepth(0, &stIspChnPort1, 4, 5);

    if (MI_SUCCESS != (s32Ret = MI_SYS_GetFd(&stIspChnPort1, &s32FdIspPort1)))
    {
        printf("GET isp Fd Failed , isp Chn = %d\n", s32IspChn);
        return NULL;
    }

    ///*******g_bThreadExitIsp*********//
    while (g_bThreadExitIsp == FALSE)
    {
        FD_ZERO(&ReadFdsIspPort1);
        FD_SET(s32FdIspPort1, &ReadFdsIspPort1);
        tv.tv_sec = 2;
        s32Ret = select(s32FdIspPort1 + 1, &ReadFdsIspPort1, NULL, NULL, &tv);
        if (s32Ret < 0)
        {
            printf("isp chn:%d select failed\n", s32IspChn);
            sleep(1);
            MI_SYS_ChnOutputPortPutBuf(hIspBufHandle);
            break;
        }
        else if (0 == s32Ret)
        {
            printf("isp chn:%d select timeout\n", s32IspChn);
            MI_SYS_ChnOutputPortPutBuf(hIspBufHandle);
            break;
        }
        else
        {
            memset(&stIspBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
            if(MI_SUCCESS == MI_SYS_ChnOutputPortGetBuf(&stIspChnPort1, &stIspBufInfo, &hIspBufHandle))
            {
                MI_SYS_ChnPort_t stSclChnInput;
                MI_SYS_BufConf_t stSclBufConf;
                MI_SYS_BufInfo_t stSclBufInfo;
                MI_SYS_BUF_HANDLE hSclHandle = 0;

                memset(&stSclChnInput, 0x0, sizeof(MI_SYS_ChnPort_t));
                memset(&stSclBufConf, 0x0, sizeof(MI_SYS_BufConf_t));
                memset(&stSclBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));

                struct timeval stTv;
                u16SclWidth  = stIspBufInfo.stFrameData.u16Width  / 2;
                u16SclHeight = stIspBufInfo.stFrameData.u16Height / 2;

                u32IspStride = +stIspBufInfo.stFrameData.u32Stride[0];

                for(int i = 0 ; i<4 ; i++)
                {
                    stSclChnInput.eModId = E_MI_MODULE_ID_SCL;
                    stSclChnInput.u32DevId = u32SclDevId;
                    stSclChnInput.u32ChnId = u32SclChnId[i];
                    stSclChnInput.u32PortId = u32SclPortId;

                    stSclBufConf.eBufType                   = E_MI_SYS_BUFDATA_FRAME;
                    gettimeofday(&stTv, NULL);
                    stSclBufConf.u64TargetPts               = stTv.tv_sec*1000000 + stTv.tv_usec;
                    stSclBufConf.stFrameCfg.eFormat         = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                    stSclBufConf.stFrameCfg.eFrameScanMode  = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
                    stSclBufConf.stFrameCfg.u16Width        = u16SclWidth;
                    stSclBufConf.stFrameCfg.u16Height       = u16SclHeight;
                    if(MI_SUCCESS  == MI_SYS_ChnInputPortGetBuf(&stSclChnInput, &stSclBufConf, &stSclBufInfo, &hSclHandle,0))
                    {
                        MI_U8 * u8SclCopyDataY  = stSclBufInfo.stFrameData.pVirAddr[0];
                        MI_U8 * u8SclCopyDataUV = stSclBufInfo.stFrameData.pVirAddr[1];

                        MI_U8 * u8IspReadDataY  = stIspBufInfo.stFrameData.pVirAddr[0] + (i % 2 * u16SclWidth);
                        MI_U8 * u8IspReadDataUV = stIspBufInfo.stFrameData.pVirAddr[1] + (i % 2 * u16SclWidth);
                        if(i>1)
                        {
                            u8IspReadDataY  = u8IspReadDataY + (u16SclWidth * u16SclHeight * 2);
                            u8IspReadDataUV = u8IspReadDataUV + (u16SclWidth * u16SclHeight);
                        }

                        for(int row = 0 ; row < u16SclHeight ; row++)
                        {
                            memcpy(u8SclCopyDataY, u8IspReadDataY, u16SclWidth);
                            u8SclCopyDataY = u8SclCopyDataY + u16SclWidth;
                            u8IspReadDataY = u8IspReadDataY + u32IspStride;

                            if( (row + 1) % 2 == 0 )
                            {
                                memcpy(u8SclCopyDataUV, u8IspReadDataUV, u16SclWidth);
                                u8SclCopyDataUV = u8SclCopyDataUV + u16SclWidth;
                                u8IspReadDataUV = u8IspReadDataUV + u32IspStride;
                            }
                        }
                        MI_SYS_ChnInputPortPutBuf(hSclHandle,&stSclBufInfo, FALSE);
                    }
                }
                //copy 2560x1440 -> 640x360
                stSclChnInput.eModId = E_MI_MODULE_ID_SCL;
                stSclChnInput.u32DevId = u32SclDevId;
                stSclChnInput.u32ChnId = u32SclChnSmallId;
                stSclChnInput.u32PortId = u32SclPortId;

                stSclBufConf.eBufType                   = E_MI_SYS_BUFDATA_FRAME;
                gettimeofday(&stTv, NULL);
                stSclBufConf.u64TargetPts               = stTv.tv_sec*1000000 + stTv.tv_usec;
                stSclBufConf.stFrameCfg.eFormat         = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                stSclBufConf.stFrameCfg.eFrameScanMode  = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
                stSclBufConf.stFrameCfg.u16Width        = stIspBufInfo.stFrameData.u16Width;
                stSclBufConf.stFrameCfg.u16Height       = stIspBufInfo.stFrameData.u16Height;
                if(MI_SUCCESS  == MI_SYS_ChnInputPortGetBuf(&stSclChnInput, &stSclBufConf, &stSclBufInfo, &hSclHandle,0))
                {
                    MI_U8 * u8SclCopyDataY  = stSclBufInfo.stFrameData.pVirAddr[0];
                    MI_U8 * u8SclCopyDataUV = stSclBufInfo.stFrameData.pVirAddr[1];

                    MI_U8 * u8IspReadDataY  = stIspBufInfo.stFrameData.pVirAddr[0];
                    MI_U8 * u8IspReadDataUV = stIspBufInfo.stFrameData.pVirAddr[1];

                    for(int row = 0 ; row < stSclBufConf.stFrameCfg.u16Height ; row++)
                    {
                        memcpy(u8SclCopyDataY, u8IspReadDataY, stIspBufInfo.stFrameData.u16Width);
                        u8SclCopyDataY = u8SclCopyDataY + stIspBufInfo.stFrameData.u16Width;
                        u8IspReadDataY = u8IspReadDataY + u32IspStride;

                        if( (row + 1) % 2 == 0 )
                        {
                            memcpy(u8SclCopyDataUV, u8IspReadDataUV, stIspBufInfo.stFrameData.u16Width);
                            u8SclCopyDataUV = u8SclCopyDataUV + stIspBufInfo.stFrameData.u16Width;
                            u8IspReadDataUV = u8IspReadDataUV + u32IspStride;
                        }
                    }
                    MI_SYS_ChnInputPortPutBuf(hSclHandle,&stSclBufInfo, FALSE);
                }
                MI_SYS_ChnOutputPortPutBuf(hIspBufHandle);
            }
            else
            {
                printf("get 4k buffer fail.\n");
                MI_SYS_ChnOutputPortPutBuf(hIspBufHandle);
                continue;
            }
        }
    }
    printf("g_bThreadExitIsp == end!!!\n ");
    MI_SYS_CloseFd(s32FdIspPort1);
    FD_ZERO(&ReadFdsIspPort1);

    return NULL;
}

void *ST_Venc_Small_Out_Task()
{
    MI_U32 u32VencDevJpedId     = 8;        //MI_VENC_DEV_ID_JPEG_0
    MI_U32 u32VencChnSmallId    = 4;

    MI_S32                       s32Ret = MI_SUCCESS;
    MI_VENC_Stream_t             stStream;
    MI_VENC_ChnStat_t            stStat;
    MI_VENC_DEV                  VencDev = u32VencDevJpedId;
    MI_VENC_CHN                  VencChn = u32VencChnSmallId;
    MI_S32                       s32Fd   = -1;
    MI_U32                       S32getBufCnt=0,streamlen;
    struct timeval               tv;
    fd_set                       read_fds;
    MI_U32                       cnt = 0;
    FILE* pFile=NULL;
    char name_tmp[256] = {0};

    s32Fd = MI_VENC_GetFd(VencDev, VencChn);
    if (s32Fd < 0)
    {
        printf("GET JPEG Fd Failed , JPEG Dev = %d Chn = %d\n", VencDev, VencChn);
        return NULL;
    }

    while (g_bThreadExitVenc == FALSE)
    {
        if(cnt == gu16SaveImgNum)
        {
            bVencDumpSmallDone = TRUE;
            continue;
        }
        FD_ZERO(&read_fds);
        FD_SET(s32Fd, &read_fds);

        tv.tv_sec  = 0;
        tv.tv_usec = 500 * 1000;

        s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &tv);
        if (s32Ret < 0)
        {
            printf("VideoEncoder Stream_Task%d select failed\n", VencChn);
            continue;
        }
        else if (0 == s32Ret)
        {
            printf("VideoEncoder Stream_Task%d select time out\n", VencChn);
            continue;
        }
        else
        {
            FD_CLR(s32Fd, &read_fds);

            memset(&stStream, 0, sizeof(stStream));
            memset(&stStat, 0, sizeof(MI_VENC_ChnStat_t));
            s32Ret = MI_VENC_Query(VencDev, VencChn, &stStat);
            if (MI_SUCCESS != s32Ret || stStat.u32CurPacks == 0)
            {
                usleep(THREAD_SLEEP_TIME_US);
                continue;
            }
            stStream.pstPack = (MI_VENC_Pack_t*)malloc(sizeof(MI_VENC_Pack_t)*stStat.u32CurPacks);
            if (NULL == stStream.pstPack)
            {
                usleep(THREAD_SLEEP_TIME_US);
                continue;
            }
            stStream.u32PackCount = stStat.u32CurPacks;
            pthread_mutex_lock(&ListMutexVenc);
            s32Ret = MI_VENC_GetStream(VencDev, VencChn, &stStream, FALSE);
            if (MI_SUCCESS == s32Ret)
            {
                cnt++;
                sprintf(name_tmp, "./out/Output_640x360_%d.jpg", cnt);

                pFile = fopen(name_tmp,"wb");
                if (pFile == NULL)
                {
                    return NULL;
                }
                streamlen=0;
                for (MI_U32 i = 0; i < stStream.u32PackCount; i++)
                {
                    streamlen += stStream.pstPack[i].u32Len;
                }
                if(streamlen <= JPEG_SINGLE_MAX_BUFSIZE)
                {
                    for (MI_U32 i = 0; i < stStream.u32PackCount; i++)
                    {
                        fwrite(stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset,  stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset*sizeof(MI_U8), 1, pFile);
                    }
                    S32getBufCnt ++;
                }
                else
                {
                    ST_INFO("jpeg len %d,but buf malloc %d,skip jpeg frame!\n",streamlen,JPEG_SINGLE_MAX_BUFSIZE);
                }
                MI_VENC_ReleaseStream(VencDev, VencChn, &stStream);

            }
            pthread_mutex_unlock(&ListMutexVenc);
            free(stStream.pstPack);
            stStream.pstPack = NULL;
        }
        fclose(pFile);
    }
    printf("g_bThreadExitVenc == end!!!\n ");

    return NULL;
}

void *ST_Venc_Splice_Out_Task()
{
    MI_U32 u32VencDevJpedId      = 8;        //MI_VENC_DEV_ID_JPEG_0
    MI_U32 u32VencChnId[4]       = {0, 1, 2, 3};

    MI_S32                       s32Ret = MI_SUCCESS;
    MI_VENC_Stream_t             stStream;
    MI_VENC_ChnStat_t            stStat;
    MI_VENC_DEV                  VencDev = u32VencDevJpedId;
    MI_S32                       s32Fd[4]   = {-1, -1, -1, -1};
    MI_U32                       S32get4KBufCnt=0,offset,streamlen;
    struct timeval               tv;
    fd_set                       read_fds[4];
    MI_U32                       cnt = 0;
    FILE*                        pFile = NULL;
    char                         name_tmp[256] = {0};

    char *output;
    int OutputBuf_Offset;
    ST_JpegSpliceParam_t stJpegSpliceParam;
    MI_U32 u32streamlen[4];

    OutputBuf_Offset = 0;
    stJpegSpliceParam.stInputInfo.u32Width = 2560;
    stJpegSpliceParam.stInputInfo.u32Height = 1440;
    stJpegSpliceParam.stOutputInfo.u32SpliceBufNum = 4;

    for(int i = 0; i < 4; i++)
    {
        s32Fd[i] = MI_VENC_GetFd(VencDev, u32VencChnId[i]);
        if (s32Fd < 0)
        {
            printf("GET JPEG Fd Failed , JPEG Dev = %d Chn = %d\n", VencDev, u32VencChnId[i]);
            return NULL;
        }
    }

    while (g_bThreadExitSplice == FALSE)
    {
        if(cnt == gu16SaveImgNum)
        {
            bVencDumpSpliceDone = TRUE;
            continue;
        }
        S32get4KBufCnt=0;
        tv.tv_sec  = 0;
        tv.tv_usec = 500 * 1000;

        for(MI_U32 i=0;i<4;i++)
        {
            stJpegSpliceParam.stOutputInfo.pbuf[i] = (char *)malloc(JPEG_SINGLE_MAX_BUFSIZE);
            if(stJpegSpliceParam.stOutputInfo.pbuf[i]==NULL)
            {
                printf("error MALLOC Jped[%d] data fail\n",i);
                return NULL;
            }
        }
        for(int num = 0; num < 4; num++)
        {
            FD_ZERO(&read_fds[num]);
            FD_SET(s32Fd[num], &read_fds[num]);

            s32Ret = select(s32Fd[num] + 1, &read_fds[num], NULL, NULL, &tv);
            if (s32Ret < 0)
            {
                printf("VideoEncoder Stream_Task%d select failed\n", u32VencChnId[num]);
                continue;
            }
            else if (0 == s32Ret)
            {
                printf("VideoEncoder Stream_Task%d select time out\n", u32VencChnId[num]);
                continue;
            }
            else
            {
                FD_CLR(s32Fd[num], &read_fds[num]);

                memset(&stStream, 0, sizeof(stStream));
                memset(&stStat, 0, sizeof(MI_VENC_ChnStat_t));

                s32Ret = MI_VENC_Query(VencDev, u32VencChnId[num], &stStat);
                if (MI_SUCCESS != s32Ret || stStat.u32CurPacks == 0)
                {
                    usleep(THREAD_SLEEP_TIME_US);
                    continue;
                }
                stStream.pstPack = (MI_VENC_Pack_t*)malloc(sizeof(MI_VENC_Pack_t)*stStat.u32CurPacks);
                if (NULL == stStream.pstPack)
                {
                    usleep(THREAD_SLEEP_TIME_US);
                    continue;
                }
                stStream.u32PackCount = stStat.u32CurPacks;
                s32Ret = MI_VENC_GetStream(VencDev, u32VencChnId[num], &stStream, FALSE);
                if (MI_SUCCESS == s32Ret)
                {
                    streamlen=0;
                    for (MI_U32 i = 0; i < stStream.u32PackCount; i++)
                    {
                        streamlen += stStream.pstPack[i].u32Len;
                    }
                    if(streamlen <= JPEG_SINGLE_MAX_BUFSIZE)
                    {
                        offset = 0;
                        for (MI_U32 i = 0; i < stStream.u32PackCount; i++)
                        {
                            memcpy(stJpegSpliceParam.stOutputInfo.pbuf[num] + offset, stStream.pstPack[i].pu8Addr, stStream.pstPack[i].u32Len);
                            offset += stStream.pstPack[i].u32Len;
                        }
                        u32streamlen[num] = streamlen;
                        S32get4KBufCnt ++;
                    }
                    else
                    {
                        ST_INFO("jpeg len %d,but buf malloc %d,skip jpeg frame!\n",streamlen,JPEG_SINGLE_MAX_BUFSIZE);
                    }
                    MI_VENC_ReleaseStream(VencDev, u32VencChnId[num], &stStream);

                }//Getstream success
                free(stStream.pstPack);
                stStream.pstPack = NULL;
            }//select success
        }//for 4 picture
        if(S32get4KBufCnt != 4)
        {
            printf("MI_VENC_GetStream to get 4 4K buffer fail!\n");
        }
        else
        {
            cnt++;
            sprintf(name_tmp, "./out/Output_5120x2880_%d_z.jpg", cnt);

            pFile = fopen(name_tmp,"wb");
            if (pFile == NULL)
            {
                return NULL;
            }
            for(int i = 0; i<4; i++)
            {
                stJpegSpliceParam.stOutputInfo.au32bufLen[i] = u32streamlen[i];
            }
            output = (char *)malloc(JPEG_SINGLE_MAX_BUFSIZE * stJpegSpliceParam.stOutputInfo.u32SpliceBufNum);
            OutputBuf_Offset = 0;
            ST_Common_VencJpegSplice_Z(&stJpegSpliceParam, output, &OutputBuf_Offset);

            fwrite(output, OutputBuf_Offset, 1, pFile);
            free(output);
            fclose(pFile);
        }
        for (int i = 0; i < 4; i++)
        {
            free(stJpegSpliceParam.stOutputInfo.pbuf[i]);
        }

    }//while
    printf("g_bThreadExitVenc == end!!!\n ");
    return NULL;
}

static MI_S32 STUB_BaseModuleInit(MI_U8 u8SensorResTndex)
{
    MI_SNR_PlaneInfo_t  stPlaneInfo;
    MI_SYS_ChnPort_t    stSrcChnPort;
    MI_SYS_ChnPort_t    stDstChnPort;
    MI_U32              u32SrcFrmrate;
    MI_U32              u32DstFrmrate;
    MI_SYS_BindType_e   eBindType;
    MI_U32              u32BindParam;
    MI_U32              u32SnrPadId         = 0;
    MI_U8               u8SensorRes         = u8SensorResTndex;
    MI_U32              u32VifGroupId       = 0;
    MI_U32              u32VifDevId         = 0;
    MI_U32              u32VifPortId        = 0;

    MI_U32              u32IspDevId         = 0;
    MI_U32              u32IspChnId         = 0;
    MI_U32              u32IspPortId        = 1;

    MI_U32              u32SclDevId         = 1;
    MI_U32              u32SclChnId[4]      = {0,1,2,3};
    MI_U32              u32SclChnSmallId    = 4;
    MI_U32              u32SclPortId        = 0;

    MI_U32              u32VencDevId        = 8;
    MI_U32              u32VencChnId[4]     = {0,1,2,3};
    MI_U32              u32VencChnSmallId   = 4;

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
    STCHECKRESULT(ST_Common_SensorInit(u32SnrPadId, FALSE, u8SensorRes,0xFF));

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
    MI_ISP_DevAttr_t stIspDevAttr;
    MI_ISP_ChannelAttr_t stIspChnAttr;
    MI_ISP_ChnParam_t stIspChnParam;
    MI_SYS_WindowRect_t stIspInputCrop;
    MI_ISP_OutPortParam_t  stIspOutPortParam;

    ST_Common_GetIspDefaultDevAttr(&stIspDevAttr);
    STCHECKRESULT(ST_Common_IspCreateDevice(u32IspDevId, &stIspDevAttr));

    ST_Common_GetIspDefaultChnAttr(&stIspChnAttr, &stIspInputCrop, &stIspChnParam);
    // stIspChnParam.eRot = E_MI_SYS_ROTATE_180;
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
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = PIPELINE_FPS;
    u32DstFrmrate          = PIPELINE_FPS;
    eBindType              = E_MI_SYS_BIND_TYPE_REALTIME;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    MI_SYS_ChnPort_t stIspOutputPort;
    stIspOutputPort.eModId    = E_MI_MODULE_ID_ISP;
    stIspOutputPort.u32DevId  = u32IspDevId;
    stIspOutputPort.u32ChnId  = u32IspChnId;
    stIspOutputPort.u32PortId = u32IspPortId;
    MI_SYS_SetChnOutputPortUserFrc(&stIspOutputPort, PIPELINE_FPS, 3);

    /************************************************
    step7 :init scl
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

    for(int i=0 ; i<4; i++)
    {
        STCHECKRESULT(ST_Common_SclStartChn(u32SclDevId, u32SclChnId[i], &stSclChnAttr, &stSclInputCrop, &stSclChnParam));
    }
    STCHECKRESULT(ST_Common_SclStartChn(u32SclDevId, u32SclChnSmallId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

    /************************************************
    step9 :init venc
    *************************************************/
    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;

    ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
    STCHECKRESULT(ST_Common_VencCreateDev(u32VencDevId, &stVencInitParam));

    MI_VENC_ModType_e eType = (MI_VENC_ModType_e)E_MI_VENC_MODTYPE_JPEGE;

    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr);
    ST_Common_SetJpegSpliceChnAttr(eType, &stVencChnAttr);

    stVencChnAttr.stVeAttr.stAttrJpeg.u32PicWidth       = 2560;
    stVencChnAttr.stVeAttr.stAttrJpeg.u32PicHeight      = 1440;
    stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth    = 2560;
    stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight   = 1440;
    stVencChnAttr.stVeAttr.stAttrJpeg.u32BufSize        = 1024*1024;
    for(int i=0 ; i<4; i++)
    {
        STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnId[i], &stVencChnAttr));
    }
    stVencChnAttr.stVeAttr.stAttrJpeg.u32PicWidth       = 640;
    stVencChnAttr.stVeAttr.stAttrJpeg.u32PicHeight      = 360;
    stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth    = 640;
    stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight   = 360;
    STCHECKRESULT(ST_Common_VencStartChn(u32VencDevId, u32VencChnSmallId,   &stVencChnAttr));

    /************************************************
    step:bind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);
    stSclOutPortParam.stSCLOutputSize.u16Width  = gstSensorSize.u16Width;
    stSclOutPortParam.stSCLOutputSize.u16Height = gstSensorSize.u16Height;
    for(int i=0 ; i<4; i++)
    {
        stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId  = u32SclDevId;
        stSrcChnPort.u32ChnId  = u32SclChnId[i];
        stSrcChnPort.u32PortId = u32SclPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = u32VencDevId;
        stDstChnPort.u32ChnId  = u32VencChnId[i];
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate          = 3;
        u32DstFrmrate          = 3;
        eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
        u32BindParam           = 0;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));
        STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnId[i], u32SclPortId, &stSclOutPortParam));
    }
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnSmallId;
    stSrcChnPort.u32PortId = u32SclPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32VencChnSmallId;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 3;
    u32DstFrmrate          = 3;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));
    stSclOutPortParam.stSCLOutputSize.u16Width  = 640;
    stSclOutPortParam.stSCLOutputSize.u16Height = 360;
    STCHECKRESULT(ST_Common_SclEnablePort(u32SclDevId, u32SclChnSmallId, u32SclPortId, &stSclOutPortParam));

    return MI_SUCCESS;
}

static MI_S32 STUB_BaseModuleDeInit()
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    MI_U32             u32SnrPadId   = 0;
    MI_U32             u32VifGroupId = 0;
    MI_U32             u32VifDevId   = 0;
    MI_U32             u32VifPortId  = 0;

    MI_U32 u32IspDevId  = 0;
    MI_U32 u32IspChnId  = 0;
    MI_U32 u32IspPortId = 1;

    MI_U32 u32SclDevId          = 1;
    MI_U32 u32SclChnId[4]       = {0,1,2,3};
    MI_U32 u32SclChnSmallId     = 4;
    MI_U32 u32SclPortId         = 0;

    MI_U32 u32VencDevId         = 8;
    MI_U32 u32VencChnId[4]      = {0,1,2,3};
    MI_U32 u32VencChnSmallId    = 4;

    /************************************************
    step1 :unbind scl->venc
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stSrcChnPort.u32DevId  = u32SclDevId;
    stSrcChnPort.u32ChnId  = u32SclChnSmallId;
    stSrcChnPort.u32PortId = u32SclPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId  = u32VencDevId;
    stDstChnPort.u32ChnId  = u32VencChnSmallId;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    for(int i=0 ; i<4; i++)
    {
        stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId  = u32SclDevId;
        stSrcChnPort.u32ChnId  = u32SclChnId[i];
        stSrcChnPort.u32PortId = u32SclPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = u32VencDevId;
        stDstChnPort.u32ChnId  = u32VencChnId[i];
        stDstChnPort.u32PortId = 0;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));
    }

    /************************************************
    step2 :deinit venc
    *************************************************/
    STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnSmallId));
    for(int i=0 ; i<4; i++)
    {
        STCHECKRESULT(ST_Common_VencStopChn(u32VencDevId, u32VencChnId[i]));
    }
    STCHECKRESULT(ST_Common_VencDestroyDev(u32VencDevId));

    /************************************************
    step4 :deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnSmallId, u32SclPortId));
    STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnSmallId));
    for(int i=0 ; i<4; i++)
    {
        STCHECKRESULT(ST_Common_SclDisablePort(u32SclDevId, u32SclChnId[i], u32SclPortId));
        STCHECKRESULT(ST_Common_SclStopChn(u32SclDevId, u32SclChnId[i]));
    }
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

void ST_Venc_SplicePipe_Usage(void)
{
    printf("Usage:./prog_venc_jpeg_splice_pipe index x iqbin xxx.bin savenum xx)set sensor Resindex and iqbin, and save xx images \n");
}

MI_S32  ST_Venc_SplicePipe_GetCmdlineParam(int argc, char **argv)
{
    gu8SensorIndex = 0xFF; // default user input
    gu16SaveImgNum = 50;
    memset(IqBinPath, 0, 128);
    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "index"))
        {
            gu8SensorIndex = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "iqbin"))
        {
            strcpy(IqBinPath, argv[i + 1]);
        }
        if (0 == strcmp(argv[i], "savenum"))
        {
            gu16SaveImgNum = atoi(argv[i + 1]);
        }
    }
    return MI_SUCCESS;
}

void STUB_Common_Pause(void)
{
    while(!((bVencDumpSmallDone == TRUE) && (bVencDumpSpliceDone == TRUE)))
    {
        continue;
    }
    printf("Have Saved %d sets of images\n",gu16SaveImgNum);
}

static MI_S32 ST_VencSplicePipeline_Preview()
{
    MI_U32               u32IspDevId  = 0;
    MI_U32               u32IspChnId  = 0;
    MI_U32               u32SensorPad = 0;
    char                 IqApiBinFilePath[128];
    MI_SNR_PlaneInfo_t   stPlaneInfo;

    STCHECKRESULT(STUB_BaseModuleInit(gu8SensorIndex));

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

    ST_Common_CheckMkdirOutFile(OUTPUT_PATH);

    pthread_create(&g_pThreadIsp, NULL, ST_ISP_Task, NULL);
    pthread_create(&g_pThreadVenc, NULL, ST_Venc_Small_Out_Task, NULL);
    pthread_create(&g_pThreadSplice, NULL, ST_Venc_Splice_Out_Task, NULL);

    STUB_Common_Pause();

    g_bThreadExitIsp = TRUE;
    pthread_join(g_pThreadIsp, NULL);

    g_bThreadExitVenc = TRUE;
    pthread_join(g_pThreadVenc, NULL);

    g_bThreadExitSplice = TRUE;
    pthread_join(g_pThreadSplice, NULL);

    STCHECKRESULT(STUB_BaseModuleDeInit());

    return MI_SUCCESS;
}

MI_S32 main(int argc, char **argv)
{
    if (argc < 1)
    {
        ST_Venc_SplicePipe_Usage();
        return -1;
    }

     ST_Venc_SplicePipe_GetCmdlineParam(argc, argv);

    STCHECKRESULT(ST_VencSplicePipeline_Preview());
    memset(&gstSensorSize, 0, sizeof(ST_SensorSize_t));
    return 0;
}