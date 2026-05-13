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

#define RAW_WIDTH        160
#define RAW_HEIGHT       120
#define INPUT_NAME_0     "./resource/input/ive/160x120_U8C1_Lksrc1.bin"
#define INPUT_NAME_1     "./resource/input/ive/160x120_U8C1_Lksrc2.bin"
#define OUTPUT_PATH      "./out/ive/"
#define OUTPUT_NAME      "./out/ive/Output_Lk_Optical_Flow_MvS9Q7_t.bin"

int main()
{
    STCHECKRESULT(ST_Common_Sys_Init());

    MI_S32 ret;
    MI_IVE_HANDLE handle = 0;
    MI_IVE_SrcImage_t src_pre, src_cur;
    MI_IVE_SrcMemInfo_t point;
    MI_IVE_MemInfo_t move;
    int x, y, count;
    MI_IVE_LkOpticalFlowCtrl_t ctrl =
    {
        .u16CornerNum  = 49,
        .u0q8MinEigThr = 255,
        .u8IterCount   = 10,
        .u0q8Epsilon   = 26
    };

    memset(&src_pre, 0, sizeof(src_pre));
    memset(&src_cur, 0, sizeof(src_cur));

    ret = ST_Common_IveCreateHandle(handle);
    if (ret != MI_SUCCESS)
    {
        printf("Could not create IVE handle\n");
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&src_pre, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer 0\n");
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateImage(&src_cur, E_MI_IVE_IMAGE_TYPE_U8C1, RAW_WIDTH, RAW_WIDTH, RAW_HEIGHT);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate input buffer 1\n");
        CHECK_AND_FREE_IMAGE(src_pre);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateBuffer(&point, sizeof(MI_IVE_PointS25Q7_t) * ctrl.u16CornerNum);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate Optical Flow point buffer\n");
        CHECK_AND_FREE_IMAGE(src_cur);
        CHECK_AND_FREE_IMAGE(src_pre);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveAllocateBuffer(&move, sizeof(MI_IVE_MvS9Q7_t) * ctrl.u16CornerNum);
    if (ret != MI_SUCCESS)
    {
        printf("Can't allocate Optical Flow move buffer\n");
        CHECK_AND_FREE_BUFFER(point);
        CHECK_AND_FREE_IMAGE(src_cur);
        CHECK_AND_FREE_IMAGE(src_pre);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveInitInputImage(&src_pre, INPUT_NAME_0);
    if (ret != MI_SUCCESS)
    {
        printf("Can't init input buffer from %s\n", INPUT_NAME_0);
        CHECK_AND_FREE_BUFFER(move);
        CHECK_AND_FREE_BUFFER(point);
        CHECK_AND_FREE_IMAGE(src_cur);
        CHECK_AND_FREE_IMAGE(src_pre);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    ret = ST_Common_IveInitInputImage(&src_cur, INPUT_NAME_1);
    if (ret != MI_SUCCESS)
    {
        printf("Can't init input buffer from %s\n", INPUT_NAME_1);
        CHECK_AND_FREE_BUFFER(move);
        CHECK_AND_FREE_BUFFER(point);
        CHECK_AND_FREE_IMAGE(src_cur);
        CHECK_AND_FREE_IMAGE(src_pre);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    count = 0;
    for (x=0; x<7; x++)
    {
        for (y=0; y<7; y++)
        {
            ((MI_IVE_PointS25Q7_t*)point.pu8VirAddr)[count].s25q7X = ((src_pre.u16Width  / 13)*(x+3)) << 7;
            ((MI_IVE_PointS25Q7_t*)point.pu8VirAddr)[count].s25q7Y = ((src_pre.u16Height / 13)*(y+3)) << 7;
            count++;
        }
    }

    ret = MI_IVE_LkOpticalFlow(handle, &src_pre, &src_cur, &point, &move, &ctrl, 0);
    if (ret != MI_SUCCESS)
    {
        printf("MI_IVE_LkOpticalFlow() return ERROR 0x%X\n", ret);
        CHECK_AND_FREE_BUFFER(move);
        CHECK_AND_FREE_BUFFER(point);
        CHECK_AND_FREE_IMAGE(src_cur);
        CHECK_AND_FREE_IMAGE(src_pre);
        ST_Common_IveDestroy(handle);
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

    ret = ST_Common_IveSaveOutputBuffer(&move, OUTPUT_NAME, sizeof(MI_IVE_MvS9Q7_t) * ctrl.u16CornerNum);
    if (ret != MI_SUCCESS)
    {
        printf("Can't save data to output file %s\n", OUTPUT_NAME);
        CHECK_AND_FREE_BUFFER(move);
        CHECK_AND_FREE_BUFFER(point);
        CHECK_AND_FREE_IMAGE(src_cur);
        CHECK_AND_FREE_IMAGE(src_pre);
        ST_Common_IveDestroy(handle);
        return ret;
    }

    CHECK_AND_FREE_BUFFER(move);
    CHECK_AND_FREE_BUFFER(point);
    CHECK_AND_FREE_IMAGE(src_cur);
    CHECK_AND_FREE_IMAGE(src_pre);
    ST_Common_IveDestroy(handle);

    return 0;
}
