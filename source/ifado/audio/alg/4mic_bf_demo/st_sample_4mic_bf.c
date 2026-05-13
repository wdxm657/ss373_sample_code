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

unsigned int _OsCounterGetMs(void)
{
    struct timeval t1;
    gettimeofday(&t1, NULL);
    unsigned int T = ((1000000 * t1.tv_sec) + t1.tv_usec) / 1000;
    return T;
}

int main(int argc, char *argv[])
{
    short input[512];
    short output[128];
    short input_tmp1[128], input_tmp2[128], input_tmp3[128], input_tmp4[128];
    FILE *fin1, *fin2, *fin3, *fin4, *fout;

    ALGO_BF_RET ret;
    int         k;

    /********common setting between SSL and BF********/
    int   point_number        = 128;
    float microphone_distance = 4.0;
    int   temperature         = 20;
    int   sample_rate         = 16000;
    int   shape               = 0;
    float direction           = 0.0;
    /*******BF data init*********/

    char *WorkingBuffer_BF;
    WorkingBuffer_BF = (char *)malloc(IaaBf_GetBufferSize());

    AudioBfInit   bf_init;
    AudioBfConfig bf_config;
    BF_HANDLE     bf_handle;

    bf_init.mic_distance       = microphone_distance;
    bf_init.point_number       = point_number;
    bf_init.sample_rate        = sample_rate;
    bf_init.channel            = 4;
    bf_config.noise_gate_dbfs  = -20;
    bf_config.temperature      = temperature;
    bf_config.noise_estimation = 0;
    bf_config.output_gain      = 0.7;
    bf_config.vad_enable       = 0;
    bf_config.diagonal_loading = 10;

    /*******Set input/output*********/
    char *input_file1 = "resource/input/audio/4mic_bf_Chn-01.wav";
    char *input_file2 = "resource/input/audio/4mic_bf_Chn-02.wav";
    char *input_file3 = "resource/input/audio/4mic_bf_Chn-03.wav";
    char *input_file4 = "resource/input/audio/4mic_bf_Chn-04.wav";
    char *output_file = "out/audio/4mic_bf_processed.pcm";

    printf("input_file1 = %s\n", input_file1);
    printf("input_file2 = %s\n", input_file2);
    printf("input_file3 = %s\n", input_file3);
    printf("input_file4 = %s\n", input_file4);
    printf("output_file = %s\n\n", output_file);

    bf_handle = IaaBf_Init((char *)WorkingBuffer_BF, &bf_init);
    if (bf_handle == NULL)
    {
        printf("BF init error\r\n");
        return -1;
    }

    ret = IaaBf_SetConfig(bf_handle, &bf_config);
    if (ret)
    {
        printf("Error occured in Config\n");
        return -1;
    }

    ret = IaaBf_SetShape(bf_handle, shape);
    if (ret)
    {
        printf("Error occured in Array shape\n");
        return -1;
    }

    /********open input file and output file*****/
    fin1 = fopen(input_file1, "rb");
    if (!fin1)
    {
        printf("the input file 0 could not be open\n");
        return -1;
    }

    fin2 = fopen(input_file2, "rb");
    if (!fin2)
    {
        printf("the input file 1 could not be open\n");
        return -1;
    }

    fin3 = fopen(input_file3, "rb");
    if (!fin3)
    {
        printf("the input file 2 could not be open\n");
        return -1;
    }

    fin4 = fopen(input_file4, "rb");
    if (!fin4)
    {
        printf("the input file 3 could not be open\n");
        return -1;
    }

    ST_Common_CheckMkdirOutFile(output_file);
    fout = fopen(output_file, "wb");
    if (!fout)
    {
        printf("the output file could not be open\n");
        return -1;
    }

    fread(input, sizeof(char), 44, fin1);
    fread(input, sizeof(char), 44, fin2);
    fread(input, sizeof(char), 44, fin3);
    fread(input, sizeof(char), 44, fin4);
    fwrite(input, sizeof(char), 44, fout);
    short *input_ptr;

    while (fread(input_tmp1, sizeof(short), bf_init.point_number, fin1))
    {
        fread(input_tmp2, sizeof(short), bf_init.point_number, fin2);
        fread(input_tmp3, sizeof(short), bf_init.point_number, fin3);
        fread(input_tmp4, sizeof(short), bf_init.point_number, fin4);
        input_ptr = input;
        for (k = 0; k < point_number; k++)
        {
            *input_ptr = input_tmp1[k];
            input_ptr++;
            *input_ptr = input_tmp2[k];
            input_ptr++;
            *input_ptr = input_tmp3[k];
            input_ptr++;
            *input_ptr = input_tmp4[k];
            input_ptr++;
        }

        ret = IaaBf_Run(bf_handle, input, output, &direction);
        if (ret)
        {
            printf("Error occured in Run\n");
            return -1;
        }
        fwrite(output, sizeof(short), point_number, fout);
    }

    IaaBf_Free(bf_handle);
    fclose(fin1);
    fclose(fin2);
    fclose(fin3);
    fclose(fin4);
    fclose(fout);
    free(WorkingBuffer_BF);
    return 0;
}
