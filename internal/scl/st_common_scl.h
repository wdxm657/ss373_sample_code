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
#ifndef _ST_COMMON_SCL_H_
#define _ST_COMMON_SCL_H_

#ifdef __cplusplus
extern "C"{
#endif	// __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_scl.h"

MI_S32 ST_Common_GetSclDefaultDevAttr(MI_SCL_DevAttr_t *pstDevAttr);
MI_S32 ST_Common_GetSclDefaultChnAttr(MI_SCL_ChannelAttr_t *pstChnAttr,
                MI_SYS_WindowRect_t *pstInputCrop, MI_SCL_ChnParam_t *pstChnParam);
MI_S32 ST_Common_GetSclDefaultPortAttr(MI_SCL_OutPortParam_t  *pstOutPortParam);

MI_S32 ST_Common_SclCreateDevice(MI_U32 SclDevId, MI_SCL_DevAttr_t *pstSclDevAttr);
MI_S32 ST_Common_SclDestroyDevice(MI_U32 SclDevId);
MI_S32 ST_Common_SclStartChn(MI_U32 SclDevId, MI_U32 SclChnId, MI_SCL_ChannelAttr_t *pstSclChnAttr,
                                    MI_SYS_WindowRect_t *pstSclInputCrop, MI_SCL_ChnParam_t *pstSclChnParam);
MI_S32 ST_Common_SclStopChn(MI_U32 SclDevId, MI_U32 SclChnId);
MI_S32 ST_Common_SclEnablePort(MI_U32 SclDevId, MI_U32 SclChnId, MI_U32 SclOutPortId,
                                    MI_SCL_OutPortParam_t *pstOutPortParam);
MI_S32 ST_Common_SclDisablePort(MI_U32 SclDevId, MI_U32 SclChnId, MI_U32 SclOutPortId);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_SCL_H_
