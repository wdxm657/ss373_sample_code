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
#ifndef _ST_COMMON_DISP_H_
#define _ST_COMMON_DISP_H_

#ifdef __cplusplus
extern "C"{
#endif// __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_disp.h"
#include "mi_disp_datatype.h"

void ST_Common_DispGetDefaultPubAttr(MI_DISP_PubAttr_t *pstDispPubAttr);
void ST_Common_DispGetDefaultLayerAttr(MI_DISP_VideoLayerAttr_t *pstLayerAttr);
void ST_Common_DispGetDefaultInputPortAttr(MI_DISP_InputPortAttr_t *pstInputPortAttr);

MI_S32 ST_Common_DispEnableDev(MI_DISP_DEV DispDev, MI_DISP_PubAttr_t *stDispPubAttr);

MI_S32 ST_Common_DispEnableLayer(MI_DISP_DEV DispDev,
                                 MI_DISP_LAYER DispLayer,
                                 MI_DISP_VideoLayerAttr_t *pstLayerAttr);

MI_S32 ST_Common_DispEnableInputPort(MI_DISP_LAYER DispLayer,
                                     MI_DISP_INPUTPORT LayerInputPort,
                                     MI_DISP_InputPortAttr_t *pstInputPortAttr);

MI_S32 ST_Common_DispDisableInputPort(MI_DISP_LAYER DispLayer, MI_DISP_INPUTPORT LayerInputPort);
MI_S32 ST_Common_DispDisableVideoLayer(MI_DISP_LAYER DispLayer, MI_DISP_DEV DispDev);
MI_S32 ST_Common_DispDisableDev(MI_DISP_DEV DispDev);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_DISP_H_

