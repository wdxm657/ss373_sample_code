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

#ifndef _PCAM_MSG_H
#define _PCAM_MSG_H

#include "cam_os_wrapper.h"

#define PCAM_FLAG_MSG           (0x00000001) // The message queue at task queue
#define PCAM_FLAG_OVERWR_MSG    (0x00000002) // The message's data block would be overwrite by next request.

#define MMP_XU_OTA_OK           0x00
#define MMP_XU_OTA_WAIT         0x81
#define MMP_XU_OTA_ERR          0x82


typedef struct _Body_pcam_t
{
    unsigned int   src_id ;
    unsigned int   dst_id ;
    unsigned int   msg_id ;
    unsigned int   msg_sub_id ;
    void        *msg_data ;
    void        *msg_sem;
    //MMP_ULONG   err ;
} Body_pcam_t, pcam_msg_t ;

typedef u32 (*PcamTaskWorkCallback_fp)(void *msg_data, unsigned long msg_data_size);

enum {
    PCAM_ERROR_NONE=0,

    /*system error*/
    PCAM_SYS_ERR,
    PCAM_SYS_TIMEOUT,

    PCAM_ERROR_MAX
};

enum {
    /*Preview part */
    PCAM_MSG_USB_INIT = 0 ,
    PCAM_MSG_USB_PREVIEW_START  ,
    PCAM_MSG_USB_PREVIEW_STOP  ,
    PCAM_MSG_USB_UPDATE_OSD,
    PCAM_MSG_USB_SET_ATTRS ,
    PCAM_MSG_USB_CAPTURE,
    PCAM_MSG_USB_EXIT ,
    /*Audio Part */
    PCAM_MSG_USB_AUDIO_START = 10,
    PCAM_MSG_USB_AUDIO_SET_SAMPLINGRATE,
    PCAM_MSG_USB_AUDIO_STOP,
    PCAM_MSG_USB_AUDIO_MUTE,
    PCAM_MSG_USB_AUDIO_VOL,
    PCAM_MSG_USB_AUDIO_BASS,
    PCAM_MSG_USB_AUDIO_MID,
    PCAM_MSG_USB_AUDIO_TREBLE,
    PCAM_MSG_USB_AUDIO_GRAPHIC_EQUALIZER,
    PCAM_MSG_USB_AUDIO_AUTOMATIC_GAIN,
    PCAM_MSG_USB_AUDIO_DELAY,
    PCAM_MSG_USB_AUDIO_BASS_BOOST,
    PCAM_MSG_USB_AUDIO_LOUDNESS,
    PCAM_MSG_USB_AUDIO_RESET,

    /*DFU part*/
    PCAM_MSG_USB_UPDATE_FW = 30,
    /*Debug*/
    PCAM_MSG_USB_CHECK_ALIVE,
    /*Take Raw data*/
    PCAM_MSG_USB_CAPTURE_RAW,
    PCAM_MSG_USB_AUDIO_TICK,
    PCAM_MSG_RAW_PROCESSING,
    /* FDTC part */
    PCAM_MSG_FDTC_INIT = 40,
    PCAM_MSG_FDTC_START,
    PCAM_MSG_FDTC_STOP,

    PCAM_MSG_FFOV_FRM_RDY,

    PCAM_MSG_ACCEL_PRCS,

    PCAM_MSG_MAX_NUM
} ;

#if 1
typedef enum {
    PCAM_USB_DL_FW_INIT,
    PCAM_USB_DL_FW_BURN,
    PCAM_USB_DL_FW_ERASE,
    PCAM_USB_DL_FW_MAX,
} PCAM_USB_DL_FW_TYPE;

typedef enum {
    PCAM_USB_DL_ERASE_ALL = 1,
    PCAM_USB_DL_ERASE_ADDR,
    PCAM_USB_DL_ERASE_PARTITION,
    PCAM_USB_DL_ERASE_MAX,
}PCAM_USB_DL_ERASE_MODE;

typedef struct
{
    PCAM_USB_DL_FW_TYPE     DL_FWType;
    PCAM_USB_DL_ERASE_MODE  EraseMode;
    unsigned int            ulAddr;
}PCAM_USB_DL_ATTR;
#endif

u32 pCam_SendMsg(PcamTaskWorkCallback_fp fp_cb, void *msg_data, unsigned long msg_data_size);
u32 pCam_PostMsg(PcamTaskWorkCallback_fp fp_cb, void *msg_data, unsigned long msg_data_size);

#endif
