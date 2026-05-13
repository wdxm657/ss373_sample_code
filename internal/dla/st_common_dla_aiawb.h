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

#ifndef _ST_COMMON_DLA_AIAWb_H_
#define _ST_COMMON_DLA_AIAWb_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_ipu.h"
#include <stdbool.h>
#include "st_common.h"
#include "mi_isp_cus3a_api.h"

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

    MI_S32 ST_Common_AIAWB_Init(MI_SYS_ChnPort_t* pstIspChnPort, MI_SYS_ChnPort_t* pstIpuChnPort,
                                MI_U8* pu8AWBModelPath);
    MI_S32 ST_Common_AIAWB_DeInit(MI_SYS_ChnPort_t* pstIpuChnPort);
    MI_S32 ST_Common_AIAWB_Run(MI_SYS_ChnPort_t* pstIspChnPort, MI_SYS_ChnPort_t* pstIpuChnPort, MI_FLOAT fUltraAWbFps);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_DLA_AIAWb_H_
