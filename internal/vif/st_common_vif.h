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
#ifndef _ST_COMMON_VIF_H_
#define _ST_COMMON_VIF_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_vif.h"
#include "mi_sensor.h"

typedef struct ST_Common_SNRSleepParam_s
{
    MI_BOOL bSleepEnable;
    MI_U32  u32FrameCntBeforeSleep;
} ST_Common_SNRSleepParam_t;

MI_S32 ST_Common_SensorInit(MI_SNR_PADID eSnrPad, MI_BOOL bPlaneMode, MI_U8 u8ResIndex, MI_U32 u32Fps);
MI_S32 ST_Common_SensorDeInit(MI_SNR_PADID eSnrPad);
MI_S32 ST_Common_VifCreateDevGroup(MI_U32 VifGroupId, MI_VIF_GroupAttr_t *pstGroupAttr);
MI_S32 ST_Common_VifDestroyDevGroup(MI_U32 VifGroupId);
MI_S32 ST_Common_VifEnableDev(MI_U32 VifDevId, MI_VIF_DevAttr_t *pstDevAttr);
MI_S32 ST_Common_VifDisableDev(MI_U32 VifDevId);
MI_S32 ST_Common_VifEnablePort(MI_U32 VifDevId, MI_U32 VifPortId, MI_VIF_OutputPortAttr_t *pstPortAttr);
MI_S32 ST_Common_VifDisablePort(MI_U32 VifDevId, MI_U32 VifPortId);

MI_S32 ST_Common_GetVifDefaultGrouptAttr(MI_VIF_GroupAttr_t *pstVifGroupAttr);
MI_S32 ST_Common_GetVifDefaultDevAttr(MI_VIF_DevAttr_t *pstVifDevAttr);
MI_S32 ST_Common_GetVifDefaultPortAttr(MI_VIF_OutputPortAttr_t *pstVifPortAttr);

MI_S32 ST_Common_GetVifGroupDevByPad(MI_SNR_PADID sPad, MI_VIF_GROUP *pVifGroup, MI_VIF_DEV *pVifDev);
MI_S32 ST_Common_GetSensorPadByVifGroup(MI_VIF_GROUP VifGroup, MI_SNR_PADID *pu32SnrPadId);
MI_S32 ST_Common_GetSensorPadByVifDev(MI_VIF_DEV VifDev, MI_SNR_PADID *pu32SnrPadId);
MI_U32 ST_Common_VifGroupStitchMaskBySnrMask(MI_U32 u32SensorBindId);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_VIF_H_
