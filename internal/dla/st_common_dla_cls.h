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
#ifndef _ST_COMMON_DLA_CLS_H_
#define _ST_COMMON_DLA_CLS_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_ipu.h"
#include <stdbool.h>
#include "sstar_cls_api.h"
#include "st_common.h"

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

MI_S32 ST_Common_CLS_Init(void** ppHandle, InitInfo_t* pstInit_info);
MI_S32 ST_Common_CLS_DeInit(void** ppHandle);
MI_S32 ST_Common_CLS_SetDefaultParam(void** ppHandle);
MI_S32 ST_Common_CLS_GetModelAttr(void** ppHandle, InputAttr_t* pstInput_attr);
MI_S32 ST_Common_CLS_GetTargetData(void** ppHandle, MI_SYS_BufInfo_t* pstScaledBufInfo,
                                   OutputInfo_t astResults[NUM_OUTPUS], MI_S32* pOutputCount);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_DLA_CLS_H_
