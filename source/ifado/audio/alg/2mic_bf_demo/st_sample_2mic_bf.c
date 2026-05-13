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
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "AudioBfProcess.h"
#include "st_common.h"

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;

int main(int argc, char *argv[])
{
    short         input[256];
    short         output[128];
    FILE *        fin, *fout;
    ALGO_BF_RET   ret;
    int           shape     = 0;
    float         direction = 0.0;
    AudioBfInit   bf_init;
    AudioBfConfig bf_config;
    BF_HANDLE     handle;

    bf_init.mic_distance       = 8.0;
    bf_init.point_number       = 128;
    bf_init.sample_rate        = 16000;
    bf_init.channel            = 2;
    bf_config.noise_gate_dbfs  = -20;
    bf_config.temperature      = 20;
    bf_config.noise_estimation = 0;
    bf_config.output_gain      = 0.7;
    bf_config.vad_enable       = 0;
    bf_config.diagonal_loading = 10;

    char *input_file  = "resource/input/audio/2mic_bf_original.wav";
    char *output_file = "out/audio/2mic_bf_processed.pcm";

    printf("input_file  = %s\n", input_file);
    printf("output_file = %s\n\n", output_file);

    char *WorkingBuffer2;
    WorkingBuffer2 = (char *)malloc(IaaBf_GetBufferSize());

    handle = IaaBf_Init((char *)WorkingBuffer2, &bf_init);
    if (handle == NULL)
    {
        printf("BF init error\r\n");
        return -1;
    }

    ret = IaaBf_SetConfig(handle, &bf_config);
    if (ret)
    {
        printf("Error occured in Config\n");
        return -1;
    }

    ret = IaaBf_SetShape(handle, shape);
    if (ret)
    {
        printf("Error occured in Array shape\n");
        return -1;
    }

    fin = fopen(input_file, "rb");
    if (!fin)
    {
        printf("the input file could not be open\n");
        return -1;
    }

    ST_Common_CheckMkdirOutFile(output_file);
    fout = fopen(output_file, "wb");
    if (!fout)
    {
        printf("the output file could not be open\n");
        return -1;
    }

    fread(input, sizeof(char), 44, fin); // read header 44 bytes

    while (fread(input, sizeof(short), bf_init.point_number * bf_init.channel, fin))
    {
        ret = IaaBf_Run(handle, input, output, &direction);
        if (ret)
        {
            printf("Error occured in Run\n");
            return -1;
        }
        fwrite(output, sizeof(short), bf_init.point_number, fout);
    }

    IaaBf_Free(handle);
    fclose(fin);
    fclose(fout);
    free(WorkingBuffer2);
    return 0;
}
