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

#if defined(CONFIG_USB_GADGET_UAC_SUPPORT)
#include "usb_ac_fu.h"
#include "uac_definition.h"
#include "usb_ac_fu_range.h"

#include "pCam_handler_audio.h"

vc_cmd_cfg FU_MUTE_CFG =
{
    (CAP_SET_CUR_CMD | CAP_GET_CUR_CMD),
    0, // no GET_INFO
    1,0,
    MUTE_RES,MUTE_DEF,MUTE_MIN,MUTE_MAX,MUTE_DEF
};

vc_cmd_cfg FU_VOL_CFG =
{
    (CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD),
    0, // no GET_INFO
    2,0,
    VOL_RES,VOL_DEF,VOL_MIN,VOL_MAX,VOL_DEF
};

int usb_ac_fu_cmd_req_fu_mute_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
    if(val >= UAC_MAX_CHANNEL_NUM)
    {
        CamOsPrintf("[%s %d!] err\n", __FUNCTION__, __LINE__);
        ret = 1;
        return ret;
    }

    usb_vc_cmd_set_cur_val(&FU_MUTE_CFG, (unsigned long)(uac_dev->fu_para[val].mute));
#endif
    usb_vc_cmd_get_cfg(cfg, &FU_MUTE_CFG);
    return ret;
}

int usb_ac_fu_cmd_req_fu_volume_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
    if(val >= UAC_MAX_CHANNEL_NUM)
    {
        CamOsPrintf("[%s %d!] err\n", __FUNCTION__, __LINE__);
        ret = 1;
        return ret;
    }

    usb_vc_cmd_set_cur_val(&FU_VOL_CFG, (unsigned long)(uac_dev->fu_para[val].volume));
#endif
    usb_vc_cmd_get_cfg(cfg, &FU_VOL_CFG);
    return ret;
}

int usb_ac_fu_cmd_req_fu_control_undefined(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

    uac_dev->request_error_code.data[0] = 6/*TBD*/;
    uac_dev->request_error_code.length = 1;
    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_req_fu_bass_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

    uac_dev->request_error_code.data[0] = 6/*TBD*/;
    uac_dev->request_error_code.length = 1;
    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_req_fu_mid_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

    uac_dev->request_error_code.data[0] = 6/*TBD*/;
    uac_dev->request_error_code.length = 1;
    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_req_fu_treble_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

    uac_dev->request_error_code.data[0] = 6/*TBD*/;
    uac_dev->request_error_code.length = 1;
    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_req_fu_graphic_equalizer_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

    uac_dev->request_error_code.data[0] = 6/*TBD*/;
    uac_dev->request_error_code.length = 1;
    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_req_fu_automatic_gain_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

    uac_dev->request_error_code.data[0] = 6/*TBD*/;
    uac_dev->request_error_code.length = 1;
    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_req_fu_delay_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

    uac_dev->request_error_code.data[0] = 6/*TBD*/;
    uac_dev->request_error_code.length = 1;
    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_req_fu_bass_boost_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

    uac_dev->request_error_code.data[0] = 6/*TBD*/;
    uac_dev->request_error_code.length = 1;
    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_req_fu_loudness_control(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

    uac_dev->request_error_code.data[0] = 6/*TBD*/;
    uac_dev->request_error_code.length = 1;
    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_out_fu_mute_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;
    u8 tmp;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

    //tmp = pbuf[0];
    usb_vc_get_req_data(pbuf, buf_len, &tmp, sizeof(tmp));

    if(tmp > 1)
    {
        // un-support
        uac_dev->request_error_code.data[0] = 4/*TBD*/;
        uac_dev->request_error_code.length = 1;
        return -USB_E_OVERFLOW;
    }
    else
    {
#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
        if(val >= UAC_MAX_CHANNEL_NUM)
        {
            CamOsPrintf("[%s %d!] err\n", __FUNCTION__, __LINE__);
            ret = 1;
            return ret;
        }

        uac_dev->fu_para[val].mute = tmp;
        USB_AudioSetMute(val/*ch id*/, PCAM_OVERWR, uac_dev->fu_para[val].mute);
#endif
    }

    return ret;
}

int usb_ac_fu_cmd_out_fu_volume_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;
    s16 tmp2;
    struct uac_device *uac_dev = (struct uac_device *)puser_data;

    //tmp2 = pbuf[0] | (pbuf[1] << 8);
    usb_vc_get_req_data(pbuf, buf_len, (u8 *)&tmp2, sizeof(tmp2));

    if(tmp2 == (s16) VOL_MUTE)
    {
#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
        if(val >= UAC_MAX_CHANNEL_NUM)
        {
            CamOsPrintf("[%s %d!] err\n", __FUNCTION__, __LINE__);
            ret = 1;
            return ret;
        }

        uac_dev->fu_para[val].volume = tmp2;
        USB_AudioSetVolume(val/*ch id*/, PCAM_OVERWR/*PCAM_API*/, uac_dev->fu_para[val].volume); // -> Use over write method
#endif
    }
    else
    {
        if(((s16) tmp2) > (s16) VOL_MAX || ((s16) tmp2) < (s16) VOL_MIN)
        {
            // un-support
            uac_dev->request_error_code.data[0] = 4/*TBD*/;
            uac_dev->request_error_code.length = 1;
            CamOsPrintf("[Vol err:%d]", tmp2);
            return -USB_E_OVERFLOW;
        }
        else
        {
#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
            if(val >= UAC_MAX_CHANNEL_NUM)
            {
                CamOsPrintf("[%s %d!] err\n", __FUNCTION__, __LINE__);
                ret = 1;
                return ret;
            }

            uac_dev->fu_para[val].volume = tmp2;
            // to do here
            USB_AudioSetVolume(val/*ch id*/, PCAM_OVERWR, uac_dev->fu_para[val].volume);
            //CamOsPrintf("[Vol:%d]", uac_dev->fu_para.volume);
#endif
        }
    }

    return ret;
}

int usb_ac_fu_cmd_out_fu_control_undefined(u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;

    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_out_fu_bass_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;

    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_out_fu_mid_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;

    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_out_fu_treble_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;

    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_out_fu_graphic_equalizer_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;

    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_out_fu_automatic_gain_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;

    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_out_fu_delay_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;

    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_out_fu_bass_boost_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;

    ret = -USB_E_INVALID;
    return ret;
}

int usb_ac_fu_cmd_out_fu_loudness_control(u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;

    ret = -USB_E_INVALID;
    return ret;
}

void usb_ac_fu_cmd_init(void)
{
    //CamOsPrintf("[%s %d!]\n", __FUNCTION__, __LINE__);
}

void usb_ac_fu_cmd_deinit(void)
{

}

void usb_ac_fu_cmd_init_parameter(u8 reset_val, void *puser_data)
{
    struct uac_device *uac_dev = (struct uac_device *)puser_data;
    u32 loop;

#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
    for(loop = 0; loop < UAC_MAX_CHANNEL_NUM; ++loop)
    {
        uac_dev->fu_para[loop].mute = MUTE_DEF;
        uac_dev->fu_para[loop].volume = VOL_DEF;
        uac_dev->fu_para[loop].bass = 0;
        uac_dev->fu_para[loop].mid = 0;
        uac_dev->fu_para[loop].treble = 0;
        uac_dev->fu_para[loop].graphic_equalizer = 0;
        uac_dev->fu_para[loop].automatic_gain = 0;
        uac_dev->fu_para[loop].delay = 0;
        uac_dev->fu_para[loop].bass_boost = 0;
        uac_dev->fu_para[loop].loudness = 0;
    }
#endif
}

int usb_ac_fu_cmd_req_entry(u8 req, u8 cs, u8 val, u16 len, vc_cmd_cfg *cfg, void *puser_data)
{
    int ret = 0;

    switch(cs)
    {
        case FU_MUTE_CONTROL:
            ret = usb_ac_fu_cmd_req_fu_mute_control(req, cs, val, len, cfg, puser_data);
            break;
        case FU_VOLUME_CONTROL:
            ret = usb_ac_fu_cmd_req_fu_volume_control(req, cs, val, len, cfg, puser_data);
            break;
        case FU_CONTROL_UNDEFINED:
            ret = usb_ac_fu_cmd_req_fu_control_undefined(req, cs, val, len, cfg, puser_data);
            break;
        case FU_BASS_CONTROL:
            ret = usb_ac_fu_cmd_req_fu_bass_control(req, cs, val, len, cfg, puser_data);
            break;
        case FU_MID_CONTROL:
            ret = usb_ac_fu_cmd_req_fu_mid_control(req, cs, val, len, cfg, puser_data);
            break;
        case FU_TREBLE_CONTROL:
            ret = usb_ac_fu_cmd_req_fu_treble_control(req, cs, val, len, cfg, puser_data);
            break;
        case FU_GRAPHIC_EQUALIZER_CONTROL:
            ret = usb_ac_fu_cmd_req_fu_graphic_equalizer_control(req, cs, val, len, cfg, puser_data);
            break;
        case FU_AUTOMATIC_GAIN_CONTROL:
            ret = usb_ac_fu_cmd_req_fu_automatic_gain_control(req, cs, val, len, cfg, puser_data);
            break;
        case FU_DELAY_CONTROL:
            ret = usb_ac_fu_cmd_req_fu_delay_control(req, cs, val, len, cfg, puser_data);
            break;
        case FU_BASS_BOOST_CONTROL:
            ret = usb_ac_fu_cmd_req_fu_bass_boost_control(req, cs, val, len, cfg, puser_data);
            break;
        case FU_LOUDNESS_CONTROL:
            ret = usb_ac_fu_cmd_req_fu_loudness_control(req, cs, val, len, cfg, puser_data);
            break;
        default:
            ret = -USB_E_INVALID;
            break;
    }

    return ret;
}

int usb_ac_fu_cmd_out_entry(u8 cs, u8 val, u8 *pbuf, u16 buf_len, void *puser_data)
{
    int ret = 0;

    switch(cs)
    {
        case FU_MUTE_CONTROL:
            ret = usb_ac_fu_cmd_out_fu_mute_control(val, pbuf, buf_len, puser_data);
            break;
        case FU_VOLUME_CONTROL:
            ret = usb_ac_fu_cmd_out_fu_volume_control(val, pbuf, buf_len, puser_data);
            break;
        case FU_CONTROL_UNDEFINED:
            ret = usb_ac_fu_cmd_out_fu_control_undefined(val, pbuf, buf_len, puser_data);
            break;
        case FU_BASS_CONTROL:
            ret = usb_ac_fu_cmd_out_fu_bass_control(val, pbuf, buf_len, puser_data);
            break;
        case FU_MID_CONTROL:
            ret = usb_ac_fu_cmd_out_fu_mid_control(val, pbuf, buf_len, puser_data);
            break;
        case FU_TREBLE_CONTROL:
            ret = usb_ac_fu_cmd_out_fu_treble_control(val, pbuf, buf_len, puser_data);
            break;
        case FU_GRAPHIC_EQUALIZER_CONTROL:
            ret = usb_ac_fu_cmd_out_fu_graphic_equalizer_control(val, pbuf, buf_len, puser_data);
            break;
        case FU_AUTOMATIC_GAIN_CONTROL:
            ret = usb_ac_fu_cmd_out_fu_automatic_gain_control(val, pbuf, buf_len, puser_data);
            break;
        case FU_DELAY_CONTROL:
            ret = usb_ac_fu_cmd_out_fu_delay_control(val, pbuf, buf_len, puser_data);
            break;
        case FU_BASS_BOOST_CONTROL:
            ret = usb_ac_fu_cmd_out_fu_bass_boost_control(val, pbuf, buf_len, puser_data);
            break;
        case FU_LOUDNESS_CONTROL:
            ret = usb_ac_fu_cmd_out_fu_loudness_control(val, pbuf, buf_len, puser_data);
            break;
        default:
            ret = -USB_E_INVALID;
            break;
    }

    return ret;
}
#endif //#if defined(CONFIG_USB_GADGET_UAC_SUPPORT)
