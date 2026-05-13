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

#ifndef __UAC_APP_H__
#define __UAC_APP_H__

#if defined(CONFIG_USB_GADGET_UAC_SUPPORT)
#include "mi_common_datatype.h"
#include "cam_os_wrapper.h"

#ifndef CONFIG_USB_GADGET_UAC_LATENCY_FINE_TUNE
#define CONFIG_USB_GADGET_UAC_LATENCY_FINE_TUNE
#endif
#if defined(CONFIG_USB_GADGET_UAC_LATENCY_FINE_TUNE)
#define AI_PERIOD_MS            8 // 8ms
#define AI_PERIOD_SIZE(rate)    (((rate) * AI_PERIOD_MS) / 1000)
#else
#define AI_PERIOD_SIZE(rate)    1024
#endif

#define AO_PERIOD_SIZE AI_PERIOD_SIZE

void uac_app_init(void);
void uac_app_deinit(void);
MI_BOOL uac_app_is_stop(void);
#endif

#endif

