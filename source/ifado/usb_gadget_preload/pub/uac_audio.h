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

#ifndef _UAC_AUDIO_H
#define _UAC_AUDIO_H

#include <stdint.h>
#include "usb_common.h"
#include "usb_class_cfg.h"

#define UAC_MAX_CHANNEL_NUM (2) //UAC_MIC_CHANNEL_NUM

struct uac_control {
    u8 ctype;
    u8 interface;
    u8 request;
    u8 control;
    u8 val;
    u8 entity;
    u16 length;
};

#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
struct uac_fu
{
    u8 mute;
    s16 volume;
    u16 bass;
    u16 mid;
    u16 treble;
    u16 graphic_equalizer;
    u16 automatic_gain;
    u16 delay;
    u16 bass_boost;
    u16 loudness;
};

#elif (CONFIG_USB_GADGET_UAC_VERSION == 2)
    //TBD
#endif

struct uac_device {
    struct usb_request_data request_error_code;
    struct uac_control control;
    u8 status;
    int exit_request;

#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
    struct uac_fu fu_para[UAC_MAX_CHANNEL_NUM];
#endif
} ;

struct cntrl_cur_lay3 {
    u32	dCUR;
};

struct cntrl_range_lay3 {
    u16	wNumSubRanges;
    u32	dMIN;
    u32	dMAX;
    u32	dRES;
} __attribute__((packed));

void uac_process_req_ac(struct urequest *ctrl, struct usb_request_data *resp);
void uac_process_data_ac(struct usb_request_data *data);

void uac_ac_process_setup_init(u8 idx, u8 if_ss, u8 if_hs, u8 if_fs);
void uac_ac_process_setup_deinit(u8 if_ss, u8 if_hs, u8 if_fs);
void uac_ac_process_setup_speed_negotiation(u8 speed, u8 interface);

void uac_audio_init();

#endif

