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

#if defined(CONFIG_USB_GADGET_UVC_SUPPORT)

#ifndef _USB_GADGET_PRELOAD_H
#define _USB_GADGET_PRELOAD_H

#include "cam_os_wrapper.h"
#include "mi_common_datatype.h"
#include "mi_sys_datatype.h"

#include "st_ptz.h"
#include "uvc_video.h"

/* ===== request user layer API for UVC feature BEGIN ===== */

//==============================================================================
// MACRO DEFINE
//==============================================================================

#define MAX_VS_IF (CONFIG_USB_GADGET_UVC_STREAM_NUM)

#define UVCD_IMPL_SNRID_0                           (0)

#define ST_VPE_MAX_CNT                              (3) /* follow VPE block */
#define EN_PTZ_FEATURE                              (0) /* enable PTZ feature */
#define EN_PT_DBGLOG_                               (0) /* For debugging pan/tilt*/
#define VID_LATENCY_MEASURE                         (0) /* measure video latency */
#if (ST_VPE_MAX_CNT < MAX_VS_IF)
#error "ST_VPE_MAX_CNT must be greater than or equal to MAX_VS_IF"
#endif

#if defined(CONFIG_IPU_IN_RTOS_ENABLE)
#define DET_STREAM_NO    (CONFIG_IPU_DETECT_STREAM) //deconfig
#endif

//==============================================================================
//                              GLOBAL FUNCTIONS
//==============================================================================

void usb_vs_set_latency_measurement_tx_enable(u8 latency_measurement_enable);
u8 usb_vs_get_latency_measurement_tx_enable(void);
void usb_vs_set_latency_measurement_tx_start(unsigned int vid_idx, u8 latency_measurement_start);
u8 usb_vs_get_latency_measurement_tx_start(unsigned int vid_idx);
u32 usb_vs_get_average_frame_time_interval(unsigned int vid_idx);
struct ST_Stream_Attr_T *uvc_get_app_attr(unsigned int vid_idx);
void usb_vs_set_one_frame_time_tx_start(unsigned int vid_idx);
void usb_vs_set_one_frame_time_tx_end(unsigned int vid_idx);
u32 usb_vs_get_one_frame_time_tx(unsigned int vid_idx);
void usb_vs_set_one_frame_size_tx(unsigned int vid_idx, u32 frame_size);
u32 usb_vs_get_one_frame_size_tx(unsigned int vid_idx);
void usb_vs_reset_latency_measurement(unsigned int vid_idx);

u32 usb_vs_get_stream_data_pts_average(u32 strm_no);
void usb_vs_set_stream_data_pts(u32 strm_no, MI_U64 u64Pts);
void usb_vs_set_stream_data_pts_mjpeg_realtime(u32 strm_no, MI_U64 u64Pts);

#endif  //#ifndef _USB_GADGET_PRELOAD_H
#endif  //#if defined(CONFIG_USB_GADGET_UVC_SUPPORT)

