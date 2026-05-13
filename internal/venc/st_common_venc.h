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
#ifndef _ST_COMMON_VENC_H_
#define _ST_COMMON_VENC_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_venc.h"

    MI_S32 ST_Common_GetVencDefaultDevAttr(MI_VENC_InitParam_t *pstInitParam);
    MI_S32 ST_Common_GetVencDefaultChnAttr(MI_VENC_ModType_e eType, MI_VENC_ChnAttr_t *pstVencChnAttr);

    MI_S32 ST_Common_VencCreateDev(MI_VENC_DEV DevId, MI_VENC_InitParam_t *pstInitParam);
    MI_S32 ST_Common_VencDestroyDev(MI_VENC_DEV DevId);
    MI_S32 ST_Common_VencStartChn(MI_VENC_DEV DevId, MI_VENC_CHN VencChn, MI_VENC_ChnAttr_t *pstVencChnAttr);
    MI_S32 ST_Common_VencStopChn(MI_VENC_DEV DevId, MI_VENC_CHN VencChn);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_VENC_H_
