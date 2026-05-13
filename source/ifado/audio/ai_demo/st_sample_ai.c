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
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include "mi_sys.h"
#include "mi_ai.h"
#include "st_common_audio.h"
#include "st_common.h"

MI_S16 g_s16Gains[] = {-10};

void ST_AI_Usage(char **argv)
{
    printf("Usage:%s gian x,x from -63.5 to 64", argv[0]);
}

MI_S32 ST_AI_GainControl(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "gain"))
        {
            g_s16Gains[0] = atoi(argv[i + 1]) * 8;
        }
    }
    return MI_SUCCESS;
}

MI_S32 ST_AddWaveHeader(WaveFileHeader_t *tWavHead, SoundMode_e enSoundMode, SampleRate_e enSampleRate, MI_U32 u32Len)
{
    tWavHead->chRIFF[0] = 'R';
    tWavHead->chRIFF[1] = 'I';
    tWavHead->chRIFF[2] = 'F';
    tWavHead->chRIFF[3] = 'F';

    tWavHead->chWAVE[0] = 'W';
    tWavHead->chWAVE[1] = 'A';
    tWavHead->chWAVE[2] = 'V';
    tWavHead->chWAVE[3] = 'E';

    tWavHead->chFMT[0] = 'f';
    tWavHead->chFMT[1] = 'm';
    tWavHead->chFMT[2] = 't';
    tWavHead->chFMT[3] = 0x20;
    tWavHead->dwFMTLen = 0x10;

    if (enSoundMode == E_SOUND_MODE_MONO)
    {
        tWavHead->wave.wChannels = 0x01;
    }
    else if (enSoundMode == E_SOUND_MODE_STEREO)
    {
        tWavHead->wave.wChannels = 0x02;
    }

    tWavHead->wave.wFormatTag      = 0x1;
    tWavHead->wave.wBitsPerSample  = 16; // 16bit
    tWavHead->wave.dwSamplesPerSec = enSampleRate;
    tWavHead->wave.dwAvgBytesPerSec =
        (tWavHead->wave.wBitsPerSample * tWavHead->wave.dwSamplesPerSec * tWavHead->wave.wChannels) / 8;
    tWavHead->wave.wBlockAlign = 1024;
    tWavHead->chDATA[0]        = 'd';
    tWavHead->chDATA[1]        = 'a';
    tWavHead->chDATA[2]        = 't';
    tWavHead->chDATA[3]        = 'a';
    tWavHead->dwDATALen        = u32Len;
    tWavHead->dwRIFFLen        = u32Len + sizeof(WaveFileHeader_t) - 8;

    return MI_SUCCESS;
}

MI_U32 ST_DumpAIData(char *pDumpFileName, MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpId)
{
    MI_AI_Data_t     stMicFrame;
    MI_AI_Data_t     stEchoFrame;
    MI_S32           s32Ret;
    struct timeval   beforeWriteFile, afterWriteFile, lastRead, nowRead, baseRead;
    MI_S64           beforeWriteFileUs = 0, afterWriteFileUs = 0, lastReadUs = 0, nowReadUs = 0;
    MI_U8            u8DumpFileIdx  = 0;
    FILE *           AiChnFd        = NULL;
    MI_U32           u32MicDumpSize = 0;
    WaveFileHeader_t stWavHead      = {0};

    // write header format
    AiChnFd = fopen((char *)pDumpFileName, "w+");
    if (NULL == AiChnFd)
    {
        printf("Failed to open output file [%s].\n", pDumpFileName);
        return -1;
    }
    STCHECKRESULT(ST_AddWaveHeader(&stWavHead, E_SOUND_MODE_MONO, E_MI_AUDIO_SAMPLE_RATE_8000, u32MicDumpSize));
    s32Ret = fwrite(&stWavHead, sizeof(WaveFileHeader_t), 1, AiChnFd);
    if (s32Ret != 1)
    {
        printf("Failed to write dump wav head.\n");
        fclose(AiChnFd);
        return -1;
    }

    // accept 10 seconds of audio
    gettimeofday(&nowRead, NULL);
    gettimeofday(&lastRead, NULL);
    gettimeofday(&baseRead, NULL);
    while ((nowRead.tv_sec - baseRead.tv_sec) < 10)
    {
        memset(&stMicFrame, 0, sizeof(MI_AI_Data_t));
        memset(&stEchoFrame, 0, sizeof(MI_AI_Data_t));

        s32Ret = MI_AI_Read(AiDevId, u8ChnGrpId, &stMicFrame, &stEchoFrame, -1);

        if (MI_SUCCESS == s32Ret)
        {
            gettimeofday(&nowRead, NULL);
            nowReadUs  = nowRead.tv_sec * 1000000 + nowRead.tv_usec;
            lastReadUs = lastRead.tv_sec * 1000000 + lastRead.tv_usec;
            printf("######## Ai Device %d ChnGrp %d cost time of getting one frame:%lldms ########\n", AiDevId,
                   u8ChnGrpId, (nowReadUs - lastReadUs) / 1000);
            lastRead = nowRead;
            /* save mic file data */
            gettimeofday(&beforeWriteFile, NULL);
            fwrite(stMicFrame.apvBuffer[0], 1, stMicFrame.u32Byte[0], AiChnFd);
            gettimeofday(&afterWriteFile, NULL);

            beforeWriteFileUs = beforeWriteFile.tv_sec * 1000000 + beforeWriteFile.tv_usec;
            afterWriteFileUs  = afterWriteFile.tv_sec * 1000000 + afterWriteFile.tv_usec;
            if (afterWriteFileUs - beforeWriteFileUs > 10 * 1000)
            {
                printf("Ai Device %d ChnGrp %d Chn %d cost time of writing one frame:%lldms.\n", AiDevId, u8ChnGrpId,
                       u8DumpFileIdx, (afterWriteFileUs - beforeWriteFileUs) / 1000);
            }
            u32MicDumpSize += stMicFrame.u32Byte[0];

            s32Ret = MI_AI_ReleaseData(AiDevId, u8ChnGrpId, &stMicFrame, &stEchoFrame);
            if (s32Ret != MI_SUCCESS)
            {
                printf("%s:%d MI_AI_ReleaseData s32Ret:%d\n", __FUNCTION__, __LINE__, s32Ret);
            }
        }
        else
        {
            printf("Failed to get frame from Ai Device %d ChnGrp %d, error:0x%x\n", AiDevId, u8ChnGrpId, s32Ret);
        }
    }

    // write data size in header
    STCHECKRESULT(ST_AddWaveHeader(&stWavHead, E_SOUND_MODE_MONO, E_MI_AUDIO_SAMPLE_RATE_8000, u32MicDumpSize));
    fseek(AiChnFd, 0, SEEK_SET);
    s32Ret = fwrite(&stWavHead, sizeof(WaveFileHeader_t), 1, AiChnFd);
    if (s32Ret != 1)
    {
        printf("Failed to write dump wav head.\n");
        fclose(AiChnFd);
        return -1;
    }
    fclose(AiChnFd);
    return NULL;
}

MI_S32 main(int argc, char **argv)
{
    ST_AI_Usage(argv);
    ST_AI_GainControl(argc, argv);
    // init sys
    STCHECKRESULT(MI_SYS_Init(0));

    // init ai
    MI_AI_Attr_t stAiDevAttr;
    MI_AUDIO_DEV stAiDevId  = 0;
    MI_U8        stChnGrpId = 0;
    MI_AI_If_e   enAiIf[]   = {E_MI_AI_IF_ADC_AB, E_MI_AI_IF_ECHO_A};

    memset(&stAiDevAttr, 0x0, sizeof(MI_AI_Attr_t));

    ST_Common_GetAiDefaultDevAttr(&stAiDevAttr);
    ST_Common_AiOpenDev(stAiDevId, &stAiDevAttr);
    ST_Common_AiAttachIf(stAiDevId, stChnGrpId, enAiIf, 2);

    MI_AI_SetGain(stAiDevId, stChnGrpId, g_s16Gains, sizeof(g_s16Gains)/sizeof(g_s16Gains[0]));

    // enable ai chngroup
    ST_Common_AiEnableChnGroup(stAiDevId, stChnGrpId);

    // dump ai data
    MI_SYS_ChnPort_t stChnOutputPort;
    memset(&stChnOutputPort, 0x0, sizeof(stChnOutputPort));
    stChnOutputPort.eModId    = E_MI_MODULE_ID_AI;
    stChnOutputPort.u32DevId  = stAiDevId;
    stChnOutputPort.u32ChnId  = stChnGrpId;
    stChnOutputPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stChnOutputPort, 3, 5));

    char *output_file = "out/audio/ai_dump.wav";
    printf("output_file = %s\n", output_file);
    ST_Common_CheckMkdirOutFile(output_file);

    ST_DumpAIData(output_file, stAiDevId, stChnGrpId);

    // deinit ai
    ST_Common_AiDisableChnGroup(stAiDevId, stChnGrpId);
    ST_Common_AiCloseDev(stAiDevId);
    // deinit sys
    STCHECKRESULT(MI_SYS_Exit(0));

    return 0;
}
