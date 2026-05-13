/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized diSclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _ST_COMMON_DLA_HGR_H_
#define _ST_COMMON_DLA_HGR_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "algo_hgr_api.h"
#include "st_common.h"

#define HGR_HANDS_ACCEPT_WIDTH (640)
#define HGR_HANDS_ACCEPT_HEIGHT (352)

#define HGR_HANDPOSE_ACCEPT_WIDTH (256)
#define HGR_HANDPOSE_ACCEPT_HEIGHT (256)

#define NV12_256X256_SIZE (98304)

#define CHECK_DLA_RESULT(_func_, _ret_, _exit_label_)                                     \
    do                                                                                    \
    {                                                                                     \
        _ret_ = _func_;                                                                   \
        if (_ret_ != MI_SUCCESS)                                                          \
        {                                                                                 \
            printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, _ret_); \
            goto _exit_label_;                                                            \
        }                                                                                 \
    } while (0)

MI_S32 ST_Common_HGR_Init(MI_HGR_HANDLE* phHgr, AlgoHgrInitParam_t* pstInitParam);
MI_S32 ST_Common_HGR_DeInit(MI_HGR_HANDLE* phHgr);
MI_S32 ST_Common_HGR_DetectHands(MI_HGR_HANDLE* phHgr, MI_SYS_BufInfo_t* pstScaledBufInfo, MI_S32* ps32Count,
                                 HgrBbox_t** ppstOutbox);
MI_S32 ST_Common_HGR_RecognizeHand(MI_HGR_HANDLE* phHgr, MI_SYS_BufInfo_t* pstHandBufInfo, HgrBbox_t* pstOutbox,
                                   HgrPoint_t** ppstOutpoint, MI_S32* ps32Pointcounts, AlgoHgrResult_t* pstHgrResult);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_DLA_HGR_H_
