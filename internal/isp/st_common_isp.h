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
#ifndef _ST_COMMON_ISP_H_
#define _ST_COMMON_ISP_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_isp.h"
#include "mi_isp_iq.h"
#include "mi_isp_cus3a_api.h"
#include "isp_cus3a_if.h"

MI_S32 ST_Common_GetIspDefaultDevAttr(MI_ISP_DevAttr_t *pstDevAttr);
MI_S32 ST_Common_GetIspDefaultChnAttr(MI_ISP_ChannelAttr_t *pstChnAttr, MI_SYS_WindowRect_t *pstInputCrop,
                                      MI_ISP_ChnParam_t *pstChnParam);
MI_S32 ST_Common_GetIspDefaultPortAttr(MI_ISP_OutPortParam_t *pstOutPortParam);

MI_S32 ST_Common_IspCreateDevice(MI_U32 IspDevId, MI_ISP_DevAttr_t *pstIspDevAttr);
MI_S32 ST_Common_IspDestroyDevice(MI_U32 IspDevId);
MI_S32 ST_Common_IspStartChn(MI_U32 IspDevId, MI_U32 IspChnId, MI_ISP_ChannelAttr_t *pstIspChnAttr,
                             MI_SYS_WindowRect_t *pstIspInputCrop, MI_ISP_ChnParam_t *pstChnParam);
MI_S32 ST_Common_IspStartChnEx(MI_U32 IspDevId, MI_U32 IspChnId, MI_ISP_ChannelAttr_t *pstIspChnAttr,
                               MI_SYS_WindowRect_t *pstIspInputCrop, MI_ISP_ChnParam_t *pstChnParam,
                               MI_ISP_ChnParam_t stSubChnParam[E_MI_ISP_SENSOR_MAX]);
MI_S32 ST_Common_IspStopChn(MI_U32 IspDevId, MI_U32 IspChnId);
MI_S32 ST_Common_IspEnablePort(MI_U32 IspDevId, MI_U32 IspChnId, MI_U32 IspOutPortId,
                               MI_ISP_OutPortParam_t *pstIspOutPortParam);
MI_S32 ST_Common_IspDisablePort(MI_U32 IspDevId, MI_U32 IspChnId, MI_U32 IspOutPortId);

MI_S32 ST_Common_GetIspBindSensorIdByPad(MI_U32 u32SnrPadId, MI_ISP_BindSnrId_e *peSensorBindId);
MI_S32 ST_Common_IspSetIqBin(MI_U32 IspDev, MI_U32 IspChn, char *pConfigPath);
MI_S32 ST_Common_IspSetCaliData(MI_U32 IspDev, MI_U32 IspChn, MI_ISP_IQ_CaliItem_e eCaliItem, char *pConfigPath);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_ISP_H_
