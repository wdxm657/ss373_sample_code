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
#include <stdlib.h>
#include "AudioMixProcess.h"

#define FILE_NUMBER (2)
#define CHN_MAX (8)

int main(int argc, char *argv[])
{
    short input[CHN_MAX*128];
    short output[128];
    char infileName[FILE_NUMBER][512];
    char outfileName[512];
    FILE *in[FILE_NUMBER];
    FILE *out;
    int i;

    char *working_buf_ptr;
    working_buf_ptr = (char*)malloc(IaaMix_GetBufferSize());

    int ret;
    MIX_HANDLE handle;
    AudioMixInit_t mixInit;
    AudioMixConfig_t mixConfig;
    /*********************User change section start*******************/
    mixInit.bitWidth = 16;
    mixInit.frameLength = 128;
    mixInit.sampleRate = 8000;
    mixInit.inputNumber = FILE_NUMBER;
    mixInit.inputType = MIX_INPUT_MONO;
    mixConfig.stepSize = 5;
    mixConfig.mode = MIX_FRAME_TO_FRAME;
    int gain_array[8]={3,0,4,5,0,0,1,2};
    for(i=0; i<FILE_NUMBER; i++)
    {
        mixConfig.chnGain[i] = gain_array[i];
    }
    /*********************User change section end*******************/

    handle = IaaMix_Init((char*)working_buf_ptr, &mixInit);
    if (handle == NULL)
    {
        printf("MIX init error !\n");
        return -1;
    }
    ret = IaaMix_SetConfig(handle, &mixConfig);

    for(i=0; i<FILE_NUMBER; i++)
    {
        sprintf(infileName[i],"resource/input/audio/mix_signal%d.wav", i+1);
        in[i] = fopen(infileName[i], "rb");
        if(!in[i])
        {
            printf("the input file %s could not be open\n",infileName[i]);
            return -1;
        }
    }
    sprintf(outfileName,"%s", "out/audio/mix_processed.wav");
    /*********************User change section end*******************/


    out = fopen(outfileName, "wb");
    if(!out)
    {
        printf("the output file %s could not be open\n",outfileName);
        return -1;
    }

    for (i=0;i<FILE_NUMBER;i++)
    {
        fread(input, sizeof(char), 44, in[i]);
    }
    fwrite(input, sizeof(char),44, out); // write 44 bytes output
    while(fread(input, sizeof(short), mixInit.frameLength*(int)(mixInit.inputType), in[0]))
    {
        for(i=1; i<FILE_NUMBER; i++)
        {
            fread(&(input[i*mixInit.frameLength*(int)(mixInit.inputType)]), sizeof(short), mixInit.frameLength*(int)(mixInit.inputType), in[i]);
        }
       ret = IaaMix_Run(handle, input, output);
       if(ret != 0)
       {
           printf("Error occured in Mix\n");
           break;
       }
       fwrite(output, sizeof(short), mixInit.frameLength*(int)(mixInit.inputType), out);
    }
    IaaMix_Free(handle);
    for(i=0; i<FILE_NUMBER; i++)
    {
        fclose(in[i]);
    }
    fclose(out);

    free(working_buf_ptr);
    printf("Done\n");
    return 0;
}
