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

#include "mi_sys.h"
#include "st_common.h"
#include "st_common_ive.h"

#define RAW_WIDTH               1280
#define RAW_HEIGHT              720
#define INPUT_NAME              "./resource/input/ive/1280x720_U8C1_square.bin"
#define OUTPUT_PATH             "./out/ive/"
#define OUTPUT_NAME_HORIZONTAL  "./out/ive/Output_Sobel_Horizontal_1280x720_S16C1.bin"
#define OUTPUT_NAME_VIRTICAL    "./out/ive/Output_Sobel_Vertical_1280x720_S16C1.bin"

int main()
{
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_S32 ret;
    MI_IVE_HANDLE handle = 0;
    MI_IVE_SrcImage_t src;
    MI_IVE_DstImage_t dst_h, dst_v;

    MI_IVE_SobelCtrl_t ctrl =
    {
        .eOutCtrl = E_MI_IVE_SOBEL_OUT_CTRL_BOTH,
        .as8Mask =
        {
             0,  0,  0,  0, 0,
             0, -1, -2, -1, 0,
             0,  0,  0,  0, 0,
             0,  1,  2,  1, 0,
             0,  0,  0,  0, 0
        }
    };

    memset(&src, 0, sizeof(src));
    memset(&dst_h, 0, sizeof(dst_h));
    memset(&dst_v, 0, sizeof(dst_v));

    ret = ST_Common_IveCreateHandle(handle);
    if (ret != MI_SUCCESS)
    {
        printf("Could not create IVE handle\n");
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&src, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer\n");
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&dst_h, E_MI_IVE_IMAGE_TYPE_S16C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate horizontal output buffer\n");
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&dst_v, E_MI_IVE_IMAGE_TYPE_S16C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate virtual output buffer\n");
        CHECK_AND_FREE_IMAGE(dst_h);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveInitInputImage(&src, INPUT_NAME);
    if (ret != MI_SUCCESS)
    {
        printf("Can't init input buffer from %s\n", INPUT_NAME);
        CHECK_AND_FREE_IMAGE(dst_v);
        CHECK_AND_FREE_IMAGE(dst_h);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = MI_IVE_Sobel(handle, &src, &dst_h, &dst_v, &ctrl, 0);
    if (ret != MI_SUCCESS)
    {
        printf("MI_IVE_Sobel() return ERROR 0x%X\n", ret);
        CHECK_AND_FREE_IMAGE(dst_v);
        CHECK_AND_FREE_IMAGE(dst_h);
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

    ret = ST_Common_IveSaveOutputImage(&dst_h, OUTPUT_NAME_HORIZONTAL);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save horizontal image to output file %s\n", OUTPUT_NAME_HORIZONTAL);
        CHECK_AND_FREE_IMAGE(dst_v);
        CHECK_AND_FREE_IMAGE(dst_h);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveSaveOutputImage(&dst_v, OUTPUT_NAME_VIRTICAL);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save vertical image to output file %s\n", OUTPUT_NAME_VIRTICAL);
        CHECK_AND_FREE_IMAGE(dst_v);
        CHECK_AND_FREE_IMAGE(dst_h);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    CHECK_AND_FREE_IMAGE(dst_v);
    CHECK_AND_FREE_IMAGE(dst_h);
    CHECK_AND_FREE_IMAGE(src);
    ST_Common_IveDestroy(handle);

    return 0;
}
