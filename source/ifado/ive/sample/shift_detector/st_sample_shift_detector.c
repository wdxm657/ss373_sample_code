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


#define RAW_WIDTH           352
#define RAW_HEIGHT          288
#define BLOCK_START_X       20
#define BLOCK_START_Y       205
#define BLOCK_WIDTH         16
#define BLOCK_HEIGHT        16
#define INPUT_NAME_0    "./resource/input/ive/352x288_U8C1_f09.bin"
#define INPUT_NAME_1    "./resource/input/ive/352x288_U8C1_f15.bin"
#define OUTPUT_PATH     "./out/ive/"
#define OUTPUT_NAME_0   "./out/ive/Output_shift_detector_MV_X_1x1_U16C1.bin"
#define OUTPUT_NAME_1   "./out/ive/Output_shift_detector_MV_Y_1x1_U16C1.bin"

int main()
{
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_S32 ret;
    MI_IVE_HANDLE handle = 0;
    MI_IVE_SrcImage_t src0, src1;
    MI_IVE_DstImage_t dstx, dsty;

    MI_IVE_SHIFT_DETECT_CTRL_t ctrl =
    {
        .enMode         = E_MI_IVE_SHIFT_DETECT_MODE_SINGLE,
        .u16Left        = BLOCK_START_X,        //20
        .u16Top         = BLOCK_START_Y,        //205
        .pyramid_level  = 2,
        .search_range   = 80,
        .u16Width       = BLOCK_WIDTH,          //16
        .u16Height      = BLOCK_HEIGHT          //16
    };

    memset(&src0, 0, sizeof(src0));
    memset(&src1, 0, sizeof(src1));
    memset(&dstx, 0, sizeof(dstx));
    memset(&dsty, 0, sizeof(dsty));

    ret = MI_IVE_Create(handle);
    if (ret != MI_SUCCESS)
    {
        printf("Could not create IVE handle\n");
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&src0, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer 0\n");
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&src1, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer 1\n");
        CHECK_AND_FREE_IMAGE(src0);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&dstx, E_MI_IVE_IMAGE_TYPE_U16C1, 1, 1, 1);
    //ret = ST_Common_IveAllocateImage(&dstx, E_MI_IVE_IMAGE_TYPE_U16C1, RAW_WIDTH / BLOCK_WIDTH, RAW_WIDTH / BLOCK_WIDTH, RAW_HEIGHT / BLOCK_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate output buffer 0\n");
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&dsty, E_MI_IVE_IMAGE_TYPE_U16C1, 1, 1, 1);
    //ret = ST_Common_IveAllocateImage(&dsty, E_MI_IVE_IMAGE_TYPE_U16C1, RAW_WIDTH / BLOCK_WIDTH, RAW_WIDTH / BLOCK_WIDTH, RAW_HEIGHT / BLOCK_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate output buffer 1\n");
        CHECK_AND_FREE_IMAGE(dstx);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveInitInputImage(&src0, INPUT_NAME_0);
    if (ret != MI_SUCCESS)
    {
        printf("Can't init input buffer 0 from %s\n", INPUT_NAME_0);
        CHECK_AND_FREE_IMAGE(dsty);
        CHECK_AND_FREE_IMAGE(dstx);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveInitInputImage(&src1, INPUT_NAME_1);
    if (ret != MI_SUCCESS)
    {
        printf("Can't init input buffer 1 from %s\n", INPUT_NAME_1);
        CHECK_AND_FREE_IMAGE(dsty);
        CHECK_AND_FREE_IMAGE(dstx);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    printf("[pattern]: %s\n",INPUT_NAME_0);
    printf("[pattern]: %s\n",INPUT_NAME_1);

    ret =  MI_IVE_Shift_Detector(handle, &src0, &src1, &dstx, &dsty, &ctrl, 0);
    if (ret != MI_SUCCESS)
    {
        printf("MI_IVE_Shift_Detector() return ERROR 0x%X\n", ret);
        CHECK_AND_FREE_IMAGE(dsty);
        CHECK_AND_FREE_IMAGE(dstx);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    printf("SINGLE Target: [%d, %d]\n",ctrl.u16Left, ctrl.u16Top);

    printf("Dst_X: ");
    for(int i=0;i<dstx.u16Height*dstx.azu16Stride[0];i+=dstx.azu16Stride[0])
    {
        for(int j=0;j<dstx.u16Width;j++)
        {
            printf("%d ", (signed char)(dstx.apu8VirAddr[0][i+j]));
        }
        printf("\n");
    }
    printf("Dst_Y: ");
    for(int i=0;i<dsty.u16Height*dsty.azu16Stride[0];i+=dsty.azu16Stride[0])
    {
        for(int j=0;j<dsty.u16Width;j++)
        {
            printf("%d ", (signed char)(dsty.apu8VirAddr[0][i+j]));
        }
        printf("\n");
    }



    ret = ST_Common_CheckMkdirOutFile(OUTPUT_PATH);
    if (ret != MI_SUCCESS)
    {
        printf("Can't mkdir output path %s\n", OUTPUT_PATH);
        return ret;
    }

    ret = ST_Common_IveSaveOutputImage(&dstx, OUTPUT_NAME_0);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save data 0 to output file %s\n", OUTPUT_NAME_0);
        CHECK_AND_FREE_IMAGE(dsty);
        CHECK_AND_FREE_IMAGE(dstx);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveSaveOutputImage(&dsty, OUTPUT_NAME_1);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save data 1 to output file %s\n", OUTPUT_NAME_1);
        CHECK_AND_FREE_IMAGE(dsty);
        CHECK_AND_FREE_IMAGE(dstx);
        CHECK_AND_FREE_IMAGE(src1);
        CHECK_AND_FREE_IMAGE(src0);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    CHECK_AND_FREE_IMAGE(dsty);
    CHECK_AND_FREE_IMAGE(dstx);
    CHECK_AND_FREE_IMAGE(src1);
    CHECK_AND_FREE_IMAGE(src0);
    ST_Common_IveDestroy(handle);

    return 0;
}
