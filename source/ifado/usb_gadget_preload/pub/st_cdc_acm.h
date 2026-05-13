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
/** @file
 */

#if defined(CONFIG_USB_GADGET_CDC_SUPPORT)

#ifndef __ST_CDC_ACM_H__
#define __ST_CDC_ACM_H__

#include "mi_common_datatype.h"
#include "cam_os_wrapper.h"
#include "cdc_acm.h"

MI_BOOL ST_Cdc_Acm_Send_Packet(MI_U8 *buf, MI_U32 size);

MI_BOOL ST_Cdc_Acm_Init(struct cdc_acm_user_ops *ops);
MI_BOOL ST_Cdc_Acm_DeInit(void);

#endif  //#ifndef __ST_CDC_ACM_H__

#endif  //#if defined(CONFIG_USB_GADGET_CDC_SUPPORT)
