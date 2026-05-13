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


#ifndef _PCAM_HANDLER_AUDIO_H
#define _PCAM_HANDLER_AUDIO_H

#include "mi_common_datatype.h"

//==============================================================================
//                              MACRO DEFINE
//==============================================================================

//==============================================================================
//                              STRUCTURES
//==============================================================================


//==============================================================================
//                              ENUMERATIONS
//==============================================================================


//==============================================================================
//                              GLOBAL VARIABLES
//==============================================================================

//==============================================================================
//                              GLOBAL FUNCTIONS
//==============================================================================

void PCAM_USB_SetAudioAttrEnable(u8 ch_id, u8 enable);
// 1 : Mute , 0 : UnMute
u32 USB_AudioSetMute(u8 ch_id, u16 nonblocking, u8 mute);
// vol : UAC unit
u32 USB_AudioSetVolume(u8 ch_id, u16 nonblocking, s16 vol);

#endif
