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

#ifndef __USB_AC_FU_H__
#define __USB_AC_FU_H__

#include "cam_os_wrapper.h"
#include "uac_audio.h"

int usb_ac_fu_cmd_req_fu_mute_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);
int usb_ac_fu_cmd_req_fu_volume_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);
int usb_ac_fu_cmd_req_fu_control_undefined(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);
int usb_ac_fu_cmd_req_fu_bass_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);
int usb_ac_fu_cmd_req_fu_mid_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);
int usb_ac_fu_cmd_req_fu_treble_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);
int usb_ac_fu_cmd_req_fu_graphic_equalizer_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);
int usb_ac_fu_cmd_req_fu_automatic_gain_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);
int usb_ac_fu_cmd_req_fu_delay_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);
int usb_ac_fu_cmd_req_fu_bass_boost_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);
int usb_ac_fu_cmd_req_fu_loudness_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);

int usb_ac_fu_cmd_out_fu_mute_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data);
int usb_ac_fu_cmd_out_fu_volume_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data);
int usb_ac_fu_cmd_out_fu_control_undefined(u8 val, u8 *pbuf, u16 buf_len, void *puser_data);
int usb_ac_fu_cmd_out_fu_bass_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data);
int usb_ac_fu_cmd_out_fu_mid_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data);
int usb_ac_fu_cmd_out_fu_treble_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data);
int usb_ac_fu_cmd_out_fu_graphic_equalizer_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data);
int usb_ac_fu_cmd_out_fu_automatic_gain_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data);
int usb_ac_fu_cmd_out_fu_delay_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data);
int usb_ac_fu_cmd_out_fu_bass_boost_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data);
int usb_ac_fu_cmd_out_fu_loudness_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data);

void usb_ac_fu_cmd_init(void);
void usb_ac_fu_cmd_deinit(void);
void usb_ac_fu_cmd_init_parameter(u8 reset_val, void *puser_data);

int usb_ac_fu_cmd_req_entry(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data);
int usb_ac_fu_cmd_out_entry(u8 cs, u8 val, u8 *pbuf, u16 buf_len, void *puser_data);

#endif //#ifndef __USB_AC_FU_H__

