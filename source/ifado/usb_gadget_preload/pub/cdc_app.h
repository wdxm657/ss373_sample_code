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

#if defined(CONFIG_USB_GADGET_CDC_SUPPORT)

#ifndef __CDC_APP_H__
#define __CDC_APP_H__


void cdc_app_init(void);
void cdc_app_deinit(void);

#endif  //#ifndef __CDC_APP_H__

#endif  //#if defined(CONFIG_USB_GADGET_CDC_SUPPORT)

