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
#define OUTPUT_NAME_MAG         "./out/ive/Output_MagAndAng_Mag_1280x720_U16C1.bin"
#define OUTPUT_NAME_ANG         "./out/ive/Output_MagAndAng_Ang_1280x720_U8C1.bin"

int main()
{
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_S32 ret;
    MI_IVE_HANDLE handle = 0;
    MI_IVE_SrcImage_t src;
    MI_IVE_DstImage_t dst_mag, dst_ang;

    MI_IVE_MagAndAngCtrl_t ctrl =
    {
        .eOutCtrl = E_MI_IVE_MAG_AND_ANG_OUT_CTRL_MAG_AND_ANG,
        .u16Thr   = 0,
        .as8Mask  =
        {
             0,  0,  0,  0, 0,
             0, -1, -2, -2, 0,
             0,  0,  0,  0, 0,
             0,  1,  2,  2, 0,
             0,  0,  0,  0, 0
        }
    };

    memset(&src, 0, sizeof(src));
    memset(&dst_mag, 0, sizeof(dst_mag));
    memset(&dst_ang, 0, sizeof(dst_ang));

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

    ret = ST_Common_IveAllocateImage(&dst_mag, E_MI_IVE_IMAGE_TYPE_U16C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate Mag output buffer\n");
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&dst_ang, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate Ang output buffer\n");
        CHECK_AND_FREE_IMAGE(dst_mag);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveInitInputImage(&src, INPUT_NAME);
    if (ret != MI_SUCCESS)
    {
        printf("Can't init input buffer from %s\n", INPUT_NAME);
        CHECK_AND_FREE_IMAGE(dst_ang);
        CHECK_AND_FREE_IMAGE(dst_mag);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = MI_IVE_MagAndAng(handle, &src, &dst_mag, &dst_ang, &ctrl, 0);
    if (ret != MI_SUCCESS)
    {
        printf("MI_IVE_MagAndAng() return ERROR 0x%X\n", ret);
        CHECK_AND_FREE_IMAGE(dst_ang);
        CHECK_AND_FREE_IMAGE(dst_mag);
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

    ret = ST_Common_IveSaveOutputImage(&dst_mag, OUTPUT_NAME_MAG);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save Mag image to output file %s\n", OUTPUT_NAME_MAG);
        CHECK_AND_FREE_IMAGE(dst_ang);
        CHECK_AND_FREE_IMAGE(dst_mag);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveSaveOutputImage(&dst_ang, OUTPUT_NAME_ANG);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save Ang image to output file %s\n", OUTPUT_NAME_ANG);
        CHECK_AND_FREE_IMAGE(dst_ang);
        CHECK_AND_FREE_IMAGE(dst_mag);
        CHECK_AND_FREE_IMAGE(src);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    CHECK_AND_FREE_IMAGE(dst_ang);
    CHECK_AND_FREE_IMAGE(dst_mag);
    CHECK_AND_FREE_IMAGE(src);
    ST_Common_IveDestroy(handle);

    return 0;
}
