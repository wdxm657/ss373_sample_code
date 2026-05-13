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
#ifndef _ST_COMMON_JPD_H_
#define _ST_COMMON_JPD_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_jpd.h"

    MI_S32 ST_Common_GetJpdDefaultDevAttr(MI_JPD_InitParam_t *pstInitParam);
    MI_S32 ST_Common_GetJpdDefaultChnAttr(MI_JPD_ChnCreatConf_t *pstChnAttr);

    MI_S32 ST_Common_JpdCreateDev(MI_JPD_DEV DevId, MI_JPD_InitParam_t *pstInitParam);
    MI_S32 ST_Common_JpdDestroyDev(MI_JPD_DEV DevId);
    MI_S32 ST_Common_JpdStartChn(MI_JPD_DEV DevId, MI_JPD_CHN Chn, MI_JPD_ChnCreatConf_t *pstChnAttr);
    MI_S32 ST_Common_JpdStopChn(MI_JPD_DEV DevId, MI_JPD_CHN VdecChn);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_JPD_H_