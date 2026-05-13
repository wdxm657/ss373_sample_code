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

#ifndef UAC_UNIT_APP_CUSTOM_H
#define UAC_UNIT_APP_CUSTOM_H

#include "cam_os_wrapper.h"

void uac_unit_app_custom_init(u8 if_ss, u8 if_hs, u8 if_fs, void *pusb_class_user_data);
void uac_unit_app_custom_deinit(u8 if_ss, u8 if_hs, u8 if_fs);
void uac_unit_app_custom_speed_negotiation(u8 speed, u8 interface);

#endif //#ifndef UAC_UNIT_APP_CUSTOM_H

