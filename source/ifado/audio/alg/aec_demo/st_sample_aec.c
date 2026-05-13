/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
 * All rights reserved.
 *
 * Unless otherwise stipulated in writing, any and all information contained
 * herein regardless in any format shall remain the sole proprietary of
 * SigmaStar and be kept in strict confidence
 * (SigmaStar Confidential Information) by the recipient.
 * Any unauthorized act including without limitation unauthorized disclosure,
 * copying, use, reproduction, sale, distribution, modification, disassembling,
 * reverse engineering and compiling of the contents of SigmaStar Confidential
 * Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
 * rights to any and all damages, losses, costs and expenses resulting therefrom.
 * */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include "AudioAecProcess.h"
#include "st_common.h"

int main(int argc, char *argv[])
{
    short          in_output[1024];
    short          input_far[1024];
    unsigned int   workingBufferSize;
    char *         workingBuffer = NULL;
    ALGO_AEC_RET   ret;
    int            tempSize;
    FILE *         fpInFar, *fpOut, *fpInNear;
    AudioAecInit   aec_init;
    AudioAecConfig aec_config;
    AEC_HANDLE     handle;

    /*********************User change section start*******************/
    unsigned int supMode_band[6]    = {20, 40, 60, 80, 100, 120};
    unsigned int supMode[7]         = {4, 4, 4, 4, 4, 4, 4};
    aec_init.point_number           = 128;
    aec_init.nearend_channel        = 1;
    aec_init.farend_channel         = 1;
    aec_init.sample_rate            = IAA_AEC_SAMPLE_RATE_16000;
    aec_config.delay_sample         = 0;
    aec_config.comfort_noise_enable = IAA_AEC_FALSE;

    char *input_file1 = "resource/input/audio/aec_farend.wav";
    char *input_file2 = "resource/input/audio/aec_nearend.wav";
    char *output_file = "out/audio/aec_processed.wav";
    /*********************User change section end*******************/

    printf("input_file1 = %s\n", input_file1);
    printf("input_file2 = %s\n", input_file2);
    printf("output_file = %s\n\n", output_file);

    memcpy(&(aec_config.suppression_mode_freq[0]), supMode_band, sizeof(supMode_band));
    memcpy(&(aec_config.suppression_mode_intensity[0]), supMode, sizeof(supMode));

    //(1)IaaAec_GetBufferSize
    workingBufferSize = IaaAec_GetBufferSize();
    workingBuffer     = (char *)malloc(workingBufferSize);
    if (NULL == workingBuffer)
    {
        printf("workingBuffer malloc failed !\n");
        return -1;
    }

    //(2)IaaAec_Init
    handle = IaaAec_Init(workingBuffer, &aec_init);
    if (NULL == handle)
    {
        printf("AEC init failed !\r\n");
        return -1;
    }

    //(3)IaaAec_Config
    ret = IaaAec_Config(handle, &aec_config);
    if (ret)
    {
        printf("IaaAec_Config failed !\n");
        return -1;
    }

    fpInFar = fopen(input_file1, "rb");
    if (NULL == fpInFar)
    {
        printf("input_file1 open failed !\n");
        return -1;
    }

    fpInNear = fopen(input_file2, "rb");
    if (NULL == fpInNear)
    {
        printf("input_file2 open failed !\n");
        return -1;
    }

    ST_Common_CheckMkdirOutFile(output_file);
    fpOut = fopen(output_file, "wb");
    if (NULL == fpOut)
    {
        printf("output_file open failed !\n");
        return -1;
    }

    fread(in_output, sizeof(char), 44, fpInNear); // Remove the 44 bytes header
    fwrite(in_output, sizeof(char), 44, fpOut);   // New file add header
    fread(input_far, sizeof(char), 44, fpInFar);  // Remove the 44 bytes header

    tempSize = aec_init.point_number * aec_init.nearend_channel;
    while (fread(in_output, sizeof(short), tempSize, fpInNear))
    {
        tempSize = aec_init.point_number * aec_init.farend_channel;
        fread(input_far, sizeof(short), tempSize, fpInFar);
        //(4)IaaAec_Run
        ret = IaaAec_Run(handle, in_output, input_far);
        if (ret < 0)
        {
            printf("IaaAec_Run failed !\n");
            break;
        }
        tempSize = aec_init.point_number * aec_init.nearend_channel;
        fwrite(in_output, sizeof(short), tempSize, fpOut);
    }

    //(5)IaaAec_Free
    IaaAec_Free(handle);
    fclose(fpInFar);
    fclose(fpInNear);
    fclose(fpOut);
    free(workingBuffer);

    return 0;
}
