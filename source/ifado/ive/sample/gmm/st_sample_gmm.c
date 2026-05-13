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
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "mi_sys.h"
#include "st_common.h"
#include "st_common_ive.h"


#define RAW_WIDTH       352
#define RAW_HEIGHT      288
#define INPUT_NAME      "./resource/input/ive/352x288_U8C1_Imgx50.bin"
#define OUTPUT_PATH     "./out/ive/"
#define OUTPUT_NAME     "./out/ive/Output_GMM_352x288_U8C1.bin"

static int ST_GetGmmBufferSize(uint16_t width, uint16_t height, uint32_t nmixtures, MI_IVE_ImageType_e color)
{
    uint32_t buf_size = 0;

    if (color == E_MI_IVE_IMAGE_TYPE_U8C1)
    {
        buf_size += (sizeof(double) * 3 * width * height * nmixtures);
    }
    else
    {
        buf_size += (sizeof(double) * 5 * width * height * nmixtures);
    }

    return (buf_size);
}


int main()
{
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_S32 ret;
    int input_file, output_file;

    MI_IVE_HANDLE handle = 0;
    MI_IVE_SrcImage_t src;
    MI_IVE_DstImage_t dst_foreground, dst_background;
    MI_IVE_MemInfo_t model;
    uint32_t i;
    MI_IVE_GmmCtrl_t ctrl =
    {
        .u22q10NoiseVar  = 230400,
        .u22q10MaxVar    = 2048000,
        .u22q10MinVar    = 204800,
        .u0q16LearnRate  = 327,
        .u0q16BgRatio    = 52428,
        .u8q8VarThr      = 1600,
        .u0q16InitWeight = 3276,
        .u8ModelNum      = 3
    };

    memset(&src, 0, sizeof(src));
    memset(&dst_foreground, 0, sizeof(dst_foreground));
    memset(&dst_background, 0, sizeof(dst_background));

    ret = MI_IVE_Create(handle);
    if (ret != MI_SUCCESS)
    {
        printf("Could not create IVE handle\n");
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&src, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer \n");
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&dst_foreground, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate foreground output buffer\n");
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&dst_background, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate background output buffer\n");
        CHECK_AND_FREE_IMAGE(dst_foreground);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateBuffer(&model, ST_GetGmmBufferSize(src.u16Width, src.u16Height, ctrl.u8ModelNum, src.eType));
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate GMM model buffer\n");
        CHECK_AND_FREE_IMAGE(dst_background);
        CHECK_AND_FREE_IMAGE(dst_foreground);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    printf("[pattern]: %s\n",INPUT_NAME);
    ret = ST_Common_CheckMkdirOutFile(OUTPUT_PATH);
    if (ret != MI_SUCCESS)
    {
        printf("Can't mkdir output path %s\n", OUTPUT_PATH);
        return ret;
    }

    input_file = open(INPUT_NAME, O_RDONLY);
    if (input_file < 0)
    {
        printf("Can't open input file %s (%d: %s)\n", INPUT_NAME, errno, strerror(errno));
        CHECK_AND_FREE_BUFFER(model);
        CHECK_AND_FREE_IMAGE(dst_background);
        CHECK_AND_FREE_IMAGE(dst_foreground);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return -1;
    }

    output_file = open(OUTPUT_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (output_file < 0)
    {
        printf("Can't open output file %s (%d: %s)\n", OUTPUT_NAME, errno, strerror(errno));
        close(output_file);
        close(input_file);
        CHECK_AND_FREE_BUFFER(model);
        CHECK_AND_FREE_IMAGE(dst_background);
        CHECK_AND_FREE_IMAGE(dst_foreground);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return -1;
    }

    for (i=0; i<3; i++)
    {
        ret = ST_Common_IveInitInputImageEx(&src, input_file);
        if (ret != MI_SUCCESS)
        {
            printf("Can't read frame data\n");
            close(output_file);
            close(input_file);
            CHECK_AND_FREE_BUFFER(model);
            CHECK_AND_FREE_IMAGE(dst_background);
            CHECK_AND_FREE_IMAGE(dst_foreground);
            CHECK_AND_FREE_IMAGE(src);
            ST_Common_IveDestroy(handle);
            return ret;
        }

        ret = MI_IVE_Gmm(handle, &src, &dst_foreground, &dst_background, &model, &ctrl, 0);
        if (ret != MI_SUCCESS)
        {
            printf("MI_IVE_Gmm() return ERROR 0x%X\n", ret);
            close(output_file);
            close(input_file);
            CHECK_AND_FREE_BUFFER(model);
            CHECK_AND_FREE_IMAGE(dst_background);
            CHECK_AND_FREE_IMAGE(dst_foreground);
            CHECK_AND_FREE_IMAGE(src);
            ST_Common_IveDestroy(handle);
            return ret;
        }

        ret = ST_Common_IveSaveOutputImageEx(&dst_foreground, output_file);
        if (ret != MI_SUCCESS)
        {
            printf("Can't save foreground image to output file %s\n", OUTPUT_NAME);
            close(output_file);
            close(input_file);
            CHECK_AND_FREE_BUFFER(model);
            CHECK_AND_FREE_IMAGE(dst_background);
            CHECK_AND_FREE_IMAGE(dst_foreground);
            CHECK_AND_FREE_IMAGE(src);
            ST_Common_IveDestroy(handle);
            return ret;
        }

        ret = ST_Common_IveSaveOutputImageEx(&dst_background, output_file);
        if (ret != MI_SUCCESS)
        {
            printf("Can't save background image to output file %s\n", OUTPUT_NAME);
            close(output_file);
            close(input_file);
            CHECK_AND_FREE_BUFFER(model);
            CHECK_AND_FREE_IMAGE(dst_background);
            CHECK_AND_FREE_IMAGE(dst_foreground);
            CHECK_AND_FREE_IMAGE(src);
            ST_Common_IveDestroy(handle);
            return ret;
        }
    }
    close(output_file);
    close(input_file);
    CHECK_AND_FREE_BUFFER(model);
    CHECK_AND_FREE_IMAGE(dst_background);
    CHECK_AND_FREE_IMAGE(dst_foreground);
    CHECK_AND_FREE_IMAGE(src);
    ST_Common_IveDestroy(handle);

    return ret;
}
