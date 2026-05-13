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
#define INPUT_NAME_0    "./resource/input/ive/1280x720_U8C1_outdoor_f00.bin"
#define INPUT_NAME_1    "./resource/input/ive/1280x720_U8C1_outdoor_f10.bin"
#define OUTPUT_PATH     "./out/ive/"
#define OUTPUT_NAME_0   "./out/ive/Output_SAD_Sad_160x90_U16C1.bin"
#define OUTPUT_NAME_1   "./out/ive/Output_SAD_Thr_160x90_U8C1.bin"

int main()
{
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_S32 ret;
    MI_IVE_HANDLE handle = 0;
    MI_IVE_SrcImage_t src0, src1;
    MI_IVE_DstImage_t dst0, dst1;
    MI_IVE_SadCtrl_t ctrl =
    {
        .eMode     = E_MI_IVE_SAD_MODE_MB_8X8,
        .eOutCtrl  = E_MI_IVE_SAD_OUT_CTRL_16BIT_BOTH,
        .u16Thr    = 480,
        .u8MinVal  = 0,
        .u8MaxVal  = 255
    };

    memset(&src0, 0, sizeof(src0));
    memset(&src1, 0, sizeof(src1));
    memset(&dst0, 0, sizeof(dst0));
    memset(&dst1, 0, sizeof(dst0));

    ret = ST_Common_IveCreateHandle(handle);
    if (ret != MI_SUCCESS)
    {
        printf("Could not create IVE handle\n");
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&src0, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer 0\n");
        MI_IVE_Destroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&src1, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer 1\n");
        CHECK_AND_FREE_IMAGE(src0);
        MI_IVE_Destroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&dst0, E_MI_IVE_IMAGE_TYPE_U16C1, RAW_WIDTH/8, RAW_WIDTH/8, RAW_HEIGHT/8);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate output buffer 0\n");
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        MI_IVE_Destroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&dst1, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH/8, RAW_WIDTH/8, RAW_HEIGHT/8);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate output buffer 1\n");
        CHECK_AND_FREE_IMAGE(dst0);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        MI_IVE_Destroy(handle);
        return ret;
    }

    ret = ST_Common_IveInitInputImage(&src0, INPUT_NAME_0);
    if (ret != MI_SUCCESS)
    {
        printf("Can't init input buffer from %s\n", INPUT_NAME_0);
        CHECK_AND_FREE_IMAGE(dst1);
        CHECK_AND_FREE_IMAGE(dst0);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        MI_IVE_Destroy(handle);
        return ret;
    }

    ret = ST_Common_IveInitInputImage(&src1, INPUT_NAME_1);
    if (ret != MI_SUCCESS)
    {
        printf("Can't init input buffer from %s\n", INPUT_NAME_1);
        CHECK_AND_FREE_IMAGE(dst1);
        CHECK_AND_FREE_IMAGE(dst0);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        MI_IVE_Destroy(handle);
        return ret;
    }

    ret = MI_IVE_Sad(handle, &src0, &src1, &dst0, &dst1, &ctrl, 0);
    if (ret != MI_SUCCESS)
    {
        printf("MI_IVE_Sad() return ERROR 0x%X\n", ret);
        CHECK_AND_FREE_IMAGE(dst1);
        CHECK_AND_FREE_IMAGE(dst0);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        MI_IVE_Destroy(handle);
        return ret;
    }

    printf("[pattern]: %s\n",INPUT_NAME_0);
    printf("[pattern]: %s\n",INPUT_NAME_1);
    ret = ST_Common_CheckMkdirOutFile(OUTPUT_PATH);
    if (ret != MI_SUCCESS)
    {
        printf("Can't mkdir output path %s\n", OUTPUT_PATH);
        return ret;
    }

    ret = ST_Common_IveSaveOutputImage(&dst0, OUTPUT_NAME_0);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save output 0 to output file %s\n", OUTPUT_NAME_0);
        CHECK_AND_FREE_IMAGE(dst1);
        CHECK_AND_FREE_IMAGE(dst0);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        MI_IVE_Destroy(handle);
        return ret;
    }

    ret = ST_Common_IveSaveOutputImage(&dst1, OUTPUT_NAME_1);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save output 0 to output file %s\n", OUTPUT_NAME_1);
        CHECK_AND_FREE_IMAGE(dst1);
        CHECK_AND_FREE_IMAGE(dst0);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        MI_IVE_Destroy(handle);
        return ret;
    }

    CHECK_AND_FREE_IMAGE(dst1);
    CHECK_AND_FREE_IMAGE(dst0);
    CHECK_AND_FREE_IMAGE(src1);
    CHECK_AND_FREE_IMAGE(src0);
    MI_IVE_Destroy(handle);

    return 0;
}