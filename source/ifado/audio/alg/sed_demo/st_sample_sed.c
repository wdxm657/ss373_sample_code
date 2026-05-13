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

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "AudioSedProcess.h"
#include "st_common.h"

#define FRAME_SIZE (256)

int main(int argc, char *argv[])
{
    int   counter = 0;
    short buf[FRAME_SIZE];
    FILE *in, *out;

    int              ret;
    int              input_len;
    int              event_idx;
    int              detect_flag;
    SED_HANDLE       handle;
    AudioSedInit_t   sedInit;
    AudioSedConfig_t sedConfig;

    float threshold[] = {0.5, 0.5};
    /*********************User change section start*******************/
    sedInit.sampleRate = 16000;
    sedInit.bitWidth   = 16;
    sedInit.ipuMaxSize = 0;
    sedInit.hadCreateDevice = 0;

    sedConfig.detectMode   = IAA_ROBUST_MODE;
    sedConfig.smoothLength = 3;
    sedConfig.vadThreshold = -30;
    memcpy(sedConfig.eventThreshold, threshold, sizeof(float) * 2);

    sprintf(sedInit.modelPath, "resource/input/audio/sed_tcbs.img");
    char *input_audio = "resource/input/audio/sed_babycry.wav";
    char *output_file = "out/audio/sed_processed.txt";
    /*********************User change section end*******************/

    printf("input_audio  = %s\n", input_audio);
    printf("input_model  = %s\n", sedInit.modelPath);
    printf("output_file = %s\n\n", output_file);

    ST_Common_CheckMkdirOutFile(output_file);
    out = fopen(output_file, "wb");
    if (!out)
    {
        printf("the output file could not be open\n");
        return -1;
    }

    // IaaSed_Init
    handle = IaaSed_Init(&sedInit);
    if (handle == NULL)
    {
        printf("SED init error !\n");
        return -1;
    }

    // IaaSed_Set
    ret = IaaSed_SetConfig(handle, &sedConfig);
    if (ret != 0)
    {
        printf("SED set failed !\n");
        return -1;
    }

    in = fopen(input_audio, "rb");
    if (!in)
    {
        perror(input_audio);
        return -1;
    }

    fread(buf, sizeof(char), 44, in);

    IaaSed_GetInputSamples(handle, &input_len);
    while (fread(buf, sizeof(short), input_len, in))
    {
        counter++;
        ret = IaaSed_LoadData(handle, buf, &detect_flag);
        if (ret !=0)
        {
            printf("%s:%dIaaSed_LoadData ret:%d\n", __FUNCTION__, __LINE__, ret);
        }

        if (detect_flag)
        {
            ret         = IaaSed_Run(handle, &event_idx);
            int seconds = (float)counter * input_len / sedInit.sampleRate;
            if (ret != 0)
            {
                printf("%s:%d IaaSed_Run ret:%d\n", __FUNCTION__, __LINE__, ret);
            }
            fprintf(out, "%d:%02d, ", seconds / 60, seconds % 60);
            if (event_idx == 0)
            {
                fprintf(out, "Detected babycry, %d, %d\n", counter, event_idx);
            }
            else
            {
                fprintf(out, "%d, %d\n", counter, event_idx);
            }
        }
    }

    fclose(in);
    fclose(out);
    IaaSed_Free(handle);
    return 0;
}
