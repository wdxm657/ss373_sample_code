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

#ifndef _ST_COMMON_VDF_H_
#define _ST_COMMON_VDF_H_

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include "mi_sys.h"
#include "mi_vdf.h"

#define VDF_MD_CHN_NO   0
#define VDF_OD_CHN_NO   0
#define VDF_VG_CHN_NO   0

MI_S32 ST_Common_VdfCheckAlign(MI_U16 mb_size, MI_U16 stride);

MI_S32 ST_Common_GetVdfMdDefaultChnAttr(MI_VDF_ChnAttr_t *pstAttr, MI_U16 g_ieWidth, MI_U16 g_ieHeight);
MI_S32 ST_Common_GetVdfOdDefaultChnAttr(MI_VDF_ChnAttr_t *pstAttr, MI_U16 g_ieWidth, MI_U16 g_ieHeight);
MI_S32 ST_Common_GetVdfVgDefaultChnAttr(MI_VDF_ChnAttr_t *pstAttr, MI_U16 g_ieWidth, MI_U16 g_ieHeight);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_VDF_H_
