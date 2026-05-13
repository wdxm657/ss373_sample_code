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
#include "AudioSslProcess.h"
#include "st_common.h"

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;

int main(int argc, char *argv[])
{
    short        input[512];
    short        input_tmp1[128], input_tmp2[128], input_tmp3[128], input_tmp4[128];
    FILE *       fin1, *fin2, *fin3, *fin4;
    FILE *       fout;
    int          k;
    ALGO_SSL_RET ret;
    int          counter2 = 0;

    /********common setting  SSL ********/
    int   point_number        = 128;
    float microphone_distance = 4.0;
    int   temperature         = 20;
    int   sample_rate         = 16000;
    int   delay_sample[4 - 1] = {0, 0, 0}; // channel-1
    int   shape               = 0;
    int   direction           = 0;
    int   frame_number        = 32;
    /********SSL data init********/

    int counter = 0;

    char *WorkingBuffer_SSL;
    WorkingBuffer_SSL = (char *)malloc(IaaSsl_GetBufferSize());

    AudioSslInit   ssl_init;
    AudioSslConfig ssl_config;
    SSL_HANDLE     ssl_handle;

    ssl_init.mic_distance          = microphone_distance;
    ssl_init.point_number          = point_number;
    ssl_init.sample_rate           = sample_rate;
    ssl_init.bf_mode               = 0;
    ssl_init.channel               = 4;
    ssl_config.temperature         = temperature;
    ssl_config.noise_gate_dbfs     = -80;
    ssl_config.direction_frame_num = frame_number;

    /*******Set input/output*********/
    char *input_file1 = "resource/input/audio/4mic_ssl_Chn-01.wav";
    char *input_file2 = "resource/input/audio/4mic_ssl_Chn-02.wav";
    char *input_file3 = "resource/input/audio/4mic_ssl_Chn-03.wav";
    char *input_file4 = "resource/input/audio/4mic_ssl_Chn-04.wav";
    char *output_file = "out/audio/4mic_ssl_processed.txt";

    printf("input_file1 = %s\n", input_file1);
    printf("input_file2 = %s\n", input_file2);
    printf("input_file3 = %s\n", input_file3);
    printf("input_file4 = %s\n", input_file4);
    printf("output_file = %s\n\n", output_file);

    /*******init algorithm *****/
    ssl_handle = IaaSsl_Init((char *)WorkingBuffer_SSL, &ssl_init);
    if (ssl_handle == NULL)
    {
        printf("Init fail\n\r");
        return -1;
    }

    ret = IaaSsl_Config(ssl_handle, &(ssl_config));
    if (ret)
    {
        printf("Error occured in SSL Config\n\r");
        return -1;
    }

    ret = IaaSsl_Set_Shape(ssl_handle, shape);
    if (ret)
    {
        printf("Error occured in Array shape\n\r");
        return -1;
    }

    ret = IaaSsl_Cal_Params(ssl_handle);
    if (ret)
    {
        printf("Error occured in Array matrix calculation\n\r");
        return -1;
    }

    /********open input file and input file*****/
    fin1 = fopen(input_file1, "rb");
    if (!fin1)
    {
        printf("the input file0 could not be open\n\r");
        return -1;
    }

    fin2 = fopen(input_file2, "rb");
    if (!fin2)
    {
        printf("the input file 1 could not be open\n\r");
        return -1;
    }

    fin3 = fopen(input_file3, "rb");
    if (!fin3)
    {
        printf("the input file 2 could not be open\n\r");
        return -1;
    }

    fin4 = fopen(input_file4, "rb");
    if (!fin4)
    {
        printf("the input file 3 could not be open\n\r");
        return -1;
    }

    ST_Common_CheckMkdirOutFile(output_file);
    fout = fopen(output_file, "w");
    if (!fout)
    {
        printf("the output file could not be open\n\r");
        return -1;
    }

    fread(input, sizeof(char), 44, fin1); // read header 44 bytes
    fread(input, sizeof(char), 44, fin2); // read header 44 bytes
    fread(input, sizeof(char), 44, fin3); // read header 44 bytes
    fread(input, sizeof(char), 44, fin4); // read header 44 bytes

    short *input_ptr;
    fprintf(fout, "%s\t%s\t%s\n\r", "time", "direction", "case");
    while (fread(input_tmp1, sizeof(short), point_number, fin1))
    {
        fread(input_tmp2, sizeof(short), point_number, fin2);
        fread(input_tmp3, sizeof(short), point_number, fin3);
        fread(input_tmp4, sizeof(short), point_number, fin4);
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
        counter++;

        ret = IaaSsl_Run(ssl_handle, input, delay_sample);
        if (ret != 0)
        {
            printf("The Run fail\n");
            return -1;
        }

        if (counter == ssl_config.direction_frame_num && ssl_init.bf_mode == 0)
        {
            counter2++;
            counter = 0;
            ret     = IaaSsl_Get_Direction(ssl_handle, &direction);
            if (ret != 0 && ret != ALGO_SSL_RET_RESULT_UNRELIABLE && ret != ALGO_SSL_RET_BELOW_NOISE_GATE
                && ret != ALGO_SSL_RET_DELAY_SAMPLE_TOO_LARGE)
            {
                printf("The Get_Direction fail\n");
                return -1;
            }

            // write txt file
            fprintf(fout, "%f\t%d", (float)(counter2 * ssl_config.direction_frame_num * 0.008), direction);
            if (ret == 0)
            {
                fprintf(fout, "\t%s\n\r", "current time is reliable!");
            }
            else if (ret == ALGO_SSL_RET_BELOW_NOISE_GATE)
            {
                fprintf(fout, "\t%s\n\r", "current time volume is too small!");
            }
            else if (ret == ALGO_SSL_RET_DELAY_SAMPLE_TOO_LARGE)
            {
                fprintf(fout, "\t%s\n\r", "current time delay_sample is out of range!");
            }
            else
            {
                fprintf(fout, "\t%s\n\r", "current time is not reliable!");
            }

            // reset voting
            ret = IaaSsl_Reset_Mapping(ssl_handle);
            if (ret != 0)
            {
                printf("The ResetVoting fail\n");
                return -1;
            }
        }
    }

    IaaSsl_Free(ssl_handle);
    fclose(fin1);
    fclose(fin2);
    fclose(fin3);
    fclose(fin4);
    fclose(fout);
    free(WorkingBuffer_SSL);
    return 0;
}
