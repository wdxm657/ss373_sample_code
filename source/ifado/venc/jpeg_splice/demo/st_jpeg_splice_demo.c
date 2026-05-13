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
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "st_common.h"
#include "st_common_venc.h"
#include "st_common_jpeg.h"


static MI_U8 gu8CmdIndex;
static MI_U8 gu8DumpFlag = 0;
static ST_JpegSpliceParam_t gstJpegSpliceParam = {0};

static MI_U32 ST_JPEG_HORIZONTAL_SPLICE(ST_JpegSpliceParam_t *pJpegSpliceParam)
{
    MI_S32 s32Ret = MI_SUCCESS;
    char *output = (char *)malloc(JPEG_SINGLE_MAX_BUFSIZE * pJpegSpliceParam->stOutputInfo.u32SpliceBufNum);
    if (NULL == output)
    {
        printf("can not malloc output buf.\n");
        return -1;
    }
    int OutputBuf_Offset = 0;

    ST_Common_VencJpegSplice_HORIZONTAL(pJpegSpliceParam, output, &OutputBuf_Offset);

    s32Ret = ST_Common_WriteFile(pJpegSpliceParam->dumpSpliceOutPath, &pJpegSpliceParam->dumpSpliceOutfp, (MI_U8 *)output, OutputBuf_Offset, &gstJpegSpliceParam.u16DumpBuffNum, true);
    if(MI_SUCCESS != s32Ret)
    {
        printf("JPEG_HORIZONTAL_SPLICE->>>>WriteFile Fail \r\n");
    }
    free(output);
    return s32Ret;
}

static MI_S32 ST_JPEG_VERTICAL_SPLICE(ST_JpegSpliceParam_t *pJpegSpliceParam)
{
    MI_S32 s32Ret = MI_SUCCESS;
    char *output = (char *)malloc(JPEG_SINGLE_MAX_BUFSIZE * pJpegSpliceParam->stOutputInfo.u32SpliceBufNum);
    if (NULL == output)
    {
        printf("can not malloc output buf.\n");
        return -1;
    }
    int OutputBuf_Offset = 0;

    ST_Common_VencJpegSplice_VERTICAL(pJpegSpliceParam, output, &OutputBuf_Offset);


    s32Ret = ST_Common_WriteFile(pJpegSpliceParam->dumpSpliceOutPath, &pJpegSpliceParam->dumpSpliceOutfp, (MI_U8 *)output, OutputBuf_Offset, &gstJpegSpliceParam.u16DumpBuffNum, true);
    if(MI_SUCCESS != s32Ret)
    {
        printf("JPEG_VERTICAL_SPLICE->>>>WriteFile Fail \r\n");
    }
    free(output);
    return s32Ret;
}

static MI_S32 ST_JPEG_Z_SPLICE(ST_JpegSpliceParam_t *pJpegSpliceParam)
{
    MI_S32 s32Ret = MI_SUCCESS;
    char *output = (char *)malloc(JPEG_SINGLE_MAX_BUFSIZE * pJpegSpliceParam->stOutputInfo.u32SpliceBufNum);
    if (NULL == output)
    {
        printf("can not malloc output buf.\n");
        return -1;
    }
    int OutputBuf_Offset = 0;

    s32Ret = ST_Common_VencJpegSplice_Z(pJpegSpliceParam, output, &OutputBuf_Offset);
    if(MI_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    s32Ret = ST_Common_WriteFile(pJpegSpliceParam->dumpSpliceOutPath, &pJpegSpliceParam->dumpSpliceOutfp, (MI_U8 *)output, OutputBuf_Offset, &gstJpegSpliceParam.u16DumpBuffNum, true);
    if(MI_SUCCESS != s32Ret)
    {
        printf("JPEG_Z_SPLICE->>>>WriteFile Fail \r\n");
    }
    free(output);
    return s32Ret;
}
static MI_U32 ST_Common_JPEG_SPLICE(ST_JpegSpliceParam_t *pJpegSpliceParam)
{
    MI_U32 ret = MI_SUCCESS;
    MI_U16 dump_number = 1;
    MI_S32 i           = 0;
    char dump_name[256] = {0};
    switch(pJpegSpliceParam->eSpliceMode)
    {
        case E_JPEG_SPLICE_MODE_HORIZONTAL:
            {
                ret |= ST_JPEG_HORIZONTAL_SPLICE(pJpegSpliceParam);
            }
            break;
        case E_JPEG_SPLICE_MODE_VERTICAL:
            {
                ret |= ST_JPEG_VERTICAL_SPLICE(pJpegSpliceParam);
            }
            break;
        case E_JPEG_SPLICE_MODE_Z:
            {
                ret |= ST_JPEG_Z_SPLICE(pJpegSpliceParam);
            }
            break;
        default:
            ST_ERR("spilice_mode %d not support\n",pJpegSpliceParam->eSpliceMode);
            ret = -1;
            break;
    }
    if(1 == gu8DumpFlag)
    {
        for (i = 0; i < 4; i++)
        {
            if(0 == dump_number)
            {
                dump_number = 1;
            }
            sprintf(dump_name, "%s_%d.jpeg", gstJpegSpliceParam.dumpJpegOutPath, i);
            STCHECKRESULT(ST_Common_WriteFile(dump_name, &pJpegSpliceParam->dumpJpegOutfp, (MI_U8 *)pJpegSpliceParam->stOutputInfo.pbuf[i], pJpegSpliceParam->stOutputInfo.au32bufLen[i], &dump_number, true));
        }
        if(gstJpegSpliceParam.dumpJpegOutfp!=NULL)
        {
            STCHECKRESULT(ST_Common_CloseSourceFile(&gstJpegSpliceParam.dumpJpegOutfp));
        }
    }
    return ret;
}

void *ST_JPEG_GetOutputThread(void *args)
{
    MI_S32                       s32Ret = MI_SUCCESS;
    MI_VENC_Stream_t             stStream;
    ST_JPEG_Output_Attr_t *      pstOutputInfo = ((ST_JPEG_Output_Attr_t *)(args));
    MI_VENC_ChnStat_t            stStat;
    MI_VENC_DEV                  VencDev = pstOutputInfo->stModuleInfo.u32DevId;
    MI_VENC_CHN                  VencChn = pstOutputInfo->stModuleInfo.u32ChnId;
    MI_S32                       s32Fd   = -1;
    MI_U32                       S32getBufCnt=0,offset,streamlen;
    struct timeval               tv;
    fd_set                       read_fds;

    if (s32Ret != MI_SUCCESS)
    {
        printf("channel %d, not valid \n", VencChn);
        return NULL;
    }

    s32Fd = MI_VENC_GetFd(VencDev, VencChn);
    if (s32Fd < 0)
    {
        printf("GET JPEG Fd Failed , JPEG Dev = %d Chn = %d\n", VencDev, VencChn);
        return NULL;
    }

    ST_INFO("[%s] JPEG Fd = %d, JPEG Dev = %d Chn = %d\n ", __FUNCTION__, s32Fd, VencDev, VencChn);

    while (!pstOutputInfo->bThreadExit)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32Fd, &read_fds);

        tv.tv_sec  = 0;
        tv.tv_usec = 500 * 1000;

        s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &tv);
        if (s32Ret < 0)
        {
            printf("VideoEncoder Stream_Task%d select failed\n", VencChn);
            usleep(THREAD_SLEEP_TIME_US);
            continue;
        }
        else if (0 == s32Ret)
        {
            usleep(THREAD_SLEEP_TIME_US);
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
            pthread_mutex_lock(&pstOutputInfo->Outputmutex);
            s32Ret = MI_VENC_GetStream(VencDev, VencChn, &stStream, FALSE);
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
                        memcpy(pstOutputInfo->pbuf[S32getBufCnt] + offset,stStream.pstPack[i].pu8Addr, stStream.pstPack[i].u32Len);
                        offset += stStream.pstPack[i].u32Len;
                    }
                    pstOutputInfo->au32bufLen[S32getBufCnt] = streamlen;
                    S32getBufCnt ++;
                }
                else
                {
                    ST_INFO("jpeg len %d,but buf malloc %d,skip jpeg frame!\n",streamlen,JPEG_SINGLE_MAX_BUFSIZE);
                }
                MI_VENC_ReleaseStream(VencDev, VencChn, &stStream);

                if(S32getBufCnt==pstOutputInfo->u32SpliceBufNum)
                {
                    S32getBufCnt = 0;
                    ST_Common_JPEG_SPLICE(&gstJpegSpliceParam);
                }
            }
            pthread_mutex_unlock(&pstOutputInfo->Outputmutex);
            free(stStream.pstPack);
            stStream.pstPack = NULL;
        }
    }
    if(gstJpegSpliceParam.dumpSpliceOutfp != NULL)
    {
        ST_Common_CloseSourceFile(&gstJpegSpliceParam.dumpSpliceOutfp);
    }
    MI_VENC_CloseFd(VencDev, VencChn);
    FD_ZERO(&read_fds);

    return NULL;
}

static MI_S32 ST_MoudleInit()
{
    MI_VENC_DEV                 s32DevId = gstJpegSpliceParam.stInputInfo.stModuleInfo.u32DevId;
    MI_VENC_CHN                 s32ChnId = gstJpegSpliceParam.stInputInfo.stModuleInfo.u32ChnId;
    MI_VENC_ModType_e           eType    = (MI_VENC_ModType_e)E_MI_VENC_MODTYPE_JPEGE;
    MI_VENC_InitParam_t         VencParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;

    STCHECKRESULT(ST_Common_Sys_Init());
    ST_Common_GetVencDefaultDevAttr(&VencParam);
    VencParam.u32MaxWidth = gstJpegSpliceParam.stInputInfo.u32Width;
    VencParam.u32MaxHeight = gstJpegSpliceParam.stInputInfo.u32Height;
    STCHECKRESULT(ST_Common_VencCreateDev(s32DevId, &VencParam));
    ST_Common_GetVencDefaultChnAttr(eType, &stVencChnAttr);
    ST_Common_SetJpegSpliceChnAttr(eType, &stVencChnAttr);

    stVencChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = gstJpegSpliceParam.stInputInfo.u32Width;
    stVencChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = gstJpegSpliceParam.stInputInfo.u32Height;
    stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = gstJpegSpliceParam.stInputInfo.u32Width;
    stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = gstJpegSpliceParam.stInputInfo.u32Height;
    stVencChnAttr.stVeAttr.stAttrJpeg.u32BufSize = 1024*1024;
    STCHECKRESULT(ST_Common_VencStartChn(s32DevId, s32ChnId, &stVencChnAttr));

    return MI_SUCCESS;
}

static MI_S32 ST_ModuleDeInit()
{
    MI_VENC_DEV s32DevId = gstJpegSpliceParam.stInputInfo.stModuleInfo.u32DevId;
    MI_VENC_CHN s32ChnId = gstJpegSpliceParam.stInputInfo.stModuleInfo.u32ChnId;
    STCHECKRESULT(ST_Common_VencStopChn(s32DevId, s32ChnId));
    STCHECKRESULT(ST_Common_VencDestroyDev(s32DevId));
    STCHECKRESULT(ST_Common_Sys_Exit());
    return MI_SUCCESS;
}

static MI_S32 ST_File2JpegPipeline_Preview()
{
    MI_VENC_DEV                 s32DevId = 8;   //jpeg
    MI_VENC_CHN                 s32ChnId = 0;
    char splice_name[64] = {0};
    int  cnt             = 0;
    gstJpegSpliceParam.stOutputInfo.u32SpliceBufNum = 4;
    for(MI_U32 i = 0; i < gstJpegSpliceParam.stOutputInfo.u32SpliceBufNum; i++)
    {
        gstJpegSpliceParam.stOutputInfo.pbuf[i] = (char *)malloc(JPEG_SINGLE_MAX_BUFSIZE);
        if(gstJpegSpliceParam.stOutputInfo.pbuf[i]==NULL)
        {
            ST_ERR("MALLOC fail\n");
            goto jumpret;
        }
    }
    gstJpegSpliceParam.stOutputInfo.stModuleInfo.eModId = E_MI_MODULE_ID_VENC;
    gstJpegSpliceParam.stOutputInfo.stModuleInfo.u32ChnId = s32ChnId;
    gstJpegSpliceParam.stOutputInfo.stModuleInfo.u32DevId = s32DevId;
    gstJpegSpliceParam.stOutputInfo.stModuleInfo.u32PortId = 0;

    gstJpegSpliceParam.stInputInfo.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    gstJpegSpliceParam.stInputInfo.u32Width = 2560;
    gstJpegSpliceParam.stInputInfo.u32Height = 1440;    //JPEG yuv420 16 align,422 8 algn
    gstJpegSpliceParam.stInputInfo.stModuleInfo.eModId = E_MI_MODULE_ID_VENC;
    gstJpegSpliceParam.stInputInfo.stModuleInfo.u32ChnId = s32ChnId;
    gstJpegSpliceParam.stInputInfo.stModuleInfo.u32DevId = s32DevId;
    gstJpegSpliceParam.stInputInfo.stModuleInfo.u32PortId = 0;
    gstJpegSpliceParam.stInputInfo.bThreadExit = false;
    sprintf(gstJpegSpliceParam.stInputInfo.InputFilePath, "./resource/input/2560_1440_nv12.yuv");
    gstJpegSpliceParam.stInputInfo.u32Fps = 30;

    switch(gu8CmdIndex)
    {
        case 0:
            {
                gstJpegSpliceParam.eSpliceMode = E_JPEG_SPLICE_MODE_HORIZONTAL;
                sprintf(splice_name, "horizontal");
            }
        break;
        case 1:
            {
                gstJpegSpliceParam.eSpliceMode = E_JPEG_SPLICE_MODE_VERTICAL;
                sprintf(splice_name, "vertical");
            }
        break;
        case 2:
            {
                gstJpegSpliceParam.eSpliceMode = E_JPEG_SPLICE_MODE_Z;
                sprintf(splice_name, "z");
            }
        break;
        default:
                gstJpegSpliceParam.eSpliceMode = E_JPEG_SPLICE_MODE_HORIZONTAL;
                sprintf(splice_name, "horizontal");
        break;
    }
    sprintf(gstJpegSpliceParam.dumpSpliceOutPath, "./out/venc/splice_demo_%s_splice.jpeg", splice_name);
    sprintf(gstJpegSpliceParam.dumpJpegOutPath, "./out/venc/splice_demo_%s_jpeg_out", splice_name);
    gstJpegSpliceParam.dumpSpliceOutfp=NULL;
    gstJpegSpliceParam.dumpJpegOutfp=NULL;

    STCHECKRESULT(ST_MoudleInit());
    gstJpegSpliceParam.u16DumpBuffNum =1;
    STCHECKRESULT(pthread_create(&gstJpegSpliceParam.stInputInfo.pPutDatathread, NULL, ST_Common_PutInputDataThread, (void *)&gstJpegSpliceParam.stInputInfo));
    STCHECKRESULT(pthread_mutex_init(&gstJpegSpliceParam.stOutputInfo.Outputmutex, NULL));
    STCHECKRESULT(pthread_create(&gstJpegSpliceParam.stOutputInfo.pOutputPortGetDataThread, NULL, ST_JPEG_GetOutputThread, (void *)&gstJpegSpliceParam.stOutputInfo));
    while(gstJpegSpliceParam.u16DumpBuffNum > 0)
    {
#define TIMEOUT 300
        printf("wait dumpfile finsh dumpnum %d\n", gstJpegSpliceParam.u16DumpBuffNum);
        if(cnt++ > TIMEOUT)
        {
            ST_ERR("[sample_code_error]>>>> timeout dumpnum %d\n", gstJpegSpliceParam.u16DumpBuffNum);
            break;
        }
        usleep(THREAD_SLEEP_TIME_US * 10);
    }
    gstJpegSpliceParam.stOutputInfo.bThreadExit = TRUE;
    gstJpegSpliceParam.stInputInfo.bThreadExit = TRUE;
    STCHECKRESULT(pthread_join(gstJpegSpliceParam.stOutputInfo.pOutputPortGetDataThread, NULL));
    STCHECKRESULT(pthread_mutex_destroy(&gstJpegSpliceParam.stOutputInfo.Outputmutex));
    STCHECKRESULT(pthread_join(gstJpegSpliceParam.stInputInfo.pPutDatathread, NULL));
    STCHECKRESULT(ST_ModuleDeInit());
jumpret:
    for(MI_U32 i = 0; i < gstJpegSpliceParam.stOutputInfo.u32SpliceBufNum; i++)
    {
        if(NULL!=gstJpegSpliceParam.stOutputInfo.pbuf[i])
        {
            free(gstJpegSpliceParam.stOutputInfo.pbuf[i]);
        }
    }
    return MI_SUCCESS;
}
static void ST_JPEG_Usage(void)
{
    printf("Usage:./prog_venc_splice_demo 0) splice H\n");
    printf("Usage:./prog_venc_splice_demo 1) splice V\n");
    printf("Usage:./prog_venc_splice_demo 2) splice Z\n");
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        ST_JPEG_Usage();
        return -1;
    }
    gu8CmdIndex = atoi(argv[1]);
    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "dump"))
        {
            gu8DumpFlag = 1;
        }
    }
    switch (gu8CmdIndex)
    {
        case 0:
        case 1:
        case 2:
            STCHECKRESULT(ST_File2JpegPipeline_Preview());
            break;
        default:
            ST_ERR("the index is invaild!\n");
            ST_JPEG_Usage();
            return -1;
    }
    memset(&gstJpegSpliceParam,0,sizeof(ST_JpegSpliceParam_t));
    return 0;
}
