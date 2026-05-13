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
    short        input[256];
    FILE *       fin;
    FILE *       fout;
    ALGO_SSL_RET ret;
    int          counter2 = 0;

    /**********common setting SSL***************/
    int   point_number        = 128;
    float microphone_distance = 12.0;
    int   temperature         = 20;
    int   sample_rate         = 16000;
    int   delay_sample[1]     = {0};
    int   shape               = 0;
    int   direction           = 0;
    int   frame_number        = 32;

    char *input_file  = "resource/input/audio/2mic_ssl_original.wav";
    char *output_file = "out/audio/2mic_ssl_processed.txt";
    /**************SSL data init***********/

    printf("input_file  = %s\n", input_file);
    printf("output_file = %s\n\n", output_file);

    int   counter = 0;
    char *WorkingBuffer2;
    WorkingBuffer2 = (char *)malloc(IaaSsl_GetBufferSize());

    AudioSslInit   ssl_init;
    AudioSslConfig ssl_config;
    SSL_HANDLE     handle;

    ssl_init.mic_distance          = microphone_distance; // cm
    ssl_init.point_number          = point_number;
    ssl_init.sample_rate           = sample_rate;
    ssl_init.bf_mode               = 0;
    ssl_init.channel               = 2;
    ssl_config.temperature         = temperature; // c
    ssl_config.noise_gate_dbfs     = -80;
    ssl_config.direction_frame_num = frame_number;

    /******init algorithm********/
    handle = IaaSsl_Init((char *)WorkingBuffer2, &ssl_init);
    if (handle == NULL)
    {
        printf("SSL init error\n\r");
        return -1;
    }

    ret = IaaSsl_Config(handle, &(ssl_config));
    if (ret)
    {
        printf("Error occured in SSL Config\n\r");
        return -1;
    }

    ret = IaaSsl_Set_Shape(handle, shape);
    if (ret)
    {
        printf("Error occured in Array shape\n\r");
        return -1;
    }

    ret = IaaSsl_Cal_Params(handle);
    if (ret)
    {
        printf("Error occured in Array matrix calculation\n\r");
        return -1;
    }

    fin = fopen(input_file, "rb");
    if (!fin)
    {
        printf("the input file 0 could not be open\n\r");
        return -1;
    }

    ST_Common_CheckMkdirOutFile(output_file);
    fout = fopen(output_file, "w");
    if (!fout)
    {
        printf("the output file could not be open\n\r");
        return -1;
    }

    fread(input, sizeof(char), 44, fin); // read header 44 bytes
    fprintf(fout, "%s\t%s\t%s\n\r", "time", "direction", "case");
    while (fread(input, sizeof(short), ssl_init.point_number * 2, fin))
    {
        counter++;
        ret = IaaSsl_Run(handle, input, delay_sample);
        if (ret != 0)
        {
            printf("The Run fail\n");
            return -1;
        }

        if (counter == ssl_config.direction_frame_num && ssl_init.bf_mode == 0)
        {
            counter2++;
            counter = 0;
            ret     = IaaSsl_Get_Direction(handle, &direction);
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
            ret = IaaSsl_Reset_Mapping(handle);
            if (ret != 0)
            {
                printf("The ResetVoting fail\n");
                return -1;
            }
        }
    }

    IaaSsl_Free(handle);
    fclose(fin);
    fclose(fout);
    free(WorkingBuffer2);
    return 0;
}
