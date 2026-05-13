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


/*
pcam message handler.
*/
#if defined(CONFIG_USB_GADGET_UAC_SUPPORT)
#include "pCam_msg.h"
#include "pCam_handler_audio.h"
#include "pCam_task.h"

#include "usb_ac_fu_range.h"

#include "mi_ai.h"

#include "uac_audio.h"
#include "usb_app_dbg.h"

//==============================================================================
//                              MACRO DEFINE
//==============================================================================

#define PCAM_AUDIO_MAX_CHANNEL_NUM (2)

//==============================================================================
//                              STRUCTURES
//==============================================================================

#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
typedef struct _PCAM_USB_INFO_FU
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
} PCAM_USB_INFO_FU;
#endif

typedef struct _PCAM_USB_MSG_PU
{
    u8 enable;
    u8 ch_id;
    u32 msg_id;
#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
    PCAM_USB_INFO_FU pcam_usb_info_fu;
#endif
} PCAM_USB_MSG_FU;

//==============================================================================
//                              ENUMERATIONS
//==============================================================================


//==============================================================================
//                              GLOBAL VARIABLES
//==============================================================================

PCAM_USB_MSG_FU gpcam_usb_msg_fu[PCAM_AUDIO_MAX_CHANNEL_NUM] =
{
    { //channel 0.
        .enable = 0,
        .ch_id = 0,
        .msg_id = 0,
#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
        .pcam_usb_info_fu =
        {
            .mute = MUTE_DEF,
            .volume = VOL_DEF,
            .bass = 0,
            .mid = 0,
            .treble = 0,
            .graphic_equalizer = 0,
            .automatic_gain = 0,
            .delay = 0,
            .bass_boost = 0,
            .loudness = 0,
        },
#endif
    },

    { //channel 1.
        .enable = 0,
        .ch_id = 1,
        .msg_id = 0,
#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
        .pcam_usb_info_fu =
        {
            .mute = MUTE_DEF,
            .volume = VOL_DEF,
            .bass = 0,
            .mid = 0,
            .treble = 0,
            .graphic_equalizer = 0,
            .automatic_gain = 0,
            .delay = 0,
            .bass_boost = 0,
            .loudness = 0,
        },
#endif
    },
};

//==============================================================================
//                              LOCAL FUNCTIONS
//==============================================================================

static u32 pcam_usb_set_audio_attr_mute(u8 ch_id, u8 mute)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_BOOL abMutes[2] = {0, 0};
#if 0
    //TBD
    extern MI_U8 StartCapture_UAC;

    if(StartCapture_UAC == FALSE)
    {
        return 0;
    }
#endif
    s32Ret = MI_AI_SetMute(0, 0, abMutes, mute);
    //CamOsPrintf("[mi mute:%d, ch:%d, ret:0x%x]", mute, ch_id, s32Ret);
    RET_VAL_ON(s32Ret != MI_SUCCESS, 1);

    return 0;
}

static u32 pcam_usb_set_audio_attr_volume(u8 ch_id, s16 volume)
{
    MI_S32 s32Ret = MI_SUCCESS;
#if 0
    //TBD
    extern MI_U8 StartCapture_UAC;

    if(StartCapture_UAC == FALSE)
    {
        return 0;
    }
#endif
    if(volume)
        volume = volume / 256 - 18;

#if 0 //TBD
    if (!ST_UAC_GetStartSt8()) {
        ST_UAC_SetDefVolume(volume);
        return PCAM_ERROR_NONE ;
    }
#endif

    //s32Ret = MI_AI_SetVqeVolume(0, 0, (MI_S32)volume);
    //CamOsPrintf("[mi vol:%d, ch:%d, ret:0x%x]", volume, ch_id, s32Ret);
    RET_VAL_ON(s32Ret != MI_SUCCESS, 1);

    return 0;
}

static PCAM_USB_MSG_FU *PCAM_USB_GetAttrFUHndl(u8 ch_id)
{
    RET_VAL_ON(ch_id >= PCAM_AUDIO_MAX_CHANNEL_NUM, NULL);
    return &(gpcam_usb_msg_fu[ch_id]);
}

static u32 PCAM_USB_SetAudioAttrByMsg(PcamTaskWorkCallback_fp fp_cb, u16 nonblocking, void *msg, unsigned long msg_data_size)
{
    u32 ret = 0;

    RET_VAL_ON(fp_cb == NULL, 1);

    if(nonblocking == PCAM_NONBLOCKING)
    {
        ret = pCam_PostMsg(fp_cb, msg, msg_data_size);
    }
    else if(nonblocking == PCAM_OVERWR)
    {
        //TBD
        ret = pCam_PostMsg(fp_cb, msg, msg_data_size);
    }
    else if(nonblocking == PCAM_BLOCKING)
    {
        ret = pCam_SendMsg(fp_cb, msg, msg_data_size);
    }
    else if(nonblocking == PCAM_API)
    {
        ret = fp_cb(msg, msg_data_size);
    }
    else
    {
        RET_VAL_ON(1, 1);
    }
    return ret;
}

static u32 PCAM_USB_SetAttrFU(void *msg, unsigned long msg_data_size)
{
    PCAM_USB_MSG_FU *ppcam_usb_fu_handle = NULL;
    u32 ret = PCAM_ERROR_NONE;

    ppcam_usb_fu_handle = (PCAM_USB_MSG_FU *)msg;
    RET_VAL_ON((ppcam_usb_fu_handle == NULL) || (msg_data_size != sizeof(PCAM_USB_MSG_FU)), 1);

    switch(ppcam_usb_fu_handle->msg_id)
    {
        case PCAM_MSG_USB_AUDIO_MUTE:
#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
            ret = pcam_usb_set_audio_attr_mute(ppcam_usb_fu_handle->ch_id, ppcam_usb_fu_handle->pcam_usb_info_fu.mute);
#endif
            break;

        case PCAM_MSG_USB_AUDIO_VOL:
#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
            ret = pcam_usb_set_audio_attr_volume(ppcam_usb_fu_handle->ch_id, ppcam_usb_fu_handle->pcam_usb_info_fu.volume);
#endif
            break;

        case PCAM_MSG_USB_AUDIO_BASS:
            //TBD
            break;

        case PCAM_MSG_USB_AUDIO_MID:
            //TBD
            break;

        case PCAM_MSG_USB_AUDIO_TREBLE:
            //TBD
            break;

        case PCAM_MSG_USB_AUDIO_GRAPHIC_EQUALIZER:
            //TBD
            break;

        case PCAM_MSG_USB_AUDIO_AUTOMATIC_GAIN:
            //TBD
            break;

        case PCAM_MSG_USB_AUDIO_DELAY:
            //TBD
            break;

        case PCAM_MSG_USB_AUDIO_BASS_BOOST:
            //TBD
            break;

        case PCAM_MSG_USB_AUDIO_LOUDNESS:
            //TBD
            break;

        default:
            CamOsPrintf("[%s] >UNKWN msg_sub_id %d ??,\r\n", __FUNCTION__, ((PCAM_USB_MSG_FU *)msg)->msg_id);
    }

    RET_VAL_ON(ret != 0, ret);
    return ret;
}

static u32 PCAM_USB_SetAttrFUCB(void *msg_data, unsigned long msg_data_size)
{
    return PCAM_USB_SetAttrFU(msg_data, msg_data_size);
}

//==============================================================================
//                              GLOBAL FUNCTIONS
//==============================================================================

void PCAM_USB_SetAudioAttrEnable(u8 ch_id, u8 enable)
{
    PCAM_USB_MSG_FU *ppcam_usb_fu_handle = NULL;

    ppcam_usb_fu_handle = PCAM_USB_GetAttrFUHndl(ch_id);
    RET_ON(ppcam_usb_fu_handle == NULL);
    ppcam_usb_fu_handle->enable = enable;
}

// 1 : Mute , 0 : UnMute
u32 USB_AudioSetMute(u8 ch_id, u16 nonblocking, u8 mute)
{
    u32 ret = 0;
    PCAM_USB_MSG_FU *ppcam_usb_fu_handle = NULL;

    ppcam_usb_fu_handle = PCAM_USB_GetAttrFUHndl(ch_id);
    RET_VAL_ON(ppcam_usb_fu_handle == NULL, 1);

#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
    ppcam_usb_fu_handle->pcam_usb_info_fu.mute = mute;

    if(ppcam_usb_fu_handle->enable)
    {
        ppcam_usb_fu_handle->msg_id = PCAM_MSG_USB_AUDIO_MUTE;
        ret = PCAM_USB_SetAudioAttrByMsg(PCAM_USB_SetAttrFUCB, nonblocking, ppcam_usb_fu_handle, sizeof(PCAM_USB_MSG_FU));
    }
#endif

    return ret;
}

// vol : UAC unit
u32 USB_AudioSetVolume(u8 ch_id, u16 nonblocking, s16 vol)
{
    u32 ret = 0;
    PCAM_USB_MSG_FU *ppcam_usb_fu_handle = NULL;

    ppcam_usb_fu_handle = PCAM_USB_GetAttrFUHndl(ch_id);
    RET_VAL_ON(ppcam_usb_fu_handle == NULL, 1);

#if (CONFIG_USB_GADGET_UAC_VERSION == 1)
    ppcam_usb_fu_handle->pcam_usb_info_fu.volume = vol;
    if(ppcam_usb_fu_handle->enable)
    {
        ppcam_usb_fu_handle->msg_id = PCAM_MSG_USB_AUDIO_VOL;
        ret = PCAM_USB_SetAudioAttrByMsg(PCAM_USB_SetAttrFUCB, nonblocking, ppcam_usb_fu_handle, sizeof(PCAM_USB_MSG_FU));
    }
#endif

    return ret;
}
#endif //#if defined(CONFIG_USB_GADGET_UAC_SUPPORT)
