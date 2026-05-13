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

#define RAW_WIDTH    640
#define RAW_HEIGHT   480
#define INPUT_NAME   "./resource/input/ive/640x480_U8C1_Img1.bin"
#define OUTPUT_PATH  "./out/ive/"
#define OUTPUT_NAME  "./out/ive/Output_Canny_640x480_U8C1.bin"

int main()
{
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_S32 ret;
    MI_IVE_HANDLE handle = 0;
    MI_IVE_SrcImage_t src;
    MI_IVE_DstImage_t dst;
    MI_IVE_DstMemInfo_t stack;
    MI_IVE_CannyHysEdgeCtrl_t ctrl =
    {
        .stMem = {0},
        .u16LowThr  = 7*2,
        .u16HighThr = 14*2,
        .as8Mask =
        {
            0,  0, 0, 0, 0,
            0, -1, 0, 1, 0,
            0, -2, 0, 2, 0,
            0, -1, 0, 1, 0,
            0,  0, 0, 0, 0
        }
    };

    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
    memset(&stack, 0, sizeof(stack));

    // Init IVE
    ret = ST_Common_IveCreateHandle(handle);
    if (ret != MI_SUCCESS)
    {
        printf("Could not create IVE handle\n");
        return ret;
    }

    // Allocate input buffer
    ret = ST_Common_IveAllocateImage(&src, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer\n");
        ST_Common_IveDestroy(handle);
        return ret;
    }

    // Allocate output buffer
    ret = ST_Common_IveAllocateImage(&dst, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate output buffer\n");
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    // Init stack buffer
    ret = ST_Common_IveAllocateBuffer(&stack, RAW_WIDTH*RAW_HEIGHT*4 + sizeof(MI_IVE_CannyStackSize_t));
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate canny stack buffer\n");
        CHECK_AND_FREE_IMAGE(dst);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    // Init Mv buffer
    ret = ST_Common_IveAllocateBuffer(&ctrl.stMem, RAW_WIDTH*(RAW_HEIGHT+3)*4);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate canny stack buffer\n");
        CHECK_AND_FREE_BUFFER(stack);
        CHECK_AND_FREE_IMAGE(dst);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    // Init input buffer
    ret = ST_Common_IveInitInputImage(&src, INPUT_NAME);
    if (ret != MI_SUCCESS)
    {
        printf("Can't init input buffer\n");
        CHECK_AND_FREE_BUFFER(ctrl.stMem);
        CHECK_AND_FREE_BUFFER(stack);
        CHECK_AND_FREE_IMAGE(dst);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    // Run MI_IVE_CannyHysEdge()
    ret = MI_IVE_CannyHysEdge(handle, &src, &dst, &stack, &ctrl, 0);
    if (ret != MI_SUCCESS)
    {
        printf("MI_IVE_CannyHysEdge() return ERROR 0x%X\n", ret);
        CHECK_AND_FREE_BUFFER(ctrl.stMem);
        CHECK_AND_FREE_BUFFER(stack);
        CHECK_AND_FREE_IMAGE(dst);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    // Run MI_IVE_CannyEdge()
    ret = MI_IVE_CannyEdge(handle, &dst, &stack, 0);
    if (ret != MI_SUCCESS)
    {
        printf("MI_IVE_CannyEdge() return ERROR 0x%X\n", ret);
        CHECK_AND_FREE_BUFFER(ctrl.stMem);
        CHECK_AND_FREE_BUFFER(stack);
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

    // Save ouput data
    ret = ST_Common_IveSaveOutputImage(&dst, OUTPUT_NAME);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save data to output file %s\n", OUTPUT_NAME);
        CHECK_AND_FREE_BUFFER(ctrl.stMem);
        CHECK_AND_FREE_BUFFER(stack);
        CHECK_AND_FREE_IMAGE(dst);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    CHECK_AND_FREE_BUFFER(ctrl.stMem);
    CHECK_AND_FREE_BUFFER(stack);
    CHECK_AND_FREE_IMAGE(dst);
    CHECK_AND_FREE_IMAGE(src);
    ST_Common_IveDestroy(handle);

    return 0;
}
