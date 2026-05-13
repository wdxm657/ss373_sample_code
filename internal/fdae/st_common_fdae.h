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
#ifndef _ST_COMMON_FDAE_H_
#define _ST_COMMON_FDAE_H_

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
#include "mi_isp_ae.h"
#include "mi_isp_af.h"

typedef struct ST_FDAE_DtBox_s
{
        MI_U16 u16Xmin;
        MI_U16 u16Ymin;
        MI_U16 u16Xmax;
        MI_U16 u16Ymax;
        MI_U32 u32Area;
        float  fScore;
} ST_FDAE_DtBox_t;

typedef struct ST_FDAE_PARA_s
{
        MI_U32 u32face_luma_target;
        MI_U8  u8face_tolerance;
        MI_U8  u8face_luma_step;

        MI_ISP_AE_IntpLutType_t stAEtarget_Storage;
        MI_BOOL bAE_INIT;
        MI_BOOL bResetAE;

        CusAFWin_t   afwin;
        CusAFStats_t af_info;

} ST_FDAE_PARA_t;

void ST_FDAE_Init3A(ST_FDAE_PARA_t** ppstFdae_para, MI_U32 u32Index);

void ST_FDAE_Adjust3A(MI_U32 DevId, MI_U32 Channel, ST_FDAE_PARA_t* pstFdae_para, ST_FDAE_DtBox_t stBox, MI_U32 u32SensorFrameCount);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_ISP_H_
