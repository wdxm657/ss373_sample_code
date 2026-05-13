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
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "AudioVadProcess.h"
#include "st_common.h"

#define FLAG (32700)
#define BUF_LEN (4096)

int main(int argc, char *argv[])
{
    short            input[BUF_LEN];
    short            output[BUF_LEN];
    int              length;
    int              mode = 1;
    FILE *           fin;
    FILE *           out;
    char *           WorkingBuffer = NULL;
    int              workingBufferSize;
    int              ret;
    VAD_HANDLE       handle;
    VadInit          vad_init;
    VadConfig        vad_config;
    int              PN = 128;

    /*********************User change section start*******************/
    vad_init.point_number = PN;
    vad_init.channel = 1;
    vad_init.sample_rate = IAA_VAD_SAMPLE_RATE_16000;

    vad_config.vote_frame = 100;
    vad_config.sensitivity = VAD_SEN_HIGH;

    char *input_file  = "resource/input/audio/vad_original.wav";
    char *output_file = "out/audio/vad_processed.wav";
    printf("input_file  = %s\n", input_file);
    printf("output_file = %s\n\n", output_file);
    /*********************User change section end*******************/
    fin = fopen(input_file, "rb");
    if (!fin)
    {
        perror(input_file);
        return -1;
    }
    fread(input, sizeof(char), 44, fin);

    ST_Common_CheckMkdirOutFile(output_file);
    out = fopen(output_file, "wb");
    if (!out)
    {
        perror(output_file);
        return -1;
    }
    fwrite(input, sizeof(char), 44, out);

    //(1)IaaVad_GetBufferSize
    workingBufferSize = IaaVad_GetBufferSize();
    WorkingBuffer     = (char *)malloc(workingBufferSize);
    if (NULL == WorkingBuffer)
    {
        printf("WorkingBuffer malloc failed !\n");
        return -1;
    }

    //(2)IaaVad_Init
    handle = IaaVad_Init((char *)WorkingBuffer, &vad_init);
    if (handle == NULL)
    {
        printf("VAD init error !\n");
        return -1;
    }

    //(3)IaaVad_Config
    if(IaaVad_Config(handle, &vad_config) == -1)
    {
        printf("Config Error!");
        return -1;
    }

    //(4)IaaVad_SetMode
    if(IaaVad_SetMode(handle, mode) == -1)
    {
        printf("SetMode Error!");
        return -1;
    }

    length = vad_init.point_number*vad_init.channel;
    while (fread(input, sizeof(short), length, fin))
    {
        ret = IaaVad_Run(handle, input);
        if(ret < 0)
        {
            printf("Error occured in Voice Activity Detection\n");
            break;
        }

        for (int i = 0; i <= length; i++)
        {
            output[i] = ret * FLAG;
        }
        fwrite(output, sizeof(short), length, out);
    }

    IaaVad_Free(handle);
    fclose(fin);
    fclose(out);
    free(WorkingBuffer);
}
