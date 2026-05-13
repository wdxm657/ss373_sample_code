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
#include <errno.h>
#include <fcntl.h>
#include "mi_common_datatype.h"
#include "mi_sys.h"
#include "mi_ao.h"
#include "st_common_audio.h"
#include "st_common.h"

MI_S16 g_s16Gains = 0;

void ST_AO_Usage(char **argv)
{
    printf("Usage:%s gian x,x from -63.5 to 64", argv[0]);
}

MI_S32 ST_AO_GainControl(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "gain"))
        {
            g_s16Gains = atoi(argv[i + 1]) * 8;
        }
    }
    return MI_SUCCESS;
}

MI_U32 ST_PlayAoData(char *pPlayFileName, MI_AUDIO_DEV AoDevId)
{
    WaveFileHeader_t stWavHeader;
    MI_U32           playfd;
    MI_S32           s32ReadSize     = 0;
    char *           pTmpBuf         = NULL;
    MI_U32           writeBufferSize = 4096;
    MI_U32           s32Ret;
    MI_BOOL          bAoExit = FALSE;

    // open play file
    playfd = open((char *)pPlayFileName, O_RDONLY, 0666);
    if (playfd < 0)
    {
        printf("Failed to open input file error[%s] \n", strerror(errno));
        return -1;
    }

    // read header
    memset(&stWavHeader, 0, sizeof(WaveFileHeader_t));
    s32Ret = read(playfd, &stWavHeader, sizeof(WaveFileHeader_t));
    if (s32Ret < 0)
    {
        close(playfd);
        printf("Failed to read wav header.\n");
        return -1;
    }

    pTmpBuf = malloc(writeBufferSize);
    if (NULL == pTmpBuf)
    {
        close(playfd);
        printf("Failed to alloc data buffer of file.\n");
        return -1;
    }
    memset(pTmpBuf, 0, sizeof(writeBufferSize));

    // play audio form input file
    while (FALSE == bAoExit)
    {
        s32ReadSize = read(playfd, pTmpBuf, writeBufferSize);
        if (s32ReadSize != writeBufferSize)
        {
            lseek(playfd, sizeof(WaveFileHeader_t), SEEK_SET);
            s32ReadSize = read(playfd, pTmpBuf, writeBufferSize);
            bAoExit     = TRUE;
        }

        s32Ret = MI_AO_Write(AoDevId, pTmpBuf, s32ReadSize, 0, -1);

        if (s32Ret != MI_SUCCESS)
        {
            printf("Failed to call MI_AO_Write of Ao DevICE %d , error is 0x%x.\n", AoDevId, s32Ret);
        }
    }

    free(pTmpBuf);
    close(playfd);
    return MI_SUCCESS;
}

int main(int argc, char **argv)
{
    ST_AO_Usage(argv);
    ST_AO_GainControl(argc, argv);
    // init sys
    STCHECKRESULT(MI_SYS_Init(0));

    // init ao
    MI_AO_Attr_t stAoSetAttr;
    MI_AUDIO_DEV stAoDevId  = 0;
    MI_AO_If_e   aenAoIfs[] = {E_MI_AO_IF_DAC_AB};
    memset(&stAoSetAttr, 0x0, sizeof(MI_AO_Attr_t));

    STCHECKRESULT(ST_Common_GetAoDefaultDevAttr(&stAoSetAttr));
    STCHECKRESULT(ST_Common_AoOpenDev(stAoDevId, &stAoSetAttr));
    STCHECKRESULT(ST_Common_AoAttachIf(stAoDevId, aenAoIfs));

    MI_AO_SetVolume(stAoDevId, g_s16Gains, g_s16Gains, 0);

    char *input_file = "resource/input/audio/ao_8K_16bit_MONO_30s.wav";
    printf("input_file = %s\n", input_file);

    // play audio form input file
    ST_PlayAoData(input_file, stAoDevId);

    // deinit ao
    ST_Common_AoCloseDev(stAoDevId);

    // deinit sys
    STCHECKRESULT(MI_SYS_Exit(0));
}
