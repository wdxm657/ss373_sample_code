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
#ifndef _ST_COMMON_VDEC_H_
#define _ST_COMMON_VDEC_H_

#ifdef __cplusplus
extern "C"{
#endif	// __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_vdec.h"

MI_S32 ST_Common_GetVdecDefaultDevAttr(MI_VDEC_InitParam_t *pstInitParam);
MI_S32 ST_Common_GetVdecDefaultChnAttr(MI_VDEC_CodecType_e      eCodecType, MI_VDEC_ChnAttr_t *pstVdecChnAttr);
MI_S32 ST_Common_GetVdecDefaultPortAttr(MI_VDEC_OutputPortAttr_t  *pstOutPortAttr);

MI_S32 ST_Common_VdecCreateDev(MI_VDEC_DEV DevId, MI_VDEC_InitParam_t *pstInitParam);
MI_S32 ST_Common_VdecDestroyDev(MI_VDEC_DEV DevId);
MI_S32 ST_Common_VdecStartChn(MI_VDEC_DEV DevId, MI_VDEC_CHN VdecChn, MI_VDEC_ChnAttr_t *pstVdecChnAttr);
MI_S32 ST_Common_VdecStopChn(MI_VDEC_DEV DevId, MI_VDEC_CHN VdecChn);
MI_S32 ST_Common_VdecSetOutputPortAttr(MI_VDEC_DEV DevId, MI_VDEC_CHN VdecChn,MI_VDEC_OutputPortAttr_t  *pstOutPortAttr);

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif //_ST_COMMON_VDEC_H_