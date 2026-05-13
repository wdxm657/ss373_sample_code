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
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mi_ai.h"
#include "mi_sys.h"
#include "st_common_rtsp_audio.h"

#define RTSP_SERVER_MAX 32
#define BUF_POOL_MAX 60

typedef struct ST_RtspInfo_s
{
    int refcnt;
    void *pool_handle;
    ss_rtsp_handle rtsphandle;
    pthread_mutex_t Outputmutex;
    pthread_t pThread;
    MI_BOOL bThreadExit;
    MI_U32 Dev;
    MI_U8 Chn;
    char url[16];
    struct rtsp_connection_fp fp;
}ST_RtspInfo_t;

static ST_RtspInfo_t gstRtspInfo[RTSP_SERVER_MAX];

void *ai_read_stream(void *args)
{
    int ret;
    MI_S32 s32Ret = MI_SUCCESS;
    ST_RtspInfo_t *pstRtspInfo = (ST_RtspInfo_t *)args;
    MI_SYS_ChnPort_t stChnOutputPort;
    MI_AI_Data_t stMicFrame;
    MI_AUDIO_DEV AiDevId = (MI_AUDIO_DEV)pstRtspInfo->Dev;
    MI_U8 u8ChnGrpIdx = pstRtspInfo->Chn;
    MI_S32 s32Fd = -1;
    struct timeval tv;
    fd_set read_fds;

    memset(&stChnOutputPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnOutputPort.eModId = E_MI_MODULE_ID_AI;
    stChnOutputPort.u32DevId = AiDevId;
    stChnOutputPort.u32ChnId = u8ChnGrpIdx;
    stChnOutputPort.u32PortId = 0;
    s32Ret = MI_SYS_GetFd(&stChnOutputPort, &s32Fd);
    if (s32Ret < 0)
    {
        printf(" Dev = %d Chn = %d, get fd %d error\n", AiDevId, u8ChnGrpIdx, s32Fd);
        return NULL;
    }

    printf("[%s] Ai Fd = %d,  Dev = %d Chn = %d\n ", __FUNCTION__, s32Fd, AiDevId, u8ChnGrpIdx);

    while (!pstRtspInfo->bThreadExit)
    {
        FD_ZERO(&read_fds);
        FD_SET(s32Fd, &read_fds);

        tv.tv_sec = 0;
        tv.tv_usec = 500 * 1000;

        s32Ret = select(s32Fd + 1, &read_fds, NULL, NULL, &tv);
        if (s32Ret < 0)
        {
            printf("select failed dev %d, chn %d\n", AiDevId, u8ChnGrpIdx);
            usleep(10 * 1000);
            continue;
        }
        else if (0 == s32Ret)
        {
            usleep(10 * 1000);
            continue;
        }
        else
        {

            pthread_mutex_lock(&pstRtspInfo->Outputmutex);
            if(pstRtspInfo->refcnt > 0)
            {
                //check pool
                ret = ss_rtsp_server_check_pool(pstRtspInfo->rtsphandle, pstRtspInfo->pool_handle);
                if(ret != 0)
                {
                    printf("Buf pool is full");
                    continue;
                }
                memset(&stMicFrame, 0, sizeof(MI_AI_Data_t));
                s32Ret = MI_AI_Read(AiDevId, u8ChnGrpIdx, &stMicFrame, NULL, 0);
                if(s32Ret == MI_AI_ERR_NOBUF)
                {
                    printf("dev %d, chn %d no buffer warning!\n", AiDevId, u8ChnGrpIdx);
                }
                else if(s32Ret != MI_SUCCESS)
                {
                    printf("dev %d, chn %d get frame error!\n", AiDevId, u8ChnGrpIdx);
                    return NULL;
                }

                if (MI_SUCCESS == s32Ret)
                {
                    struct rtsp_frame_packet frame_package;
                    struct timeval stamp;
                    memset(&frame_package, 0, sizeof(struct rtsp_frame_packet));
                    memset(&stamp, 0, sizeof(struct timeval));
                    frame_package.packet_count = 1;
                    for (unsigned int i = 0; i < frame_package.packet_count; i++)
                    {
                        frame_package.packet_data[i].data =  (char*)stMicFrame.apvBuffer[i];
                        frame_package.packet_data[i].size = stMicFrame.u32Byte[i];
                    }

                    frame_package.stamp.tv_sec  = stStream.pstPack[0].u64PTS / 1000000;
                    frame_package.stamp.tv_usec = stStream.pstPack[0].u64PTS % 1000000;

                    ss_rtsp_server_send_package(pstRtspInfo->rtsphandle, pstRtspInfo->pool_handle, &frame_package);

                    MI_AI_ReleaseData(AiDevId, u8ChnGrpIdx, &stMicFrame, NULL);
                }
            }
            pthread_mutex_unlock(&pstRtspInfo->Outputmutex);
        }
    }

    MI_SYS_CloseFd(s32Fd);
    FD_ZERO(&read_fds);

    return NULL;
}


void ai_open_stream(const char *url, void *user_data)
{
    ST_RtspInfo_t *pstRtspInfo = (ST_RtspInfo_t *)user_data;

    if(!pstRtspInfo)
    {
        printf("ai_open_stream fail, pstRtspInfo NULL\n");
        return;
    }
    pthread_mutex_lock(&pstRtspInfo->Outputmutex);
    pstRtspInfo->refcnt++;
    pthread_mutex_unlock(&pstRtspInfo->Outputmutex);

    printf("ai_open_stream success dev %d, chn %d\n", pstRtspInfo->Dev, pstRtspInfo->Chn);
}

void ai_close_stream(const char *url, void *user_data)
{
    ST_RtspInfo_t *pstRtspInfo = (ST_RtspInfo_t *)user_data;

    if(!pstRtspInfo)
    {
        printf("ai_close_stream fail, pstRtspInfo NULL\n");
        return;
    }

    pthread_mutex_lock(&pstRtspInfo->Outputmutex);
    pstRtspInfo->refcnt--;
    pthread_mutex_unlock(&pstRtspInfo->Outputmutex);

    printf("venc_close_stream success, dev %d, chn %d\n",  pstRtspInfo->Dev, pstRtspInfo->Chn);
}

MI_S32 ST_Common_RtspServerStartAudio(ST_AudioStreamInfo_t *pstStreamAttr)
{
    int ret = 0;
    struct rtsp_audio_info info;
    struct rtsp_pool_config config;
    ST_RtspInfo_t *pstRtspInfo = NULL;

    if(pstStreamAttr == NULL)
    {
        printf("pointer null\n");
        return -1;
    }

    pstRtspInfo = &gstRtspInfo[pstStreamAttr->rtspIndex];

    //create rtsp handle
    pstRtspInfo->rtsphandle = ss_rtsp_server_create();
    if(pstRtspInfo->rtsphandle == NULL)
    {
        return -1;
    }

    //add audio url
    memset(&info, 0x00, sizeof(struct rtsp_audio_info));
    if(pstStreamAttr->enFormat == E_MI_AUDIO_FORMAT_PCM_S16_LE)
    {
        info.format = RTSP_ES_FMT_AUDIO_PCM;
    }
    else
    {
        printf("audio format %d not support\n", pstStreamAttr->enFormat);
        return -1;
    }
    info.channels = pstStreamAttr->enSoundMode;
    info.sample_rate = pstStreamAttr->enSampleRate;
    sprintf(pstRtspInfo->url, "77%d%d", pstStreamAttr->AiDevId, pstStreamAttr->u8ChnGrpIdx);
    info.sample_width = pstStreamAttr->sample_width;

    printf("rtsp audio dev%d, chn %d, format %d, url %s\n", pstStreamAttr->AiDevId,
        pstStreamAttr->u8ChnGrpIdx, pstStreamAttr->enFormat, pstRtspInfo->url);

    pstRtspInfo->Dev = pstStreamAttr->AiDevId;
    pstRtspInfo->Chn = pstStreamAttr->u8ChnGrpIdx;

    //set connect/disconnect callbackfun
    pstRtspInfo->fp.connect = ai_open_stream;
    pstRtspInfo->fp.disconnect = ai_close_stream;
    pstRtspInfo->fp.user_data  = (void *)pstRtspInfo;

    ret = ss_rtsp_server_add_audio_url(pstRtspInfo->rtsphandle, pstRtspInfo->url, &info, pstRtspInfo->fp);
    if(ret != 0)
    {
        return -1;
    }

    //start server
    ret = ss_rtsp_server_start(pstRtspInfo->rtsphandle);
    if(ret != 0)
    {
        return -1;
    }

    //get pool handle
    pstRtspInfo->pool_handle = ss_rtsp_server_get_audio_pool(pstRtspInfo->rtsphandle, pstRtspInfo->url);

    //config pool
    memset(&config, 0x00, sizeof(struct rtsp_pool_config));
    config.depth = BUF_POOL_MAX;
    ss_rtsp_server_config_pool(pstRtspInfo->rtsphandle, pstRtspInfo->pool_handle, &config);

    pstRtspInfo->refcnt = 0;

    //satrt thread read stream
    pthread_mutex_init(&pstRtspInfo->Outputmutex, NULL);
    pthread_create(&pstRtspInfo->pThread, NULL, ai_read_stream, (void *)pstRtspInfo);

    return ret;
}

MI_S32 ST_Common_RtspServerStopAudio(ST_AudioStreamInfo_t *pstStreamAttr)
{
    int ret = 0;
    ST_RtspInfo_t *pstRtspInfo = &gstRtspInfo[pstStreamAttr->rtspIndex];

    if(pstStreamAttr == NULL)
    {
        printf("pointer null\n");
        return -1;
    }

    if(!pstRtspInfo->rtsphandle)
    {
        printf("stop rtsp error, rtspHandle Null\n");
        return -1;
    }
    //stop thread
    pstRtspInfo->bThreadExit = TRUE;
    pthread_join(pstRtspInfo->pThread, NULL);
    pthread_mutex_destroy(&pstRtspInfo->Outputmutex);

    ret = ss_rtsp_server_stop(pstRtspInfo->rtsphandle);
    if(ret != 0)
    {
        return -1;
    }

    ret = ss_rtsp_server_del_audio_url(pstRtspInfo->rtsphandle, pstRtspInfo->url);
    if(ret != 0)
    {
        return -1;
    }

    pstRtspInfo->fp.connect    = NULL;
    pstRtspInfo->fp.disconnect = NULL;
    pstRtspInfo->fp.user_data  = NULL;

    //destroy server
    ss_rtsp_server_destroy(pstRtspInfo->rtsphandle);

    return 0;
}

