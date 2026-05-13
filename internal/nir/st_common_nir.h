
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
#ifndef _ST_COMMON_NIR_H_
#define _ST_COMMON_NIR_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_nir.h"

    MI_S32 ST_Common_GetNirDefaultDevAttr(MI_NIR_DevAttr_t *pstDevAttr);
    MI_S32 ST_Common_GetNirDefaultChnAttr(MI_NIR_ChannelAttr_t *pstChnAttr);

    MI_S32 ST_Common_NirCreateDevice(MI_U32 NirDevId, MI_NIR_DevAttr_t *pstDevAttr);
    MI_S32 ST_Common_NirDestroyDevice(MI_U32 NirDevId);
    MI_S32 ST_Common_NirStartChn(MI_U32 NirDevId, MI_U32 NirChnId, MI_NIR_ChannelAttr_t *pstChnAttr,
                                 MI_NIR_ChnParam_t *pstChnParam);
    MI_S32 ST_Common_NirStopChn(MI_U32 NirDevId, MI_U32 NirChnId);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_NIR_H_
