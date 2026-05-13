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

#define RAW_WIDTH       1280
#define RAW_HEIGHT      720
#define INPUT_NAME    "./resource/input/ive/1280x720_YUV420SP_Img1.bin"
#define OUTPUT_PATH   "./out/ive/"
#define OUTPUT_NAME   "./out/ive/Output_Resize_320x360_YUV420sp.bin"

#define OUTPUT_RAW_WIDTH    320
#define OUTPUT_RAW_HEIGHT   360

int main()
{
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_S32 ret;
    MI_IVE_HANDLE handle = 0;
    MI_IVE_SrcImage_t src;
    MI_IVE_DstImage_t dst;
    MVE_IVE_ResizeCtrl_t ctrl =
    {
        .eMode = E_MI_IVE_RESIZE_TYPE_YUV420SP
    };

    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));

    ret = ST_Common_IveCreateHandle(handle);
    if (ret != MI_SUCCESS)
    {
        printf("Could not create IVE handle\n");
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&src, E_MI_IVE_IMAGE_TYPE_YUV420SP, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer\n");
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&dst, E_MI_IVE_IMAGE_TYPE_YUV420SP, OUTPUT_RAW_WIDTH, OUTPUT_RAW_WIDTH, OUTPUT_RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate output buffer\n");
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveInitInputImage(&src, INPUT_NAME);
    if (ret != MI_SUCCESS)
    {
        printf("Can't init input buffer from %s\n", INPUT_NAME);
        CHECK_AND_FREE_IMAGE(dst);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = MI_IVE_Resize(handle, &src, &dst, &ctrl, 0);
    if (ret != MI_SUCCESS)
    {
        printf("MI_IVE_Resize() return ERROR 0x%X\n", ret);
        CHECK_AND_FREE_IMAGE(dst);
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

    ret = ST_Common_IveSaveOutputImage(&dst, OUTPUT_NAME);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save output to output file %s\n", OUTPUT_NAME);
        CHECK_AND_FREE_IMAGE(dst);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    CHECK_AND_FREE_IMAGE(dst);
    CHECK_AND_FREE_IMAGE(src);
    ST_Common_IveDestroy(handle);

    return 0;
}
