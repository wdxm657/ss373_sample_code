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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "st_common.h"

MI_S32 ST_Common_Sys_Init(void)
{
    MI_U64           u64Pts = 0;
    MI_SYS_Version_t stVersion;
    struct timeval   curStamp;
    STCHECKRESULT(MI_SYS_Init(0));
    STCHECKRESULT(MI_SYS_GetVersion(0, &stVersion));
    printf("Get MI_SYS_Version:%s\n", stVersion.u8Version);

    gettimeofday(&curStamp, NULL);
    u64Pts = (curStamp.tv_sec) * 1000000ULL + curStamp.tv_usec;
    STCHECKRESULT(MI_SYS_InitPtsBase(0, u64Pts));

    gettimeofday(&curStamp, NULL);
    u64Pts = (curStamp.tv_sec) * 1000000ULL + curStamp.tv_usec;
    STCHECKRESULT(MI_SYS_SyncPts(0, u64Pts));

    STCHECKRESULT(MI_SYS_GetCurPts(0, &u64Pts));
    printf("%s:%d Get MI_SYS_CurPts:0x%llx(%ds,%dus)\n", __func__, __LINE__, u64Pts, (int)curStamp.tv_sec,
           (int)curStamp.tv_usec);

    return MI_SUCCESS;
}

MI_S32 ST_Common_Sys_Exit(void)
{
    STCHECKRESULT(MI_SYS_Exit(0));

    return MI_SUCCESS;
}

void ST_Common_Flush(void)
{
    char c;

    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void ST_Common_Pause(void)
{
    char ch;
    printf("press q to exit\n");
    while ((ch = getchar()) != 'q')
    {
        printf("press q to exit\n");
        usleep(1 * 1000 * 1000);
    }
}

MI_S32 ST_Common_OpenSourceFile(const char *pFileName, FILE **fp)
{
    *fp = fopen(pFileName, "r");
    if (*fp == NULL)
    {
        perror("open");
        return -1;
    }

    return 0;
}

MI_S32 ST_Common_CloseSourceFile(FILE **fp)
{
    int ret = 0;

    ret = fclose(*fp);
    if (ret != 0)
    {
        perror("close");
        return -1;
    }
    *fp = NULL;

    return 0;
}

MI_S32 ST_Common_GetSourceFileSize(FILE *fp, unsigned int *size)
{
    if (fp == NULL)
    {
        perror("fp NULL");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp);
    rewind(fp);

    printf("get file size %d\n", *size);
    return 0;
}

MI_S32 ST_Common_GetOneFrame(FILE *fp, char *pData, int yuvSize)
{
    MI_U32 current, end;
    if (fp == NULL)
    {
        perror("fp NULL");
        return -1;
    }

    current = ftell(fp);
    fseek(fp, 0, SEEK_END);
    end = ftell(fp);
    rewind(fp);

    if ((end - current) == 0 || (end - current) < yuvSize)
    {
        fseek(fp, 0, SEEK_SET);
        current = ftell(fp);
    }

    fseek(fp, current, SEEK_SET);
    if (fread(pData, 1, yuvSize, fp) == yuvSize)
    {
        return 1;
    }

    return 0;
}

MI_S64 ST_Common_CalcDiffTime_MS(struct timeval *pstBeforeStamp, struct timeval *pstAfterStamp)
{
    return (((pstAfterStamp->tv_sec - pstBeforeStamp->tv_sec) * 1000 * 1000)
            + (pstAfterStamp->tv_usec - pstBeforeStamp->tv_usec))
           / 1000;
}

MI_U32 ST_Common_CalculateFrameSize(MI_SYS_BufInfo_t *pstBufInfo)
{
    MI_U32 u32FrameSize = 0XFFFFFFFF;
    if (E_MI_SYS_BUFDATA_FRAME == pstBufInfo->eBufType)
    {
        if (E_MI_SYS_COMPRESS_MODE_NONE == pstBufInfo->stFrameData.eCompressMode
            && (E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 == pstBufInfo->stFrameData.ePixelFormat
                || E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21 == pstBufInfo->stFrameData.ePixelFormat))
        {
            u32FrameSize = pstBufInfo->stFrameData.u32Stride[0] * pstBufInfo->stFrameData.u16Height * 3 / 2;
            if (pstBufInfo->bCrcCheck == TRUE)
            {
                u32FrameSize += 16 * 2; // CRC SIZE 16BYTE
            }
        }
        else
        {
            u32FrameSize = pstBufInfo->stFrameData.u32BufSize;
        }
    }
    else
    {
        printf("%s eBufType %d  not support in demo\n", __func__, pstBufInfo->eBufType);
    }

    return u32FrameSize;
}

MI_S32 ST_Common_CheckMkdirOutFile(char *pFilePath) // file path and file name
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U16 u16size;
    MI_U8  i;
    char   FilePath[256];
    char * p = NULL;

    strcpy(FilePath, pFilePath);

    p = strrchr(FilePath, '/'); // get file path
    if (p)
    {
        *(p + 1) = '\0';
    }
    u16size = strlen(FilePath);
    ST_INFO("ST_CheckMkdirOutFile FilePath: %s ,u16size = %d \n", FilePath, u16size);

    // filepath is exist
    s32Ret = access(pFilePath, 0);
    if (s32Ret == 0)
    {
        goto EXIT;
    }

    // skip the first '/', e.g:/tmp/stable_m6
    for (i = 1; i < u16size; i++)
    {
        if (FilePath[i] == '/')
        {
            FilePath[i] = '\0';
            s32Ret      = access(FilePath, 0);
            if (s32Ret != MI_SUCCESS)
            {
                s32Ret = mkdir(FilePath, 0777);
                if (s32Ret != MI_SUCCESS)
                {
                    ST_ERR("FilePath %s mkdir fail\n", FilePath);
                    goto EXIT;
                }
            }
            FilePath[i] = '/';
        }
    }
    s32Ret = access(FilePath, 0);
    if (u16size > 0 && s32Ret != 0)
    {
        s32Ret = mkdir(FilePath, 0777);
        if (s32Ret != MI_SUCCESS)
        {
            printf("[%s]:%d FilePath %s mkdir fail\n", __FUNCTION__, __LINE__, FilePath);
            goto EXIT;
        }
    }

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_WriteFile(char *pFilePath, FILE **fp, MI_U8 *pData, MI_U32 u32BufSize, MI_U16 *pu16DumpNum,
                           MI_BOOL bRecover) // pFilePath=/path/name
{
    MI_S32 s32Ret = MI_SUCCESS;

    if (*fp == NULL)
    {
        const char *mode = bRecover ? "wb" : "ab";
        s32Ret           = ST_Common_CheckMkdirOutFile(pFilePath);
        if (s32Ret != MI_SUCCESS)
        {
            printf("[%s]:%d \n", __FUNCTION__, __LINE__);
            *pu16DumpNum = 0;
            return -1;
        }

        *fp = fopen(pFilePath, mode);
        if (*fp == NULL)
        {
            printf("file %s open fail\n", pFilePath);
            *pu16DumpNum = 0;
            return -1;
        }
    }

    fwrite(pData, u32BufSize, 1, *fp);
    ST_INFO("write file %s id %d done  \n", pFilePath, *pu16DumpNum);

    if (*pu16DumpNum > 0)
    {
        *pu16DumpNum -= 1;
    }

    if (*pu16DumpNum == 0)
    {
        fclose(*fp);
        *fp = NULL;
        ST_INFO("close file %s done  \n", pFilePath);
    }

    return s32Ret;
}

MI_S32 ST_Common_GetOutputBufInfo(MI_SYS_ChnPort_t stChnPort, MI_SYS_BufInfo_t *pstBufInfo, MI_SYS_BUF_HANDLE *pHandle,
                                  struct timeval *pstTimeoutVal)
{
    ST_CHECK_POINTER(pstBufInfo);
    ST_CHECK_POINTER(pHandle);

    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 s32Fd  = 0;
    fd_set read_fds;

    s32Ret = MI_SYS_GetFd(&stChnPort, &s32Fd);
    if (s32Ret != MI_SUCCESS)
    {
        ST_ERR("MI_SYS_GetFd error %d\n", s32Ret);
        return -1;
    }

    FD_ZERO(&read_fds);
    FD_SET(s32Fd, &read_fds);

    s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, pstTimeoutVal);

    if (s32Ret < 0)
    {
        ST_ERR("select fail\n");
    }
    else if (s32Ret == 0)
    {
        ST_INFO("select timeout\n");
        s32Ret = -1;
    }
    else if (s32Ret > 0)
    {
        if (FD_ISSET(s32Fd, &read_fds))
        {
            MI_SYS_ChnOutputPortGetBuf(&stChnPort, pstBufInfo, pHandle);
            s32Ret = MI_SUCCESS;
        }
    }

    MI_SYS_CloseFd(s32Fd);

    return s32Ret;
}

MI_S32 ST_Common_PutOutputBufInfo(MI_SYS_BUF_HANDLE *pHandle)
{
    ST_CHECK_POINTER(pHandle);

    if (MI_SUCCESS != MI_SYS_ChnOutputPortPutBuf(*pHandle))
    {
        ST_ERR("put buf error\n");
    }

    return MI_SUCCESS;
}

void *ST_Common_GetOutputDataThread(void *args)
{
    MI_SYS_BufInfo_t             stBufInfo;
    MI_SYS_BUF_HANDLE            hHandle;
    ST_Common_OutputFile_Attr_t *pstOutFileAttr = ((ST_Common_OutputFile_Attr_t *)(args));
    MI_S32                       s32Ret         = MI_SUCCESS;
    MI_S32                       s32Fd          = 0;
    fd_set                       read_fds;
    struct timeval               TimeoutVal;
    MI_SYS_ChnPort_t             stChnPort;
    char                         FilePath[256];
    MI_BOOL                      bCoverFile;
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId    = pstOutFileAttr->stModuleInfo.eModId;
    stChnPort.u32DevId  = pstOutFileAttr->stModuleInfo.u32DevId;
    stChnPort.u32ChnId  = pstOutFileAttr->stModuleInfo.u32ChnId;
    stChnPort.u32PortId = pstOutFileAttr->stModuleInfo.u32PortId;

    s32Ret = MI_SYS_GetFd(&stChnPort, &s32Fd);
    if (s32Ret != MI_SUCCESS)
    {
        ST_ERR("MI_SYS_GetFd error %d\n", s32Ret);
        return NULL;
    }

    bCoverFile = ~pstOutFileAttr->bAppendFile;
    ST_INFO("[%s] module %d, dev %d, chn %d, port %d\n", __FUNCTION__, stChnPort.eModId, stChnPort.u32DevId,
            stChnPort.u32ChnId, stChnPort.u32PortId);

    while (!pstOutFileAttr->bThreadExit)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32Fd, &read_fds);
        TimeoutVal.tv_sec  = 1;
        TimeoutVal.tv_usec = 0;
        s32Ret             = select(s32Fd + 1, &read_fds, NULL, NULL, &TimeoutVal);

        if (s32Ret < 0)
        {
            ST_ERR("select fail\n");
            usleep(THREAD_SLEEP_TIME_US);
            continue;
        }
        else if (s32Ret == 0)
        {
            usleep(THREAD_SLEEP_TIME_US);
            continue;
        }
        else
        {
            if (FD_ISSET(s32Fd, &read_fds))
            {
                pthread_mutex_lock(&pstOutFileAttr->Outputmutex);
                if (MI_SUCCESS == MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &hHandle))
                {
                    if (pstOutFileAttr->u16DumpBuffNum > 0)
                    {
                        sprintf(FilePath, "%s.yuv", pstOutFileAttr->FilePath);
                        if (MI_SUCCESS != ST_Common_WriteFile(FilePath, &pstOutFileAttr->fp,
                                                              (MI_U8 *)stBufInfo.stFrameData.pVirAddr[0],
                                                              stBufInfo.stFrameData.u32BufSize,
                                                              &pstOutFileAttr->u16DumpBuffNum, bCoverFile))
                        {
                            printf("wirtefile %s fail\n", FilePath);
                            if (MI_SUCCESS != MI_SYS_ChnOutputPortPutBuf(hHandle))
                            {
                                ST_ERR("(%d-%d-%d-%d) put buf error \n", stChnPort.eModId, stChnPort.u32DevId,
                                       stChnPort.u32ChnId, stChnPort.u32PortId);
                            }
                            pthread_mutex_unlock(&pstOutFileAttr->Outputmutex);
                            return NULL;
                        }
                    }

                    if (MI_SUCCESS != MI_SYS_ChnOutputPortPutBuf(hHandle))
                    {
                        ST_ERR("(%d-%d-%d-%d) put buf error \n", stChnPort.eModId, stChnPort.u32DevId,
                               stChnPort.u32ChnId, stChnPort.u32PortId);
                    }
                }
                pthread_mutex_unlock(&pstOutFileAttr->Outputmutex);
            }
        }
    }

    MI_SYS_CloseFd(s32Fd);

    return NULL;
}

void *ST_Common_GetVencOutThread(void *args)
{
    MI_S32                       s32Ret = MI_SUCCESS;
    MI_VENC_Stream_t             stStream;
    ST_Common_OutputFile_Attr_t *pstVencOutFileAttr = ((ST_Common_OutputFile_Attr_t *)(args));
    char                         FilePath[256];
    MI_VENC_ChnStat_t            stStat;
    MI_VENC_ChnAttr_t            stVencAttr;
    MI_VENC_DEV                  VencDev = pstVencOutFileAttr->stModuleInfo.u32DevId;
    MI_VENC_CHN                  VencChn = pstVencOutFileAttr->stModuleInfo.u32ChnId;
    MI_S32                       s32Fd   = -1;
    MI_BOOL                      bCoverFile;
    int                          i;
    struct timeval               tv;
    fd_set                       read_fds;

    memset(&stVencAttr, 0x0, sizeof(MI_VENC_ChnAttr_t));
    s32Ret = MI_VENC_GetChnAttr(VencDev, VencChn, &stVencAttr);
    if (s32Ret != MI_SUCCESS)
    {
        printf("channel %d, not valid \n", VencChn);
        return NULL;
    }

    s32Fd = MI_VENC_GetFd(VencDev, VencChn);
    if (s32Fd < 0)
    {
        printf("GET Venc Fd Failed , Venc Dev = %d Chn = %d\n", VencDev, VencChn);
        return NULL;
    }

    bCoverFile = ~pstVencOutFileAttr->bAppendFile;
    ST_INFO("[%s] Venc Fd = %d, Venc Dev = %d Chn = %d\n ", __FUNCTION__, s32Fd, VencDev, VencChn);

    while (!pstVencOutFileAttr->bThreadExit)
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
            stStream.pstPack = (MI_VENC_Pack_t *)malloc(sizeof(MI_VENC_Pack_t) * stStat.u32CurPacks);
            if (NULL == stStream.pstPack)
            {
                usleep(THREAD_SLEEP_TIME_US);
                continue;
            }
            stStream.u32PackCount = stStat.u32CurPacks;
            pthread_mutex_lock(&pstVencOutFileAttr->Outputmutex);
            s32Ret = MI_VENC_GetStream(VencDev, VencChn, &stStream, FALSE);
            if (MI_SUCCESS == s32Ret)
            {
                if (pstVencOutFileAttr->u16DumpBuffNum > 0)
                {
                    if (stVencAttr.stVeAttr.eType == E_MI_VENC_MODTYPE_JPEGE)
                        sprintf(FilePath, "%s.jpg", pstVencOutFileAttr->FilePath);
                    else
                        sprintf(FilePath, "%s.es", pstVencOutFileAttr->FilePath);

                    ST_INFO(
                        "##########Start to write file %s, id %d packcnt "
                        "%d!!!#####################\n",
                        FilePath, pstVencOutFileAttr->u16DumpBuffNum, stStream.u32PackCount);
                    for (i = 0; i < stStream.u32PackCount; i++)
                    {
                        if (MI_SUCCESS
                            != ST_Common_WriteFile(FilePath, &pstVencOutFileAttr->fp,
                                                   stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset,
                                                   stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset,
                                                   &pstVencOutFileAttr->u16DumpBuffNum, bCoverFile))
                        {
                            ST_ERR("venc write file err \n");
                            MI_VENC_ReleaseStream(VencDev, VencChn, &stStream);
                            pthread_mutex_unlock(&pstVencOutFileAttr->Outputmutex);
                            free(stStream.pstPack);
                            stStream.pstPack = NULL;
                            return NULL;
                        }
                        else
                            printf("venc write file success \n");
                    }
                }

                MI_VENC_ReleaseStream(VencDev, VencChn, &stStream);
            }
            pthread_mutex_unlock(&pstVencOutFileAttr->Outputmutex);
            free(stStream.pstPack);
            stStream.pstPack = NULL;
        }
    }

    MI_VENC_CloseFd(VencDev, VencChn);
    FD_ZERO(&read_fds);

    return NULL;
}

void *ST_Common_PutInputDataThread(void *args)
{
    MI_SYS_ChnPort_t            stChnInput;
    MI_SYS_BUF_HANDLE           hHandle = 0;
    MI_SYS_BufConf_t            stBufConf;
    MI_SYS_BufInfo_t            stBufInfo;
    ST_Common_InputFile_Attr_t *pstInputFileattr = (ST_Common_InputFile_Attr_t *)(args);
    MI_S32                      s32Ret;
    MI_U32                      u32OneFrameSize = 0;
    FILE *                      fp              = NULL;
    time_t                      stTime          = 0;
    char                        src_file[256];
    MI_U32                      u32FileSize    = 0;
    MI_PHY                      phyFileAddr    = 0;
    void *                      pFileAddr      = NULL;
    MI_U32                      u32YFrameSize  = 0;
    MI_U32                      u32UVFrameSize = 0;
    struct timeval              stthreadstart;
    struct timeval              stthreadend;
    MI_U32                      u32InputFps = 0;

    memset(&stChnInput, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&stBufConf, 0x0, sizeof(MI_SYS_BufConf_t));
    memset(&stBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));

    if (0 != pstInputFileattr->u32Fps)
    {
        u32InputFps = 1000000 / pstInputFileattr->u32Fps;
    }

    stChnInput.eModId    = pstInputFileattr->stModuleInfo.eModId;
    stChnInput.u32DevId  = pstInputFileattr->stModuleInfo.u32DevId;
    stChnInput.u32ChnId  = pstInputFileattr->stModuleInfo.u32ChnId;
    stChnInput.u32PortId = pstInputFileattr->stModuleInfo.u32PortId;

    ST_INFO("[%s]module %d, dev %d, chn %d, port %d start get input  buffer\n", __FUNCTION__, stChnInput.eModId,
            stChnInput.u32DevId, stChnInput.u32ChnId, stChnInput.u32PortId);
    stBufConf.eBufType                  = E_MI_SYS_BUFDATA_FRAME;
    stBufConf.u64TargetPts              = time(&stTime);
    stBufConf.stFrameCfg.eFormat        = pstInputFileattr->ePixelFormat;
    stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
    stBufConf.stFrameCfg.eCompressMode  = E_MI_SYS_COMPRESS_MODE_NONE;
    stBufConf.stFrameCfg.u16Width       = pstInputFileattr->u32Width;
    stBufConf.stFrameCfg.u16Height      = pstInputFileattr->u32Height;
    sprintf(src_file, "%s", pstInputFileattr->InputFilePath);

    s32Ret = ST_Common_OpenSourceFile(src_file, &fp);
    if (s32Ret < 0)
    {
        ST_ERR("open file %s fail!\n", src_file);
        return NULL;
    }

    ST_Common_GetSourceFileSize(fp, &u32FileSize);
    if (u32FileSize == 0)
    {
        ST_Common_CloseSourceFile(&fp);
        return NULL;
    }

    if(stBufConf.stFrameCfg.eFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
    {
        u32YFrameSize  = pstInputFileattr->u32Width * pstInputFileattr->u32Height;
        u32UVFrameSize = u32YFrameSize / 2;
    }
    else if (stBufConf.stFrameCfg.eFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
    {
        u32YFrameSize  = pstInputFileattr->u32Width * pstInputFileattr->u32Height;
        u32UVFrameSize = u32YFrameSize;
    }

    MI_SYS_MMA_Alloc(0, (MI_U8 *)"mma_heap_name0", u32YFrameSize + u32UVFrameSize, &phyFileAddr);
    if (phyFileAddr == 0)
    {
        ST_Common_CloseSourceFile(&fp);
        ST_ERR("get mma size fail \n");
        return NULL;
    }

    MI_SYS_Mmap(phyFileAddr, u32YFrameSize + u32UVFrameSize, &pFileAddr, FALSE);
    if (pFileAddr == NULL)
    {
        ST_Common_CloseSourceFile(&fp);
        ST_ERR("mmap fail \n");
        MI_SYS_MMA_Free(0, phyFileAddr);
        return NULL;
    }
#if 0
    if (1 != ST_Common_GetOneFrame(fp, (char *)pFileAddr, u32FileSize))
    {
        ST_Common_CloseSourceFile(&fp);
        ST_ERR("read file size fail \n");
        return NULL;
    }
#endif
    while (!pstInputFileattr->bThreadExit)
    {
        gettimeofday(&stthreadstart, NULL);

        if (MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stChnInput, &stBufConf, &stBufInfo, &hHandle, 0))
        {
            u32OneFrameSize = ST_Common_CalculateFrameSize(&stBufInfo);
            if ((u32OneFrameSize <= u32YFrameSize+ u32UVFrameSize )  && (1 == ST_Common_GetOneFrame(fp, (char *)pFileAddr, u32OneFrameSize)))
            {
                if (stBufConf.stFrameCfg.eFormat == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420)
                {
                    MI_SYS_MemcpyPa(0, stBufInfo.stFrameData.phyAddr[0], phyFileAddr, u32YFrameSize);
                    MI_SYS_MemcpyPa(0, stBufInfo.stFrameData.phyAddr[1], phyFileAddr + u32YFrameSize,u32UVFrameSize);
                }
                else if(stBufConf.stFrameCfg.eFormat == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV)
                {
                    MI_SYS_MemcpyPa(0, stBufInfo.stFrameData.phyAddr[0], phyFileAddr, u32YFrameSize + u32UVFrameSize);
                }
                s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, FALSE);
                if (s32Ret != MI_SUCCESS)
                {
                    printf("%s:%d MI_SYS_ChnInputPortPutBuf ret:%d\n", __FUNCTION__, __LINE__, s32Ret);
                }
            }
            else
            {
                printf("u32OneFrameSize = %d,u32YFrameSize+ u32UVFrameSize=%d\n",u32OneFrameSize,u32YFrameSize+ u32UVFrameSize);
                s32Ret = MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, FALSE);
                if (s32Ret != MI_SUCCESS)
                {
                    printf("%s:%d MI_SYS_ChnInputPortPutBuf ret:%d\n", __FUNCTION__, __LINE__, s32Ret);
                }
                ST_ERR("read file ST_Common_GetOneFrame fail or finish \n");
                pstInputFileattr->bThreadExit = TRUE;
            }
        }
        if (0 == u32InputFps)
        {
            usleep(THREAD_SLEEP_TIME_US);
        }
        else
        {
            gettimeofday(&stthreadend, NULL);
            while (((stthreadend.tv_sec - stthreadstart.tv_sec) * 1000000 + (stthreadend.tv_usec - stthreadstart.tv_usec))
                   < u32InputFps)
            {
                usleep(100);
                gettimeofday(&stthreadend, NULL);
            }
        }
    }

    MI_SYS_Munmap(pFileAddr, u32YFrameSize + u32UVFrameSize);
    MI_SYS_MMA_Free(0, phyFileAddr);

    ST_Common_CloseSourceFile(&fp);
    return NULL;
}

MI_S32 ST_Common_WaitDumpFinsh(ST_Common_OutputFile_Attr_t *pstOutFileAttr)
{
#define TIMEOUT 300
    int cnt = 0;

    while (pstOutFileAttr->u16DumpBuffNum > 0)
    {
        printf("wait dumpfile finsh dumpnum %d\n", pstOutFileAttr->u16DumpBuffNum);
        if (cnt++ > TIMEOUT)
        {
            ST_ERR("[sample_code_error]>>>> timeout dumpnum %d\n", pstOutFileAttr->u16DumpBuffNum);
            break;
        }
        usleep(THREAD_SLEEP_TIME_US * 10);
    }
    pstOutFileAttr->bThreadExit = TRUE;
    pthread_join(pstOutFileAttr->pGetDataThread, NULL);
    pthread_mutex_destroy(&pstOutFileAttr->Outputmutex);

    return 0;
}

MI_S32 ST_Common_CheckResult(ST_Common_OutputFile_Attr_t *pstOutFileAttr)
{
    if (pstOutFileAttr->stModuleInfo.eModId == E_MI_MODULE_ID_VENC)
    {
        pthread_mutex_init(&pstOutFileAttr->Outputmutex, NULL);
        pthread_create(&pstOutFileAttr->pGetDataThread, NULL, ST_Common_GetVencOutThread, (void *)pstOutFileAttr);
    }
    else
    {
        pthread_mutex_init(&pstOutFileAttr->Outputmutex, NULL);
        pthread_create(&pstOutFileAttr->pGetDataThread, NULL, ST_Common_GetOutputDataThread, (void *)pstOutFileAttr);
    }

    return MI_SUCCESS;
}

MI_S32 ST_Common_AllocateMemory(MI_U32 size, MI_PHY *phys_addr, MI_U8 **virt_addr)
{
    MI_PHY phys;
    MI_S32 ret;

    ret = MI_SYS_MMA_Alloc(0, (MI_U8 *)"mma_heap_name0", size, &phys);

    if (ret != MI_SUCCESS)
    {
        printf("can't allocate from MI SYS (size: %d, ret: %d)\n", size, ret);
        return ret;
    }
    ret = MI_SYS_Mmap(phys, size, (void **)virt_addr, FALSE);
    if (ret != MI_SUCCESS)
    {
        printf("can't map memory from MI SYS (phys: 0x%llx, size: %d, ret: %d)\n", phys, size, ret);

        MI_SYS_MMA_Free(0, phys);

        return ret;
    }

    *phys_addr = phys;

    return MI_SUCCESS;
}

MI_S32 ST_Common_FreeMemory(MI_PHY phys_addr, MI_U8 *virt_addr, MI_U32 size)
{
    MI_S32 ret;

    ret = MI_SYS_Munmap((void *)virt_addr, size);

    ret |= MI_SYS_MMA_Free(0, phys_addr);

    if (ret != MI_SUCCESS)
    {
        printf("error occurs when free buffer to MI SYS\n");
        return ret;
    }

    return MI_SUCCESS;
}

MI_U32 ST_Common_GetMs(void)
{
    struct timeval st_time    = {0};
    MI_U32         u32Time_ms = 0;

    gettimeofday(&st_time, NULL);
    u32Time_ms = ((1000000 * st_time.tv_sec) + st_time.tv_usec) / 1000;

    return u32Time_ms;
}
