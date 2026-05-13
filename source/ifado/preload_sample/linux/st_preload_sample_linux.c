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
#include <poll.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>

#include "ss_thread.h"
#include "st_common_rtsp_video.h"
#include "st_common.h"
#include "st_common_aov.h"
#include "mi_venc.h"
#include "mi_scl.h"
#include "st_preload_sample_linux.h"
#include "mi_iqserver.h"
#include "st_uart.h"

static FILE *    gPile[AOV_MAX_SENSOR_NUM] = {};
static MI_BOOL   g_bExit                   = FALSE;
static MI_BOOL   g_bSavePicFlag            = FALSE;
static MI_BOOL   g_bRun                    = FALSE;
static MI_S32    g_S32Ret                  = MI_SUCCESS;
static MI_U8     g_AovStatus               = AOV_DEFAULT_STATUS;
static pthread_t g_ptSaveStream;

ST_pthreadHandel_T pthreadMsg;
ST_pthreadHandel_T pthreadAi;
ST_pthreadHandel_T pthreadAo;

char                         u8PicSavePath[64] = {"./"};
char                         u8AiFIle[64]      = {"./"};
char                         u8AoFIle[64]      = {"./"};
static ST_Common_AovHandle_t gstAovHandle;
MI_BOOL                      bTestModeFlag    = FALSE;
MI_BOOL                      bAudioAiFlag     = FALSE;
MI_BOOL                      bAudioAoFlag     = FALSE;
MI_BOOL                      bAudioAiDone     = FALSE;
MI_BOOL                      bAudioAoDone     = FALSE;
MI_BOOL                      bWifiPreviewFlag = FALSE;
MI_U8                        u8SimulateCmd    = '\0';
MI_U8                        u8StCmd          = '\0';
MI_U32                       u32SensorNum     = 1;

int ST_Thread_Start(void *args, const char *threadname, void *(*CB)(void *))
{
    ST_pthreadHandel_T *ppthreadHandel = (ST_pthreadHandel_T *)args;
    memset(ppthreadHandel, 0, sizeof(ST_pthreadHandel_T));
    ppthreadHandel->threadname = threadname;
    prctl(PR_SET_NAME, threadname);
    pthread_create(&ppthreadHandel->pthread, NULL, CB, args);
    return 0;
}

int ST_Thread_Stop(void *args, const char *threadname)
{
    ST_pthreadHandel_T *ppthreadHandel = (ST_pthreadHandel_T *)args;

    if (!ppthreadHandel->pthread)
    {
        printf("[%s] nothing exit\n", __FUNCTION__);
        return 0;
    }

    ppthreadHandel->exit = TRUE;

    while (ppthreadHandel->thread_exit == FALSE)
    {
        printf("[%s] wait for %s exit\n", __FUNCTION__, ppthreadHandel->threadname);
        usleep(200 * 1000);
    }

    printf("[%s] thread join+\n", __FUNCTION__);
    pthread_join(ppthreadHandel->pthread, NULL);
    printf("[%s] thread join-\n", __FUNCTION__);

    memset(ppthreadHandel, 0, sizeof(ST_pthreadHandel_T));
    return 0;
}

int CheckIpuFile(void)
{
    char MiscFilename[32]    = {"/misc/spdy36.img"};
    char RofilesFilename[32] = {"/rofiles/spdy36.img"};
    if ((access(MiscFilename, F_OK | R_OK) != 0) && (access(RofilesFilename, F_OK | R_OK) != 0))
    {
        printf("IPU file does not exist failed\n");
        return -1;
    }

    return 0;
}

void SafeCloseSaveFile(void)
{
    g_bRun = FALSE;
    while (!g_bRun)
    {
        usleep(10 * 1000);
    }
    for (int i = 0; i < AOV_MAX_SENSOR_NUM; i++)
    {
        if (gPile[i] != NULL)
        {
            if (fsync(fileno(gPile[i])) != 0)
            {
                ST_ERR("Sensor [%d] Error syncing to disk", i);
            }
            fclose(gPile[i]);
            gPile[i] = NULL;
        }
    }
    usleep(10 * 1000);
}

void *ST_FastBootSteamSaveThread(void *p)
{
    MI_S32            s32Ret = MI_SUCCESS;
    MI_VENC_Stream_t  stStream;
    MI_VENC_Pack_t    stPack[16];
    MI_VENC_ChnStat_t stStat;
    MI_S32            s32VencDev   = 0;
    MI_S32            s32VencChn   = 0;
    MI_S32            gu32FileSeq  = 0;
    MI_S32            s32VencFd;
    struct            timeval tv;
    fd_set            read_fds;

    while (g_bRun)
    {
        if (g_bSavePicFlag || bTestModeFlag)
        {
            for (MI_U32 j = 0; j < u32SensorNum; j++)
            {
                s32VencChn = j;
                s32VencFd = MI_VENC_GetFd(s32VencDev, s32VencChn);
                if (s32VencFd < 0)
                {
                    ST_ERR("get venc fd error\n");
                    break;
                }

                FD_ZERO(&read_fds);
                FD_SET(s32VencFd, &read_fds);

                tv.tv_sec  = 2;
                tv.tv_usec = 0;

                s32Ret = select(s32VencFd + 1, &read_fds, NULL, NULL, &tv);
                if (s32Ret < 0)
                {
                    ST_ERR("select err\n");
                    break;
                }
                else if (0 == s32Ret)
                {
                    ST_ERR("__AovGetVencStream timeout\n");
                    if(bTestModeFlag == FALSE)
                    {
                        continue;
                    }
                    else
                    {
                        g_bExit      = TRUE;
                        g_S32Ret       = -1;
                        break;
                    }
                }
                else
                {
                    if (FD_ISSET(s32VencFd, &read_fds))
                    {
                        do
                        {
                            memset(&stStream, 0, sizeof(stStream));
                            memset(stPack, 0, sizeof(stPack));
                            stStream.pstPack = stPack;
                            s32Ret           = MI_VENC_Query(s32VencDev, s32VencChn, &stStat);
                            if (s32Ret != MI_SUCCESS)
                            {
                                ST_INFO("MI_VENC_Query failed\n");
                                return NULL;
                            }

                            if (0 == stStat.u32CurPacks)
                            {
                                continue;
                            }

                            if (0 == stStat.u32LeftStreamFrames)
                            {
                                break;
                            }

                            stStream.u32PackCount = stStat.u32CurPacks;
                            s32Ret                = MI_VENC_GetStream(s32VencDev, s32VencChn, &stStream, -1);

                            if (MI_SUCCESS == s32Ret)
                            {
                                if (bTestModeFlag)
                                {
                                    if (gPile[j] == NULL)
                                    {
                                        char szFileName[256] = {0};
                                        if (j == SENSOR_0)
                                        {
                                            sprintf(szFileName, "%s/%d_snr1_low_power_mode.es", u8PicSavePath, gu32FileSeq);
                                        }
                                        else if (j == SENSOR_1)
                                        {
                                            sprintf(szFileName, "%s/%d_snr2_low_power_mode.es", u8PicSavePath, gu32FileSeq);
                                        }
                                        while (1)
                                        {
                                            gPile[j] = fopen(szFileName, "r");
                                            if (gPile[j] == NULL)
                                            {
                                                gPile[j] = fopen(szFileName, "wb+");
                                                break;
                                            }
                                            else
                                            {
                                                fclose(gPile[j]);
                                                gu32FileSeq++;
                                                if (j == SENSOR_0)
                                                {
                                                    sprintf(szFileName, "%s/%d_snr1_low_power_mode.es", u8PicSavePath, gu32FileSeq);
                                                }
                                                else if (j == SENSOR_1)
                                                {
                                                    sprintf(szFileName, "%s/%d_snr2_low_power_mode.es", u8PicSavePath, gu32FileSeq);
                                                }
                                            }
                                        }
                                        printf("save es file name is %s\n", szFileName);
                                    }
                                }
                                else
                                {
                                    if ((gPile[j] == NULL) && (g_bSavePicFlag == TRUE))
                                    {
                                        char szFileName[256] = {0};
                                        if (j == SENSOR_0)
                                        {
                                            sprintf(szFileName, "%s/%d_snr1_low_power_mode.es", u8PicSavePath, gu32FileSeq);
                                        }
                                        else if (j == SENSOR_1)
                                        {
                                            sprintf(szFileName, "%s/%d_snr2_low_power_mode.es", u8PicSavePath, gu32FileSeq);
                                        }
                                        while (1)
                                        {
                                            gPile[j] = fopen(szFileName, "r");
                                            if (gPile[j] == NULL)
                                            {
                                                gPile[j] = fopen(szFileName, "wb+");
                                                break;
                                            }
                                            else
                                            {
                                                fclose(gPile[j]);
                                                gu32FileSeq++;
                                                if (j == SENSOR_0)
                                                {
                                                    sprintf(szFileName, "%s/%d_snr1_low_power_mode.es", u8PicSavePath, gu32FileSeq);
                                                }
                                                else if (j == SENSOR_1)
                                                {
                                                    sprintf(szFileName, "%s/%d_snr2_low_power_mode.es", u8PicSavePath, gu32FileSeq);
                                                }
                                            }
                                        }
                                        printf("save es file name is %s\n", szFileName);
                                    }
                                }
                                if (bTestModeFlag)
                                {
                                    if (NULL != gPile[j])
                                    {
                                        for (MI_U32 i = 0; i < stStream.u32PackCount; i++)
                                        {
                                            fwrite(stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset, 1,
                                                    stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset, gPile[j]);
                                        }
                                    }
                                }
                                else
                                {
                                    if (g_bSavePicFlag)
                                    {
                                        if (NULL != gPile[j])
                                        {
                                            for (MI_U32 i = 0; i < stStream.u32PackCount; i++)
                                            {
                                                fwrite(stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset, 1,
                                                        stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset, gPile[j]);
                                            }
                                        }
                                    }
                                }
                                MI_VENC_ReleaseStream(s32VencDev, s32VencChn, &stStream);
                            }
                        }while((stStat.u32LeftStreamFrames > 0) && (g_bRun));
                    }
                    usleep(10 * 1000);
                }
            }
        }
    }
    printf("ST_FastBootSteamSaveThread exit !!!!!!!!!!\n");
    g_bRun = TRUE; /*for safe exit,aovid venc is using.*/
    pthread_exit(NULL);
    return NULL;
}

MI_S32 ST_VencStart(void)
{
    g_bRun = TRUE;
    pthread_create(&g_ptSaveStream, NULL, ST_FastBootSteamSaveThread, NULL);
    return MI_SUCCESS;
}
MI_S32 ST_VencStop(void)
{
    if (g_ptSaveStream != 0)
    {
        g_bRun = FALSE;
        pthread_join(g_ptSaveStream, NULL);
    }
    for (MI_U32 i = 0; i < u32SensorNum; i++)
    {
        MI_VENC_DestroyChn(0, i);
    }
    ST_MICLIRtosQuit();

    return MI_SUCCESS;
}

void ST_HandleSig(MI_S32 signo)
{
    if (signo == SIGINT)
    {
        ST_INFO("catch Ctrl + C, exit normally\n");
    }
    else if (signo == SIGTERM)
    {
        ST_INFO("catch SIGTERM, exit normally\n");
    }

    if (g_bExit)
        goto END_HANDLE_SIG;

    g_bExit = TRUE;
    usleep(100 * 1000);
    ST_VencStop();

END_HANDLE_SIG:
    ST_INFO("%s exit\n", __FUNCTION__);
    _exit(0);
}

int ST_MICLISendDataToRtos(void)
{
    MI_U8 ret = AOV_DEFAULT_STATUS;
    int   fd  = open(DEVICE_FILE, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open the device");
        return -1;
    }

    CHECK_L2R_RESULT(MI_CLI_IOCTL(fd, IOCTL_PRELOAD_DATA_L2R, AOV_GET_STATUS), ret);

    close(fd);
    return ret;
}

int ST_MICLIGetSensorNum(void)
{
    MI_U8 ret = 1;
    int   fd  = open(DEVICE_FILE, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open the device");
        return -1;
    }

    CHECK_L2R_RESULT(MI_CLI_IOCTL(fd, IOCTL_PRELOAD_DATA_L2R, AOV_GET_SENSORNUM), ret);

    close(fd);
    return ret;
}

int ST_MICLIRtosQuit(void)
{
    MI_U8 ret = 0;
    int   fd  = open(DEVICE_FILE, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open the device");
        return -1;
    }

    CHECK_L2R_RESULT(MI_CLI_IOCTL(fd, IOCTL_PRELOAD_DATA_L2R, "quit"), ret);

    close(fd);
    return ret;
}

void *ST_DealAovStatus(void *args)
{
    ST_pthreadHandel_T *ppthreadHandel = (ST_pthreadHandel_T *)args;
    MI_U8               Msg_Ret;
    while (ppthreadHandel->exit == FALSE)
    {
        usleep(10 * 1000);
        Msg_Ret     = ST_MICLISendDataToRtos();
        g_AovStatus = Msg_Ret;
        if (Msg_Ret == AOV_IPU_GET_PEOPLE)
        {
            // det get people need to save picture
            g_bSavePicFlag = TRUE;
        }
        else if ((Msg_Ret == AOV_POWER_DOWN) && (bWifiPreviewFlag == FALSE))
        {
            SafeCloseSaveFile();
            if ('v' == u8StCmd)
            {
                ST_Uart_SetWakeupSource(eUartWakeSrcWifi);
                printf("ST_DealAovStatus switch to wifi wakeup\n");
                // power off
                __PowerOff(E_ST_SYSOFF_POWEROFF, FALSE);
            }

            ST_BatteryLevel_e eBattery_level = E_ST_BATTERYLEVEL_LOW;
            eBattery_level                   = ST_Common_AovBatteryLevelCheck(&u8StCmd);
            if (bAudioAiFlag)
            {
                while (!bAudioAiDone)
                {
                    usleep(100);
                }
            }
            if (bAudioAoFlag)
            {
                while (!bAudioAoDone)
                {
                    usleep(100);
                }
            }
            if (eBattery_level == E_ST_BATTERYLEVEL_NORMAL)
            {
                // change mode & reboot
                ST_Common_AovEnterNormalPowerMode();
            }
            else
            {
                // power off
                __PowerOff(E_ST_SYSOFF_POWEROFF, FALSE);
            }
        }
    }

    ppthreadHandel->thread_exit = TRUE;
    return NULL;
}

void *ST_DealAiAudio(void *args)
{
    ST_pthreadHandel_T *ppthreadHandel = (ST_pthreadHandel_T *)args;
    MI_S32              s32Ret         = MI_SUCCESS;
    // init ai
    MI_AUDIO_DEV     stAiDevId    = 0;
    MI_U8            stAiChnGrpId = 0;
    static MI_S32    gAiFileCount = 0;
    char             u8AiFileName[512];
    WaveFileHeader_t stAiWavHead = {0};
    FILE *           fAi;
    MI_U32           u32MicDumpSize = 0;

    if (MI_AI_DupChnGroup(0, 0) != 0)
    {
        printf("error: MI_AI_DupChnGroup failed! \n");
        return NULL;
    }

    sprintf(u8AiFileName, "%s/%dst_ai_dump.wav", u8AiFIle, gAiFileCount++);
    fAi = fopen((char *)u8AiFileName, "w+");
    if (NULL == fAi)
    {
        printf("Failed to open output file [%s].\n", u8AiFileName);
        bAudioAiDone = TRUE;
        return NULL;
    }
    // write header format
    __AudioAddWaveHeader(&stAiWavHead, E_SOUND_MODE_MONO, E_MI_AUDIO_SAMPLE_RATE_8000, u32MicDumpSize);
    s32Ret = fwrite(&stAiWavHead, sizeof(WaveFileHeader_t), 1, fAi);
    if (s32Ret != 1)
    {
        printf("Failed to write dump wav head.\n");
        goto EXIT;
    }
    printf("ai start .\n");

    while (ppthreadHandel->exit == FALSE)
    {
        if (u8SimulateCmd == 'd')
        {
            break;
        }
        if ((g_AovStatus == AOV_POWER_DOWN) && (bWifiPreviewFlag == FALSE))
        {
            break;
        }
        if(g_bSavePicFlag)
        {
            __AudioDumpAIData(fAi, &u32MicDumpSize, stAiDevId, stAiChnGrpId);
        }
    }

    printf("audio stop .\n");
    // write data size in header
    __AudioAddWaveHeader(&stAiWavHead, E_SOUND_MODE_MONO, E_MI_AUDIO_SAMPLE_RATE_8000, u32MicDumpSize);
    fseek(fAi, 0, SEEK_SET);
    s32Ret = fwrite(&stAiWavHead, sizeof(WaveFileHeader_t), 1, fAi);
    if (s32Ret != 1)
    {
        printf("Failed to write dump wav head.\n");
    }

EXIT:
    bAudioAiDone = TRUE;
    fclose(fAi);
    ppthreadHandel->thread_exit = TRUE;
    return NULL;
}

void *ST_DealAoAudio(void *args)
{
    ST_pthreadHandel_T *ppthreadHandel = (ST_pthreadHandel_T *)args;
    MI_S32              s32Ret         = MI_SUCCESS;
    // init ao
    WaveFileHeader_t stAoWavHeader;
    MI_U32           playfd;
    char *           pTmpBuf         = NULL;
    MI_U32           writeBufferSize = AO_WRITE_SIZE;
    MI_AUDIO_DEV     stAoDevId       = 0;

    if (MI_AO_Dup(0) != 0)
    {
        printf("error: MI AO is no create! \n");
        bAudioAoDone = TRUE;
        return NULL;
    }
    // open play file
    playfd = open(u8AoFIle, O_RDONLY, 0666);
    if (playfd < 0)
    {
        printf("Failed to open input file error \n");
        bAudioAoDone = TRUE;
        return NULL;
    }
    // read header
    memset(&stAoWavHeader, 0, sizeof(WaveFileHeader_t));
    s32Ret = read(playfd, &stAoWavHeader, sizeof(WaveFileHeader_t));
    if (s32Ret < 0)
    {
        printf("Failed to read wav header.\n");
        goto EXIT;
    }

    pTmpBuf = malloc(writeBufferSize);
    if (NULL == pTmpBuf)
    {
        printf("Failed to alloc data buffer of file.\n");
        goto EXIT;
    }
    memset(pTmpBuf, 0, sizeof(writeBufferSize));

    printf("ao start .\n");

    while (ppthreadHandel->exit == FALSE)
    {
        if (u8SimulateCmd == 'd')
        {
            break;
        }
        if ((g_AovStatus == AOV_POWER_DOWN) && (bWifiPreviewFlag == FALSE))
        {
            break;
        }
        if (bAudioAoFlag && g_bSavePicFlag)
        {
            __AudioPlayAoData(playfd, pTmpBuf, writeBufferSize, stAoDevId);
        }
    }
    printf("ao stop .\n");
    free(pTmpBuf);
EXIT:
    bAudioAoDone = TRUE;
    close(playfd);
    ppthreadHandel->thread_exit = TRUE;
    return NULL;
}

void *ST_TestWaitQThread(void *args)
{
    printf("press c to change mode\n press q to exit\n press p to set rtc alarm 5s and poweroff\n");
    while (!g_bExit)
    {
        u8SimulateCmd = getchar();
        if (u8SimulateCmd == 'q')
        {
            g_bExit = TRUE;
            break;
        }
        else if (u8SimulateCmd == 'c')
        {
            SafeCloseSaveFile();
            ST_BatteryLevel_e eBattery_level = E_ST_BATTERYLEVEL_LOW;
            eBattery_level                   = ST_Common_AovBatteryLevelCheck(&u8SimulateCmd);
            if (eBattery_level == E_ST_BATTERYLEVEL_NORMAL)
            {
                // change mode & reboot
                ST_Common_AovEnterNormalPowerMode();
            }
            else
            {
                // power off
                __PowerOff(E_ST_SYSOFF_POWEROFF, FALSE);
            }
        }
        else if (u8SimulateCmd == 'p')
        {
            SafeCloseSaveFile();
            // set rtc alarm 4s and poweroff
            __PowerOff(E_ST_SYSOFF_POWEROFF, TRUE);
        }
        else
        {
            printf("press c to change mode\n press q to exit\n press p to set rtc alarm 5s and poweroff\n");
        }
        usleep(200 * 1000);
    }
    return NULL;
}

void *ST_WaitQThread(void *args)
{
    while (!g_bExit)
    {
        u8SimulateCmd = getchar();
        if (u8SimulateCmd == 'c')
        {
            u8StCmd = 'c';
        }
        else if(u8SimulateCmd == 'v')
        {
            u8StCmd = 'v';
            printf("ST_WaitQThread get wifi wakeup command\n");
        }
        else if (u8SimulateCmd == 'd')
        {
            if (bWifiPreviewFlag)
            {
                SafeCloseSaveFile();
                ST_BatteryLevel_e eBattery_level = E_ST_BATTERYLEVEL_LOW;
                eBattery_level                   = ST_Common_AovBatteryLevelCheck(&u8StCmd);
                if (bAudioAiFlag)
                {
                    while (!bAudioAiDone)
                    {
                        usleep(100);
                    }
                }
                if (bAudioAoFlag)
                {
                    while (!bAudioAoDone)
                    {
                        usleep(100);
                    }
                }
                if (eBattery_level == E_ST_BATTERYLEVEL_NORMAL)
                {
                    // change mode & reboot
                    ST_Common_AovEnterNormalPowerMode();
                }
                else
                {
                    ST_Uart_SetWakeupSource(eUartWakeSrcPir);
                    // power off
                    __PowerOff(E_ST_SYSOFF_POWEROFF, FALSE);
                }
            }
        }
        usleep(200 * 1000);
    }
    return NULL;
}

MI_S32 ST_StartRtspServer(void)
{
    ST_VideoStreamInfo_t stStreamInfo;

    memset(&stStreamInfo, 0x00, sizeof(ST_VideoStreamInfo_t));
    stStreamInfo.eType        = E_MI_VENC_MODTYPE_H265E;
    stStreamInfo.VencDev      = 0;
    stStreamInfo.VencChn      = 0;
    stStreamInfo.u32Width     = MI_VENC_WIDTH;
    stStreamInfo.u32Height    = MI_VENC_HEIGHT;
    stStreamInfo.u32FrameRate = 15;
    stStreamInfo.rtspIndex    = 0;

    // start rtsp
    STCHECKRESULT(ST_Common_RtspServerStartVideo(&stStreamInfo));

    if (u32SensorNum == 2)
    {
        stStreamInfo.VencChn   = 1;
        stStreamInfo.u32Width  = MI_VENC_WIDTH;
        stStreamInfo.u32Height = MI_VENC_HEIGHT;
        stStreamInfo.rtspIndex = 1;
        ST_Common_RtspServerStartVideo(&stStreamInfo);
    }

    ST_Common_Pause();

    if (u32SensorNum == 2)
    {
        ST_Common_RtspServerStopVideo(&stStreamInfo);
        stStreamInfo.VencChn   = 0;
        stStreamInfo.u32Width  = MI_VENC_WIDTH;
        stStreamInfo.u32Height = MI_VENC_HEIGHT;
        stStreamInfo.rtspIndex = 0;
    }

    // stop rtsp
    ST_Common_RtspServerStopVideo(&stStreamInfo);

    return MI_SUCCESS;
}

MI_S32 ST_GetCmdlineParam(int argc, char **argv)
{
    if (CheckIpuFile() == -1) /*if not ipu file that is not AOV,start rtsp to preview.*/
    {
        ST_StartRtspServer();
        return 1;
    }

    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "-t"))
        {
            bTestModeFlag                          = TRUE;
            gstAovHandle.stAutoTestParam.bAutoTest = bTestModeFlag;
        }
        else if (0 == strcmp(argv[i], "-ai"))
        {
            bAudioAiFlag = TRUE;
            strcpy(u8AiFIle, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "-aio"))
        {
            bAudioAiFlag = TRUE;
            bAudioAoFlag = TRUE;
            memset(u8AoFIle, 0x0, 64);
            strcpy(u8AiFIle, argv[i + 1]);
            strcpy(u8AoFIle, argv[i + 2]);
        }
        else if (0 == strcmp(argv[i], "dump"))
        {
            memset(u8PicSavePath, 0x0, 64);
            strcpy(u8PicSavePath, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "-p"))
        {
            ST_StartRtspServer();
            return 1;
        }
        else if ((0 == strcmp(argv[i], "-h")) || (0 == strcmp(argv[i], "--help")))
        {
            printf("Options are: \n");
            printf("    -h --help Display option list\n");
            printf("    -t Enter test mode\n");
            printf("    -p Enter RTSP preview mode\n");
            printf(
                "    dump Choose the es file save directory\n"
                "        sample:\"dump /tmp\"\n");
            printf(
                "    -ai Enable audio input and choose the wav file save directory\n"
                "        sample:\"-ai /tmp\"\n");
            printf(
                "    -aio Enable audio input and output and choose the wav file save and display directory\n"
                "        sample:\"-aio /tmp /tmp/xxx.wav\"\n");
            return 1;
        }
    }

    return MI_SUCCESS;
}

int main(int argc, char **argv)
{
    struct sigaction sigAction;
    pthread_t        pGetWaitQTheead;
    ST_WakeupType_e  eCurrentWakeupType;
    MI_S32           s32Ret = MI_SUCCESS;

    sigAction.sa_handler = ST_HandleSig;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    sigaction(SIGINT, &sigAction, NULL);  //-2
    sigaction(SIGKILL, &sigAction, NULL); //-9
    sigaction(SIGTERM, &sigAction, NULL); //-15

    if (E_ST_OS_DUALOS != ST_Common_AovOSCheck())
    {
        ST_INFO("Does not match the expected OS\n");
        return -1;
    }

    MI_SYS_Init(0);
    MI_IQSERVER_Open();
    u32SensorNum = ST_MICLIGetSensorNum();
    printf("exit ST_MICLIGetSensorNum!\n");
    if (u32SensorNum == -1)
    {
        u32SensorNum = 1;
    }
    for (u32 i = 0; i < u32SensorNum; i++)
    {
        MI_VENC_DupChn(0, i);
    }
    s32Ret = ST_GetCmdlineParam(argc, argv);
    if (s32Ret == 1)
    {
        MI_SYS_Exit(0);
        return 0;
    }
    gstAovHandle.stAovPipeAttr.u32SensorNum = u32SensorNum;
    ST_Common_AovMCUUartInit();
    eCurrentWakeupType = ST_Common_AovWakeupCheck(&gstAovHandle);
    STCHECKRESULT(ST_VencStart());
    if (bTestModeFlag) /*test mode*/
    {
        pthread_create(&pGetWaitQTheead, NULL, ST_TestWaitQThread, NULL);
    }
    else
    {
        if (eCurrentWakeupType == E_ST_WAKEUP_PREVIEW) /*wifi preview mode*/
        {
            printf("enter wifi previwe mode!\n");
            g_bSavePicFlag   = TRUE;
            bWifiPreviewFlag = TRUE;
        }
        pthread_create(&pGetWaitQTheead, NULL, ST_WaitQThread, NULL);
        ST_Thread_Start(&pthreadMsg, MSG_THEADNAME, ST_DealAovStatus);
        if (bAudioAiFlag)
        {
            ST_Thread_Start(&pthreadAi, AI_THEADNAME, ST_DealAiAudio);
        }
        if (bAudioAoFlag)
        {
            ST_Thread_Start(&pthreadAo, AO_THEADNAME, ST_DealAoAudio);
        }
    }

    while (!g_bExit)
    {
        usleep(100 * 1000);
    }

    if (!bTestModeFlag)
    {
        ST_Thread_Stop(&pthreadMsg, MSG_THEADNAME);
        if (bAudioAiFlag)
        {
            ST_Thread_Stop(&pthreadAi, AI_THEADNAME);
        }
        if (bAudioAoFlag)
        {
            ST_Thread_Stop(&pthreadAo, AO_THEADNAME);
        }
    }

    if (pGetWaitQTheead != 0)
    {
        pthread_join(pGetWaitQTheead, NULL);
    }

    SafeCloseSaveFile();
    STCHECKRESULT(ST_VencStop());
    MI_IQSERVER_Close();
    MI_SYS_Exit(0);

    return g_S32Ret;
}
