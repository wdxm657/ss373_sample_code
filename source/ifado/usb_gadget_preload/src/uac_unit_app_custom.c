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

#include "uac_unit_app_custom.h"

#include "uac.h"
#include "usb_class_ac_vc.h"
#include "usb_ac_fu.h"

void uac_unit_app_custom_reset_parameter(u8 reset_val, void *puser_data)
{
    usb_ac_fu_cmd_init_parameter(reset_val, puser_data);
}

void uac_unit_app_custom_init(u8 if_ss, u8 if_hs, u8 if_fs, void *pusb_class_user_data)
{
#if defined(CONFIG_USB_GADGET_UAC_SUPPORT)
    CamOsPrintf("uac unit ac fs:0x%x hs:0x%x ss:0x%x\r\n", if_fs, if_hs, if_ss);

    usb_class_ac_vc_app_create(if_ss, if_hs, if_fs, pusb_class_user_data);

    usb_class_ac_vc_app_set_speed_cur(USB_CLASS_FULL_SPEED, if_fs);
    usb_class_unit_app_list_create(if_fs, USB_IN_FU_ID);
    usb_class_unit_app_handler_register(if_fs, USB_IN_FU_ID, usb_ac_fu_cmd_req_entry, usb_ac_fu_cmd_out_entry);
    usb_class_ac_vc_app_reset_parameter_handler_register(if_fs, uac_unit_app_custom_reset_parameter);

    usb_class_ac_vc_app_set_speed_cur(USB_CLASS_HIGH_SPEED, if_hs);
    usb_class_unit_app_list_create(if_hs, USB_IN_FU_ID);
    usb_class_unit_app_handler_register(if_hs, USB_IN_FU_ID, usb_ac_fu_cmd_req_entry, usb_ac_fu_cmd_out_entry);
    usb_class_ac_vc_app_reset_parameter_handler_register(if_hs, uac_unit_app_custom_reset_parameter);

    usb_class_ac_vc_app_set_speed_cur(USB_CLASS_SUPER_SPEED, if_ss);
    usb_class_unit_app_list_create(if_ss, USB_IN_FU_ID);
    usb_class_unit_app_handler_register(if_ss, USB_IN_FU_ID, usb_ac_fu_cmd_req_entry, usb_ac_fu_cmd_out_entry);
    usb_class_ac_vc_app_reset_parameter_handler_register(if_ss, uac_unit_app_custom_reset_parameter);

    usb_ac_fu_cmd_init();
#endif
}

void uac_unit_app_custom_deinit(u8 if_ss, u8 if_hs, u8 if_fs)
{
#if defined(CONFIG_USB_GADGET_UAC_SUPPORT)
    usb_ac_fu_cmd_deinit();

    usb_class_ac_vc_app_set_speed_cur(USB_CLASS_FULL_SPEED, if_fs);
    usb_class_ac_vc_app_reset_parameter_handler_unregister(if_fs);
    usb_class_unit_app_list_destroy(if_fs, 0, 1);

    usb_class_ac_vc_app_set_speed_cur(USB_CLASS_HIGH_SPEED, if_hs);
    usb_class_ac_vc_app_reset_parameter_handler_unregister(if_hs);
    usb_class_unit_app_list_destroy(if_hs, 0, 1);

    usb_class_ac_vc_app_set_speed_cur(USB_CLASS_SUPER_SPEED, if_ss);
    usb_class_ac_vc_app_reset_parameter_handler_unregister(if_ss);
    usb_class_unit_app_list_destroy(if_ss, 0, 1);

    usb_class_ac_vc_app_destroy(if_ss, if_hs, if_fs);
#endif
}

void uac_unit_app_custom_speed_negotiation(u8 speed, u8 interface)
{
#if defined(CONFIG_USB_GADGET_UAC_SUPPORT)
    usb_class_ac_vc_app_set_speed_cur(speed, interface);
#endif
}
