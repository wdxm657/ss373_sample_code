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
#if defined(CONFIG_USB_GADGET_UAC_SUPPORT)
#include <string.h>
#include "cam_os_wrapper.h"
#include "usb_common.h"
#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
#include "uac.h"
#else
#include "uac2.h"
#endif
#include "uac_audio.h"
#include "usb_class_ac_vc.h"
#include "uac_unit_app_custom.h"

struct uac_device guac_dev;

#if 0 //master branch TBD
static __inline__ char *uac_request_str(u8 request)
{
    if (request == UAC_GET_CUR)
        return "UAC GET CUR";
    else if (request == UAC_SET_CUR)
        return "UAC SET CUR";
    else if (request == UAC_GET_RES)
        return "UAC GET RES";
    else if (request == UAC_GET_MIN)
        return "UAC GET MIN";
    else if (request == UAC_GET_MAX)
        return "UAC_GET_MAX";
    else
        return "unknown req";
}

static __inline__ char *uac_cs_str(u8 cs)
{
    if (cs == UAC_FU_VOLUME)
        return "VOLUME";
    else if (cs == UAC_FU_MUTE)
        return "MUTE";
    else if (cs == UAC_FU_BASS)
        return "BASS";
    else if (cs == UAC_FU_MID)
        return "MID";
    else if (cs == UAC_FU_TREBLE)
        return "TREBLE";
    else
        return "unknown cs";
}
#endif

#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
void uac_process_req_ac(struct urequest *ctrl, struct usb_request_data *resp)
{
    guac_dev.control.ctype = 0/*audio control*/;
    guac_dev.control.interface = (u8)(ctrl->wIndex & 0xff);
    guac_dev.control.request = (u8)(ctrl->bRequest);
    guac_dev.control.entity = (unsigned char)(ctrl->wIndex >> 8);  // entity_id
    guac_dev.control.control = (unsigned char)(ctrl->wValue >> 8);  // CS
    guac_dev.control.val = (unsigned char)(ctrl->wValue & 0xff);
    guac_dev.control.length = (unsigned short)(ctrl->wLength);      // Length.

#if 0
    CamOsPrintf("ac req if:%d, id:%d, req:0x%x, cs:0x%x val:0x%x!\n",
            (u8)(ctrl->wIndex & 0xff),
            (u8)(ctrl->wIndex >> 8),
            (u8)(ctrl->bRequest),
            (u8)(ctrl->wValue >> 8),
            (u8)(ctrl->wValue & 0xff));
#endif

#if 1
    usb_vc_cs_req_do((u8)(ctrl->wIndex & 0xff), //interface
            (u8)(ctrl->wIndex >> 8),// entity_id
            (u8)(ctrl->bRequest), // req
            (u8)(ctrl->wValue >> 8),// CS
            (u8)(ctrl->wValue & 0xff),// val
            (u16)(ctrl->wLength), // Length.
            resp);
#endif
}

#else
void uac_process_req_ac(struct urequest *ctrl, struct usb_request_data *resp)
{
    u8 req, cs, entity_id;
    u16 len;
    struct cntrl_cur_lay3 cur;
    struct cntrl_range_lay3 range;

    req = ctrl->bRequest;
    cs = ctrl->wValue >> 8;
    entity_id = ctrl->wIndex >> 8;
    len = ctrl->wLength;

    memset(&cur, 0, sizeof(struct cntrl_cur_lay3));
    memset(&range, 0, sizeof(struct cntrl_range_lay3));

    if (req == UAC2_CS_CUR)
    {
        if (cs == UAC2_CS_CONTROL_SAM_FREQ)
        {
            if (entity_id == UAC2_IN_CLK_SRC_ID)
            {
                 cur.dCUR = 48000;
            }
            else if (entity_id == UAC2_OUT_CLK_SRC_ID)
            {
                cur.dCUR = 48000;
            }
            len = CAM_OS_MIN(len ,sizeof cur);
            memcpy(resp->data, &cur, len);
            resp->length = len;
            resp->direct = USB_REQ_DATA_DIR_IN;
        }
        else if (cs == UAC2_CS_CONTROL_CLOCK_VALID)
        {
            resp->data[0] = 1;
            len = CAM_OS_MIN(len ,1);
            resp->direct = USB_REQ_DATA_DIR_IN;
        }
        else
        {
        }
    }
    else if (req == UAC2_CS_RANGE)
    {
        if (cs == UAC2_CS_CONTROL_SAM_FREQ)
        {
            if (entity_id == UAC2_IN_CLK_SRC_ID)
            {
                range.dMIN = 48000;
            }
            else if (entity_id == UAC2_OUT_CLK_SRC_ID)
            {
                range.dMIN = 48000;
            }

            range.wNumSubRanges = 1;
            range.dMAX = 48000;
            range.dRES = 0;
            len = CAM_OS_MIN(len ,sizeof range);
            memcpy(resp->data, &range, len);
            resp->length = len;
            resp->direct = USB_REQ_DATA_DIR_IN;
        }
        else
        {

        }
    }
    else
    {

    }
}



#endif


void uac_process_data_ac(struct usb_request_data *req_data)
{
    u8 interface;
    //u8 request;
    u8 entity;
    u8 cs, val;
    u32 len;
    int ret_vc_cs_out_cmd = 0;

    interface = guac_dev.control.interface;
    //request = guac_dev.control.request;
    entity = guac_dev.control.entity;
    cs = guac_dev.control.control;
    val = guac_dev.control.val;
    len = guac_dev.control.length;

    //======================================
    //Check request is UVC_SET_CUR or not?
    //======================================
    guac_dev.request_error_code.data[0] = 0;
    guac_dev.request_error_code.length = 1;
    ret_vc_cs_out_cmd = usb_class_unit_app_handler_out_do(interface, entity, cs, val, (u8 *)(req_data->data), len);
    if(ret_vc_cs_out_cmd)
    {
        CamOsPrintf("%s err:%d!\n",__FUNCTION__, ret_vc_cs_out_cmd);
    }
}

void uac_ac_process_setup_init(u8 idx, u8 if_ss, u8 if_hs, u8 if_fs)
{
#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
    uac_unit_app_custom_init(if_ss, if_hs, if_fs, &guac_dev);
    usb_class_ac_vc_app_reset_parameter_handler_do(if_ss, 1);
#endif
}

void uac_ac_process_setup_deinit(u8 if_ss, u8 if_hs, u8 if_fs)
{
#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
    uac_unit_app_custom_deinit(if_ss, if_hs, if_fs);
#endif
}

void uac_ac_process_setup_speed_negotiation(u8 speed, u8 interface)
{
#if defined(CONFIG_USB_GADGET_UAC_MIC_SUPPORT)
    uac_unit_app_custom_speed_negotiation(
            (speed == USB_SPEED_SUPER) ? USB_CLASS_SUPER_SPEED :
            (speed == USB_SPEED_HIGH) ? USB_CLASS_HIGH_SPEED :
            USB_CLASS_FULL_SPEED,
            interface);
#endif
}

void uac_audio_init()
{
    //NOP
}

#endif
