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

#define RAW_WIDTH    16
#define RAW_HEIGHT   4
#define INPUT_NAME   "./resource/input/ive/16x4_U8C1_CCL.bin"
#define OUTPUT_PATH  "./out/ive/"
#define OUTPUT_NAME0  "./out/ive/Output_CCL_16x4_U8C1_grid.bin"
#define OUTPUT_NAME1  "./out/ive/Output_CCL_CcBlob_t.bin"

int main()
{
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_S32 ret;
    MI_IVE_HANDLE handle = 0;
    MI_IVE_SrcImage_t src;
    MI_IVE_DstMemInfo_t blob;
    MI_IVE_CclCtrl_t ctrl  =
    {
        .u16InitAreaThr = 10,
        .u16Step        = 4
    };

    memset(&src, 0, sizeof(src));
    memset(&blob, 0, sizeof(blob));

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

    ret = ST_Common_IveAllocateBuffer(&blob, sizeof(MI_IVE_CcBlob_t));
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
        printf("Can't init input buffer\n");
        CHECK_AND_FREE_IMAGE(src);
        CHECK_AND_FREE_BUFFER(blob);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = MI_IVE_Ccl(handle, &src, &blob, &ctrl, 0);
    if (ret != MI_SUCCESS)
    {
        printf("MI_IVE_Ccl() return ERROR 0x%X\n", ret);
        CHECK_AND_FREE_IMAGE(src);
        CHECK_AND_FREE_BUFFER(blob);
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

    ret = ST_Common_IveSaveOutputImage(&src, OUTPUT_NAME0);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save output to output file %s\n", OUTPUT_NAME0);
        CHECK_AND_FREE_IMAGE(src);
        CHECK_AND_FREE_BUFFER(blob);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveSaveOutputBuffer(&blob, OUTPUT_NAME1, sizeof(MI_IVE_CcBlob_t));
    if (ret != MI_SUCCESS)
    {
        printf("Can't save data to output file %s\n", OUTPUT_NAME1);
        CHECK_AND_FREE_IMAGE(src);
        CHECK_AND_FREE_BUFFER(blob);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    CHECK_AND_FREE_IMAGE(src);
    CHECK_AND_FREE_BUFFER(blob);
    ST_Common_IveDestroy(handle);

    return 0;
}