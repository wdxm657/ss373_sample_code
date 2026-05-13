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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"
#include "sys_sys_isw_cli.h"
#include "sys_sys_time.h"
#include "sys_memmap.h"
#include "sys_MsWrapper_cus_os_util.h"
#include "sys_MsWrapper_cus_os_flag.h"
#include "sys_MsWrapper_cus_os_mem.h"
#include "initcall.h"
#include "drv_dualos.h"
#include "sys_sys_console.h"
#include "application_selector.h"
#include "mi_device.h"
#include "mi_sys_internal.h"
#include "mi_common_internal.h"
#include "mi_common_modparam.h"
#include "mi_sys.h"
#if defined(CONFIG_MI_SDK_SUPPORT)
#if INTERFACE_VIF
#include "mi_vif.h"
#include "mi_vif_impl.h"
#endif
#if INTERFACE_SCL
#include "mi_scl.h"
#endif
#if INTERFACE_VENC
#include "mi_venc.h"
#include "mi_venc_datatype.h"
#include "mi_venc_impl.h"
#endif
#if INTERFACE_RGN
#include "mi_rgn.h"
#include "mi_rgn_internal.h"
#endif
#if INTERFACE_ISP
#include "mi_isp.h"
#include "mi_isp_cus3a_api.h"
#include "mi_isp_iq.h"
#include "isp_cus3a_if.h"
#include "mi_isp_cus3a_api.h"
#include "mi_isp_iq.h"
#endif
#if INTERFACE_SENSOR
#include "mi_sensor_datatype.h"
#include "mi_sensor.h"
#include "mi_sensor_impl.h"
#endif
#include "mi_ai.h"
#include "mi_ao.h"
#include "CameraSetting.h"
#include "cpu_mem_map.hc"
#include "mi_vdf.h"
#include "sigma_algo.h"
#include "mi_shadow_impl.h"
#include "mi_ipu.h"
#include "mi_sed.h"
#include "mi_disp.h"
#ifdef CONFIG_PANEL_IN_RTOS_ENABLE
#include "mi_panel.h"
#endif
#include "mi_fb.h"
#include "mi_vdisp.h"
#include "sys_sys_boot_timestamp.h"
#include "sys_sys_early_param.h"
#include <mhal_earlyinit_para.h>
#include "env_util.h"
#include "ms_msys.h"
#include <drv_gpio_io.h>
#include <gpio.h>
#if INTERFACE_LDC
#include "mi_ldc.h"
#endif //INTERFACE_LDC
#endif // CONFIG_MI_SDK_SUPPORT

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
#include "earlyinit_preload_api.h"
#include "earlyinit_rtos_api.h"
#endif


////////////////////////////////////////////////////////////////////////////////

#if (defined CONFIG_SIGMASTAR_CHIP_MARUKO) && (CONFIG_SIGMASTAR_CHIP_MARUKO == 1) && (defined MHAL_VCODEC)
#define CODEC_HEAP_SIZE     0x00200000
#else
#define CODEC_HEAP_SIZE     0x00000000
#endif
#if INTERFACE_IPU
#define IPU_HEAP_SIZE       0x0164000
#else
#define IPU_HEAP_SIZE       0x0000000
#endif

extern int MI_DEVICE_Init(void *pMmaConfig);
extern int mi_debug_init(void);
#if defined(CONFIG_USB_GADGET_UVC_SUPPORT)
#include "usb_device.h"
#include "uvc_common.h"
#include "uvc_video.h"
#include "pCam_handler.h"
#include "usb_vc_ct_range.h"
#include "usb_app_dbg.h"
#include "st_ptz.h"
#include "composite.h"
#include "usb_gadget_preload.h"
#include "uac_app.h"
#include "cdc_app.h"

#define RED_BOLD "\x1b[;31;1m"
#define GRN_BOLD "\x1b[;32;1m"
#define BLU_BOLD "\x1b[;34;1m"
#define PURPLE "\033[0;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define WHITE "\033[1;37m"

#define USB_CAMERA0_INDEX 0
#define USB_CAMERA1_INDEX 1
#define USB_CAMERA2_INDEX 2
#define USB_CAMERA3_INDEX 3
#define USB_CAMERA4_INDEX 4
#define USB_CAMERA5_INDEX 5

#define UVC_STREAM0 "uvc_stream0"
#define UVC_STREAM1 "uvc_stream1"
#define UVC_STREAM2 "uvc_stream2"
#define UVC_STREAM3 "uvc_stream3"
#define UVC_STREAM4 "uvc_stream4"
#define UVC_STREAM5 "uvc_stream5"

#define TASK_PCAM_PRIORITY (96)
#define TASK_PCAM_STACK_SIZE (6144)
#define TASK_PCAM_NAME "PCAM"

#define TASK_USB_APP_PRIORITY (100)
#define TASK_USB_APP_STACK_SIZE (4096)
#define TASK_USB_APP_NAME "USB_APP_CTRL"
CamOsThread composite_thread = NULL;
CamOsTsem_t composite_sem = {0};
u8 composite_thread_exit = 0;

#if (VID_LATENCY_MEASURE)
#define UVC_VS_CAMOS_TIME_RES_2_US(res_sec, res_nanosec) ((res_sec * 1000000) + (res_nanosec / 1000))
#define UVC_VS_CAMOS_TIME_RES_DIFF_US(res_start, res_end, return_type)                                                 \
    (return_type)((((s64)res_end->nSec - (s64)res_start->nSec) * 1000000) +                                            \
                  (((s64)res_end->nNanoSec - (s64)res_start->nNanoSec) / 1000))

u8 guvc_latency_measurement_enable = 0;
#endif

struct frm_prerdy {
    MI_U16 height;
    MI_U16 num;
    MI_U16 dem;
};

static enum usb_speed_type udc_speed = USB_SPEED_UNKNOWN;
static struct frm_prerdy prerdy_ratio_ss[4] = {
    { 480, 7, 8},   // H < 480
    { 720, 3, 4},   // H < 720
    {1200, 1, 4},   // H < 1200
    {1296, 1, 2}    // H >= 1200
};

static struct frm_prerdy prerdy_ratio_hs[2] = {
    { 360, 3, 4},   // H < 360
    { 720, 1, 4}    // H >= 360
};

#if (VID_LATENCY_MEASURE)
typedef struct _UVC_LATENCY_MEASUREMENT_t_
{
    u8 uvc_latency_measurement_start;
    CamOsTimespec_t uvc_current_frame_time_tx_start; // unit: us.
    u32 uvc_average_frame_time_tx;                   // unit: us.
    u32 uvc_average_frame_size_tx;                   // unit: kbytes.
    u32 uvc_average_frame_time_interval;
    u32 uvc_stream_pts_average_diff; // unit: us.
} UVC_LATENCY_MEASUREMENT_t;
#endif

struct ST_Uvc_Attr_T
{
    MI_U32 u32SclDev;
    MI_U32 u32SclChn;
    MI_U32 u32SclPort;
    MI_U32 u32VencDev;
    MI_U32 u32VencChn;
    MI_U32 u32VencPort;
#if (EN_PTZ_FEATURE == 1)
    MI_U32 u32Pipe; //the pipe bind moduel information.
#endif
    /* keep flag to identify stream started(1) or stop(0) */
    MI_U32 u32StrmSt;

    MI_U32 u32MaxFps;
    char *pszStreamName;
    MI_BOOL bForceIdr;
    Stream_Params_t stream_params;
    MI_SYS_ChnPort_t dstChnPort;
};

typedef struct uvc_app_para
{
    MI_U32 uvc_idx;

    CamOsThread videoIntfThread;
    CamOsMsgQueue viMsgQueue;
    u32 viMsgQueueTimeout;
#if defined(CONFIG_USB_GADGET_UVC_INTERRUPT_EP_SUPPORT)
    CamOsThread videoCtrlThread;
    CamOsTsem_t videoCtrlSem;
    u32 vcSemWaitTimeout;
    u8 vcThreadShouldStop;
#endif
    MI_U8 stream_init;
    MI_U8 uvc_status;
    MI_U8 flag_resume;

    struct ST_Uvc_Attr_T *attr;
    MI_U64 reqIdr_cnt;

    void *puvc_frm_buf;
    MI_PHY uvc_frm_dma;

#if defined(CONFIG_CUS3A_SUPPORT)
    CUS3A_ALGO_STATUS_t cus_3a_status;
#endif

    ST_USBUVC_IMPL_PTZ_t ptz_impl_handle;
    PCAM_USB_ZOOM zoom_cur;
    PCAM_USB_PANTILT pan_tilt_cur;

#if (VID_LATENCY_MEASURE)
    UVC_LATENCY_MEASUREMENT_t uvc_latency_para;
#endif
} ST_UVC_ARGS;
enum msg_type
{
    UVC_STOP_CAPTURE,
    UVC_START_CAPTURE,
    UVC_SUSPEND,
};
struct uvc_app_msg
{
    enum msg_type type;
    CamOsTsem_t *pblocking_call_sem;
};

static struct uvc_app_para guvc_app_para[MAX_VS_IF];
static MI_U8 g_maxbuf_cnt = 3;
//static MI_U32 g_device_num = MAX_VS_IF;
static struct ST_Uvc_Attr_T g_uvc_attr[] = {
    [USB_CAMERA0_INDEX] =
        {
            .u32SclDev = 0,
            .u32SclChn = 0,
            .u32SclPort = 1,
            .u32VencDev = MI_VENC_DEV_ID_H264_H265_0,
            .u32VencChn = 1,
            .u32VencPort = 0,
            .u32StrmSt   = FALSE,
            .pszStreamName = UVC_STREAM0,
            .bForceIdr = FALSE,
            .u32MaxFps = 30,
        },
	[USB_CAMERA1_INDEX] =
        {
            .u32SclDev = 0,
            .u32SclChn = 1,
            .u32SclPort = 1,
            .u32VencDev = MI_VENC_DEV_ID_H264_H265_0,
            .u32VencChn = 2,
            .u32VencPort = 0,
            .u32StrmSt   = FALSE,
            .pszStreamName = UVC_STREAM1,
            .bForceIdr = FALSE,
            .u32MaxFps = 30,
        },
};

static inline void composite_thread_wakeup(void)
{
    CamOsTsemUp(&composite_sem);
}
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UVC_FUNC)||defined(CONFIG_USB_COMPOSITE_DEV_HAS_UAC_FUNC)
#include "pCam_task.h"
#include "usb_class_ac_vc.h"

static CamOsThread pcam_thread;

static s8 pcam_app_init(void)
{
    CamOsThreadAttrb_t task_attr = { .nPriority = TASK_PCAM_PRIORITY,
                                     .szName = TASK_PCAM_NAME,
                                     .nStackSize = TASK_PCAM_STACK_SIZE};
    RET_VAL_ON(CamOsThreadCreate(&pcam_thread, &task_attr, pCam_Task, NULL) != CAM_OS_OK, -1);
    return 0;
}

static s8 pcam_app_deinit(void)
{
    pCam_Task_Stop();
    RET_VAL_ON(CamOsThreadStop(pcam_thread) != CAM_OS_OK, -1);
    return 0;
}

static u8 composite_is_pcam(struct usb_composite_info *info)
{
    u8 enable = 0;

#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UVC_FUNC)
    enable += info->uvc_enable[0];
#endif
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UAC_FUNC)
    enable += info->uac_enable;
#endif
    return enable;
}
#endif
static s8 uvc_app_msg_enqueue(CamOsMsgQueue uvc_app_mq, enum msg_type type, u32 timeout, u8 blocking_call,
                              u32 blocking_timeout)
{
    struct uvc_app_msg *pmsg;
    CamOsTsem_t blocking_call_sem = {0};

    pmsg = CamOsMemAlloc(sizeof(*pmsg));
    RET_VAL_ON(pmsg == NULL, -1);
    memset((void *)pmsg, 0x0, sizeof(*pmsg));
    pmsg->type = type;
    if (blocking_call)
    {
        if (CAM_OS_OK != CamOsTsemInit(&blocking_call_sem, 0))
        {
            CamOsMemRelease(pmsg);
            CamOsPrintf(KERN_ERR"uvc app sem fail! type:%d\n", type);
            return -1;
        }
        pmsg->pblocking_call_sem = &blocking_call_sem;
    }

    if (CAM_OS_OK != CamOsMsgQueueEnqueue(uvc_app_mq, pmsg, timeout))
    {
        CamOsMemRelease(pmsg);
        CamOsPrintf(KERN_ERR"uvc app mq enq fail! type:%d\n", type);
        return -1;
    }

    if (blocking_call)
    {
        CamOsRet_e ret = CAM_OS_OK;
        do {
            ret = CamOsTsemTimedDown(&blocking_call_sem, blocking_timeout);
            if (ret == CAM_OS_TIMEOUT) {
                CamOsPrintf(KERN_ERR"uvc app msg %d timeout!\n", type);
            }
        } while(ret == CAM_OS_TIMEOUT);
        RET_VAL_ON(CAM_OS_OK != CamOsTsemDeinit(&blocking_call_sem), -1);
    }

    return 0;
}

static MI_U32 uvc_app_video_buffer_allocate(struct uvc_app_para *puvc_para)
{
    MI_S32 s32Ret          = -1;
    MI_U32 bufSizeRequired = 0;
    u32    xfer_sz, hdr_sz;

    RET_VAL_ON(puvc_para == NULL, 0);
    RET_VAL_ON(puvc_para->attr->stream_params.maxframesize == 0, 0);

    xfer_sz = sstar_usbd_uvc_bytes_per_intvl(puvc_para->uvc_idx);
    hdr_sz  = sizeof(struct uvc_payload_header);
#if defined(CONFIG_UVC_HEADER_METADATA)
    if (puvc_para->uvc_idx == 1)
    {
        hdr_sz += sstar_usbd_uvc_get_ms_metadata_hdr_size();
    }
#endif
    bufSizeRequired =
        ALIGN_UP(puvc_para->attr->stream_params.maxframesize +
               ((puvc_para->attr->stream_params.maxframesize / (xfer_sz - hdr_sz) + 1) * hdr_sz),
                 1024);

    switch (puvc_para->attr->stream_params.fcc)
    {
    case V4L2_PIX_FMT_YUYV:
        // NOP
        break;

    case V4L2_PIX_FMT_NV12:
        // NOP
        break;

    case V4L2_PIX_FMT_MJPG:
    case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_H265:
        /* memory alloc */
        s32Ret = MI_SYS_MMA_Alloc(0, NULL, bufSizeRequired, &(puvc_para->uvc_frm_dma));
        RET_VAL_ON(s32Ret != MI_SUCCESS, 0);
        s32Ret = MI_SYS_Mmap(puvc_para->uvc_frm_dma, bufSizeRequired, &(puvc_para->puvc_frm_buf), TRUE);
        if (s32Ret != MI_SUCCESS)
        {
            MI_SYS_MMA_Free(0, puvc_para->uvc_frm_dma);
            CamOsPrintf("MI_SYS_Mmap failed !!!!\r\n");
        }
        break;

    default:
        break;
    }

    return bufSizeRequired;
}

static void uvc_app_video_buffer_free(struct uvc_app_para *puvc_para, MI_U32 bufSizeRequired)
{
    MI_S32 s32Ret = -1;

    switch (puvc_para->attr->stream_params.fcc)
    {
    case V4L2_PIX_FMT_YUYV:
        // NOP
        break;

    case V4L2_PIX_FMT_NV12:
        // NOP
        break;

    case V4L2_PIX_FMT_MJPG:
    case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_H265:
        s32Ret = MI_SYS_Munmap(puvc_para->puvc_frm_buf, bufSizeRequired);
        RET_ON(s32Ret != MI_SUCCESS);
        s32Ret = MI_SYS_MMA_Free(0, puvc_para->uvc_frm_dma);
        RET_ON(s32Ret != MI_SUCCESS);
        break;

    default:
        break;
    }
}

void ST_DefaultArgs(void)
{
    MI_U32 idx;
    ST_UVC_ARGS *args;
    for (idx = 0; idx < MAX_VS_IF; idx++)
    {
        args = &guvc_app_para[idx];
        memset(args, 0, sizeof(ST_UVC_ARGS));

        args->attr = &g_uvc_attr[idx];
        args->uvc_idx = idx;

        args->viMsgQueueTimeout = 1;
        args->uvc_status = UVC_STOP_CAPTURE;
        args->stream_init = 0;

        args->ptz_impl_handle.wCurZSNum[0] = 100;
        args->ptz_impl_handle.wCurZSNum[1] = 100;
        args->ptz_impl_handle.wCurZSNum[2] = 100;
        args->ptz_impl_handle.wCurZSDenom[0] = 100;
        args->ptz_impl_handle.wCurZSDenom[1] = 100;
        args->ptz_impl_handle.wCurZSDenom[2] = 100;
#if defined(CONFIG_CUS3A_SUPPORT)
        args->cus_3a_status.Ae = E_ALGO_STATUS_UNINIT;
#endif
    }
}

#if (VID_LATENCY_MEASURE)
void usb_vs_set_latency_measurement_tx_enable(u8 latency_measurement_enable)
{
    guvc_latency_measurement_enable = latency_measurement_enable;
}

u8 usb_vs_get_latency_measurement_tx_enable(void)
{
    return guvc_latency_measurement_enable;
}

void usb_vs_set_latency_measurement_tx_start(unsigned int vid_idx, u8 latency_measurement_start)
{
    guvc_app_para[vid_idx].uvc_latency_para.uvc_latency_measurement_start = latency_measurement_start;
}

u8 usb_vs_get_latency_measurement_tx_start(unsigned int vid_idx)
{
    return guvc_app_para[vid_idx].uvc_latency_para.uvc_latency_measurement_start;
}

u32 usb_vs_get_average_frame_time_interval(unsigned int vid_idx)
{
    return guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_time_interval;
}

void usb_vs_set_one_frame_time_tx_start(unsigned int vid_idx)
{
    if ((!usb_vs_get_latency_measurement_tx_enable()) || (!usb_vs_get_latency_measurement_tx_start(vid_idx)))
    {
        return;
    }
    else
    {
        u32 uvc_current_frame_time_interval_diff = 0; // unit is us.
        CamOsTimespec_t uvc_previous_frame_time_tx = {0};

        uvc_previous_frame_time_tx = guvc_app_para[vid_idx].uvc_latency_para.uvc_current_frame_time_tx_start;
        CamOsGetMonotonicTime(&(guvc_app_para[vid_idx].uvc_latency_para.uvc_current_frame_time_tx_start));

        if ((uvc_previous_frame_time_tx.nNanoSec != 0) && (uvc_previous_frame_time_tx.nSec != 0))
        {
            uvc_current_frame_time_interval_diff = UVC_VS_CAMOS_TIME_RES_DIFF_US(
                (&uvc_previous_frame_time_tx),
                (&(guvc_app_para[vid_idx].uvc_latency_para.uvc_current_frame_time_tx_start)), u32);
            guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_time_interval =
                (guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_time_interval == 0)
                    ? uvc_current_frame_time_interval_diff
                    : (((guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_time_interval * ((1 << 3) - 1)) +
                        uvc_current_frame_time_interval_diff) >>
                       3);
        }
    }
}

void usb_vs_set_one_frame_time_tx_end(unsigned int vid_idx)
{
    if ((!usb_vs_get_latency_measurement_tx_enable()) || (!usb_vs_get_latency_measurement_tx_start(vid_idx)))
    {
        return;
    }
    else
    {
        u32 uvc_current_frame_time_tx_diff = 0; // unit is us.
        CamOsTimespec_t tRes;

        CamOsGetMonotonicTime(&tRes);
        uvc_current_frame_time_tx_diff = UVC_VS_CAMOS_TIME_RES_DIFF_US(
            (&(guvc_app_para[vid_idx].uvc_latency_para.uvc_current_frame_time_tx_start)), (&tRes), u32);
        guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_time_tx =
            (guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_time_tx == 0)
                ? uvc_current_frame_time_tx_diff
                : (((guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_time_tx * ((1 << 3) - 1)) +
                    uvc_current_frame_time_tx_diff) >>
                   3);
    }
}

u32 usb_vs_get_one_frame_time_tx(unsigned int vid_idx)
{
    return guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_time_tx;
}

void usb_vs_set_one_frame_size_tx(unsigned int vid_idx, u32 frame_size)
{
    if ((!usb_vs_get_latency_measurement_tx_enable()) || (!usb_vs_get_latency_measurement_tx_start(vid_idx)))
    {
        return;
    }
    else
    {
        guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_size_tx =
            (guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_size_tx == 0)
                ? (frame_size >> 10)
                : (((guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_size_tx * ((1 << 3) - 1)) +
                    (frame_size >> 10)) >>
                   3);
    }
}

u32 usb_vs_get_one_frame_size_tx(unsigned int vid_idx)
{
    return guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_size_tx;
}

void usb_vs_reset_latency_measurement(unsigned int vid_idx)
{
    memset(&guvc_app_para[vid_idx].uvc_latency_para.uvc_current_frame_time_tx_start, 0x0,
           sizeof(guvc_app_para[vid_idx].uvc_latency_para.uvc_current_frame_time_tx_start));
    guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_time_tx = 0;
    guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_size_tx = 0;
    guvc_app_para[vid_idx].uvc_latency_para.uvc_average_frame_time_interval = 0;
    usb_vs_set_latency_measurement_tx_start(vid_idx, 0);
}

void usb_vs_set_stream_data_pts_average(u32 strm_no, u32 u32pts_cur_diff)
{
    guvc_app_para[strm_no].uvc_latency_para.uvc_stream_pts_average_diff =
        (guvc_app_para[strm_no].uvc_latency_para.uvc_stream_pts_average_diff == 0)
            ? u32pts_cur_diff
            : (((guvc_app_para[strm_no].uvc_latency_para.uvc_stream_pts_average_diff * ((1 << 3) - 1)) +
                u32pts_cur_diff) >>
               3);
}

u32 usb_vs_get_stream_data_pts_average(u32 strm_no)
{
    return guvc_app_para[strm_no].uvc_latency_para.uvc_stream_pts_average_diff;
}

void usb_vs_set_stream_data_pts(u32 strm_no, MI_U64 u64Pts)
{
    MI_U64 u64pts_cur = 0;
    u32 u32pts_cur_diff;

    mi_sys_GetCurPts(&u64pts_cur);
    u32pts_cur_diff = (u32)(u64pts_cur - u64Pts);
    usb_vs_set_stream_data_pts_average(strm_no, u32pts_cur_diff);
}

void usb_vs_set_stream_data_pts_mjpeg_realtime(u32 strm_no, MI_U64 u64Pts)
{
    u32 u32pts_cur_diff;
    CamOsTimespec_t ts;

    CamOsGetMonotonicTime(&ts);
    u32pts_cur_diff = (u32)((ts.nSec * 1000000ULL + ts.nNanoSec / 1000) - u64Pts);
    usb_vs_set_stream_data_pts_average(strm_no, u32pts_cur_diff);
}
#endif

static MI_U32 ST_UVC_GetFramePreRdy(MI_U16 width, MI_U16 height)
{
    MI_U32 entries = 0;
    MI_U32 i;
    MI_U32 line = height;
    struct frm_prerdy *ratio;

    if (udc_speed >= USB_SPEED_SUPER) {
        entries = sizeof(prerdy_ratio_ss) / sizeof(struct frm_prerdy);
        ratio = prerdy_ratio_ss;
    }
    else {
        entries = sizeof(prerdy_ratio_hs) / sizeof(struct frm_prerdy);
        ratio = prerdy_ratio_hs;
    }
    for(i = 0; i < entries; i++) {
        if (height < ratio[i].height) {
            line = (height * ratio[i].num) / ratio[i].dem;
            break;
        }
    }
    CamOsPrintf(KERN_DEBUG"Speed %d Entries %d FramePreRdyLine: %d\n", udc_speed, entries, line);
    return line;
}

static int32_t UVC_MM_FillBuffer(MI_U32 uvc_idx, Stream_Params_t params, MI_U32 vsize)
{
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U32 u32Size = 0, i = 0;
#if defined(CONFIG_FD3A_IN_RTOS_ENABLE)
    MI_U32 idx = 0, u32ValidFDNum = 0;
#endif
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE stBufHandle;
    MI_SYS_ChnPort_t dstChnPort;
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t stPack[4];
    MI_VENC_ChnStat_t stStat;
    u32 MaxBufSize = (params.width * params.height) * 3; // use max 829440
    u32 bufSizeRequired = 0;
    MI_U32 size_all = 0;
    MI_PHY frame_dma;
    void *frame_buf;
    ST_UVC_ARGS *puvc_para;
    MI_U32 VencChnId;
    MI_U32 VencDevId;
#if defined(CONFIG_UVC_HEADER_METADATA)
    struct uvc_metadata_frame_info mdata = {0};
#endif

    puvc_para = &guvc_app_para[uvc_idx];

    dstChnPort = puvc_para->attr->dstChnPort;
    VencChnId = puvc_para->attr->u32VencChn;

    sstar_usbd_uvc_set_pts(uvc_idx);
    switch (params.fcc)
    {
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_NV12:
        memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
        memset(&stBufHandle, 0, sizeof(MI_SYS_BUF_HANDLE));

        s32Ret = MI_SYS_ChnOutputPortGetBuf(&dstChnPort, &stBufInfo, &stBufHandle);
        if (MI_SUCCESS != s32Ret)
            return -1;

        frame_dma = stBufInfo.stFrameData.phyAddr[0];
        frame_buf = stBufInfo.stFrameData.pVirAddr[0];
        u32Size = stBufInfo.stFrameData.u32BufSize;
        if (u32Size > vsize)
        {
            CamOsPrintf(KERN_ERR"YUYV/NV12 Get video size err:%d,%d\n", u32Size, vsize);
            s32Ret = MI_SYS_ChnOutputPortPutBuf(stBufHandle);
            RET_VAL_ON(MI_SUCCESS != s32Ret, -1);
            return -1;
        }

#if (VID_LATENCY_MEASURE)
        usb_vs_set_stream_data_pts(uvc_idx, stBufInfo.u64Pts);
        if (/*(stBufInfo.u32SequenceNumber < 60) && */ (stBufInfo.u32SequenceNumber > 30))
        {
            usb_vs_set_latency_measurement_tx_start(uvc_idx, 1);
        }
        usb_vs_set_one_frame_size_tx(uvc_idx, stBufInfo.stFrameData.u32BufSize);
        usb_vs_set_one_frame_time_tx_start(uvc_idx);
#endif

#if defined(CONFIG_UVC_HEADER_METADATA)
        mdata.ir_flag = (u32)stBufInfo.u32IrFlag;
        sstar_usbd_uvc_set_metadata(uvc_idx, &mdata);
#endif
        if (puvc_para->uvc_status != UVC_STOP_CAPTURE)
        {
            sstar_usbd_uvc_send_frame(uvc_idx, frame_buf, frame_dma, u32Size);
        }

#if (VID_LATENCY_MEASURE)
        usb_vs_set_one_frame_time_tx_end(uvc_idx);
#endif
        s32Ret = MI_SYS_ChnOutputPortPutBuf(stBufHandle);
        RET_VAL_ON(MI_SUCCESS != s32Ret, -1);
        break;

    case V4L2_PIX_FMT_MJPG:
    case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_H265:
        VencDevId = (params.fcc == V4L2_PIX_FMT_MJPG) ? MI_VENC_DEV_ID_JPEG_0 : MI_VENC_DEV_ID_H264_H265_0;
        memset(&stStream, 0, sizeof(MI_VENC_Stream_t));
        memset(&stPack, 0, sizeof(MI_VENC_Pack_t) * 4);
        stStream.pstPack = stPack;

        s32Ret = MI_VENC_Query(VencDevId, VencChnId, &stStat);
        if (s32Ret != MI_SUCCESS || stStat.u32CurPacks == 0)
        {
            // CamOsPrintf("venc query failed,s32Ret:%x \n", s32Ret );
            // CamOsMemRelease(pBufVirt); //???
            return -1;
        }

        stStream.u32PackCount = stStat.u32CurPacks;

        s32Ret = MI_VENC_GetStream(VencDevId, VencChnId, &stStream, 40);
        RET_VAL_ON(MI_SUCCESS != s32Ret, -1);
#if (VID_LATENCY_MEASURE)
        if ((params.fcc == V4L2_PIX_FMT_MJPG) && 0 /*(gu32mjpg_realtime_mode)*/)
        {
            usb_vs_set_stream_data_pts_mjpeg_realtime(uvc_idx /*stream number*/, stStream.pstPack->u64PTS);
        }
        else
        {
            usb_vs_set_stream_data_pts(uvc_idx /*stream number*/, stStream.pstPack->u64PTS);
        }
#endif
        for (i = 0; i < stStat.u32CurPacks; i++)
            bufSizeRequired += stStream.pstPack[i].u32Len;
        if (bufSizeRequired > vsize)
        {
            CamOsPrintf("size exceed max "
                        "!!!!,size_max:%d,realbufSize:%d,return\r\n",
                        MaxBufSize, bufSizeRequired);
            MI_VENC_ReleaseStream(VencDevId, VencChnId, &stStream);
            return -1;
        }

        if (stStat.u32CurPacks > 1)
        {
            frame_buf = puvc_para->puvc_frm_buf;
            frame_dma = puvc_para->uvc_frm_dma;

            for (i = 0; i < stStat.u32CurPacks; i++)
            {
                u32Size = stStream.pstPack[i].u32Len;
                memcpy(frame_buf + size_all, stStream.pstPack[i].pu8Addr, u32Size);
                size_all += u32Size;
            }
        }
        else
        {
            size_all = stStream.pstPack[0].u32Len;
            frame_buf = stStream.pstPack[0].pu8Addr;
            frame_dma = stStream.pstPack[0].phyAddr;
        }

#if (VID_LATENCY_MEASURE)
        if (/*(stStream.u32Seq < 60) && */ (stStream.u32Seq > 30))
        {
            usb_vs_set_latency_measurement_tx_start(uvc_idx, 1);
        }

        usb_vs_set_one_frame_size_tx(uvc_idx, size_all);
        usb_vs_set_one_frame_time_tx_start(uvc_idx);
#endif
        sstar_usbd_uvc_send_frame(uvc_idx, frame_buf, frame_dma, size_all);

#if (VID_LATENCY_MEASURE)
        usb_vs_set_one_frame_time_tx_end(uvc_idx);
#endif
        s32Ret = MI_VENC_ReleaseStream(VencDevId, VencChnId, &stStream);
        if (MI_SUCCESS != s32Ret)
            CamOsPrintf(KERN_ERR"%s Release Frame Failed\n", __func__);

        if (params.fcc != V4L2_PIX_FMT_MJPG && puvc_para->reqIdr_cnt++ < 5)
        {
            MI_VENC_RequestIdr(VencDevId, VencChnId, TRUE);
        }
        break;
    default:
        CamOsPrintf(KERN_ERR"unknown format %d\n", params.fcc);
        return -1;
    }
    return MI_SUCCESS;
}

MI_S32 UVC_StartCapture(Stream_Params_t params, MI_U32 uvc_idx)
{
    MI_S32 s32Ret = -1;
    /************************************************
    Step0:  Init General Param
    *************************************************/
    MI_U32 fcc = params.fcc;
    MI_U32 u32Width = params.width;
    MI_U32 u32Height = params.height;
    MI_U32 u32FrameRate = params.frameRate;
    struct ST_Uvc_Attr_T *pstStreamAttr = guvc_app_para[uvc_idx].attr;
    MI_SYS_ChnPort_t *dstChnPort = &pstStreamAttr->dstChnPort;

    /************************************************
    Step1:  Init Scl Output Param
    *************************************************/
    MI_SCL_DEV SclDevId = (MI_SCL_DEV)pstStreamAttr->u32SclDev;
    MI_SCL_CHANNEL SclChnId = (MI_SCL_CHANNEL)pstStreamAttr->u32SclChn;
    MI_SCL_PORT SclPortId = (MI_SCL_PORT)pstStreamAttr->u32SclPort;
    MI_SCL_OutPortParam_t stSclOutputParam;
    memset(&stSclOutputParam, 0x0, sizeof(MI_SCL_OutPortParam_t));
    stSclOutputParam.stSCLOutputSize.u16Width = u32Width;
    stSclOutputParam.stSCLOutputSize.u16Height = u32Height;
    stSclOutputParam.bMirror = FALSE;
    stSclOutputParam.bFlip = FALSE;
    stSclOutputParam.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;

    /************************************************
    Step2:  Init Venc Param
    *************************************************/
    MI_VENC_DEV VencDevId;
    MI_VENC_CHN VencChnId = (MI_VENC_CHN)pstStreamAttr->u32VencChn;

    MI_VENC_ChnAttr_t stVencChnAttr;
    memset(&stVencChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));

    MI_U32 u32VenBitRate;
    MI_U32 u32VenQfactor = 80;
    bool bByFrame = TRUE;

    /************************************************
    Step3:  Init Bind Param
    *************************************************/
    ST_Sys_BindInfo_T stBindInfo;
    MI_SYS_ChnPort_t stChnPort;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
    stBindInfo.stSrcChnPort.u32DevId = SclDevId;
    stBindInfo.stSrcChnPort.u32ChnId = SclChnId;
    stBindInfo.stSrcChnPort.u32PortId = SclPortId;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32ChnId = VencChnId;
    stBindInfo.u32SrcFrmrate = u32FrameRate;
    stBindInfo.u32DstFrmrate = u32FrameRate;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    
    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = E_MI_MODULE_ID_SCL;
    stChnPort.u32DevId = SclDevId;
    stChnPort.u32ChnId = SclChnId;
    stChnPort.u32PortId = SclPortId;

    /************************************************
    Step4:  Init User Param
    *************************************************/
    if (u32Width * u32Height > 2560 * 1440)
    {
        u32VenBitRate = 1024 * 1024 * 8;
    }
    else if (u32Width * u32Height >= 1920 * 1080)
    {
        u32VenBitRate = 1024 * 1024 * 4;
    }
    else if (u32Width * u32Height < 640 * 480)
    {
        u32VenBitRate = 1024 * 500;
    }
    else
    {
        u32VenBitRate = 1024 * 1024 * 2;
    }
    /************************************************
    Step4:  Start
    *************************************************/
    switch (fcc)
    {
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_NV12:
        stSclOutputParam.ePixelFormat =
            (fcc == V4L2_PIX_FMT_YUYV) ? E_MI_SYS_PIXEL_FRAME_YUV422_YUYV : E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

        STCHECKRESULT(MI_SCL_SetOutputPortParam(SclDevId, SclChnId, SclPortId, &stSclOutputParam));

        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 2, 4));
        STCHECKRESULT(MI_SCL_EnableOutputPort(SclDevId, SclChnId, SclPortId));
        if (uvc_idx == 0) {
            STCHECKRESULT(MI_SYS_EnableChnOutputPortLowLatency(0, &stChnPort, TRUE,
                                                               ST_UVC_GetFramePreRdy(u32Width, u32Height)));
        }
        else {
            STCHECKRESULT(MI_SYS_EnableChnOutputPortLowLatency(0, &stChnPort, FALSE, u32Height));
        }
        STCHECKRESULT(MI_SYS_SetChnOutputPortUserFrc(&stChnPort, pstStreamAttr->u32MaxFps, u32FrameRate));
        *dstChnPort = stBindInfo.stSrcChnPort;
#if (EN_PTZ_FEATURE == 1)
        pstStreamAttr->u32Pipe = VIDEO_PIPE_SCL;
#endif
        break;

    case V4L2_PIX_FMT_MJPG:
    case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_H265:
        if (fcc == V4L2_PIX_FMT_MJPG)
        {
            stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
        }
        else
        {
            stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        }

        STCHECKRESULT(MI_SCL_SetOutputPortParam(SclDevId, SclChnId, SclPortId, &stSclOutputParam));

        VencDevId = (fcc == V4L2_PIX_FMT_MJPG) ? MI_VENC_DEV_ID_JPEG_0 : MI_VENC_DEV_ID_H264_H265_0;
        stBindInfo.stDstChnPort.u32DevId = VencDevId;

        if (fcc == V4L2_PIX_FMT_MJPG)
        {
            stVencChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = u32Width;
            stVencChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = u32Height;
            stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = u32Width;
            stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = u32Height;
            stVencChnAttr.stVeAttr.stAttrJpeg.bByFrame = bByFrame;
            stVencChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
            stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            stVencChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor = u32VenQfactor;
            stVencChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum = u32FrameRate;
            stVencChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = 1;
        }
        else if (fcc == V4L2_PIX_FMT_H264)
        {
            stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth = u32Width;
            stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight = u32Height;
            stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = u32Width;
            stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = u32Height;
            stVencChnAttr.stVeAttr.stAttrH264e.bByFrame = bByFrame;
            stVencChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2;
            stVencChnAttr.stVeAttr.stAttrH264e.u32Profile = 1;
            stVencChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
            stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = u32VenBitRate;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = u32FrameRate;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = 30;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;
        }
        else if (fcc == V4L2_PIX_FMT_H265)
        {
            stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth = u32Width;
            stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight = u32Height;
            stVencChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = u32Width;
            stVencChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = u32Height;
            stVencChnAttr.stVeAttr.stAttrH265e.bByFrame = bByFrame;
            stVencChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H265E;
            stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = u32VenBitRate;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = u32FrameRate;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = 30;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;
        }

        STCHECKRESULT(MI_VENC_CreateChn(VencDevId, VencChnId, &stVencChnAttr));
        if (fcc == V4L2_PIX_FMT_H264)
        {
            MI_VENC_ParamH264SliceSplit_t stSliceSplit = {TRUE, ALIGN_UP(ALIGN_UP(u32Height, MI_AVC_HW_ENC_UNIT) / MI_AVC_HW_ENC_UNIT, 2)};
            STCHECKRESULT(MI_VENC_SetH264SliceSplit(VencDevId, VencChnId, &stSliceSplit));
        }
        else if (fcc == V4L2_PIX_FMT_H265)
        {
            MI_VENC_ParamH265SliceSplit_t stSliceSplit = {TRUE, ALIGN_UP(ALIGN_UP(u32Height, MI_HEVC_HW_ENC_SIZE) / MI_HEVC_HW_ENC_SIZE, 2)};
            STCHECKRESULT(MI_VENC_SetH265SliceSplit(VencDevId, VencChnId, &stSliceSplit));
        }
        STCHECKRESULT(MI_VENC_SetMaxStreamCnt(VencDevId, VencChnId, g_maxbuf_cnt + 1));
        STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 0, g_maxbuf_cnt + 1));
        STCHECKRESULT(MI_SCL_EnableOutputPort(SclDevId, SclChnId, SclPortId));
        STCHECKRESULT(MI_SYS_EnableChnOutputPortLowLatency(0, &stChnPort, FALSE, u32Height));
        STCHECKRESULT(MI_VENC_StartRecvPic(VencDevId, VencChnId));
        *dstChnPort = stBindInfo.stDstChnPort;
#if (EN_PTZ_FEATURE == 1)
        pstStreamAttr->u32Pipe = VIDEO_PIPE_SCL_VENC;
#endif
        break;

    default:
        return s32Ret;
    }
    CamOsPrintf(KERN_INFO GRN_BOLD"Capture u32Width: %d, u32height: %d, format: %s\n"COLOR_NONE, u32Width, u32Height,
                    fcc == V4L2_PIX_FMT_YUYV ? "YUYV"
                        : (fcc == V4L2_PIX_FMT_NV12 ? "NV12"
                               : (fcc == V4L2_PIX_FMT_MJPG ? "MJPEG"
                                      : (fcc == V4L2_PIX_FMT_H264 ? "H264" : "H265"))));

    return MI_SUCCESS;
}

MI_S32 UVC_StopCapture(Stream_Params_t params, MI_U32 uvc_idx)
{
    MI_S32 s32Ret = -1;
    /************************************************
    Step0:  Init General Param
    *************************************************/
    MI_U32 fcc = params.fcc;
    MI_U32 u32Width = params.width;
    MI_U32 u32Height = params.height;
    struct ST_Uvc_Attr_T *pstStreamAttr = guvc_app_para[uvc_idx].attr;
    MI_SCL_DEV SclDevId = (MI_SCL_DEV)pstStreamAttr->u32SclDev;
    MI_SCL_CHANNEL SclChnId = (MI_SCL_CHANNEL)pstStreamAttr->u32SclChn;
    MI_SCL_PORT SclPortId = (MI_SCL_PORT)pstStreamAttr->u32SclPort;

    MI_VENC_DEV VencDevId;
    MI_VENC_CHN VencChnId = (MI_VENC_CHN)pstStreamAttr->u32VencChn;

    /************************************************
    Step0:  General Param Set
    *************************************************/
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
    stBindInfo.stSrcChnPort.u32DevId = SclDevId;
    stBindInfo.stSrcChnPort.u32ChnId = SclChnId;
    stBindInfo.stSrcChnPort.u32PortId = SclPortId;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stBindInfo.stDstChnPort.u32ChnId = VencChnId;
    stBindInfo.u32SrcFrmrate = params.frameRate;
    stBindInfo.u32DstFrmrate = params.frameRate;
    
    /************************************************
    Step1:  Stop Port And Unbind
    *************************************************/
    switch (params.fcc)
    {
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_NV12:
        STCHECKRESULT(MI_SCL_DisableOutputPort(SclDevId, SclChnId, SclPortId));
        break;

    case V4L2_PIX_FMT_MJPG:
    case V4L2_PIX_FMT_H264:
    case V4L2_PIX_FMT_H265:
        VencDevId = (params.fcc == V4L2_PIX_FMT_MJPG) ? MI_VENC_DEV_ID_JPEG_0 : MI_VENC_DEV_ID_H264_H265_0;
        stBindInfo.stDstChnPort.u32DevId = VencDevId;
        STCHECKRESULT(MI_VENC_StopRecvPic(VencDevId, VencChnId));
        STCHECKRESULT(MI_SCL_DisableOutputPort(SclDevId, SclChnId, SclPortId));
        STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
        STCHECKRESULT(MI_VENC_DestroyChn(VencDevId, VencChnId));
        break;

    default:
        return s32Ret;
    }
    CamOsPrintf(KERN_INFO GRN_BOLD"UVC_StopCapture u32Width: %d, u32height: %d, format: %s\n"COLOR_NONE, u32Width, u32Height,
                    fcc == V4L2_PIX_FMT_YUYV ? "YUYV"
                        : (fcc == V4L2_PIX_FMT_NV12 ? "NV12"
                               : (fcc == V4L2_PIX_FMT_MJPG ? "MJPEG"
                                      : (fcc == V4L2_PIX_FMT_H264 ? "H264" : "H265"))));

    return MI_SUCCESS;
}

void *UVC_Video_Process_Task(void *data)
{
    MI_S32 s32Ret = -1;
    MI_U32 bufSizeRequired = 0;
    struct uvc_app_msg *pmsg = NULL;
    char __attribute__((unused)) strFlag = 0;
    CamOsTimespec_t tTvStart = {0}, tTvEnd = {0};
    CamOsTsem_t *pblocking_call_sem_received = NULL;

    ST_UVC_ARGS *pArgs = (ST_UVC_ARGS *)data;

    while (CAM_OS_OK != CamOsThreadShouldStop())
    {
        pblocking_call_sem_received = NULL;
        if (CamOsMsgQueueDequeue(pArgs->viMsgQueue, (void *)(&pmsg), pArgs->viMsgQueueTimeout) == CAM_OS_OK)
        {
            if (pmsg->type == UVC_SUSPEND)
            {
                CamOsPrintf(KERN_INFO"received msg %s\n", "UVC_SUSPEND");
                strFlag = 1;
                pArgs->flag_resume = 0;
                CamOsGetMonotonicTime(&tTvStart);
                CamOsGetMonotonicTime(&tTvEnd);
            }
            else
            {
                CamOsPrintf(KERN_INFO"vid:%d %s => %s\n", pArgs->uvc_idx,
                            pArgs->uvc_status == UVC_STOP_CAPTURE ? "STOP_CAPTURE" : "START_CAPTURE",
                            pmsg->type == UVC_STOP_CAPTURE ? "STOP_CAPTURE" : "START_CAPTURE");

                pArgs->uvc_status = (MI_U8)pmsg->type;
            }

            pblocking_call_sem_received = pmsg->pblocking_call_sem;
            CamOsMemRelease(pmsg);
        }

        if (pArgs->uvc_status == UVC_START_CAPTURE)
        {
            strFlag = 0;
            if (pArgs->stream_init == 0)
            {
#if (VID_LATENCY_MEASURE)
                usb_vs_reset_latency_measurement((unsigned int)pArgs->uvc_idx);
#endif
                CamOsPrintf(KERN_INFO GRN_BOLD"Webcam-UVC: start preview %d, "
                              "info(format:%c%c%c%c, "
                              "Width:%lu, High:%lu)\n"COLOR_NONE,
                              (u8)(pArgs->uvc_idx),
                              (u8)(pArgs->attr->stream_params.fcc), (u8)(pArgs->attr->stream_params.fcc >> 8),
                              (u8)(pArgs->attr->stream_params.fcc >> 16), (u8)(pArgs->attr->stream_params.fcc >> 24),
                              pArgs->attr->stream_params.width, pArgs->attr->stream_params.height);
                s32Ret = UVC_StartCapture(pArgs->attr->stream_params, pArgs->uvc_idx);
                if (s32Ret != MI_SUCCESS)
                {
                    CamOsPrintf(KERN_ERR"Webcam-UVC: start failed \n");
                    continue;
                }
                pArgs->stream_init = 1;

                bufSizeRequired = uvc_app_video_buffer_allocate(pArgs);

#if defined(CONFIG_CUS3A_SUPPORT)
                // NOP
#else
                PCAM_USB_SetAttrEnable(pArgs->uvc_idx, 1);
#endif
            }

#if defined(CONFIG_CUS3A_SUPPORT)
            if (pArgs->cus_3a_status.Ae != E_ALGO_STATUS_RUNNING)
            {
                CUS3A_GetAlgoStatus(0, pArgs->uvc_idx, &pArgs->cus_3a_status);
                if (pArgs->cus_3a_status.Ae == E_ALGO_STATUS_RUNNING)
                {
                    CamOsPrintf(KERN_INFO"Ae E_ALGO_STATUS_RUNNING!\r\n");
                    PCAM_USB_SetAttrEnable(pArgs->uvc_idx, 1);
                }
            }
#endif
            if (pblocking_call_sem_received != NULL)
            {
                CamOsTsemUp(pblocking_call_sem_received);
                pblocking_call_sem_received = NULL;
            }

            pArgs->viMsgQueueTimeout =
                (MI_SUCCESS == UVC_MM_FillBuffer(pArgs->uvc_idx, pArgs->attr->stream_params, bufSizeRequired)) ? 0 : 1;
        }
        else
        {
            if (pArgs->stream_init == 1)
            {
                // int ret = 0;
                PCAM_USB_SetAttrEnable(pArgs->uvc_idx, 0);
#if defined(CONFIG_CUS3A_SUPPORT)
                pArgs->cus_3a_status.Ae = E_ALGO_STATUS_UNINIT;
                pArgs->cus_3a_status.Af = E_ALGO_STATUS_UNINIT;
                pArgs->cus_3a_status.Awb = E_ALGO_STATUS_UNINIT;
#endif
                s32Ret = UVC_StopCapture(pArgs->attr->stream_params, pArgs->uvc_idx);
                if (s32Ret != MI_SUCCESS)
                    CamOsPrintf(KERN_ERR"UVC_StopCapture failed \n");

                pArgs->stream_init = 0;
                uvc_app_video_buffer_free(pArgs, bufSizeRequired);
            }

            /* This is multi-sensor case. If it output multi-stream
            *  with the same sensor, de-init sensor when last stop capture.
            */

            pArgs->viMsgQueueTimeout = 1;
        }

        // CamOsUsSleep(1);
        if (pblocking_call_sem_received != NULL)
        {
            CamOsTsemUp(pblocking_call_sem_received);
        }
    }
    return NULL;
}

#if defined(CONFIG_USB_GADGET_UVC_INTERRUPT_EP_SUPPORT)
void *UVC_Video_Control_Task(void *data)
{
    u32 status_size = 0;

    ST_UVC_ARGS *pArgs;
    __attribute__((__aligned__(64))) struct uvc_status_packet status_pkt;
    CamOsRet_e eRet;

    pArgs = (ST_UVC_ARGS *)data;

    while (CAM_OS_OK != CamOsThreadShouldStop())
    {
        eRet = CamOsTsemTimedDown(&(pArgs->videoCtrlSem), pArgs->vcSemWaitTimeout);

        if (pArgs->vcThreadShouldStop != 0)
            break;

        if (eRet == CAM_OS_OK)
        {
            status_size = uvc_video_get_status_packet(pArgs->uvc_idx, &status_pkt);
            if (status_size > 0)
            {
                sstar_usbd_uvc_send_status(pArgs->uvc_idx, &status_pkt, status_size);
            }
        }
        else if (eRet == CAM_OS_TIMEOUT)
        {
            // NOP
        }
    }

    return NULL;
}
#endif
MI_S32 ST_UVC_SetSensrFpsEx(MI_U32 ep_id, MI_U32 u32FrmRateNum, MI_U32 u32FrmRateDenom)
{
    MI_S32 s32Ret = MI_SUCCESS;
    return s32Ret;
}
MI_S32 ST_UVC_CfgSNRHDRSetting(MI_U32 SensorId, MI_BOOL SnrHDREnable)
{
    MI_S32 s32Ret = MI_SUCCESS;
    return s32Ret;
}
MI_S32 ST_UVC_SetVideoMute(MI_U32 uvc_idx, MI_U32 enMute)
{
    MI_S32 s32Ret = MI_SUCCESS;
    return s32Ret;
}void usb_uvc_GetPrevwSts(u8 *pIsPreviewEnable)
{

}

MI_S32 ST_UVC_SwitchIQBinByChannel( MI_U32 u32Ch, MI_U8 u8Mod)
{
    MI_S32 s32Ret = MI_SUCCESS;
    return s32Ret;
}

void uvc_app_init(void)
{
    CamOsThreadAttrb_t viAttr[MAX_VS_IF];
    ST_UVC_ARGS *pArgs;
    u32 vid_idx = 0;

    ST_DefaultArgs();

    uvc_video_init();

    for (vid_idx = 0; vid_idx < MAX_VS_IF; ++vid_idx)
    {
        pArgs = &guvc_app_para[vid_idx];

        viAttr[vid_idx].nPriority = 97;
        viAttr[vid_idx].nStackSize = 16*1024;
        viAttr[vid_idx].szName = pArgs->attr->pszStreamName;
        RET_ON(CamOsMsgQueueCreate(&(pArgs->viMsgQueue), 10) != CAM_OS_OK);

        RET_ON(CamOsThreadCreate(&(pArgs->videoIntfThread), &viAttr[vid_idx], UVC_Video_Process_Task, pArgs) !=
                        CAM_OS_OK);

#if defined(CONFIG_USB_GADGET_UVC_INTERRUPT_EP_SUPPORT)
        CamOsThreadAttrb_t vcAttr[MAX_VS_IF];

        vcAttr[vid_idx].nPriority = 50;
        vcAttr[vid_idx].nStackSize = 6*1024;
        vcAttr[vid_idx].szName = pArgs->attr->pszStreamName;
        /* Initialize condition wait object*/
        pArgs->vcSemWaitTimeout = 1000;
        pArgs->vcThreadShouldStop = 0;
        CamOsTsemInit(&(pArgs->videoCtrlSem), 0);

        RET_ON(CamOsThreadCreate(&(pArgs->videoCtrlThread), &vcAttr[vid_idx], UVC_Video_Control_Task, pArgs) !=
                        CAM_OS_OK);
#endif
    }

}
void uvc_app_deinit(void)
{
    ST_UVC_ARGS *pArgs;
    u32 vid_idx = 0;

    for (vid_idx = 0; vid_idx < MAX_VS_IF; ++vid_idx)
    {
        pArgs = &guvc_app_para[vid_idx];
        RET_ON(CamOsThreadStop(pArgs->videoIntfThread) != CAM_OS_OK);
        RET_ON(CamOsMsgQueueDestroy(pArgs->viMsgQueue) != CAM_OS_OK);

#if defined(CONFIG_USB_GADGET_UVC_INTERRUPT_EP_SUPPORT)
        pArgs->vcThreadShouldStop = 1;
        CamOsTsemUp(&(pArgs->videoCtrlSem));
        RET_ON(CamOsThreadStop(pArgs->videoCtrlThread) != CAM_OS_OK);
        RET_ON(CamOsTsemDeinit(&(pArgs->videoCtrlSem)) != CAM_OS_OK);
#endif
    }

}
/* callback from usb driver */
static void uvc_app_stop_capture(u8 uvc_idx)
{
    ST_UVC_ARGS *pArgs;

    pArgs = &guvc_app_para[uvc_idx];
    RET_ON(0 != uvc_app_msg_enqueue(pArgs->viMsgQueue, UVC_STOP_CAPTURE, 1, 1, 5000));
    //Fixed: Preview would be failed after run CV TD9.4/TD9.9 and exit app.
    uvc_video_clear_stream_params(uvc_idx);
}

static s8 uvc_app_start_capture(u8 uvc_idx)
{
    Stream_Params_t  *pstream_params = NULL;
    ST_UVC_ARGS *pArgs;

    pArgs = &guvc_app_para[uvc_idx];
    uvc_video_get_stream_params(uvc_idx, &pArgs->attr->stream_params);
    pstream_params = &(pArgs->attr->stream_params);

    //CV tool sends set_alt without UVC probe/commit in advance. (Configured State:TD9.4 & TD9.9)
    RET_VAL_ON((pstream_params == NULL)
            || (pstream_params->width == 0)
            || (pstream_params->height == 0)
            || (pstream_params->frameRate == 0)
            || (pstream_params->maxframesize == 0), 1);

    RET_VAL_ON(0 != uvc_app_msg_enqueue(pArgs->viMsgQueue, UVC_START_CAPTURE, 1, 1, 1000), 1);
    return 0;
}

static void uvc_app_set_config(u8 config)
{
    return;
}

static void uvc_app_suspend(u8 idx)
{
    ST_UVC_ARGS *pArgs;

    pArgs = &guvc_app_para[idx];
    pArgs->flag_resume = 0;
    for (u32 loop = 0; loop < MAX_VS_IF; ++loop)
    {
        pArgs = &guvc_app_para[loop];
        if (pArgs->flag_resume)
        {
            return;
        }
    }
    RET_ON(0 != uvc_app_msg_enqueue(pArgs->viMsgQueue, UVC_SUSPEND, 10, 0, 0));
}

static void uvc_app_resume(u8 uvc_idx)
{
    ST_UVC_ARGS *pArgs;

    pArgs = &guvc_app_para[uvc_idx];
    pArgs->flag_resume = 1;
}

static void uvc_app_usb_reset(u8 uvc_idx)
{
    ST_UVC_ARGS *pArgs;

    pArgs = &guvc_app_para[uvc_idx];
    pArgs->flag_resume = 1;
}

static s8 uvc_app_process_vc_data(u8 idx, struct usb_request_data *data)
{
    s8 ret;

    ret = uvc_video_process_vc_data(idx, data);

#if defined(CONFIG_USB_GADGET_UVC_INTERRUPT_EP_SUPPORT)
    if (uvc_video_get_status_updated(idx) != 0)
        CamOsTsemUp(&(guvc_app_para[idx].videoCtrlSem));
#endif

    return ret;
}

static void uvc_app_speed_negotiate(u8 speed, u8 interface)
{
    udc_speed = speed;
    uvc_video_speed_negotiate(speed, interface);
}

static void class_app_init(struct usb_composite_info *info)
{
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UVC_FUNC)||defined(CONFIG_USB_COMPOSITE_DEV_HAS_UAC_FUNC)
    if (composite_is_pcam(info))
    {
        usb_class_ac_vc_app_init();
        pcam_app_init();
    }
#endif

#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UVC_FUNC)
    if (info->uvc_enable[0])
    {
        uvc_app_init();
    }
#endif
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UAC_FUNC)
    if (info->uac_enable)
    {
        uac_app_init();
    }
#endif
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_HID_FUNC)
    if (info->hid_enable)
    {
        hid_app_init();
    }
#endif
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_CDC_FUNC)
    if (info->cdc_acm_enable)
    {
        cdc_app_init();
    }
#endif
#if 0//defined(CONFIG_USB_COMPOSITE_DEV_HAS_MSC_FUNC)
    if (info->msdc_enable)
    {
        // msdc_app_init();
    }
#endif
#if 0//defined(CONFIG_USB_GADGET_UFU_SUPPORT)
    if (info->ufu_enable)
    {
        // ufu_app_init();
    }
#endif
#if 0//defined(CONFIG_USB_GADGET_RNDIS_SUPPORT)
    if (info->rndis_enable)
    {
        // rndis_app_init();
    }
#endif
}

static void class_app_deinit(struct usb_composite_info *info)
{
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UVC_FUNC)
    if (info->uvc_enable[0])
    {
        uvc_app_deinit();
    }
#endif
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UAC_FUNC)
    if (info->uac_enable)
    {
        uac_app_deinit();
    }
#endif
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_HID_FUNC)
    if (info->hid_enable)
    {
        hid_app_deinit();
    }
#endif
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_CDC_FUNC)
    if (info->cdc_acm_enable)
    {
        cdc_app_deinit();
    }
#endif
#if 0//defined(CONFIG_USB_COMPOSITE_DEV_HAS_MSC_FUNC)
    if (info->msdc_enable)
    {
        // msdc_app_deinit();
    }
#endif
#if 0//defined(CONFIG_USB_GADGET_UFU_SUPPORT)
    if (info->ufu_enable)
    {
        // ufu_app_deinit();
    }
#endif
#if 0//defined(CONFIG_USB_GADGET_RNDIS_SUPPORT)
    if (info->rndis_enable)
    {
        // rndis_app_deinit();
    }
#endif
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UVC_FUNC)||defined(CONFIG_USB_COMPOSITE_DEV_HAS_UAC_FUNC)
    if (composite_is_pcam(info))
    {
        usb_class_ac_vc_app_deinit();
        pcam_app_deinit();
    }
#endif
}

static void composite_dev_init(struct usb_composite_info *pinfo)
{
    class_app_init(pinfo);
    RET_ON(sstar_usb_composite_init(pinfo) != TRUE);
}

static void composite_dev_deinit(struct usb_composite_info *pinfo)
{
    RET_ON(sstar_usb_composite_deinit() != TRUE);
    class_app_deinit(pinfo);
}

struct uvc_user_ops uvc_app_ops = {
    .process_vc_req  = uvc_video_process_vc_req,
    .process_vs_req  = uvc_video_process_vs_req,
    .process_vc_data = uvc_app_process_vc_data,
    .process_vs_data = uvc_video_process_vs_data,

    .stop_stream  = uvc_app_stop_capture,
    .start_stream = uvc_app_start_capture,
    .set_config   = uvc_app_set_config,
    .suspend      = uvc_app_suspend,
    .resume       = uvc_app_resume,
    .reset        = uvc_app_usb_reset,

    .clear_format = uvc_video_clear_format,
    .add_format   = uvc_video_add_format,
    .xfer_cfg     = uvc_video_xfer_cfg,

    .init_req_param   = uvc_video_init_req_param,
    .deinit_req_param = uvc_video_deinit_req_param,
    .speed_negotiate  = uvc_app_speed_negotiate,
};

#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UAC_FUNC)
extern struct uac_user_ops uac_app_ops;
#endif

#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_CDC_FUNC)
extern struct cdc_acm_user_ops cdc_acm_ops;
#endif

static u8 composite_dev_get_classes(struct usb_composite_info *pinfo)
{
    u8 func_num = 0;
#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UVC_FUNC)
    u8 i;
#endif

    memset(pinfo, 0, sizeof(struct usb_composite_info));

    CamOsPrintf(KERN_INFO"\nUSB class:");

#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UVC_FUNC)
    for (i = 0; i < CONFIG_USB_GADGET_UVC_STREAM_NUM; i++)
    {
        pinfo->uvc_enable[i] = 1;
    }
    pinfo->uvc_ops = &uvc_app_ops;
    func_num++;
    CamOsPrintf(KERN_INFO" UVC");
#endif

#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_UAC_FUNC)
    pinfo->uac_enable = 1;
    pinfo->uac_ops    = &uac_app_ops;
    func_num++;
    CamOsPrintf(KERN_INFO" UAC");
#endif

#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_HID_FUNC)
    pinfo->hid_enable = 1;
    // pinfo->hid_ops[0] = &hid_app_ops;
    func_num++;
    CamOsPrintf(KERN_INFO" HID");
#endif

#if defined(CONFIG_USB_COMPOSITE_DEV_HAS_CDC_FUNC)
    pinfo->cdc_acm_enable = 1;
    pinfo->cdc_acm_ops    = &cdc_acm_ops;
    func_num++;
    CamOsPrintf(KERN_INFO" CDC");
#endif

#if 0//defined(CONFIG_USB_COMPOSITE_DEV_HAS_MSC_FUNC)
    pinfo->msdc_enable = 1;
    func_num++;
    CamOsPrintf(KERN_INFO" MDSC");
#endif

    CamOsPrintf(KERN_INFO"\n");
    return func_num;
}

void *composite_thread_entry(void *data)
{
    CamOsRet_e eRet;
    struct usb_composite_info pre_info = {0};
    u8 has_inited = FALSE;  // the composite device has been inited or not
    CamOsPrintf(KERN_INFO"composite_thread_entry\n");
    while (CAM_OS_OK != CamOsThreadShouldStop())
    {
        eRet = CamOsTsemTimedDown(&composite_sem, 1000);

        if (composite_thread_exit)
            break;

        if (eRet == CAM_OS_OK)
        {
            u8 do_init = FALSE;

#if defined(CONFIG_USB_GADGET_VBUS_DETECT)
            if (vbus_state_changed) // VBus state changed
            {
                vbus_state_changed = 0;
                if (vbus_state) // Vbus plug-in
                {
                    do_init = !has_inited;
                }
                else // Vbus plug-out
                {
                    sstar_usbd_device_disconnect();
                }
            }
            else
#endif
            {
                do_init = TRUE;
            }

            if (do_init)
            {
                struct usb_composite_info info = {0};
                u8 class_num = 0;

                class_num = composite_dev_get_classes(&info);

                if (has_inited)
                {
                    composite_dev_deinit(&pre_info);
                    memset(&pre_info, 0, sizeof(struct usb_composite_info));
                    has_inited = FALSE;
                }

                if (class_num > 0)
                {
                    composite_dev_init(&info);
                    memcpy(&pre_info, &info, sizeof(struct usb_composite_info));
                    has_inited = TRUE;
                }
            }
        }
    }

    return NULL;
}

#endif

#if defined(CONFIG_USB_GADGET_VBUS_DETECT)
static u8 vbus_state_changed = 0;
static int vbus_state = 0;
#include "gpio.h"
#include "drv_gpio_io.h"
#define TASK_VBUS_PRIORITY (1)
#define TASK_VBUS_STACK_SIZE (2048)
#define TASK_VBUS_NAME "USB_VBUS_DETECT"
CamOsThread vbus_thread = 0;

#endif

#if defined(CONFIG_USB_GADGET_VBUS_DETECT)
static void *vbus_detect_entry(void *data)
{
    int cur = 0;
    int vbus_gpio = *(u16 *)(data);

    if (vbus_gpio < GPIO_NR)
    {
        camdriver_gpio_request(NULL, vbus_gpio);
        camdriver_gpio_direction_input(NULL, vbus_gpio);
    }

    while (CAM_OS_OK != CamOsThreadShouldStop())
    {
        cur = (vbus_gpio < GPIO_NR) ? camdriver_gpio_get(NULL, vbus_gpio) : 1;
        if (vbus_state != cur)
        {
            vbus_state = cur;
            vbus_state_changed = 1;
            composite_thread_wakeup();
        }
        CamOsMsSleep(200);
    }

    return NULL;
}

static void usb_vbus_init(void)
{
    UsbBootCust_t *usb_setting = UsbBootSettingGetHandle();
    CamOsThreadAttrb_t task_attr = {.nPriority = TASK_VBUS_PRIORITY,
                                    .szName = TASK_VBUS_NAME,
                                    .nStackSize = TASK_VBUS_STACK_SIZE};

    CamOsPrintf(KERN_INFO"VBus detect gpio:%d\n", usb_setting->u16VbusGpio);
    RET_ON(CamOsThreadCreate(&vbus_thread, &task_attr,
                                      vbus_detect_entry, &usb_setting->u16VbusGpio) != CAM_OS_OK);
}

static void usb_vbus_deinit(void)
{
    RET_ON(CamOsThreadStop(vbus_thread) != CAM_OS_OK);
}
#endif

#if defined(CONFIG_MI_SDK_SUPPORT) && (CONFIG_ENABLE_MI_SDK_PIPELINE_FLOW == 1)

static bool RtosPreloadIsUseReduceMemCfg(void)
{
    return (SysMmapGetLimitDramSize() <= 0x04000000) ? TRUE : FALSE;
}

#define MULTIRING_SUPPORT     1
#define MI_AUDIO_SAMPLE_PER_FRAME 1024

#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE              MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED                MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN              MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE               MAKE_YUYV_VALUE(29,225,107)

#define _EXT_BIN            ".bin"
#define _ISP_API            "isp_api"
#define _ISP_API_HDR        "isp_api_hdr"
#define _ISP_API_HDR_3F     "isp_api_hdr_3f"

#ifdef CONFIG_PANEL_IN_RTOS_ENABLE
#define PANEL_ini_name           "config.ini"
MI_PHY phyPanelIni;
void* g_PanelIniAddr = NULL;
MI_U32 g_u32PanelIniSize = 0;

#define DISP_INPUT_PORT_MAX 16
#define DISP_LAYER_MAX 2
#define DISP_DEV_MAX 2

typedef struct stDispUtDev_s
{
    MI_BOOL bDevEnable;
    MI_BOOL bDevBindLayer[DISP_LAYER_MAX];
    MI_DISP_PubAttr_t stPubAttr;
    MI_PANEL_IntfType_e eIntfType;
}stDispUtDev_t;

typedef struct stDispUtLayer_s
{
    MI_BOOL bLayerEnable;
    MI_DISP_RotateMode_e eRotateMode;
    MI_DISP_VideoLayerAttr_t stLayerAttr;
}stDispUtLayer_t;

typedef struct stDispUtPort_s
{
    MI_BOOL bPortEnable;
    MI_SYS_PixelFormat_e    ePixFormat;         /* Pixel format of the video layer */
    MI_DISP_VidWinRect_t stCropWin;                     /* rect of video out chn */
    MI_DISP_InputPortAttr_t stInputPortAttr;
}stDispUtPort_t;

static stDispUtDev_t g_astDispUtDev[DISP_DEV_MAX];
static stDispUtLayer_t g_astDispUtLayer[DISP_LAYER_MAX];
static stDispUtPort_t g_astDispUtPort[DISP_LAYER_MAX][DISP_INPUT_PORT_MAX];

#endif
/*=============================================================*/
// Global Variable definition
/*=============================================================*/
static MI_U8 g_u8dlaosdoff = 0;
static MI_U8 g_u8timeosdoff = 0;
static MI_U8 g_u8stoppipe = 0;
static MI_U8 g_u8deintmoudle = 0;

extern int DrvAlgoEntry(void);

#ifndef STCHECKRESULT
#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        CamOsPrintf("[%s %d]exec function failed 0x%x\n", __FUNCTION__, __LINE__,result);\
        return 1;\
    }
#endif

#define RGB_TO_CRYCB(r, g, b)                                                            \
    (((unsigned int)(( 0.439f * (r) - 0.368f * (g) - 0.071f * (b)) + 128.0f)) << 16) |    \
    (((unsigned int)(( 0.257f * (r) + 0.564f * (g) + 0.098f * (b)) + 16.0f)) << 8) |        \
    (((unsigned int)((-0.148f * (r) - 0.291f * (g) + 0.439f * (b)) + 128.0f)))

typedef struct _SnrInfo_t
{
    MI_U32 idx;
    MI_U16 u16SnrW;
    MI_U16 u16SnrH;
    MI_U32 u32SnrMaxFps;
    MI_U32 u32SnrMinFps;
} SnrInfo_t;

#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)
#include "gb2312_code.h"
#else
MI_U16 gb2312code[] ={0};
#endif
typedef struct STUB_Resolution_s
{
    MI_U32 u32MaxWidth;
    MI_U32 u32MaxHeight;
    MI_U32 u32OutWidth;
    MI_U32 u32OutHeight;
} STUB_Resolution_t;


typedef struct STUB_VencRes_s
{
    MI_VENC_ModType_e eModType;
    MI_U32 u32DevId;
    MI_U32 u32ChnId;
    MI_U32 u32PortId;
    CamOsThread tid;
    MI_BOOL bThreadRunning;
} STUB_VencRes_t;

#define STUB_VENC_CHN_NUM 3
#define STUB_VENC_RESOLUTION_NUM 3

STUB_Resolution_t _stVpeResolution[STUB_VENC_RESOLUTION_NUM] =
{
    {1920, 1080, 1920, 1080},
    {1280, 720, 1280, 720},
    {640, 360, 640, 360},
};
#if INTERFACE_RGN
MI_RGN_PaletteTable_t _gstPaletteTable =
{
    { //index0 ~ index15
         {255,   0,   0,   0}, {255, 255,   0,   0}, {255,   0, 255,   0}, {255,   0,   0, 255},
         {255, 255, 255,   0}, {255,   0, 112, 255}, {255,   0, 255, 255}, {255, 255, 255, 255},
         {255, 128,   0,   0}, {255, 128, 128,   0}, {255, 128,   0, 128}, {255,   0, 128,   0},
         {255,   0,   0, 0}, {255,   0, 128, 128}, {255, 128, 128, 128}, {255,  64,  64,  64},
         //index16 ~ index31
         {  0,   0,   0,   0}, {  0,   0,   0,  30}, {  0,   0, 255,  60}, {  0, 128,   0,  90},
         {255,   0,   0, 120}, {  0, 255, 255, 150}, {255, 255,   0, 180}, {  0, 255,   0, 210},
         {255,   0, 255, 240}, {192, 192, 192, 255}, {128, 128, 128,  10}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index32 ~ index47
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index48 ~ index63
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index64 ~ index79
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index80 ~ index95
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index96 ~ index111
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index112 ~ index127
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index128 ~ index143
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index144 ~ index159
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index160 ~ index175
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index176 ~ index191
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index192 ~ index207
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index208 ~ index223
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index224 ~ index239
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         // (index236 :192,160,224 defalut colorkey)
         {192, 160, 224, 255}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         //index240 ~ index255
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0},
         {  0,   0,   0,   0}, {  0,   0,   0,   0}, {  0,   0,   0,   0}, {192, 160, 224, 255}
    }
};
#endif
typedef struct _MaxFrameSize {
    MI_U32 frameSizeI;
    MI_U32 frameSizeP;
} MaxFrameSize;

static const MaxFrameSize _maxFrameSize[8] =
{
    {84 * 8 * 1024 * 8,  63 * 8 * 1024 * 8 },
    {63 * 1024 * 8,  47 * 1024 * 8 },
    {42 * 1024 * 8,  32 * 1024 * 8 },
    {32 * 1024 * 8,  24 * 1024 * 8 },
    {21 * 1024 * 8,  16 * 1024 * 8 },
    {21 * 1024 * 8,  16 * 1024 * 8 },
    {20 * 1024 * 8,  15 * 1024 * 8 },
    {20 * 1024 * 8,  15 * 1024 * 8 }
};

#define MAIN_STREAM                "main_stream"
#define SUB_STREAM0                "sub_stream0"
#define SUB_STREAM1                "sub_stream1"
#define SUB_STREAM2                "sub_stream2"
#define SUB_STREAM3                "sub_stream3"
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#ifndef CONFIG_USB_GADGET_UVC_SUPPORT
typedef enum
{
    ST_Sys_Input_SCL = 0,
    ST_Sys_Input_VENC = 0,
    ST_Sys_Input_BUTT,
} ST_Sys_Input_E;

typedef enum
{
    ST_Sys_Func_RTSP = 0x01,
    ST_Sys_Func_CAPTURE = 0x02,
    ST_Sys_Func_DISP = 0x04,
    ST_Sys_Func_UVC = 0x08,

    ST_Sys_Func_BUTT = 0x0,
} ST_Sys_Func_E;
#endif
typedef struct ST_Stream_Attr_S
{
    MI_BOOL             bEnable;
    ST_Sys_Input_E      enInput;
    MI_U32              u32InputChn;
    MI_U32              u32InputPort;
    MI_VENC_CHN         vencChn;
    MI_VENC_ModType_e   eType;
    MI_U32              u32Mbps;
    MI_U32              u32Width;
    MI_U32              u32Height;
    MI_U32              u32MaxWidth;
    MI_U32              u32MaxHeight;
    MI_U32              u32CropX;
    MI_U32              u32CropY;
    MI_U32              u32CropWidth;
    MI_U32              u32CropHeight;
    MI_U32              enFunc;
    const char          *pszStreamName;
    MI_SYS_BindType_e   eBindType;
    MI_U32              u32BindPara;
    MI_U32              u32Cover1Handle;
    MI_U32              u32Cover2Handle;
 }ST_Stream_Attr_T;


static ST_Stream_Attr_T g_stStreamAttr[] =
{
    {
        .bEnable = TRUE,
        .enInput = ST_Sys_Input_SCL,
        .u32InputChn = 0,
        .u32InputPort = 0,
        .vencChn = 0,
        .eType = E_MI_VENC_MODTYPE_H264E,
        .u32Mbps = 2,
        .u32Width = 1920,
        .u32Height = 1080,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32CropX = 0,
        .u32CropY = 0,
        .u32CropWidth = 0,
        .u32CropHeight = 0,
        .enFunc = ST_Sys_Func_RTSP,
        .pszStreamName = MAIN_STREAM,
#if (defined MULTIRING_SUPPORT) && (MULTIRING_SUPPORT == 1)
        .eBindType = E_MI_SYS_BIND_TYPE_HW_RING,
#else
        .eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE,
#endif
        .u32BindPara = 0,
        .u32Cover1Handle = 3,
        .u32Cover2Handle = 4,
    },
    {
        .bEnable = TRUE,
        .enInput = ST_Sys_Input_SCL,
        .u32InputChn = 1,
        .u32InputPort = 0,
        .vencChn = 1,
        .eType = E_MI_VENC_MODTYPE_H264E,
        .u32Mbps = 2,
        .u32Width = 640,
        .u32Height = 360,
        .u32MaxWidth = 640,
        .u32MaxHeight = 360,
        .u32CropX = 0,
        .u32CropY = 0,
        .u32CropWidth = 0,
        .u32CropHeight = 0,
        .enFunc = ST_Sys_Func_RTSP,
        .pszStreamName = SUB_STREAM0,
        .eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE,
        .u32BindPara = 0,
        .u32Cover1Handle = 7,
        .u32Cover2Handle = 8,
    },
    {
        .bEnable = TRUE,
        .enInput = ST_Sys_Input_SCL,
        .u32InputChn = 1,
        .u32InputPort = 0,
        .vencChn = 2,
        .eType = E_MI_VENC_MODTYPE_JPEGE,
        .u32Mbps = 2,
        .u32Width = 1920,
        .u32Height = 1080,
        .u32MaxWidth = 1920,
        .u32MaxHeight = 1080,
        .u32CropX = 0,
        .u32CropY = 0,
        .u32CropWidth = 0,
        .u32CropHeight = 0,
        .enFunc = ST_Sys_Func_RTSP,
        .pszStreamName = SUB_STREAM1,
        .eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE,
        .u32BindPara = 0,
        .u32Cover1Handle = 5,
        .u32Cover2Handle = 6,
    }
};


#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP) || defined(CONFIG_IPU_IN_RTOS_ENABLE) || defined(CONFIG_VDF_IN_RTOS_ENABLE)
#define VPE_PORT0_OSD_FOR_VDF_HANDLE    3
#define VPE_PORT0_OSD_FOR_TIME_HANDLE   2
#define VPE_PORT0_OSD_FOR_PIC_HANDLE    1
#define VPE_PORT0_OSD_FOR_FD_HANDLE     0
#define MAX_RGN_NUM                     5
#define RGN_OSD_TIME_WIDTH              200
#define RGN_OSD_TIME_HEIGHT             32
#define FONT_ASCII_8x16                 "ascii_8x16"
#define FONT_HZ_16x16                   "hanzi_16x16"
#define OSD_PIC_ARGB1555_200X131_PATH   "200X131.argb"
#define OSD_PIC_BMP_200X133_PATH        "200X133.bmp"
#define ST_OSD_HANDLE_INVALID           0xFFFF
#define RGB2PIXEL1555(r,g,b)            ((((r) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((b) & 0xf8) >> 3) | 0x8000)
#define I4_RED                          (1)
#define I4_GREEN                        (2)
#define I4_BLUE                         (3)
#define I4_BLACK                        (12)
#define OSD_RECT_BORDERWIDTH            4
#define DIVP_CHN_FOR_VDF                4
#define RAW_W                           384
#define RAW_H                           288
#define MAX_FULL_RGN_NULL               1//1 only for main stream

#define MAX_FD_coordinate               100
#ifndef MAX
#define MAX(a,b)                        ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)                        ((a) < (b) ? (a) : (b))
#endif
#define ALIGN_MULTI(x, align)           (((x) % (align)) ? ((x) / (align) + 1) : ((x) / (align)))
#define MAX_BUF_LEN                     1024
#define BITS_PER_PIXEL                  16
#define MAX_LINES                       16
#define BMP_HEAD_LEN                    54


#define ST_OSD_INIT_CHECK(handle) \
    do{  \
        if (hHandle < 0 || hHandle >= MAX_RGN_NUM) \
        { \
            return 1; \
        } \
        if (g_stRgnInfo[hHandle].hHandle == -1) \
        { \
            return 1; \
        } \
    }while(0);


typedef struct
{
    MI_RGN_HANDLE hHandle;
    MI_RGN_CanvasInfo_t stCanvasInfo;
    MI_RGN_PixelFormat_e ePixelFmt;
} ST_RGN_Info_T;

typedef enum
{
    UTF8_NO_BOM = 0,
    UTF8_WITH_BOM,
    GBK,

    Encoding_Butt,
} DMF_Encoding_Type_E;

typedef enum
{
    DMF_Font_Type_ASCII = 0,
    DMF_Font_Type_HZ,

    DMF_Font_Type_BUTT,
} DMF_Font_Type_E;
typedef enum
{
    DMF_Font_Size_16x16 = 0,    // ascii 8x16
    //DMF_Font_Size_32x32,        // ascii 16x32
    //DMF_Font_Size_48x48,        // ascii 24x48
    //DMF_Font_Size_64x64,        // ascii 32x64

    DMF_Font_Size_BUTT,
} DMF_Font_Size_E;

typedef struct
{
    char            szFile[64];
    int             fd;
    MI_PHY        pBitMapAddr;
    int             width;
    int             height;
} DMF_BitMapFile_S;
typedef struct
{
    int         charNumPerLine;
    int         bgColor;
    int         fgColor;
    int         leftMargin;
    int         rightMargin;
    int         upMargin;
    int         downMargin;
    int         verticalFlag;
    int         charSpace;
    int         lineSpace;
} DMF_BitMapAttr_S;

typedef struct
{
    char                    fileName[50];
    MI_RGN_PixelFormat_e    ePixelFmt;
    uint8_t*                pTestFileAddr;
    MI_U16                  u16RgnWidth;
    MI_U16                  u16RgnHeight;
}ST_TestFileInfo_t;

typedef struct
{
    MI_U32 u32X;
    MI_U32 u32Y;
} ST_Point_T;

typedef struct ST_Sys_Rect_s
{
    MI_U32 u32X;
    MI_U32 u32Y;
    MI_U16 u16PicW;
    MI_U16 u16PicH;
} ST_Rect_T;

#if defined(CONFIG_IPU_IN_RTOS_ENABLE)
#define PERSON_DETECT
// IPU data type
#define SGS_MAGIC                       "SGSI"
#define SGS_MAGIC_SIZE                  4
#define SGS_MAX_INPUTS                  6
#define SGS_MAX_OUTPUTS                 8
#define SGS_MAX_DIMS                    8
#define SGS_IMG_WIDTH                   640
#define SGS_IMG_HEIGHT                  352

typedef struct FD_coordinate_s {
    float x1, y1;
    float x2, y2;
} FD_coordinate_t;

static void *Rtos_DIVPCheckFD_Func(void* p);
static void FD_deinit(void);
static int dla_InferenceFD(MI_IPU_CHN chn, MI_IPU_SubNet_InputOutputDesc_t *desc, MI_SYS_BufInfo_t *stBufInfo);
#endif

#if defined(CONFIG_VDF_IN_RTOS_ENABLE)
typedef struct ST_MDOD_Area_s
{
    MI_U32 u32Chn;
    ST_Rect_T stArea;
} ST_MDOD_Area_T;

typedef struct
{
    MI_U16 u16VdfInWidth;
    MI_U16 u16VdfInHeight;
    MI_U16 u16stride;

    MI_U16 u16OdNum;                    // od chn num
    MI_U16 u16MdNum;                    // md chn num
    MI_U16 u16VgNum;                    // vg chn num
    ST_MDOD_Area_T stOdArea[4];         // od detect area
    ST_MDOD_Area_T stMdArea[4];         // md detect area
    ST_MDOD_Area_T stVgArea[4];         // vg detect area
} ST_VdfChnArgs_t;

typedef struct ST_VdfSetting_Attr_S
{
    ST_Sys_Input_E enInput;
    MI_U32  u32InputChn;
    MI_U32  u32InputPort;
    ST_VdfChnArgs_t stVdfArgs;
}ST_VdfSetting_Attr_T;

typedef struct
{
    CamOsThread pThreadId;
    CamOsThreadAttrb_t stThreadAttrb;
    MI_VDF_CHANNEL vdfChn;
    MI_BOOL bRunFlag;
    MI_VDF_WorkMode_e enWorkMode;
    MI_U16 u16Width;
    MI_U16 u16Height;
} VDF_Thread_Args_t;

typedef struct
{
    MI_RGN_HANDLE hHandle;
    MI_ModuleId_e eModId;
    MI_U32 u32Chn;
    MI_U32 u32Port;
} ST_VDF_OSD_Info_T;

static MI_S32 ST_ModuleInit_VDF_MDOD_Rect(void);
static MI_S32 ST_VdfStart(void);
static void *ST_VDFGetResult(void *args);
static MI_S32 ST_ModuleInit_VDF(void);

#endif


static MI_S32 ST_OSD_Init(void);
static MI_S32 ST_OSD_Create(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion);
static MI_S32 ST_OSD_Destroy(MI_RGN_HANDLE hHandle);
static MI_S32 ST_OSD_Deinit(void);
static MI_S32 CreatePicOsd(MI_RGN_HANDLE handle, ST_TestFileInfo_t *pstTestFileInfo);
static void ST_OSDTimer_Exit(void);
static void *Rtos_UpdateOSDTimer_Func(void* p);
static MI_S32 ST_OSD_DrawPoint(MI_RGN_HANDLE hHandle, ST_Point_T stPoint, MI_U32 u32Color);

static MI_PHY phyOSDPicFile;

static CamOsMutex_t g_stRNGOsdMutex;
static ST_TestFileInfo_t g_stFileInfo[] =
{
#if defined(CONFIG_OSD_USE_BMP)
    {
        OSD_PIC_BMP_200X133_PATH,
        E_MI_RGN_PIXEL_FORMAT_ARGB1555,
        NULL,
        200,
        133
    }
#else
    {
        OSD_PIC_ARGB1555_200X131_PATH,
        E_MI_RGN_PIXEL_FORMAT_ARGB1555,
        NULL,
        200,
        131
    }
#endif
};

static DMF_BitMapFile_S g_dmf_bitmapfile[DMF_Font_Type_BUTT][DMF_Font_Size_BUTT] =
{
    {
        {
            .szFile = FONT_ASCII_8x16,
            .fd = -1,
            .pBitMapAddr = NULL,
            .width = 8,
            .height = 16,
        },
#if 0
        {
            .szFile = DMF_FONT_PREFIX DMF_FONT_ASCII_16x32,
            .fd = -1,
            .pBitMapAddr = NULL,
            .width = 16,
            .height = 32,
        },
        {
            .szFile = DMF_FONT_PREFIX DMF_FONT_ASCII_24x48,
            .fd = -1,
            .pBitMapAddr = NULL,
            .width = 24,
            .height = 48,
        },
        {
            .szFile = DMF_FONT_PREFIX DMF_FONT_ASCII_32x64,
            .fd = -1,
            .pBitMapAddr = NULL,
            .width = 32,
            .height = 64,
        },
#endif
    },
    {
        {
            .szFile = FONT_HZ_16x16,
            .fd = -1,
            .pBitMapAddr = NULL,
            .width = 16,
            .height = 16,
        },
#if 0
        {
            .szFile = DMF_FONT_PREFIX DMF_FONT_HZ_32x32,
            .fd = -1,
            .pBitMapAddr = NULL,
            .width = 32,
            .height = 32,
        },
        {
            .szFile = DMF_FONT_PREFIX DMF_FONT_HZ_48x48,
            .fd = -1,
            .pBitMapAddr = NULL,
            .width = 48,
            .height = 48,
        },
        {
            .szFile = DMF_FONT_PREFIX DMF_FONT_HZ_64x64,
            .fd = -1,
            .pBitMapAddr = NULL,
            .width = 64,
            .height = 64,
        },
#endif
    },
};

static DMF_BitMapAttr_S g_dmf_bitmapattr =
{
    .charNumPerLine = 32,
    .bgColor = 0x2323,
    .fgColor = 0xFFFFFF,
    .leftMargin = 1,
    .rightMargin = 1,
    .upMargin = 1,
    .downMargin = 1,
    .verticalFlag = 0,
    //.verticalFlag = 1,
    .charSpace = 1,
    .lineSpace = 1,
};

static ST_RGN_Info_T g_stRgnInfo[MAX_RGN_NUM];
static MI_BOOL g_bInit = FALSE;
//#define MI_RGN_OK  MI_SUCCESS
#if defined(CONFIG_IPU_IN_RTOS_ENABLE)
static MI_IPU_CHN channel = 0;
static MI_IPU_SubNet_InputOutputDesc_t dla_desc;
static int dla_ready = 0;
static MI_U32 inttestflag = 0;
static MI_U32 u32waitflag = 0;
static MI_PHY phyModel = 0;
static void *memIPUModel = NULL;
static MI_U32 modelSize = 0;

CamOsThreadAttrb_t threadAttr_FD = {.nPriority = 0,.szName = "DIVPCheckFD_func",.nStackSize = 20480};
CamOsThread threadid;
#endif

CamOsThreadAttrb_t threadAttr_OSDTimer = {.nPriority = 0,.szName = "UpdateOSDTimer_Func",.nStackSize = 8192};
CamOsThread threadOSDTimer;

#if defined(CONFIG_VDF_IN_RTOS_ENABLE)
static MI_U32 g_md_detect_cnt_bak[MAX_FULL_RGN_NULL] = {0};
static ST_Rect_T g_stRect_Bak[(RAW_W / 4) * (RAW_H / 4)][MAX_FULL_RGN_NULL] = {0};
static VDF_Thread_Args_t g_stVdfThreadArgs[MAX_FULL_RGN_NULL]= {0};
static ST_VDF_OSD_Info_T g_stVDFOsdInfo[MAX_FULL_RGN_NULL]= {0};
static MI_U8 g_stSadDataArry[(RAW_W / 4) * (RAW_H / 4) * 2] = {0};
static ST_Rect_T g_stRect[(RAW_W / 4) * (RAW_H / 4)] = {0};

static ST_VdfSetting_Attr_T g_stVdfSettingAttr[] =
{
    {
        .enInput = ST_Sys_Input_SCL,
        .u32InputChn = 0,
        .u32InputPort = 2,
        .stVdfArgs =
        {
            .u16VdfInWidth = 384,
            .u16VdfInHeight = 288,
            .u16stride = 384,
            .u16OdNum = 0,
            .u16MdNum = 1,
        },
    },
};
#endif
#endif


#if INTERFACE_LDC
#define LDC_CfgBin_name           "calibout.json"
#define LDC_CalibPoly_name        "CalibPoly_new.bin"
typedef struct
{
    MI_PHY phyAddr;
    void * pVirAddr;
    MI_U32 u32BufSize;
} mma_buf_t;

typedef struct ST_LDC_ChnAttr_s
{    //rtos preload
    mma_buf_t stConfig;
    mma_buf_t stCalibBin;
} ST_LDC_ChnAttr_t;

ST_LDC_ChnAttr_t g_stLdcChnAttr = {};
//static MI_U32 g_u32LdcViewNum = 1;
static MI_LDC_DEV g_u32LdcDevId = 0;
static MI_LDC_CHN g_u32LdcChnId = 0;
#endif // INTERFACE_LDC

extern CamOsTsem_t tPreloadFileTsem;
extern CamOsTsem_t tIspReadFileTsem;

#if INTERFACE_LDC
static MI_S32 _MI_LDC_GetFileSize(const char *file)
{
    CamFsFd tFD;
    MI_U32 filelen;
    MI_S32 s32Ret;

    s32Ret = CamFsOpen(&tFD, file, O_RDONLY, 0644);
    if (CAM_FS_OK != s32Ret)
    {
        CamOsPrintf("Open %s FAIL\n", file);
        return s32Ret;
    }
    else
    {
        CamOsPrintf("Open %s Success\n", file);
        filelen = CamFsSeek(tFD, 0, SEEK_END);
        CamFsClose(tFD);
        return filelen;
    }
}

static MI_S32 _MI_LDC_AllocBuf(mma_buf_t * pstMmaBuf, MI_U32 u32Size)
{
    MI_S32 s32Ret = 0;
    void * pVirbuf = NULL;
    MI_PHY phyAddr = NULL;
    s32Ret = MI_SYS_MMA_Alloc(0, NULL, u32Size ,&phyAddr);
    if (s32Ret)
    {
        CamOsPrintf("Failed to alloc buf: size %d\n", u32Size);
        s32Ret = -1;
        goto __exit_func;
    }
    s32Ret = MI_SYS_Mmap(phyAddr, u32Size, (void **)&pVirbuf, TRUE);
    if (s32Ret)
    {
        CamOsPrintf("Failed to Mmap buf:%p size %d\n", phyAddr, u32Size);
        s32Ret = -1;
        goto __exit_func;
    }
    pstMmaBuf->phyAddr = phyAddr;
    pstMmaBuf->pVirAddr = pVirbuf;
    pstMmaBuf->u32BufSize = u32Size;
    return 0;
__exit_func:
    if (pVirbuf)
        MI_SYS_Munmap(pVirbuf, u32Size);
    if (phyAddr)
        MI_SYS_MMA_Free(0, phyAddr);
    return s32Ret;
}

static void _MI_LDC_FreeBuf(mma_buf_t * pstMmaBuf)
{
    if (pstMmaBuf->pVirAddr)
        MI_SYS_Munmap(pstMmaBuf->pVirAddr, pstMmaBuf->u32BufSize);
    if (pstMmaBuf->phyAddr)
        MI_SYS_MMA_Free(0, pstMmaBuf->phyAddr);
}

static MI_S32 _MI_LDC_GetFileContentFromPath(void * pVirbuf, char *path, MI_U32 u32Size)
{
    MI_S32 s32Ret = 0;
    CamFsFd tFD;

    s32Ret = CamFsOpen(&tFD, path, O_RDONLY, 0644);
    if (CAM_FS_OK != s32Ret)
    {
        CamOsPrintf("Open %s FAIL, ret:%d \n", path, s32Ret);
        s32Ret = -1;
        goto __exit_func;
    }

    if (u32Size != CamFsRead(tFD, pVirbuf, u32Size))
    {
        CamOsPrintf("Failed to read size %d from cfg file:%s \n", u32Size, path);
        s32Ret = -1;
        goto __exit_func;
    }

__exit_func:
    if (tFD)
        CamFsClose(tFD);
    return s32Ret;
}

static void ST_PreReadLDCCfgFile(void)
{
    ST_LDC_ChnAttr_t * pstLdcChnAttr = &g_stLdcChnAttr;
    MI_S32 s32FileLen = 0;
    char CfgBin_file[128] = {0};
    char CalibPoly_file[128] = {0};

    CamOsSprintf(&CfgBin_file[0], "%s/%s", application_selector_get_rofile_path(), LDC_CfgBin_name);
    s32FileLen = _MI_LDC_GetFileSize(&CfgBin_file[0]);
    if (s32FileLen > 0)
    {
        _MI_LDC_AllocBuf(&pstLdcChnAttr->stConfig, s32FileLen);
        _MI_LDC_GetFileContentFromPath(pstLdcChnAttr->stConfig.pVirAddr, &CfgBin_file[0], s32FileLen);
        MI_SYS_FlushInvCache(pstLdcChnAttr->stConfig.pVirAddr, s32FileLen);
    }
    CamOsSprintf(&CalibPoly_file[0], "%s/%s", application_selector_get_rofile_path(), &CfgBin_file[0]);
    s32FileLen = _MI_LDC_GetFileSize(&CalibPoly_file[0]);
    if (s32FileLen > 0)
    {
        _MI_LDC_AllocBuf(&pstLdcChnAttr->stCalibBin, s32FileLen);
        _MI_LDC_GetFileContentFromPath(pstLdcChnAttr->stCalibBin.pVirAddr, &CalibPoly_file[0], s32FileLen);
        MI_SYS_FlushInvCache(pstLdcChnAttr->stCalibBin.pVirAddr, s32FileLen);
    }
}

static void ST_ReleaseLDCCfgFile()
{
    ST_LDC_ChnAttr_t * pstLdcChnAttr = &g_stLdcChnAttr;

    _MI_LDC_FreeBuf(&pstLdcChnAttr->stConfig);
    _MI_LDC_FreeBuf(&pstLdcChnAttr->stCalibBin);
}
#endif //INTERFACE_LDC

static MI_S32 STUB_GetVencConfig(MI_VENC_ModType_e eModType, MI_VENC_ChnAttr_t *pstVencChnAttr, MI_U8 u8ChnId)
{
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    MI_U32 u32Profile =0, u32BitRate = 0, u32Gop = 0, u32MaxQp = 0, u32MinQp = 0, u32SrcFrmRateNum = 0, u32MaxBitRate = 0;
    MI_U8 u8VideoH264RcMode = 0, u8VideoH265RcMode = 0;
    if(0 == u8ChnId)
    {
        u8VideoH264RcMode = pCameraBootSetting->u8Video0H264RcMode;
        u8VideoH265RcMode = pCameraBootSetting->u8Video0H265RcMode;
        u32Profile = pCameraBootSetting->u32Video0Profile;
        u32BitRate = pCameraBootSetting->u32Video0Bitrate;
        u32Gop = pCameraBootSetting->u16Video0Gop;
        u32MaxQp = pCameraBootSetting->u32Video0Maxqp;
        u32MinQp = pCameraBootSetting->u32Video0Minqp;
        u32SrcFrmRateNum  = pCameraBootSetting->u8Video0EncFrameRate;
        u32MaxBitRate = pCameraBootSetting->u32Video0Bitrate;
    }
    else if(1 == u8ChnId)
    {
        u8VideoH264RcMode = pCameraBootSetting->u8Video1H264RcMode;
        u8VideoH265RcMode = pCameraBootSetting->u8Video1H265RcMode;
        u32Profile = pCameraBootSetting->u32Video1Profile;
        u32BitRate = pCameraBootSetting->u32Video1Bitrate;
        u32Gop = pCameraBootSetting->u16Video1Gop;
        u32MaxQp = pCameraBootSetting->u32Video1Maxqp;
        u32MinQp = pCameraBootSetting->u32Video1Minqp;
        u32SrcFrmRateNum  = pCameraBootSetting->u8Video1EncFrameRate;
        u32MaxBitRate = pCameraBootSetting->u32Video1Bitrate;
    }
    else if(2 == u8ChnId)
    {
        u8VideoH264RcMode = pCameraBootSetting->u8Video2H264RcMode;
        u8VideoH265RcMode = pCameraBootSetting->u8Video2H265RcMode;
        u32Profile = pCameraBootSetting->u32Video2Profile;
        u32BitRate = pCameraBootSetting->u32Video2Bitrate;
        u32Gop = pCameraBootSetting->u16Video2Gop;
        u32MaxQp = pCameraBootSetting->u32Video2Maxqp;
        u32MinQp = pCameraBootSetting->u32Video2Minqp;
        u32SrcFrmRateNum  = pCameraBootSetting->u8Video2EncFrameRate;
        u32MaxBitRate = pCameraBootSetting->u32Video2Bitrate;
    }
    else
    {
        u8VideoH264RcMode = pCameraBootSetting->u8Video0H264RcMode;
        u8VideoH265RcMode = pCameraBootSetting->u8Video0H265RcMode;
        u32Profile = pCameraBootSetting->u32Video0Profile;
        u32BitRate = pCameraBootSetting->u32Video0Bitrate;
        u32Gop = pCameraBootSetting->u16Video0Gop;
        u32MaxQp = pCameraBootSetting->u32Video0Maxqp;
        u32MinQp = pCameraBootSetting->u32Video0Minqp;
        u32SrcFrmRateNum  = pCameraBootSetting->u8Video0EncFrameRate;
        u32MaxBitRate = pCameraBootSetting->u32Video0Bitrate;
        CamOsPrintf("unsupport video ch[%u]\n", u8ChnId);
    }

    switch(eModType)
    {
        case E_MI_VENC_MODTYPE_H264E:
        {
            pstVencChnAttr->stVeAttr.eType = eModType;
            pstVencChnAttr->stVeAttr.stAttrH264e.u32PicWidth = pstStreamAttr[u8ChnId].u32Width;
            pstVencChnAttr->stVeAttr.stAttrH264e.u32PicHeight = pstStreamAttr[u8ChnId].u32Height;
            pstVencChnAttr->stVeAttr.stAttrH264e.u32MaxPicWidth = pstStreamAttr[u8ChnId].u32MaxWidth;
            pstVencChnAttr->stVeAttr.stAttrH264e.u32MaxPicHeight = pstStreamAttr[u8ChnId].u32MaxHeight;
            pstVencChnAttr->stVeAttr.stAttrH264e.bByFrame = TRUE;
            pstVencChnAttr->stVeAttr.stAttrH264e.u32Profile = u32Profile;
            if(0 == u8VideoH264RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32BitRate = u32BitRate;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32StatTime = 0;
            }
            else if(1 == u8VideoH264RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264VBR;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32MaxQp = u32MaxQp;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32MinQp = u32MinQp;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32StatTime = 0;
                pstVencChnAttr->stRcAttr.stAttrH264Vbr.u32MaxBitRate = u32MaxBitRate;
            }
            else if(2 == u8VideoH264RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264FIXQP;
                pstVencChnAttr->stRcAttr.stAttrH264FixQp.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH264FixQp.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH264FixQp.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH264FixQp.u32IQp = 30;
                pstVencChnAttr->stRcAttr.stAttrH264FixQp.u32PQp = 30;
            }
            else if(3 == u8VideoH264RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264AVBR;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32MaxQp = u32MaxQp;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32MinQp = u32MinQp;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32StatTime = 0;
                pstVencChnAttr->stRcAttr.stAttrH264Avbr.u32MaxBitRate = u32MaxBitRate;
            }
        }
        break;
        case E_MI_VENC_MODTYPE_H265E:
        {
            pstVencChnAttr->stVeAttr.eType = eModType;
            pstVencChnAttr->stVeAttr.stAttrH265e.u32PicWidth = pstStreamAttr[u8ChnId].u32Width;
            pstVencChnAttr->stVeAttr.stAttrH265e.u32PicHeight = pstStreamAttr[u8ChnId].u32Height;
            pstVencChnAttr->stVeAttr.stAttrH265e.u32MaxPicWidth = pstStreamAttr[u8ChnId].u32MaxWidth;
            pstVencChnAttr->stVeAttr.stAttrH265e.u32MaxPicHeight = pstStreamAttr[u8ChnId].u32MaxHeight;
            pstVencChnAttr->stVeAttr.stAttrH265e.bByFrame = TRUE;

            if(0 == u8VideoH265RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32StatTime = 0;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32BitRate = u32BitRate;
                pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 1;
            }
            else if(1 == u8VideoH265RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265VBR;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32StatTime = 0;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32MaxBitRate = u32MaxBitRate;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32MaxQp = u32MaxQp;
                pstVencChnAttr->stRcAttr.stAttrH265Vbr.u32MinQp = u32MinQp;

            }
            else if(2 == u8VideoH265RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265FIXQP;
                pstVencChnAttr->stRcAttr.stAttrH265FixQp.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH265FixQp.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH265FixQp.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH265FixQp.u32IQp = 30;
                pstVencChnAttr->stRcAttr.stAttrH265FixQp.u32PQp = 30;
            }
            else if(3 == u8VideoH265RcMode)
            {
                pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265AVBR;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32Gop = u32Gop;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32StatTime = 0;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32SrcFrmRateNum = u32SrcFrmRateNum;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32SrcFrmRateDen = 1;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32MaxBitRate = u32MaxBitRate;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32MaxQp = u32MaxQp;
                pstVencChnAttr->stRcAttr.stAttrH265Avbr.u32MinQp = u32MinQp;
            }
        }
        break;
        case E_MI_VENC_MODTYPE_JPEGE:
        {
            pstVencChnAttr->stVeAttr.eType = eModType;
            pstVencChnAttr->stVeAttr.stAttrJpeg.u32MaxPicWidth = pstStreamAttr[u8ChnId].u32MaxWidth;
            pstVencChnAttr->stVeAttr.stAttrJpeg.u32MaxPicHeight = pstStreamAttr[u8ChnId].u32MaxHeight;
            pstVencChnAttr->stVeAttr.stAttrJpeg.u32PicWidth = pstStreamAttr[u8ChnId].u32Width;
            pstVencChnAttr->stVeAttr.stAttrJpeg.u32PicHeight = pstStreamAttr[u8ChnId].u32Height;
            pstVencChnAttr->stVeAttr.stAttrJpeg.bByFrame = TRUE;
            pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
        }
        break;
        default:
            CamOsPrintf("unsupport eModType[%u]\n", eModType);
            return E_MI_ERR_FAILED;
    }
    return MI_SUCCESS;
}
static CamOsThread PreloadMiPipe_tid;
static CamOsThread PreloadFile_tid;

#ifdef CONFIG_PANEL_IN_RTOS_ENABLE
static void GetLayerDisplaySize(MI_DISP_OutputTiming_e eOutputTiming, MI_U32 *LayerDisplayWidth, MI_U32 *LayerDisplayHeight)
{

    if(eOutputTiming == E_MI_DISP_OUTPUT_USER){
        MI_PANEL_ParamConfig_t stPanelParam;
        MI_PANEL_GetPanelParam(E_MI_PNL_INTF_MIPI_DSI,&stPanelParam);
        *LayerDisplayWidth = stPanelParam.u16Width;
        *LayerDisplayHeight = stPanelParam.u16Height;
        return;
    }
}
static MI_U32 disp_ut_portshowsize(MI_U8 u8ChnNum, MI_DISP_OutputTiming_e eOutputTiming)
{
    MI_U8 n = 0;
    MI_U8 u8Factor = 4;
    MI_U8 u8PortId = 0;
    MI_U32 u32PortOutWidth = 0;
    MI_U32 u32PortOutHeight = 0;
    MI_U32 u32LayerDispWidth = 0;
    MI_U32 u32LayerDispHeight = 0;

    GetLayerDisplaySize(eOutputTiming, &u32LayerDispWidth, &u32LayerDispHeight);

    if(/*u8ChnNum > DISP_INPUT_PORT_MAX ||*/ u8ChnNum == 0)
    {
        CamOsPrintf("port num is invalid\n");
        return E_MI_ERR_FAILED;
    }
    if(u8ChnNum == 1)
    {
        u32PortOutWidth = u32LayerDispWidth;
        u32PortOutHeight = u32LayerDispHeight;
        u8Factor = 1;
    }
    else if(u8ChnNum <= 4)
    {
        u32PortOutWidth = u32LayerDispWidth/2;
        u32PortOutHeight = u32LayerDispHeight/2;
        u8Factor = 2;
    }
    else if(u8ChnNum <= 9)
    {
        u32PortOutWidth = u32LayerDispWidth/3;
        u32PortOutHeight = u32LayerDispHeight/3;
        u8Factor = 3;
    }
    else
    {
        u32PortOutWidth = u32LayerDispWidth/4;
        u32PortOutHeight = u32LayerDispHeight/4;
        u8Factor = 4;
    }
    for(n = 0; n < u8ChnNum; n++)
    {
        u8PortId = n;
        MI_DISP_VideoLayerAttr_t *pstLayerAttr;
        MI_DISP_InputPortAttr_t *pstPortAttr;
        pstLayerAttr = &g_astDispUtLayer[0].stLayerAttr;
        pstPortAttr = &g_astDispUtPort[0][u8PortId].stInputPortAttr;

        pstLayerAttr->stVidLayerSize.u16Height = u32LayerDispHeight;
        pstLayerAttr->stVidLayerSize.u16Width = u32LayerDispWidth;
        pstLayerAttr->stVidLayerDispWin.u16X = 0;
        pstLayerAttr->stVidLayerDispWin.u16Y = 0;
        pstLayerAttr->stVidLayerDispWin.u16Width = u32LayerDispWidth;
        pstLayerAttr->stVidLayerDispWin.u16Height = u32LayerDispHeight;
        if(!pstPortAttr->stDispWin.u16Width || !pstPortAttr->stDispWin.u16Height)
        {
            pstPortAttr->stDispWin.u16X = (n%u8Factor)*u32PortOutWidth;
            pstPortAttr->stDispWin.u16Y = ((n/u8Factor)% u8Factor)*u32PortOutHeight;
            pstPortAttr->stDispWin.u16Width =(n>=16) ? u32PortOutWidth<<1 : u32PortOutWidth;
            pstPortAttr->stDispWin.u16Height = (n>=16) ? u32PortOutHeight<<1 : u32PortOutHeight;
        }
     }
    return u8ChnNum;
}

static MI_BOOL disp_ut_setdev(MI_DISP_DEV DispDev)
{
    MI_DISP_PubAttr_t stPubAttr;

    memcpy(&stPubAttr, &g_astDispUtDev[DispDev].stPubAttr, (size_t)sizeof(MI_DISP_PubAttr_t));
    //set disp pub
    stPubAttr.u32BgColor = YUYV_BLACK;
    MI_DISP_SetPubAttr(DispDev,  &stPubAttr);
    if(E_MI_DISP_INTF_HDMI == stPubAttr.eIntfType){
        stPubAttr.eIntfType = E_MI_DISP_INTF_VGA;
        MI_DISP_SetPubAttr(DispDev,  &stPubAttr);
    }
    else if(E_MI_DISP_INTF_VGA == stPubAttr.eIntfType){
        stPubAttr.eIntfType = E_MI_DISP_INTF_HDMI;
        MI_DISP_SetPubAttr(DispDev,  &stPubAttr);
    }
    MI_DISP_Enable(DispDev);
    g_astDispUtDev[DispDev].bDevEnable = TRUE;

    return MI_SUCCESS;
}

static MI_BOOL disp_ut_setlayer(MI_DISP_DEV DispDev, MI_DISP_LAYER DispLayer)
{
    MI_DISP_VideoLayerAttr_t stLayerAttr;

    memcpy(&stLayerAttr, &g_astDispUtLayer[DispLayer].stLayerAttr, (size_t)sizeof(MI_DISP_VideoLayerAttr_t));
    MI_DISP_BindVideoLayer(DispLayer,DispDev);
    MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr);
    MI_DISP_EnableVideoLayer(DispLayer);
    g_astDispUtDev[DispDev].bDevBindLayer[DispLayer] = TRUE;
    g_astDispUtLayer[DispLayer].bLayerEnable = TRUE;

    return MI_SUCCESS;
}

static MI_BOOL disp_ut_setport(MI_DISP_LAYER DispLayer, MI_U32 DispInport)
{
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_DISP_VidWinRect_t stWinRect;
    MI_DISP_RotateConfig_t stRotateConfig;

    memcpy(&stInputPortAttr, &g_astDispUtPort[DispLayer][DispInport].stInputPortAttr, (size_t)sizeof(MI_DISP_InputPortAttr_t));
    memcpy(&stWinRect, &g_astDispUtPort[DispLayer][DispInport].stCropWin, (size_t)sizeof(MI_DISP_VidWinRect_t));
    CamOsPrintf("%s:%d layer:%d port:%d srcwidth:%d srcheight:%d x:%d y:%d outwidth:%d outheight:%d\n",__FUNCTION__,__LINE__,
        DispLayer,DispInport,
        stInputPortAttr.u16SrcWidth,stInputPortAttr.u16SrcHeight,
        stInputPortAttr.stDispWin.u16X,stInputPortAttr.stDispWin.u16Y,
        stInputPortAttr.stDispWin.u16Width,stInputPortAttr.stDispWin.u16Height);

    stRotateConfig.eRotateMode = g_astDispUtLayer[DispLayer].eRotateMode;
    MI_DISP_SetInputPortAttr(DispLayer, DispInport, &stInputPortAttr);
    MI_DISP_SetZoomInWindow(DispLayer, DispInport, &stWinRect);
    MI_DISP_SetVideoLayerRotateMode(DispLayer, &stRotateConfig);
    MI_DISP_EnableInputPort(DispLayer, DispInport);
    MI_DISP_SetInputPortSyncMode(DispLayer, DispInport, E_MI_DISP_SYNC_MODE_FREE_RUN);

    g_astDispUtPort[DispLayer][DispInport].bPortEnable = TRUE;

    return MI_SUCCESS;
}

static MI_BOOL disp_ut_disabledev(MI_DISP_DEV DispDev)
{
    MI_DISP_Disable(DispDev);
    g_astDispUtDev[DispDev].bDevEnable = FALSE;

    return MI_SUCCESS;
}
static MI_BOOL disp_ut_disablelayer(MI_DISP_DEV DispDev, MI_DISP_LAYER DispLayer)
{
    MI_DISP_DisableVideoLayer(DispLayer);
    MI_DISP_UnBindVideoLayer(DispLayer, DispDev);
    g_astDispUtDev[DispDev].bDevBindLayer[DispLayer] = FALSE;
    g_astDispUtLayer[DispLayer].bLayerEnable = FALSE;

    return MI_SUCCESS;
}

static MI_BOOL disp_ut_disableport(MI_DISP_LAYER DispLayer, MI_U32 DispInport)
{
    MI_DISP_DisableInputPort(DispLayer, DispInport);
    g_astDispUtPort[DispLayer][DispInport].bPortEnable = FALSE;
    return MI_SUCCESS;
}
#endif

#if 0
MI_S32 GetVifChnMapPad(MI_U8 ch, MI_SNR_PADID *sPad)
{
    MI_S32 nRet = 0x0;

    if(NULL == sPad)
    {
        return -1;
    }
    if(0x0 == ch)
    {
        *sPad = 0;
    }
    else if(0x01 == ch)
    {
        *sPad = 2;
    }
    else if(0x02 == ch)
    {
        *sPad = 1;
    }
    else if(0x03 == ch)
    {
        *sPad = 3;
    }
    else
    {
        nRet = -1;
    }
    return nRet;
}
#endif

static inline MI_S32 GetVifPadMapGroup(MI_SNR_PADID sPad, MI_VIF_GROUP *mGroup)
{
    MI_S32 nRet = 0x0;
    if(NULL == mGroup)
    {
        return -1;
    }

    if(0 == sPad)
    {
        *mGroup = 0;
    }
    else if(1 == sPad)
    {
        *mGroup = 2;
    }
    else if(2 == sPad)
    {
        *mGroup = 1;
    }
    else if(3 == sPad)
    {
        *mGroup = 3;
    }
    else
    {
     nRet = -1;
    }
    return nRet;
}

#if INTERFACE_VDISP
static MI_S32 ST_StartPipeLine_Multi2One(MI_U32 u32SensorNum)
{
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MI_U32 u32SnsFrmRate = pCameraBootSetting->u8SensorFrameRate;
    MI_U32 u32EncFrmRate = pCameraBootSetting->u8Video0EncFrameRate;
    MI_ISP_OutPortParam_t  stIspOutputParam;
    MI_SCL_OutPortParam_t  stSclOutputParam;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32BindParam = 0;
    int i = 0;

    MI_VDISP_OutputPortAttr_t stVdispOutputAttr = {
        .u32BgColor = YUYV_BLUE,
        .ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420,
        .u64pts = 0,
        .u32FrmRate = u32EncFrmRate,
        .u32Width = pstStreamAttr[0].u32Width,
        .u32Height = pstStreamAttr[0].u32Height,
    };
    MI_U32 u32VdispInChn[] = {1, VDISP_OVERLAYINPUTCHNID};
    MI_VDISP_InputChnAttr_t stVdispInputAttr[] = {
        {   .u32OutX = 0,
            .u32OutY = 0,
            .u32OutWidth = stVdispOutputAttr.u32Width,
            .u32OutHeight = stVdispOutputAttr.u32Height,
            .s32IsFreeRun = TRUE
        },
        {   .u32OutX = 0,
            .u32OutY = stVdispOutputAttr.u32Height - stVdispOutputAttr.u32Height/3,
            .u32OutWidth = stVdispOutputAttr.u32Width/3,
            .u32OutHeight = stVdispOutputAttr.u32Height/3,
            .s32IsFreeRun = TRUE
        }
    };

    MI_VENC_ChnAttr_t stVencChnAttr;
    MI_VENC_SuperFrameCfg_t stSuperFrameCfg = { E_MI_VENC_SUPERFRM_NONE, 0, 0, 0 };
    MI_U32 u32VencDevId = 0;

    /*2. enable vdisp, bind divp to vdisp*/
    STCHECKRESULT(MI_VDISP_Init());
    STCHECKRESULT(MI_VDISP_OpenDevice(0));
    for(i = 0; i < u32SensorNum; i++)
        STCHECKRESULT(MI_VDISP_SetInputChannelAttr(0, u32VdispInChn[i], &stVdispInputAttr[i]));
    STCHECKRESULT(MI_VDISP_SetOutputPortAttr(0, 0, &stVdispOutputAttr));
    for(i = 0; i < u32SensorNum; i++)
        STCHECKRESULT(MI_VDISP_EnableInputChannel(0, u32VdispInChn[i]));
    STCHECKRESULT(MI_VDISP_StartDev(0));
    for(i = 0; i < u32SensorNum; i++)
    {
        stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId = 0;
        stSrcChnPort.u32ChnId = i;
        stSrcChnPort.u32PortId = 0;
        stDstChnPort.eModId = E_MI_MODULE_ID_VDISP;
        stDstChnPort.u32DevId = 0;
        stDstChnPort.u32ChnId = u32VdispInChn[i];
        stDstChnPort.u32PortId = 0;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SnsFrmRate, u32EncFrmRate, E_MI_SYS_BIND_TYPE_FRAME_BASE, u32BindParam));

        memset(&stIspOutputParam, 0x0, (size_t)sizeof(MI_ISP_OutPortParam_t));
        STCHECKRESULT(MI_ISP_GetInputPortCrop(0, i, &stIspOutputParam.stCropRect));

        memset(&stSclOutputParam, 0x0, (size_t)sizeof(MI_SCL_OutPortParam_t));
        stSclOutputParam.stSCLOutCropRect.u16X = 0;
        stSclOutputParam.stSCLOutCropRect.u16Y = 0;
        stSclOutputParam.stSCLOutCropRect.u16Width = stIspOutputParam.stCropRect.u16Width;
        stSclOutputParam.stSCLOutCropRect.u16Height = stIspOutputParam.stCropRect.u16Height;
        stSclOutputParam.stSCLOutputSize.u16Width = stVdispInputAttr[i].u32OutWidth;
        stSclOutputParam.stSCLOutputSize.u16Height = stVdispInputAttr[i].u32OutHeight;
        stSclOutputParam.bMirror = FALSE;
        stSclOutputParam.bFlip = FALSE;
        stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_NONE;
        stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stIspOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

        STCHECKRESULT(MI_ISP_SetOutputPortParam(0, i, 0, &stIspOutputParam));
        STCHECKRESULT(MI_SCL_SetOutputPortParam(0, i, 0, &stSclOutputParam));
        STCHECKRESULT(MI_ISP_EnableOutputPort(0, i, 0));
        STCHECKRESULT(MI_SCL_EnableOutputPort(0, i, 0));
    }

    /*3. enable venc, bind vdisp to venc*/
    memset(&stVencChnAttr, 0x0, (size_t)sizeof(MI_VENC_ChnAttr_t));
    STUB_GetVencConfig(pstStreamAttr[0].eType, &stVencChnAttr, 0);
    STCHECKRESULT(MI_VENC_CreateChn(0, pstStreamAttr[0].vencChn, &stVencChnAttr));
    STCHECKRESULT(MI_VENC_SetMaxStreamCnt(0, pstStreamAttr[0].vencChn, pCameraBootSetting->u32PreloadVideoFrame));
    if(pstStreamAttr[0].eType != E_MI_VENC_MODTYPE_JPEGE)
    {
        stSuperFrameCfg.u32SuperIFrmBitsThr = _maxFrameSize[0].frameSizeI;
        stSuperFrameCfg.u32SuperPFrmBitsThr = _maxFrameSize[0].frameSizeP;
        STCHECKRESULT(MI_VENC_SetSuperFrameCfg(0, pstStreamAttr[0].u32InputChn, &stSuperFrameCfg));
    }
    STCHECKRESULT(MI_VENC_StartRecvPic(0, pstStreamAttr[0].vencChn));
    stSrcChnPort.eModId = E_MI_MODULE_ID_VDISP;
    stSrcChnPort.u32DevId = 0;
    stSrcChnPort.u32ChnId = 0;
    stSrcChnPort.u32PortId = 0;
    stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId = u32VencDevId;
    stDstChnPort.u32ChnId = 0;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32EncFrmRate, u32EncFrmRate, E_MI_SYS_BIND_TYPE_FRAME_BASE, u32BindParam));

    return MI_SUCCESS;
}
#endif

static MI_S32 ST_StartPipeLine(MI_U8 i, MI_U32 u32Width, MI_U32 u32Height, MI_U32 u32CropW, MI_U32 u32CropH, MI_U32 u32CropX, MI_U32 u32CropY)
{
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MI_VENC_DEV VencDevId = 0;
    MI_VENC_InitParam_t tVencParam;
    MI_VENC_ChnAttr_t stVencChnAttr;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32FrameCnt = 0;
    MI_U32 u32SrcFrmrate = 0;
    MI_U32 u32DstFrmrate = 0;
    MI_U32 u32BindParam = 0;
    MI_SCL_CHANNEL SclChnId = 0;
    MI_ISP_CHANNEL IspChnId = 0;
    MI_U8 SclPortId = 0;
    MI_U8 IspPortId = 0;

#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)
    MI_RGN_ChnPort_t stAttachChnPort;
    MI_RGN_ChnPortParam_t stChnPortParam;
#endif

    if (RtosPreloadIsUseReduceMemCfg())
        u32FrameCnt = 3;
    else
        u32FrameCnt = pCameraBootSetting->u32PreloadVideoFrame;

    if(2 == pCameraBootSetting->u8SensorNum)
    {
        IspChnId = i;
        SclChnId = pstStreamAttr[i].u32InputChn;
        SclPortId = pstStreamAttr[i].u32InputPort;
    }
    else
    {
        IspChnId = 0;
        IspPortId = 0;
        SclChnId = 0;
        SclPortId = i;
    }

    if(pstStreamAttr[i].enInput == ST_Sys_Input_SCL)
    {
        MI_ISP_DEV IspDevId = 0;

        MI_SCL_DEV SclDevId = 0;
        MI_ISP_OutPortParam_t  stIspOutputParam;
        MI_SCL_OutPortParam_t  stSclOutputParam;
        MI_VENC_SuperFrameCfg_t stSuperFrameCfg = { E_MI_VENC_SUPERFRM_NONE, 0, 0, 0 };

        memset(&stIspOutputParam, 0x0, (size_t)sizeof(MI_ISP_OutPortParam_t));
        STCHECKRESULT(MI_ISP_GetInputPortCrop(IspDevId, IspChnId, &stIspOutputParam.stCropRect));
        _stVpeResolution[i].u32OutWidth = u32Width;
        _stVpeResolution[i].u32OutHeight = u32Height;

        memset(&stSclOutputParam, 0x0, (size_t)sizeof(MI_SCL_OutPortParam_t));
        stSclOutputParam.stSCLOutCropRect.u16X = stIspOutputParam.stCropRect.u16X;
        stSclOutputParam.stSCLOutCropRect.u16Y = stIspOutputParam.stCropRect.u16Y;
        stSclOutputParam.stSCLOutCropRect.u16Width = stIspOutputParam.stCropRect.u16Width;
        stSclOutputParam.stSCLOutCropRect.u16Height = stIspOutputParam.stCropRect.u16Height;
        stSclOutputParam.stSCLOutputSize.u16Width = u32Width;
        stSclOutputParam.stSCLOutputSize.u16Height = u32Height;
        stSclOutputParam.bMirror = FALSE;
        stSclOutputParam.bFlip = FALSE;

        if (pstStreamAttr[i].eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
        {
            stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_NONE;
        }

        stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stIspOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

        STCHECKRESULT(MI_SCL_SetOutputPortParam(SclDevId, SclChnId, SclPortId, &stSclOutputParam));
        STCHECKRESULT(MI_ISP_SetOutputPortParam(IspDevId, IspChnId, IspPortId, &stIspOutputParam));

#if INTERFACE_LDC
        if((TRUE == pCameraBootSetting->u8enableLDC)&&(0 == i)) //for main stream
        {
            MI_LDC_OutputPortAttr_t stLdcOutputAttr;
            MI_LDC_DevAttr_t stLdcDevAttr = {};
            MI_LDC_ChnAttr_t stLdcChnAttr = {};
            MI_LDC_ChnLDCAttr_t stAttr = {};
            MI_LDC_OutputPortAttr_t stLdcOutputPortAttr = {};
            MI_LDC_InputPortAttr_t stLdcInputPortAttr = {};
            MI_LDC_RegionAttr_t *pstRegionAttr = NULL;

            CamOsTsemDown(&tPreloadFileTsem);

            STCHECKRESULT(MI_LDC_CreateDevice(g_u32LdcDevId, &stLdcDevAttr));

            stLdcChnAttr.eWorkMode = MI_LDC_WORKMODE_LDC;
            stLdcChnAttr.eInputBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
            STCHECKRESULT(MI_LDC_CreateChannel(g_u32LdcDevId, g_u32LdcChnId, &stLdcChnAttr));

            //refence ut config/infinity6f/json/ldc/base/ldc0@Case=0007,F=base,Item=normal,wall,widelen.json
            stAttr.bBgColor     = true;
            stAttr.u32BgColor   = 0xFFFFFF;
            stAttr.eMountMode   = MI_LDC_WALL_MOUNT;
            stAttr.u32RegionNum = 1;
            stAttr.stCalibInfo.pCalibPolyBinAddr    = g_stLdcChnAttr.stCalibBin.pVirAddr;
            stAttr.stCalibInfo.u32CalibPolyBinSize  = g_stLdcChnAttr.stCalibBin.u32BufSize;
            stAttr.stCalibInfo.s32CenterXOffset     = -16;
            stAttr.stCalibInfo.s32CenterXOffset     = -10;
            stAttr.stCalibInfo.s32FisheyeRadius     = 1130;
            pstRegionAttr = &stAttr.stRegionAttr[0];
            pstRegionAttr->eRegionMode          = MI_LDC_REGION_NORMAL;
            pstRegionAttr->stOutRect.u16X       = 0;
            pstRegionAttr->stOutRect.u16Y       = 0;
            pstRegionAttr->stOutRect.u16Width   = stSclOutputParam.stSCLOutputSize.u16Width;
            pstRegionAttr->stOutRect.u16Height  = stSclOutputParam.stSCLOutputSize.u16Height;
            pstRegionAttr->stRegionPara.eCropMode           = MI_LDC_REGION_CROP_NONE;
            pstRegionAttr->stRegionPara.s32Pan              = 0;
            pstRegionAttr->stRegionPara.s32Tilt             = 0;
            pstRegionAttr->stRegionPara.s32ZoomV            = 0;
            pstRegionAttr->stRegionPara.s32ZoomH            = 150;
            pstRegionAttr->stRegionPara.s32InRadius         = 0;
            pstRegionAttr->stRegionPara.s32FocalRatio       = 0;
            pstRegionAttr->stRegionPara.s32DistortionRatio  = 0;
            pstRegionAttr->stRegionPara.s32Rotate           = 0;
            pstRegionAttr->stRegionPara.s32OutRotate        = 0;
            STCHECKRESULT(MI_LDC_SetChnLDCAttr(g_u32LdcDevId, g_u32LdcChnId, &stAttr));

            MI_LDC_GetInputPortAttr(g_u32LdcDevId, g_u32LdcChnId, &stLdcInputPortAttr);
            stLdcInputPortAttr.u16Width = stSclOutputParam.stSCLOutputSize.u16Width;
            stLdcInputPortAttr.u16Height = stSclOutputParam.stSCLOutputSize.u16Height;
            STCHECKRESULT(MI_LDC_SetInputPortAttr(g_u32LdcDevId, g_u32LdcChnId, &stLdcInputPortAttr));

            MI_LDC_GetOutputPortAttr(g_u32LdcDevId, g_u32LdcChnId, &stLdcOutputPortAttr);
            stLdcOutputPortAttr.ePixelFmt = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            stLdcOutputPortAttr.u16Width = stSclOutputParam.stSCLOutputSize.u16Width;
            stLdcOutputPortAttr.u16Height = stSclOutputParam.stSCLOutputSize.u16Height;
            STCHECKRESULT(MI_LDC_SetOutputPortAttr(g_u32LdcDevId, g_u32LdcChnId, &stLdcOutputPortAttr));

            STCHECKRESULT(MI_LDC_StartChannel(g_u32LdcDevId, g_u32LdcChnId));
            ST_ReleaseLDCCfgFile();

            memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            #if 0 //bind isp
            stSrcChnPort.eModId = E_MI_MODULE_ID_ISP;
            stSrcChnPort.u32DevId = 0;
            stSrcChnPort.u32ChnId = pstStreamAttr[i].u32InputChn;
            stSrcChnPort.u32PortId = 1;
            stIspOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            STCHECKRESULT(MI_ISP_SetOutputPortParam(IspDevId, 0, stSrcChnPort.u32PortId, &stIspOutputParam));
            STCHECKRESULT(MI_ISP_EnableOutputPort(IspDevId, 0, stSrcChnPort.u32PortId));
            #else
            stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
            stSrcChnPort.u32DevId = SclDevId;
            stSrcChnPort.u32ChnId = SclChnId;
            stSrcChnPort.u32PortId = SclPortId;
            STCHECKRESULT(MI_SCL_EnableOutputPort(SclDevId, 0, stSrcChnPort.u32PortId));
            #endif
            stDstChnPort.eModId = E_MI_MODULE_ID_LDC;
            stDstChnPort.u32DevId = g_u32LdcDevId;
            stDstChnPort.u32ChnId = g_u32LdcChnId;
            stDstChnPort.u32PortId = 0;
            u32SrcFrmrate = pCameraBootSetting->u8SensorFrameRate;
            u32DstFrmrate = pCameraBootSetting->u8SensorFrameRate;
            STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, stLdcChnAttr.eInputBindType, 0));

            memset(&stLdcOutputAttr, 0x0, (size_t)sizeof(MI_LDC_OutputPortAttr_t));
            STCHECKRESULT(MI_LDC_GetOutputPortAttr(g_u32LdcDevId, g_u32LdcChnId, &stLdcOutputAttr));
            CamOsPrintf("ldc on:ldc w[%u]h[%u],venc w[%u]h[%u],set venc w[%u]h[%u]\n", stLdcOutputAttr.u16Width, stLdcOutputAttr.u16Height, pstStreamAttr[i].u32Width,pstStreamAttr[i].u32Height, stLdcOutputAttr.u16Width, stLdcOutputAttr.u16Height);
            pstStreamAttr[i].u32Width = stLdcOutputAttr.u16Width;
            pstStreamAttr[i].u32Height = stLdcOutputAttr.u16Height;
        }
#endif // INTERFACE_LDC
        /************************************************
        init VENC
        *************************************************/
        if(pstStreamAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
            VencDevId = MI_VENC_DEV_ID_JPEG_0;
        else
            VencDevId = MI_VENC_DEV_ID_H264_H265_0;

        memset(&stVencChnAttr, 0x0, (size_t)sizeof(MI_VENC_ChnAttr_t));
        STUB_GetVencConfig(pstStreamAttr[i].eType, &stVencChnAttr, i);

        tVencParam.u32MaxWidth = pCameraBootSetting->u32VencMhalMaxWidth;
        tVencParam.u32MaxHeight = pCameraBootSetting->u32VencMhalMaxHeight;
        MI_VENC_CreateDev(VencDevId, &tVencParam);
        STCHECKRESULT(MI_VENC_CreateChn(VencDevId, pstStreamAttr[i].vencChn, &stVencChnAttr));
        STCHECKRESULT(MI_VENC_SetMaxStreamCnt(VencDevId, pstStreamAttr[i].vencChn, u32FrameCnt));
        if(pstStreamAttr[i].eType != E_MI_VENC_MODTYPE_JPEGE && pstStreamAttr[i].eBindType != E_MI_SYS_BIND_TYPE_HW_RING)
        {
            stSuperFrameCfg.u32SuperIFrmBitsThr = _maxFrameSize[0].frameSizeI;
            stSuperFrameCfg.u32SuperPFrmBitsThr = _maxFrameSize[0].frameSizeP;
            STCHECKRESULT(MI_VENC_SetSuperFrameCfg(VencDevId, pstStreamAttr[i].u32InputChn, &stSuperFrameCfg));
        }
        if (pstStreamAttr[i].eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
        {
            if(pstStreamAttr[i].u32BindPara == 0 || pstStreamAttr[i].u32BindPara == pstStreamAttr[i].u32Height)
            {
                pstStreamAttr[i].u32BindPara = pstStreamAttr[i].u32Height;
            }
#if (defined MULTIRING_SUPPORT) && (MULTIRING_SUPPORT == 1)
            if (i == 0 && pstStreamAttr[i].eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
            {
                MI_SYS_GlobalPrivPoolConfig_t stConfig;
                memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

                stConfig.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
                stConfig.bCreate = TRUE;
                stConfig.uConfig.stpreDevPrivRingPoolConfig.eModule = E_MI_MODULE_ID_VENC;
                stConfig.uConfig.stpreDevPrivRingPoolConfig.u32Devid = VencDevId;
                #if (defined(CONFIG_SIGMASTAR_CHIP_SOUFFLE))
                stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth = stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth;
                stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight;
                stConfig.uConfig.stpreDevPrivRingPoolConfig.u16RingLine = stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight;
                #else
                stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth = stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth/2;
                stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight/2;
                stConfig.uConfig.stpreDevPrivRingPoolConfig.u16RingLine = stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight/2;
                #endif
                STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stConfig));
            }
#endif
        }

#if INTERFACE_LDC
        if((TRUE == pCameraBootSetting->u8enableLDC)&&(0 == i)) //for main stream
        {
            memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            stSrcChnPort.eModId = E_MI_MODULE_ID_LDC;
            stSrcChnPort.u32DevId = g_u32LdcDevId;
            stSrcChnPort.u32ChnId = g_u32LdcChnId;
            stSrcChnPort.u32PortId = 0;
            stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
            stDstChnPort.u32DevId = VencDevId;
            stDstChnPort.u32ChnId = pstStreamAttr[i].vencChn;
            stDstChnPort.u32PortId = 0;
            u32SrcFrmrate = pCameraBootSetting->u8SensorFrameRate;
            if(0 == i)
            {
                u32DstFrmrate = pCameraBootSetting->u8Video0EncFrameRate;
            }
            else if(1 == i)
            {
                u32DstFrmrate = pCameraBootSetting->u8Video1EncFrameRate;
            }
            else if(2 == i)
            {
                u32DstFrmrate = pCameraBootSetting->u8Video2EncFrameRate;
            }
            else
            {
                u32DstFrmrate = pCameraBootSetting->u8Video0EncFrameRate;
                CamOsPrintf("unsupport video ch[%u]\n", i);
            }
            STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate,E_MI_SYS_BIND_TYPE_FRAME_BASE, 0));

        }
        else
        {
            /************************************************
            Bind VPE->VENC
            *************************************************/
            memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
            stSrcChnPort.u32DevId = SclDevId;
            stSrcChnPort.u32ChnId = SclChnId;
            stSrcChnPort.u32PortId = SclPortId;
            stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
            stDstChnPort.u32DevId = VencDevId;
            stDstChnPort.u32ChnId = pstStreamAttr[i].vencChn;
            stDstChnPort.u32PortId = 0;
            u32SrcFrmrate = pCameraBootSetting->u8SensorFrameRate;
            if(0 == i)
            {
                u32DstFrmrate = pCameraBootSetting->u8Video0EncFrameRate;
            }
            else if(1 == i)
            {
                u32DstFrmrate = pCameraBootSetting->u8Video1EncFrameRate;
            }
            else if(2 == i)
            {
                u32DstFrmrate = pCameraBootSetting->u8Video2EncFrameRate;
            }
            else
            {
                u32DstFrmrate = pCameraBootSetting->u8Video0EncFrameRate;
                CamOsPrintf("unsupport video ch[%u]\n", i);
            }
#if defined(CONFIG_SIGMASTAR_CHIP_MARUKO)
            if (pstStreamAttr[i].eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
            {
                pstStreamAttr[i].eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
                CamOsPrintf("I6E don't support ringmode\n");
            }
#endif
            STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, pstStreamAttr[i].eBindType, u32BindParam));
        }
#else
        /************************************************
        Bind VPE->VENC
        *************************************************/
        memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId = SclDevId;
        stSrcChnPort.u32ChnId = SclChnId;
        stSrcChnPort.u32PortId = SclPortId;
        stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId = VencDevId;
        stDstChnPort.u32ChnId = pstStreamAttr[i].vencChn;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate = pCameraBootSetting->u8SensorFrameRate;
        if(0 == i)
        {
            u32DstFrmrate = pCameraBootSetting->u8Video0EncFrameRate;
        }
        else if(1 == i)
        {
            u32DstFrmrate = pCameraBootSetting->u8Video1EncFrameRate;
        }
        else if(2 == i)
        {
            u32DstFrmrate = pCameraBootSetting->u8Video2EncFrameRate;
        }
        else
        {
            u32DstFrmrate = pCameraBootSetting->u8Video0EncFrameRate;
            CamOsPrintf("unsupport video ch[%u]\n", i);
        }
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, pstStreamAttr[i].eBindType, u32BindParam));
#endif // INTERFACE_LDC
        STCHECKRESULT(MI_ISP_EnableOutputPort(IspDevId, IspChnId, IspPortId));
        STCHECKRESULT(MI_SCL_EnableOutputPort(SclDevId, SclChnId, SclPortId));

        STCHECKRESULT(MI_VENC_StartRecvPic(VencDevId, pstStreamAttr[i].vencChn));
        CamOsPrintf("chn %d startPipeLine:vpeChn[%d],vpePort[%d],vencChn[%d],venc bindtype[%d]\n", i,
                pstStreamAttr[i].u32InputChn,pstStreamAttr[i].u32InputPort,pstStreamAttr[i].vencChn,pstStreamAttr[i].eBindType);
#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)
        // attach
        // create osd, scl port 2 has't OSD
        if(2 != pstStreamAttr[i].u32InputPort)
        {
            if(TRUE == pCameraBootSetting->u8RegionsCun)
            {
                memset(&stAttachChnPort, 0, (size_t)sizeof(MI_RGN_ChnPort_t));
                stAttachChnPort.eModId = E_MI_MODULE_ID_SCL;
                stAttachChnPort.s32DevId = SclDevId;
                stAttachChnPort.s32ChnId =  SclChnId;
                stAttachChnPort.s32PortId = SclPortId;
                memset(&stChnPortParam, 0, (size_t)sizeof(MI_RGN_ChnPortParam_t));
                stChnPortParam.bShow = TRUE;
                stChnPortParam.stPoint.u32X = 10;
                stChnPortParam.stPoint.u32Y = 10;
                stChnPortParam.unPara.stOsdChnPort.u32Layer = VPE_PORT0_OSD_FOR_TIME_HANDLE;
                stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
                stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;
                stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 0xFF;
                STCHECKRESULT(MI_RGN_AttachToChn(0, VPE_PORT0_OSD_FOR_TIME_HANDLE, &stAttachChnPort, &stChnPortParam));
                stChnPortParam.bShow = TRUE;
                stChnPortParam.stPoint.u32X = 10;
                stChnPortParam.stPoint.u32Y = 100;
                stChnPortParam.unPara.stOsdChnPort.u32Layer = VPE_PORT0_OSD_FOR_PIC_HANDLE;
                stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
                stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;
                stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 0xFF;
                STCHECKRESULT(MI_RGN_AttachToChn(0, VPE_PORT0_OSD_FOR_PIC_HANDLE, &stAttachChnPort, &stChnPortParam));

            }
        }
#endif
    }
    else if(pstStreamAttr[i].enInput == ST_Sys_Input_VENC)
    {
        /************************************************
        init SUB VENC
        *************************************************/
        if(pstStreamAttr[i].eType == E_MI_VENC_MODTYPE_H264E || pstStreamAttr[i].eType == E_MI_VENC_MODTYPE_H265E)
        {
            VencDevId = MI_VENC_DEV_ID_H264_H265_0;
            memset(&stVencChnAttr, 0x0, (size_t)sizeof(MI_VENC_ChnAttr_t));
            STUB_GetVencConfig(pstStreamAttr[i].eType, &stVencChnAttr, i);
            STCHECKRESULT(MI_VENC_CreateChn(VencDevId, pstStreamAttr[i].vencChn, &stVencChnAttr));
            STCHECKRESULT(MI_VENC_SetMaxStreamCnt(VencDevId, pstStreamAttr[i].vencChn, u32FrameCnt));
            /************************************************
            Bind VENC->SUB VENC
            *************************************************/
            memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            stSrcChnPort.eModId = E_MI_MODULE_ID_VENC;
            stSrcChnPort.u32DevId = VencDevId;
            stSrcChnPort.u32ChnId = pstStreamAttr[i].u32InputChn;
            stSrcChnPort.u32PortId = pstStreamAttr[i].u32InputPort;
            stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
            stDstChnPort.u32DevId = VencDevId;
            stDstChnPort.u32ChnId = pstStreamAttr[i].vencChn;
            stDstChnPort.u32PortId = 0;
            u32SrcFrmrate = pCameraBootSetting->u8SensorFrameRate;
            u32DstFrmrate = pCameraBootSetting->u8SensorFrameRate;
            STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, pstStreamAttr[i].eBindType, u32BindParam));
            STCHECKRESULT(MI_VENC_StartRecvPic(VencDevId, pstStreamAttr[i].vencChn));
            CamOsPrintf("chn %d startPipeLine:vpeChn[%d],vpePort[%d],vencChn[%d],venc bindtype[%d]\n", i,
                pstStreamAttr[i].u32InputChn,pstStreamAttr[i].u32InputPort,pstStreamAttr[i].vencChn,pstStreamAttr[i].eBindType);
        }
    }
    return MI_SUCCESS;
}

static MI_S32 ST_ExitPipeLine(void)
{
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MI_SCL_DEV SclDevId = 0;
    MI_SCL_CHANNEL SclChnId = 0;
    MI_U8 SclPortId = 0;
    MI_VENC_DEV VencDevId = 0;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U8 i = 0;
    MI_U8 u8VideoNum = pCameraBootSetting->u8VideoNum;


    /************************************************
    Destroy VENC
    *************************************************/
    for(i = 0; i < u8VideoNum; i ++)
    {
        if(pstStreamAttr[i].enInput == ST_Sys_Input_SCL)
        {
            if(2 == pCameraBootSetting->u8SensorNum)
            {
                SclChnId = pstStreamAttr[i].u32InputChn;
                SclPortId = pstStreamAttr[i].u32InputPort;
            }
            else
            {
                SclChnId = 0;
                SclPortId = i;
            }

            if(pstStreamAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
                VencDevId = MI_VENC_DEV_ID_JPEG_0;
            else
                VencDevId = MI_VENC_DEV_ID_H264_H265_0;
            memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
            stSrcChnPort.u32DevId = SclDevId;
            stSrcChnPort.u32ChnId = SclChnId;
            stSrcChnPort.u32PortId = SclPortId;
            stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
            stDstChnPort.u32DevId = VencDevId;
            stDstChnPort.u32ChnId = pstStreamAttr[i].vencChn;
            stDstChnPort.u32PortId = 0;
            MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort);
            MI_VENC_StopRecvPic(VencDevId, pstStreamAttr[i].vencChn);
            MI_VENC_DestroyChn(VencDevId, pstStreamAttr[i].vencChn);
        }
        else if(pstStreamAttr[i].enInput == ST_Sys_Input_VENC)
        {
            VencDevId = MI_VENC_DEV_ID_H264_H265_0;
            memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            stSrcChnPort.eModId = E_MI_MODULE_ID_VENC;
            stSrcChnPort.u32DevId = VencDevId;
            stSrcChnPort.u32ChnId = pstStreamAttr[i].u32InputChn;
            stSrcChnPort.u32PortId = pstStreamAttr[i].u32InputPort;
            stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
            stDstChnPort.u32DevId = VencDevId;
            stDstChnPort.u32ChnId = pstStreamAttr[i].vencChn;
            stDstChnPort.u32PortId = 0;
            STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));
            MI_VENC_StopRecvPic(VencDevId, pstStreamAttr[i].vencChn);
            MI_VENC_DestroyChn(VencDevId, pstStreamAttr[i].vencChn);
        }
    }
    return MI_SUCCESS;
}

static MI_VENC_ModType_e ST_GetVideoModType(MI_U8 u8VideoFormat)
{
    MI_VENC_ModType_e eVideoModType = E_MI_VENC_MODTYPE_H264E;
    switch(u8VideoFormat)
    {
        case 0:
            eVideoModType = E_MI_VENC_MODTYPE_H264E;
            break;
        case 1:
            eVideoModType = E_MI_VENC_MODTYPE_H265E;
            break;
        case 2:
            eVideoModType = E_MI_VENC_MODTYPE_JPEGE;
            break;
        default:
            CamOsPrintf("unsupport VideoFormat[%u]\n", u8VideoFormat);
            eVideoModType = E_MI_VENC_MODTYPE_H264E;
            break;
    }
    return eVideoModType;
}

static void ST_ResetArgs(void)
{
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    MI_U32 u32ArraySize = ARRAY_SIZE(g_stStreamAttr);
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MI_U32 u32Temp = 0;
    MI_U8 i = 0;

    pstStreamAttr[0].eType = ST_GetVideoModType(pCameraBootSetting->u8Video0Format);
    pstStreamAttr[0].eBindType = pCameraBootSetting->u8Video0BindType;
    pstStreamAttr[0].u32Mbps = pCameraBootSetting->u32Video0Bitrate;
    pstStreamAttr[0].u32Width = pCameraBootSetting->u16Video0Width;
    pstStreamAttr[0].u32Height = pCameraBootSetting->u16Video0Height;
    pstStreamAttr[0].u32MaxWidth = pCameraBootSetting->u16Video0MaxWidth;
    pstStreamAttr[0].u32MaxHeight = pCameraBootSetting->u16Video0MaxHeight;
    pstStreamAttr[0].u32CropX = pCameraBootSetting->u16Video0VPECropx;
    pstStreamAttr[0].u32CropY = pCameraBootSetting->u16Video0VPECropy;
    pstStreamAttr[0].u32CropWidth = pCameraBootSetting->u16Video0VPECropw;
    pstStreamAttr[0].u32CropHeight = pCameraBootSetting->u16Video0VPECroph;

    pstStreamAttr[1].eType = ST_GetVideoModType(pCameraBootSetting->u8Video1Format);
    pstStreamAttr[1].u32Mbps = pCameraBootSetting->u32Video1Bitrate;
    pstStreamAttr[1].u32Width = pCameraBootSetting->u16Video1Width;
    pstStreamAttr[1].u32Height = pCameraBootSetting->u16Video1Height;
    pstStreamAttr[1].u32MaxWidth = pCameraBootSetting->u16Video1Width;
    pstStreamAttr[1].u32MaxHeight = pCameraBootSetting->u16Video1Height;
    pstStreamAttr[1].u32CropX = pCameraBootSetting->u16Video1VPECropx;
    pstStreamAttr[1].u32CropY = pCameraBootSetting->u16Video1VPECropy;
    pstStreamAttr[1].u32CropWidth = pCameraBootSetting->u16Video1VPECropw;
    pstStreamAttr[1].u32CropHeight = pCameraBootSetting->u16Video1VPECroph;

    pstStreamAttr[2].eType = ST_GetVideoModType(pCameraBootSetting->u8Video2Format);
    pstStreamAttr[2].u32Mbps = pCameraBootSetting->u32Video2Bitrate;
    pstStreamAttr[2].u32Width = pCameraBootSetting->u16Video2Width;
    pstStreamAttr[2].u32Height = pCameraBootSetting->u16Video2Height;
    pstStreamAttr[2].u32MaxWidth = pCameraBootSetting->u16Video2Width;
    pstStreamAttr[2].u32MaxHeight = pCameraBootSetting->u16Video2Height;
    pstStreamAttr[2].u32CropX = pCameraBootSetting->u16Video2VPECropx;
    pstStreamAttr[2].u32CropY = pCameraBootSetting->u16Video2VPECropy;
    pstStreamAttr[2].u32CropWidth = pCameraBootSetting->u16Video2VPECropw;
    pstStreamAttr[2].u32CropHeight = pCameraBootSetting->u16Video2VPECroph;
    if(2 == pCameraBootSetting->u8SensorNum)
    {
        pstStreamAttr[0].enInput = 0;
        pstStreamAttr[0].u32InputChn = 0;
        pstStreamAttr[0].u32InputPort = 0;
        pstStreamAttr[0].vencChn = 0;
        pstStreamAttr[1].enInput = 0;
        pstStreamAttr[1].u32InputChn = 1;
        pstStreamAttr[1].u32InputPort = 0;
        pstStreamAttr[1].vencChn = 1;
 #if (defined MULTIRING_SUPPORT) && (MULTIRING_SUPPORT == 1)
        pstStreamAttr[1].eBindType = E_MI_SYS_BIND_TYPE_HW_RING;
#endif

    }
    if((1 == pCameraBootSetting->u8Orientation) || (3 == pCameraBootSetting->u8Orientation))
    {
        for(i = 0; i < u32ArraySize; i ++)
        {
            u32Temp = pstStreamAttr[i].u32Width;
            pstStreamAttr[i].u32Width = CAM_OS_ALIGN_DOWN(pstStreamAttr[i].u32Height, 8);
            pstStreamAttr[i].u32Height = CAM_OS_ALIGN_DOWN(u32Temp, 2);
        }
    }
    #if defined(CONFIG_VDF_IN_RTOS_ENABLE)
    if(TRUE == pCameraBootSetting->u8enableVDF)
    {
        ST_ModuleInit_VDF_MDOD_Rect();
    }
    #endif
}

static void ST_DeinitMoudle(void)
{
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MI_SNR_PADID u8SnrPad = 0;
    MI_VIF_DEV VifDevId = 0;
    MI_VIF_GROUP nVCapGroup = 0;
    MI_ISP_DEV IspDevId = 0;
    MI_SCL_DEV SclDevId = 0;
    MI_VENC_DEV VencDevId = 0;
    MI_SCL_CHANNEL SclChnId = 0;
    MI_ISP_CHANNEL IspChnId = 0;
    MI_U8 IspPortId = 0;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_SYS_GlobalPrivPoolConfig_t stConfig;
    MI_U8 i = 0;
    MI_U8 u8SensorNum = pCameraBootSetting->u8SensorNum;

#if (defined MULTIRING_SUPPORT) && (MULTIRING_SUPPORT == 1)
    if(pstStreamAttr[0].eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
    {
        memset(&stConfig, 0x0, sizeof(MI_SYS_GlobalPrivPoolConfig_t));

        stConfig.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
        stConfig.bCreate = FALSE;
        stConfig.uConfig.stpreDevPrivRingPoolConfig.eModule = E_MI_MODULE_ID_VENC;
        stConfig.uConfig.stpreDevPrivRingPoolConfig.u32Devid = VencDevId;
        #if (defined(CONFIG_SIGMASTAR_CHIP_I6F))
        stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth = pstStreamAttr[0].u32MaxWidth;
        stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = pstStreamAttr[0].u32MaxHeight;
        stConfig.uConfig.stpreDevPrivRingPoolConfig.u16RingLine = pstStreamAttr[0].u32MaxHeight;
        #else
        stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth = pstStreamAttr[0].u32MaxWidth/2;
        stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = pstStreamAttr[0].u32MaxHeight/2;
        stConfig.uConfig.stpreDevPrivRingPoolConfig.u16RingLine = pstStreamAttr[0].u32MaxHeight/2;
        #endif
        MI_SYS_ConfigPrivateMMAPool(0, &stConfig);
    }
#endif
    for(i = 0; i < u8SensorNum; i++)
    {
        u8SnrPad = i*2;//2+2lane

        IspChnId = i;
        SclChnId = i;
        switch(u8SnrPad)
        {
            case 0:
                VifDevId = 0;
                break;
            case 1:
                VifDevId = 8;
                break;
            case 2:
                VifDevId = 4;
                break;
            case 3:
                VifDevId = 12;
                break;
            default:
                VifDevId = 0;
        }
#if INTERFACE_LDC
        if((TRUE == pCameraBootSetting->u8enableLDC)) //for main stream
        {
            memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            #if 0
            stSrcChnPort.eModId = E_MI_MODULE_ID_ISP;
            stSrcChnPort.u32DevId = IspDevId;
            stSrcChnPort.u32ChnId = 0;
            stSrcChnPort.u32PortId = 1;
            #else
            stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
            stSrcChnPort.u32DevId = SclDevId;
            stSrcChnPort.u32ChnId = SclChnId;
            stSrcChnPort.u32PortId = 0;
            #endif
            stDstChnPort.eModId = E_MI_MODULE_ID_LDC;
            stDstChnPort.u32DevId = g_u32LdcDevId;
            stDstChnPort.u32ChnId = g_u32LdcChnId;
            stDstChnPort.u32PortId = 0;
            MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort);
            MI_LDC_StopChannel(stDstChnPort.u32DevId, stDstChnPort.u32ChnId);
            MI_LDC_DestroyChannel(stDstChnPort.u32DevId, stDstChnPort.u32ChnId);
            MI_LDC_DestroyDevice(stDstChnPort.u32DevId);
        }
#endif // INTERFACE_LDC
        memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId = IspDevId;
        stSrcChnPort.u32ChnId = IspChnId;
        stSrcChnPort.u32PortId = IspPortId;
        stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId = SclDevId;
        stDstChnPort.u32ChnId = SclChnId;
        stDstChnPort.u32PortId = 0;
        MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort);

        memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
        stSrcChnPort.u32DevId = VifDevId;
        stSrcChnPort.u32ChnId = 0;
        stSrcChnPort.u32PortId = 0;
        stDstChnPort.eModId = E_MI_MODULE_ID_ISP;
        stDstChnPort.u32DevId = IspDevId;
        stDstChnPort.u32ChnId = IspChnId;
        stDstChnPort.u32PortId = 0;
        MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort);
        /************************************************
        Destroy SCL
        *************************************************/
        MI_SCL_StopChannel(SclDevId, SclChnId);
        MI_SCL_DestroyChannel(SclDevId, SclChnId);
    #if (defined MULTIRING_SUPPORT) && (MULTIRING_SUPPORT == 1)
        if(i == u8SensorNum -1 && pstStreamAttr[0].eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
        {
            memset(&stConfig, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

            stConfig.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
            stConfig.bCreate = FALSE;
            stConfig.uConfig.stpreDevPrivRingPoolConfig.eModule = E_MI_MODULE_ID_SCL;
            stConfig.uConfig.stpreDevPrivRingPoolConfig.u32Devid = SclDevId;
            stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth = pstStreamAttr[0].u32MaxWidth;
            stConfig.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = pstStreamAttr[0].u32MaxHeight;
            stConfig.uConfig.stpreDevPrivRingPoolConfig.u16RingLine = pstStreamAttr[0].u32MaxHeight;

            MI_SYS_ConfigPrivateMMAPool(0, &stConfig);
        }
    #endif

        /************************************************
        Destroy ISP
        ******************************************/
        CamOsPrintf("%d,%d--------------------------\r\n",IspDevId,IspChnId);
        MI_ISP_StopChannel(IspDevId, IspChnId);
        MI_ISP_DestroyChannel(IspDevId, IspChnId);

        /************************************************
        Destroy VIF
        *************************************************/
        MI_VIF_DisableOutputPort(VifDevId, 0);
        MI_VIF_DisableDev(VifDevId);
        GetVifPadMapGroup(u8SnrPad, &nVCapGroup);
        MI_VIF_DestroyDevGroup(nVCapGroup);
    }
    MI_SCL_DestroyDevice(SclDevId);
    MI_ISP_DestoryDevice(IspDevId);

    MI_VENC_DestroyDev(MI_VENC_DEV_ID_JPEG_0);
    MI_VENC_DestroyDev(MI_VENC_DEV_ID_H264_H265_0);
}

#define MI_CLICMD_PRELOAD 0
static int _MI_Cli_GetDataL2R(void *data)
{
    char *string = (char *)data;
    int len = 0;
    len = strlen((const char *)string);
    if(len > 0)
    {
        printf("recv from linux data: %s\n", string);
    }
    else
    {
        printf("recv from linux data is null\n");
    }

    return 0;
}


static void _Exit_PreloadForce(void)
{
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();

#if defined(CONFIG_IPU_IN_RTOS_ENABLE)
    g_u8dlaosdoff = 1;
    CamOsPrintf("%s g_u8dlaosdoff:%d\n",__FUNCTION__, g_u8dlaosdoff);
    FD_deinit();
    if(TRUE == pCameraBootSetting->u8enableIPU)
    {
    }
#endif
#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)
    g_u8timeosdoff = 1;
    CamOsPrintf("%s g_u8timeosdoff:%d\n",__FUNCTION__, g_u8timeosdoff);
    if(pCameraBootSetting->u8RegionsCun)
    {
        ST_OSDTimer_Exit();
    }
#endif
    if(pCameraBootSetting->u8enableVDF)
    {
#if defined(CONFIG_VDF_IN_RTOS_ENABLE)
        CamOsPrintf("%s vdfoff\n",__FUNCTION__);
        for(MI_U8 j = 0 ; j < MAX_FULL_RGN_NULL ; j ++)
        {
            g_stVdfThreadArgs[j].bRunFlag = FALSE;
        }
#endif
    }
#if defined(CONFIG_IPU_IN_RTOS_ENABLE) || defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP) || defined(CONFIG_VDF_IN_RTOS_ENABLE)
    ST_OSD_Deinit();
#endif

#ifdef CONFIG_PANEL_IN_RTOS_ENABLE
    MI_U8 u8DevIndex = 0;
    MI_U8 u8LayerIndex = 0;
    MI_U8 u8PortIndex = 0;
    for(u8DevIndex = 0; u8DevIndex < DISP_DEV_MAX; u8DevIndex++)
    {
        if(g_astDispUtDev[u8DevIndex].bDevEnable == TRUE)
        {
            for(u8LayerIndex = 0; u8LayerIndex < DISP_LAYER_MAX; u8LayerIndex++)
            {
                if(g_astDispUtLayer[u8LayerIndex].bLayerEnable == TRUE)
                {
                    for(u8PortIndex = 0; u8PortIndex < DISP_INPUT_PORT_MAX; u8PortIndex++)
                    {
                        if(g_astDispUtPort[u8LayerIndex][u8PortIndex].bPortEnable == TRUE)
                        {
                            disp_ut_disableport(u8LayerIndex, u8PortIndex);
                            CamOsPrintf("disable layerid:%d portid:%d\n",u8LayerIndex,u8PortIndex);
                        }
                    }
                    disp_ut_disablelayer(u8DevIndex, u8LayerIndex);
                    CamOsPrintf("disable layerid:%d\n",u8LayerIndex);
                }
            }
            disp_ut_disabledev(u8DevIndex);
            CamOsPrintf("disable devid:%d\n",u8DevIndex);
        }
    }
#endif
#if INTERFACE_VDISP
    MI_VDISP_Exit();
#endif

    MI_DEVICE_RegMiSysExitCall(NULL);
    MI_DEVICE_RegMiCliGetDataL2R(MI_CLICMD_PRELOAD, NULL);
    CamOsTsemDeinit(&tPreloadFileTsem);
    CamOsTsemDeinit(&tIspReadFileTsem);
    CamOsThreadStop(PreloadFile_tid);
    CamOsThreadStop(PreloadMiPipe_tid);
}

#ifdef _LPC_SUPPORT_

#define VPE_DEV                 0
#define VPE_OUTPUT_PORT         0
#define FRAME_RATE              5
#define VENC_DEV                0
#define VENC_PORT               0
#define PIXEL_FORMAT            (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(E_MI_SYS_DATA_PRECISION_8BPP, E_MI_SYS_PIXEL_BAYERID_BG)
#define PAG7290_1CH_SENSOR_ID   0x0
#define PAG7290_2CH_SENSOR_ID   0x1
#define PAG7290_3CH_SENSOR_ID   0x2
#define HM360_SENSOR_ID         0x3


static int _InitLpcCfg(u8 SensorID, u8 Path, LpcInitParam_t *pCfg)
{
    pCfg->uMdSrc                            = (Path == LPC_PATH_RESERVED2) ? LPC_MD_SRC_RESERVED2 : LPC_MD_SRC_BAYERGAMMA;

    //IMI (MCU)
    pCfg->uMcuCodeSize                      = 8*8*1024;     //65536
    pCfg->uMcuXdataSize                     = 3*8*1024;     //24576

    //SP_Timer
    pCfg->uSPTimer0Timeout                  = 0x2000000;
    pCfg->uSPTimer1Timeout                  = 0xC00000*2;   //2Sec

    //PIR
    pCfg->uPirSerInPadSel                   = 3;
    pCfg->uPirDirLinkPadSel                 = 3;
    pCfg->PirSerInCfg.Sensitivity           = 0x20;     //0..255
    pCfg->PirSerInCfg.BlindTime             = 0x03;     //0..15
    pCfg->PirSerInCfg.PulseCounter          = 0x00;     //0..3
    pCfg->PirSerInCfg.WindowTime            = 0x00;     //0..3
    pCfg->PirSerInCfg.OperationMode         = 0x02;     //0..3
    pCfg->PirSerInCfg.FilterSource          = 0x00;     //0..3
    pCfg->PirSerInCfg.Reserved              = 0x10;     //0..31

    //Host2Mcu
    pCfg->uIspFuncEn[0]                     = 1;                            // Ae Enable
    pCfg->uIspFuncEn[1]                     = (Path == LPC_PATH_RESERVED2)?1:0;     // Awb Enable
    pCfg->uIspFuncEn[2]                     = (Path == LPC_PATH_RESERVED2)?1:0;     // Iq Enable
    pCfg->uIspFuncEn[3]                     = ((Path == LPC_PATH_RESERVED2)&&(SensorID >= LPC_SNR_1x_HM0360))?1:0; // Color Enable, by Sensor
    pCfg->uMdEn                             = 1;
    pCfg->uMdSwitch                         = pCfg->uIspFuncEn[0];          // Ae Enable
    pCfg->uMdAlgorithm                      = 1;
    pCfg->uPirEn                            = 1;
    pCfg->uPirAlgorithm                     = 1;
    pCfg->uPirChannel                       = 3;

    /*MD Day*/
    pCfg->MdNightConfig.MdBlkSize[0]        = 0x2; //DrvMDMbSize_16x16;
    pCfg->MdNightConfig.MdSadThd[0]         = 18;
    pCfg->MdNightConfig.MdBgImgLearnRate[0] = 0;
    pCfg->MdNightConfig.MdBlkMinThd[0]      = 9;
    pCfg->MdNightConfig.MdBlkMaxThd[0]      = 99;
    pCfg->MdNightConfig.MdBlkSize[1]        = 0x0; //DrvMDMbSize_4x4;
    pCfg->MdNightConfig.MdSadThd[1]         = 19;
    pCfg->MdNightConfig.MdBgImgLearnRate[1] = 3;
    pCfg->MdNightConfig.MdBlkMinThd[1]      = 9;
    pCfg->MdNightConfig.MdBlkMaxThd[1]      = 99;
    pCfg->MdNightConfig.MdBlkSize[2]        = 0x1; //DrvMDMbSize_8x8;
    pCfg->MdNightConfig.MdSadThd[2]         = 20;
    pCfg->MdNightConfig.MdBgImgLearnRate[2] = 7;
    pCfg->MdNightConfig.MdBlkMinThd[2]      = 9;
    pCfg->MdNightConfig.MdBlkMaxThd[2]      = 99;

    /*MD Night*/
    pCfg->MdNightConfig.MdBlkSize[0]        = 0x2; //DrvMDMbSize_16x16;
    pCfg->MdNightConfig.MdSadThd[0]         = 18;
    pCfg->MdNightConfig.MdBgImgLearnRate[0] = 0;
    pCfg->MdNightConfig.MdBlkMinThd[0]      = 9;
    pCfg->MdNightConfig.MdBlkMaxThd[0]      = 99;
    pCfg->MdNightConfig.MdBlkSize[1]        = 0x0; //DrvMDMbSize_4x4;
    pCfg->MdNightConfig.MdSadThd[1]         = 19;
    pCfg->MdNightConfig.MdBgImgLearnRate[1] = 3;
    pCfg->MdNightConfig.MdBlkMinThd[1]      = 9;
    pCfg->MdNightConfig.MdBlkMaxThd[1]      = 99;
    pCfg->MdNightConfig.MdBlkSize[2]        = 0x1; //DrvMDMbSize_8x8;
    pCfg->MdNightConfig.MdSadThd[2]         = 20;
    pCfg->MdNightConfig.MdBgImgLearnRate[2] = 7;
    pCfg->MdNightConfig.MdBlkMinThd[2]      = 9;
    pCfg->MdNightConfig.MdBlkMaxThd[2]      = 99;

    //JPE
    pCfg->uJpeDelaySec                      = (Path == LPC_PATH_RESERVED2)?5:0;  //UNIT:Sec

    //WDT
    pCfg->uWdtIntSec                        = 30;       //0: disable WDT, other: If WDT is not cleared within N seconds, WDT_INT will be triggered

    //Pad
    pCfg->uPwrEn                            = 0;
    pCfg->uPwrGpio                          = 0;
    pCfg->uRstGpio                          = 7;

    return 0;
}

static int initVpeVenc(u8 u8VpeChn, u8 u8VencChn, u32 u32FrameCnt, MI_VPE_ChannelAttr_t* pstVpeChnAttr, MI_VENC_ChnAttr_t* pstVencChnAttr, MI_SYS_WindowRect_t* pstCropWin, MI_VENC_SuperFrameCfg_t* pstSuperFrameCfg)
{
    MI_VPE_PortMode_t stVpeMode;
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    /********************************************/
    /*  Create VPE Channel                      */
    /********************************************/
    STCHECKRESULT(MI_VPE_CreateChannel(u8VpeChn, pstVpeChnAttr));
    STCHECKRESULT(MI_VPE_StartChannel (u8VpeChn));

    /********************************************/
    /*  Enable VPE Output Port                  */
    /********************************************/
    memset(&stVpeMode, 0, (size_t)sizeof(stVpeMode));
    STCHECKRESULT(MI_VPE_GetPortMode(u8VpeChn, VPE_OUTPUT_PORT, &stVpeMode));
    stVpeMode.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stVpeMode.u16Width = pstCropWin->u16Width;
    stVpeMode.u16Height= pstCropWin->u16Height;
    STCHECKRESULT(MI_VPE_SetPortMode(u8VpeChn, VPE_OUTPUT_PORT, &stVpeMode));
    STCHECKRESULT(MI_VPE_EnablePort(u8VpeChn, VPE_OUTPUT_PORT));

    /********************************************/
    /*  Create VENC Channel                     */
    /********************************************/
    STCHECKRESULT(MI_VENC_CreateChn(u8VencChn, pstVencChnAttr));
    STCHECKRESULT(MI_VENC_SetMaxStreamCnt(u8VencChn, u32FrameCnt));
    STCHECKRESULT(MI_VENC_StartRecvPic(u8VencChn));

    STCHECKRESULT(MI_VENC_SetSuperFrameCfg(u8VencChn, pstSuperFrameCfg));

    /********************************************/
    /*  Bind VPE->VENC                          */
    /********************************************/
    memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId = E_MI_MODULE_ID_VPE;
    stSrcChnPort.u32DevId = VPE_DEV;
    stSrcChnPort.u32ChnId = u8VpeChn;
    stSrcChnPort.u32PortId = VPE_OUTPUT_PORT;
    stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
    stDstChnPort.u32DevId = VENC_DEV;
    stDstChnPort.u32ChnId = u8VencChn;
    stDstChnPort.u32PortId = VENC_PORT;
    STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, FRAME_RATE, FRAME_RATE, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0));

    return 0;
}

static MI_S32 lpcPipelineInit(void)
{
    /********************************************/
    /*  MI Init                                 */
    /********************************************/
    MI_SYS_Init(0);

    CamOsMsSleep(500);

    /********************************************/
    /*  Check LPC Status                        */
    /********************************************/
    LpcResetParam_t tResetParam;
    LpcCheckWakeupEventParam_t pParam;
    MHalLpc_CheckWakeupEvent(&pParam);
    CamOsPrintf("[LPC] %s: Event Id = %u\n", __func__, pParam.uEventId);

    /********************************************/
    /*  Init LPC                                */
    /********************************************/
    LpcInitParam_t tInitParam;
    memset(&tInitParam, 0, (size_t)sizeof(tInitParam));
    if(pParam.uEventId == 0)    // No Init
    {
        CamOsPrintf("[LPC] %s: Reset and Init LPC\n", __func__);
        // Reset
        tResetParam.uReserved = 0;
        MHalLpc_Reset(&tResetParam);

        // Init
        /*Sensor and LPC pipe config*/
        tInitParam.uPath = LPC_PATH_1;
        tInitParam.uSensorID = HM360_SENSOR_ID;
        _InitLpcCfg(tInitParam.uSensorID, tInitParam.uPath, &tInitParam);

        MHalLpc_Init(&tInitParam);

        // Run
        LpcRunParam_t tRunParam;
        tRunParam.uOneShootMode = 0;
        MHalLpc_Run(&tRunParam);

        CamOsMsSleep(5000);
    }
    else if(pParam.uEventId == 1)
    {
        CamOsPrintf("[LPC] %s: Motion Detected!\n", __func__);
        tInitParam.uPath = LPC_PATH_1;
        tInitParam.uSensorID = HM360_SENSOR_ID;
        _InitLpcCfg(tInitParam.uSensorID, tInitParam.uPath, &tInitParam);

        MHalLpc_Restore(&tInitParam);
    }

    /********************************************/
    /* Get LPC Frame information                */
    /********************************************/
    u32 u32ChnCnt, u32FrameCnt, u32FrameWidth, u32FrameHeight, u32FrameSize;

    LpcStopParam_t tStopParam;
    tStopParam.uReserved = 0;
    MHalLpc_Stop(&tStopParam);

    LpcQueryFrameQueueInfoParam_t tFrameQueueInfo;
    MHalLpc_QueryFrameQueueInfo(&tFrameQueueInfo);

    u32ChnCnt = tFrameQueueInfo.uNumCh;
    u32FrameCnt = tFrameQueueInfo.uNumFrameSet;

    LpcQueryFrameInfoParam_t tFrameInfo;
    tFrameInfo.uCh = 0;
    tFrameInfo.uFrameID = 0;
    MHalLpc_QueryFrameInfo(&tFrameInfo);

    u32FrameWidth = tFrameInfo.uImgW;
    u32FrameHeight = tFrameInfo.uImgH;
    u32FrameSize = u32FrameWidth * u32FrameHeight;

    CamOsPrintf("[LPC] %s: ChnCnt = %u, FrameCnt = %u, FrameRes = %ux%u, FrameSize = %u\n",
            __func__, u32ChnCnt, u32FrameCnt, u32FrameWidth, u32FrameHeight, u32FrameSize);

    /********************************************/
    /*  Init VPE Channel Attr Structur          */
    /********************************************/
    MI_SYS_WindowRect_t stCropWin = {0, 0, u32FrameWidth, u32FrameHeight};
    MI_VPE_ChannelAttr_t stVpeChnAttr;
    memset(&stVpeChnAttr, 0x0, (size_t)sizeof(MI_VPE_ChannelAttr_t));
    stVpeChnAttr.u16MaxW = u32FrameWidth;
    stVpeChnAttr.u16MaxH = u32FrameHeight;
    stVpeChnAttr.bNrEn= FALSE;
    stVpeChnAttr.bEdgeEn= FALSE;
    stVpeChnAttr.bEsEn= FALSE;
    stVpeChnAttr.bContrastEn= FALSE;
    stVpeChnAttr.bUvInvert= FALSE;
    stVpeChnAttr.ePixFmt = PIXEL_FORMAT;
    stVpeChnAttr.eHDRType = E_MI_VPE_HDR_TYPE_OFF;
    stVpeChnAttr.eSensorBindId = 0;
    stVpeChnAttr.eRunningMode = E_MI_VPE_RUN_CAM_MODE;
    stVpeChnAttr.u32ChnPortMode = 7;

    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MasterEarlyInitParam_t *pstEarlyInitParam;
    pstEarlyInitParam = (MasterEarlyInitParam_t*) &stVpeChnAttr.tIspInitPara.u8Data[0];
    pstEarlyInitParam->u16SnrEarlyFps = pCameraBootSetting->u8SensorFrameRate;
    pstEarlyInitParam->u16SnrEarlyFlicker = pCameraBootSetting->u8AntiFlicker;
    pstEarlyInitParam->u32SnrEarlyShutter = pCameraBootSetting->u32shutter;
    pstEarlyInitParam->u32SnrEarlyGainX1024 = pCameraBootSetting->u32SensorGain;
    pstEarlyInitParam->u32SnrEarlyDGain = pCameraBootSetting->u32DigitalGain;
    pstEarlyInitParam->u16SnrEarlyAwbRGain = pCameraBootSetting->u16AWBRGain;
    pstEarlyInitParam->u16SnrEarlyAwbGGain = pCameraBootSetting->u16AWBGGain;
    pstEarlyInitParam->u16SnrEarlyAwbBGain = pCameraBootSetting->u16AWBBGain;
    stVpeChnAttr.tIspInitPara.u32Revision = EARLYINIT_PARAM_TYPE_MASTER;
    stVpeChnAttr.tIspInitPara.u32Size = sizeof(MasterEarlyInitParam_t);

    /********************************************/
    /* Init VENC Channel Relative Attr Structur */
    /********************************************/
    MI_VENC_ChnAttr_t stVencChnAttr;
    memset(&stVencChnAttr, 0, (size_t)sizeof(MI_VENC_ChnAttr_t));
    stVencChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_H264E;
    stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth = u32FrameWidth;
    stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight = u32FrameHeight;
    stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = u32FrameWidth;
    stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = u32FrameHeight;
    stVencChnAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;
    stVencChnAttr.stVeAttr.stAttrH264e.u32Profile = 1; //0 is baseline, 1 is main profile

    stVencChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
    stVencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = 2097152;
    stVencChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
    stVencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = FRAME_RATE;
    stVencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = FRAME_RATE;
    stVencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
    stVencChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;

    MI_VENC_SuperFrameCfg_t stSuperFrameCfg = {E_MI_VENC_SUPERFRM_NONE, 0, 0, 0};
    stSuperFrameCfg.u32SuperIFrmBitsThr = 84 * 2 * 1024 * 8;
    stSuperFrameCfg.u32SuperPFrmBitsThr = 63 * 3 * 1024 * 8;

    /********************************************/
    /*  Init Pipeline                           */
    /********************************************/
    MI_SYS_ChnPort_t stVpeChnInputPort0;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hHandle = 0;
    u32 lpcChnId, frameId;
    u8 u8VpeChn, u8VencChn;

    for(lpcChnId = 0; lpcChnId < u32ChnCnt; lpcChnId++)
    {
        u8VpeChn = lpcChnId + 1;
        u8VencChn = lpcChnId + 1;

        initVpeVenc(u8VpeChn, u8VencChn, u32FrameCnt, &stVpeChnAttr, &stVencChnAttr, &stCropWin, &stSuperFrameCfg);

        /********************************************/
        /* Get and then put frame into input port   */
        /********************************************/
        memset(&stVpeChnInputPort0, 0, (size_t)sizeof(stVpeChnInputPort0));
        stVpeChnInputPort0.eModId = E_MI_MODULE_ID_VPE;
        stVpeChnInputPort0.u32DevId = VPE_DEV;
        stVpeChnInputPort0.u32ChnId = u8VpeChn;
        stVpeChnInputPort0.u32PortId = VPE_OUTPUT_PORT;

        memset(&stBufConf ,  0 , (size_t)sizeof(stBufConf));
        stBufConf.eBufType = E_MI_SYS_BUFDATA_FRAME;
        stBufConf.u64TargetPts = 0;

        stBufConf.stFrameCfg.eFormat = PIXEL_FORMAT;
        stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        stBufConf.stFrameCfg.u16Width = u32FrameWidth;
        stBufConf.stFrameCfg.u16Height = u32FrameHeight;

        for(frameId = 0; frameId < u32FrameCnt; frameId++)
        {
            if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stVpeChnInputPort0, &stBufConf, &stBufInfo, &hHandle, 0))
            {
                LpcRecvOneFrameParam_t tFrameData;

                tFrameData.uCh = lpcChnId;
                tFrameData.uFrameID = frameId;
                tFrameData.tBuf.uPhys = stBufInfo.stFrameData.phyAddr[0];
                tFrameData.tBuf.uSize = u32FrameSize;
                MHalLpc_RecvOneFrame(&tFrameData);  //transfer frame data to user buffer

                MI_SYS_ChnInputPortPutBuf(hHandle, &stBufInfo, FALSE);
            }
            else
                CamOsPrintf("[LPC] %s: MI_SYS_ChnInputPortGetBuf FAIL, frameId = %u\n", __func__, frameId);

            CamOsMsSleep(200);
        }
    }

    /********************************************/
    /* Reset and release LPC                    */
    /********************************************/
    tResetParam.uReserved = 0;
    MHalLpc_Reset(&tResetParam);

    LpcReleaseParam_t tReseaseParam;
    tReseaseParam.uReserved = 0;
    MHalLpc_Release(&tReseaseParam);

    return MI_SUCCESS;
}

#endif

static MI_S32 STUB_BaseModuleInit_vif_vpe_venc(void)
{
    MI_VIF_DevAttr_t stVifDevAttr;
    MI_U32 u32VifDevId;
    MI_U32 u32VifChnId;
    MI_U32 u32VifPortId;

    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32SrcFrmrate;
    MI_U32 u32DstFrmrate;
    MI_SYS_BindType_e eBindType;

#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)
    MI_RGN_Attr_t stRgnAttr;
#endif
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MasterEarlyInitParam_t *pstEarlyInitParam;
    MI_U32 u32SensorNum = 1;
    MI_U8 u8VideoNum = 1;
    MI_U32 i = 0;
    MI_BOOL bMirror = FALSE, bFlip = FALSE;
    MI_SYS_Rotate_e eSetType = E_MI_SYS_ROTATE_NONE;
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    MI_SNR_PADID u8SnrPad = 0;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    MI_VIF_GroupAttr_t stGroupAttr;
    MI_VIF_OutputPortAttr_t stVifPortAttr;
    MI_SNR_PADInfo_t  stSnrPadInfo;
    MI_VIF_GROUP nVCapGroup = 0;
    MI_SCL_DEV SclDevId = 0;
    MI_SCL_CHANNEL SclChnId = 0;
    MI_SCL_DevAttr_t stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t  stSclChnParam;
    MI_ISP_DEV IspDevId = 0;
    MI_ISP_CHANNEL IspChnId = 0;
    MI_ISP_DevAttr_t stIspDevAttr;
    MI_ISP_ChannelAttr_t  stIspChnAttr;
    MI_ISP_ChnParam_t stIspChnParam;
    MI_U32 u32snr_fps = 0;

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
    unsigned char earlyinit_enable = 0;
    const EarlyInitPreloadCfg_t *pearlyinit_cfg = NULL;
#endif

    MI_DEVICE_RegMiSysExitCall(_Exit_PreloadForce);
    MI_DEVICE_RegMiCliGetDataL2R(MI_CLICMD_PRELOAD, _MI_Cli_GetDataL2R);
    u32SensorNum = pCameraBootSetting->u8SensorNum;
    u8VideoNum = pCameraBootSetting->u8VideoNum;

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
    pearlyinit_cfg = DrvEarlyInitGetPreloadCfg();
    for(i = 0; i < pearlyinit_cfg->u32NumSnr; ++i)
    {
        earlyinit_enable |= DrvEarlyInitForPreloadIsEnabled(i);
    }

    u32SensorNum = (earlyinit_enable) ? pearlyinit_cfg->u32NumSnr : u32SensorNum;
    CamOsPrintf("[earlyinit] en:%u, snr num:%u.\n", earlyinit_enable, u32SensorNum);
#endif
    CamOsPrintf("pCameraBootSetting->u8Orientation=%d\n",pCameraBootSetting->u8Orientation);
    switch(pCameraBootSetting->u8Orientation)
    {
        case 0:
            bMirror = FALSE;
            bFlip = FALSE;
            eSetType = E_MI_SYS_ROTATE_NONE;
            break;
        case 1:
            bMirror = FALSE;
            bFlip = TRUE;
            eSetType = E_MI_SYS_ROTATE_90;
            break;
        case 2:
            bMirror = TRUE;
            bFlip = TRUE;
            eSetType = E_MI_SYS_ROTATE_180;
            break;
        case 3:
            bMirror = TRUE;
            bFlip = FALSE;
            eSetType = E_MI_SYS_ROTATE_270;
            break;
        default:
            bMirror = FALSE;
            bFlip = FALSE;
            eSetType = E_MI_SYS_ROTATE_NONE;
            break;
    }
    CamOsPrintf("%d,eSetType=%d\n",__LINE__,eSetType);
   for(i=0;i<u32SensorNum;++i)
   {
        MI_U32 u32SnrResCount = 0;
        MI_U32 u32GetSnrResCount = 0;
        MI_U8 index = 0x0, l=0x0, j = 0x0, k = 0x0;
        MI_U8 tIndex = (MI_U8)(-1);
        MI_SNR_Res_t stSensorRes[20], stSensorCurRes;
        MI_U8 tRecordIndexj[20] = {0x0}, tRecordIndexk[20] = {0x0};
        MI_U32  sensorDvalue = 0;
        MI_U32 minSensorDvalue = -1;
        MI_BOOL  statue = false;
        MI_U32  Dvalue = 0;
        MI_U32 minDvalue = -1;
        MI_U32 sFps = 30;
        MI_BOOL bPlaneMode = pCameraBootSetting->u8HDRMode == 0 ? FALSE : TRUE;
        char stPatch[128] = {0};

        u8SnrPad = i*2;//2+2lane

        IspChnId = i;
        SclChnId = i;

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
        if(earlyinit_enable && pearlyinit_cfg)
        {
            u8SnrPad = pearlyinit_cfg->ChCfg[i].u8SnrPad;
            bMirror = pearlyinit_cfg->ChCfg[i].bMirror;
            bFlip = pearlyinit_cfg->ChCfg[i].bFlip;
            bPlaneMode = pearlyinit_cfg->ChCfg[i].bHDREn;
            if(bMirror == FALSE)
            {
                if(bFlip == FALSE)
                {
                    eSetType = E_MI_SYS_ROTATE_NONE;
                    pCameraBootSetting->u8Orientation = 0;
                }
                else
                {
                    eSetType = E_MI_SYS_ROTATE_90;
                    pCameraBootSetting->u8Orientation = 1;
                }
            }
            else
            {
                if(bFlip == FALSE)
                {
                    eSetType = E_MI_SYS_ROTATE_270;
                    pCameraBootSetting->u8Orientation = 3;
                }
                else
                {
                    eSetType = E_MI_SYS_ROTATE_180;
                    pCameraBootSetting->u8Orientation = 2;
                }
            }
        }
#endif
    CamOsPrintf("%d,eSetType=%d\n",__LINE__,eSetType);
        /************************************************
        Step2:  init VIF(for IPC, only one dev)
        *************************************************/
        MI_SNR_SetPlaneMode(u8SnrPad,bPlaneMode);
        MI_SNR_QueryResCount(u8SnrPad, &u32GetSnrResCount);
        u32SnrResCount = u32GetSnrResCount;

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
        if(earlyinit_enable && pearlyinit_cfg)
        {
            if(pearlyinit_cfg->ChCfg[i].u8ResIdx >= u32SnrResCount)
            {
                CamOsPrintf("[earlyinit] snr res idx %d,%d err\n", pearlyinit_cfg->ChCfg[i].u8ResIdx, u32SnrResCount);
                continue;
            }

            tIndex = pearlyinit_cfg->ChCfg[i].u8ResIdx;
            u32snr_fps = pearlyinit_cfg->ChCfg[i].u32SensorFrameRate / 1000;
        }
#endif

        if(tIndex == (MI_U8)(-1)) //Not set by earlyinit yet
        {
            for(index = 0; index < u32SnrResCount; index++)
            {
                if(MI_SUCCESS != MI_SNR_GetRes(u8SnrPad, index, &stSensorRes[index]))
                {
                    CamOsPrintf("%s:%d Get sensor resolution index %d error!\n", __func__, __LINE__, index);
                    continue;
                }

                CamOsPrintf("index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n",
                            index, stSensorRes[index].stCropRect.u16X, stSensorRes[index].stCropRect.u16Y,
                            stSensorRes[index].stCropRect.u16Width, stSensorRes[index].stCropRect.u16Height,
                            stSensorRes[index].stOutputSize.u16Width, stSensorRes[index].stOutputSize.u16Height,
                            stSensorRes[index].u32MaxFps, stSensorRes[index].u32MinFps, stSensorRes[index].strResDesc);
            }

            for(k = 0x0, j = 0x0, index = 0; index < u32SnrResCount; index++)
            {
                if((stSensorRes[index].stCropRect.u16Width == g_stStreamAttr[0].u32Width) &&
                    (stSensorRes[index].stCropRect.u16Height == g_stStreamAttr[0].u32Height))
                {
                    tRecordIndexj[j] = index;
                    j++;
                    CamOsPrintf("find res, j:%d, index:%d.\n", j, index);
                }
                else if((stSensorRes[index].stCropRect.u16Width > g_stStreamAttr[0].u32Width) &&
                    (stSensorRes[index].stCropRect.u16Height > g_stStreamAttr[0].u32Height))
                {
                    sensorDvalue = stSensorRes[index].stCropRect.u16Width * stSensorRes[index].stCropRect.u16Height - g_stStreamAttr[0].u32Width * g_stStreamAttr[0].u32Height;
                    if(minSensorDvalue > sensorDvalue)
                    {
                        minSensorDvalue = sensorDvalue;

                        k = 0x0;
                        tRecordIndexk[k] = index;
                        k++;
                    }
                    else if(minSensorDvalue == sensorDvalue)
                    {
                        tRecordIndexk[k] = index;
                        k++;
                    }
                }
            }

            //set snr resolution
            if(0x0 != j)
            {
                for(l = 0; l < j; l++)
                {
                    index = tRecordIndexj[l];
                    if(sFps == stSensorRes[index].u32MaxFps)
                    {
                        statue = true;
                        tIndex = index;
                        CamOsPrintf("index:%d.\n", tRecordIndexj[l]);
                        break;
                    }
                    else if(sFps < stSensorRes[index].u32MaxFps)
                    {
                        Dvalue = stSensorRes[index].u32MaxFps - sFps;
                        if(Dvalue < minDvalue)
                        {
                            minDvalue = Dvalue;
                            statue = true;
                            tIndex = index;
                        }
                        continue;
                    }
                    else
                    {
                        CamOsPrintf("target fps is larger than sensor maxfps.\n");
                        statue = true;
                        tIndex = index;
                    }
                }
            }
            else if(0x0 != k)
            {
                for(l = 0; l < k; l++)
                {
                    index = tRecordIndexk[l];
                    if(sFps == stSensorRes[index].u32MaxFps)
                    {
                        statue = true;
                        tIndex = index;
                        CamOsPrintf("index:%d.\n", tRecordIndexk[l]);
                        break;
                    }
                    else if(sFps < stSensorRes[index].u32MaxFps)
                    {
                        Dvalue = stSensorRes[index].u32MaxFps - sFps;
                        if(Dvalue < minDvalue)
                        {
                            minDvalue = Dvalue;
                            statue = true;
                            tIndex = index;
                        }
                        continue;
                    }
                    else
                    {
                        CamOsPrintf("target fps is larger than sensor maxfps.\n");
                        statue = true;
                        tIndex = index;
                    }
                }
            }

            if(statue == false)
            {
                CamOsPrintf("can not find res, index user 0.\n");
                tIndex = 0x0;
            }
            else
            {
                CamOsPrintf("find res. index:%d.\n", tIndex);
            }
        }

        MI_SNR_SetRes(u8SnrPad, tIndex);
        memset(&stSensorCurRes, 0x00, (size_t)sizeof(MI_SNR_Res_t));
        MI_SNR_GetRes(u8SnrPad, tIndex, &stSensorCurRes);
        CamOsPrintf("%s:set Snrfps idx = %d\n", __func__, u32snr_fps);
        u32snr_fps = (u32snr_fps == 0) ? pCameraBootSetting->u8SensorFrameRate : u32snr_fps;
        if(stSensorCurRes.u32MinFps <= u32snr_fps && u32snr_fps <= stSensorCurRes.u32MaxFps)
        {
            MI_SNR_SetFps(u8SnrPad, u32snr_fps);
            CamOsPrintf("%s:%d current sensor fps(min:%d, max:%d), Set new sensor fps:%d\n", __func__, __LINE__,
                stSensorCurRes.u32MinFps, stSensorCurRes.u32MaxFps, u32snr_fps);
        }
        MI_SNR_Enable(u8SnrPad);
        MI_SNR_SetOrien(u8SnrPad, bMirror, bFlip);
#if defined(CONFIG_USB_GADGET_UVC_SUPPORT)
        //guvc_app_para[i].attr->u32MaxFps = u32snr_fps;
#endif
        /************************************************
        Step2:  init VIF
        *************************************************/
        switch(u8SnrPad)
        {
            case 0:
                u32VifDevId = 0;
                break;
            case 1:
                u32VifDevId = 8;
                break;
            case 2:
                u32VifDevId = 4;
                break;
            case 3:
                u32VifDevId = 12;
                break;
            default:
                return -1;
        }
        u32VifChnId = 0;
        u32VifPortId = 0;
        memset(&stVifDevAttr, 0x0, (size_t)sizeof(MI_VIF_DevAttr_t));

        memset(&stGroupAttr, 0x0, (size_t)sizeof(MI_VIF_GroupAttr_t));
        memset(&stSnrPadInfo, 0x0, (size_t)sizeof(MI_SNR_PADInfo_t));

        STCHECKRESULT(MI_SNR_GetPadInfo((MI_SNR_PADID)u8SnrPad, &stSnrPadInfo));

        STCHECKRESULT(GetVifPadMapGroup(u8SnrPad, &nVCapGroup));

        stGroupAttr.eWorkMode = E_MI_VIF_WORK_MODE_1MULTIPLEX;
        stGroupAttr.eHDRType = ((stSnrPadInfo.eHDRMode == E_MI_SNR_HDR_TYPE_MAX)?E_MI_VIF_HDR_TYPE_OFF:(MI_VIF_HDRType_e)stSnrPadInfo.eHDRMode);
        stGroupAttr.eIntfMode = (MI_VIF_IntfMode_e)stSnrPadInfo.eIntfMode;
        if(E_MI_VIF_MODE_BT656 == stGroupAttr.eIntfMode)
        {
            stGroupAttr.eClkEdge = (MI_VIF_ClkEdge_e)stSnrPadInfo.unIntfAttr.stBt656Attr.eClkEdge;
        }
        else
        {
            stGroupAttr.eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;
        }
        stGroupAttr.eScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        STCHECKRESULT(MI_VIF_CreateDevGroup(nVCapGroup, &stGroupAttr));
        memset(&stSnrPlane0Info, 0x0, (size_t)sizeof(MI_SNR_PlaneInfo_t));
        MI_SNR_GetPlaneInfo(u8SnrPad, 0, &stSnrPlane0Info);
        stVifDevAttr.stInputRect.u16X = stSnrPlane0Info.stCapRect.u16X;
        stVifDevAttr.stInputRect.u16Y = stSnrPlane0Info.stCapRect.u16Y;
        stVifDevAttr.stInputRect.u16Width = stSnrPlane0Info.stCapRect.u16Width;
        stVifDevAttr.stInputRect.u16Height = stSnrPlane0Info.stCapRect.u16Height;

        if(stSnrPlane0Info.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
        {
            stVifDevAttr.eInputPixel = stSnrPlane0Info.ePixel;
        }
        else
        {
            stVifDevAttr.eInputPixel = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);
        }
        STCHECKRESULT(MI_VIF_SetDevAttr(u32VifDevId, &stVifDevAttr));
        STCHECKRESULT(MI_VIF_EnableDev(u32VifDevId));

        memset(&stVifPortAttr, 0, (size_t)sizeof(stVifPortAttr));
        // no need outputport crop, so set X,Y = 0
        stVifPortAttr.stCapRect.u16X = 0;
        stVifPortAttr.stCapRect.u16Y = 0;
        stVifPortAttr.stCapRect.u16Width = stSnrPlane0Info.stCapRect.u16Width;
        stVifPortAttr.stCapRect.u16Height = stSnrPlane0Info.stCapRect.u16Height;
        stVifPortAttr.stDestSize.u16Width = stSnrPlane0Info.stCapRect.u16Width;
        stVifPortAttr.stDestSize.u16Height = stSnrPlane0Info.stCapRect.u16Height;
        stVifPortAttr.eFrameRate = E_MI_VIF_FRAMERATE_FULL;
        if(stSnrPlane0Info.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
            stVifPortAttr.ePixFormat = stSnrPlane0Info.ePixel;
        else
            stVifPortAttr.ePixFormat = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);
        stVifPortAttr.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
        STCHECKRESULT(MI_VIF_SetOutputPortAttr(u32VifDevId, u32VifPortId, &stVifPortAttr));
        STCHECKRESULT(MI_VIF_EnableOutputPort(u32VifDevId, u32VifPortId));

        /************************************************
        Step3:  Init Isp
        *************************************************/
        memset(&stIspDevAttr, 0x0, (size_t)sizeof(MI_ISP_DevAttr_t));
        memset(&stIspChnAttr, 0x0, (size_t)sizeof(MI_ISP_ChannelAttr_t));
        memset(&stIspChnParam, 0x0, (size_t)sizeof(MI_ISP_ChnParam_t));

        stIspDevAttr.u32DevStitchMask = E_MI_ISP_DEVICEMASK_ID0;
        if(u32SensorNum > 1)
        {
            stIspDevAttr.u32DevStitchMask |= E_MI_ISP_DEVICEMASK_ID1;
        }
        if (i == 0)
        {
            STCHECKRESULT(MI_ISP_CreateDevice(IspDevId, &stIspDevAttr));
        }

        switch(u8SnrPad)
        {
            case 0:
                stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR0;
                break;
            case 1:
                stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR1;
                break;
            case 2:
                stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR2;
                break;
            case 3:
                stIspChnAttr.u32SensorBindId = E_MI_ISP_SENSOR3;
                break;
            default:
                CamOsPrintf("Invalid Snr pad id:%d\n", (int)u8SnrPad);
                return -1;
        }

        stIspChnParam.eHDRType = (MI_ISP_HDRType_e)stGroupAttr.eHDRType;

        if (RtosPreloadIsUseReduceMemCfg())
            stIspChnParam.e3DNRLevel = E_MI_ISP_3DNR_LEVEL_OFF;
        else
            stIspChnParam.e3DNRLevel = E_MI_ISP_3DNR_LEVEL_OFF;

        stIspChnParam.bMirror = FALSE;
        stIspChnParam.bFlip = FALSE;
        stIspChnParam.eRot = eSetType = E_MI_SYS_ROTATE_NONE;
        pstEarlyInitParam = (MasterEarlyInitParam_t*) &stIspChnAttr.stIspCustIqParam.stVersion.u8Data[0];
        pstEarlyInitParam->u16SnrEarlyFps = u32snr_fps;
        pstEarlyInitParam->u16SnrEarlyFlicker = pCameraBootSetting->u8AntiFlicker;
        pstEarlyInitParam->u32SnrEarlyShutter = pCameraBootSetting->u32shutter;
        pstEarlyInitParam->u32SnrEarlyGainX1024 = pCameraBootSetting->u32SensorGain;
        pstEarlyInitParam->u32SnrEarlyDGain = pCameraBootSetting->u32DigitalGain;
        pstEarlyInitParam->u16SnrEarlyAwbRGain = pCameraBootSetting->u16AWBRGain;
        pstEarlyInitParam->u16SnrEarlyAwbGGain = pCameraBootSetting->u16AWBGGain;
        pstEarlyInitParam->u16SnrEarlyAwbBGain = pCameraBootSetting->u16AWBBGain;
        stIspChnAttr.stIspCustIqParam.stVersion.u32Revision = EARLYINIT_PARAM_TYPE_MASTER;
        stIspChnAttr.stIspCustIqParam.stVersion.u32Size = sizeof(MasterEarlyInitParam_t);

#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
        if(earlyinit_enable && pearlyinit_cfg)
        {
            pstEarlyInitParam->u16SnrEarlyFlicker = pearlyinit_cfg->ChCfg[i].u16SnrEarlyFlicker;
            pstEarlyInitParam->u32SnrEarlyShutter = pearlyinit_cfg->ChCfg[i].u32ShutterLEF;
            pstEarlyInitParam->u32SnrEarlyGainX1024 = pearlyinit_cfg->ChCfg[i].u32SensorGainLEFx1024; //long frame, gain x 1024
        }
#endif

        STCHECKRESULT(MI_ISP_CreateChannel(IspDevId, IspChnId, &stIspChnAttr));
        STCHECKRESULT(MI_ISP_SetChnParam(IspDevId, IspChnId, &stIspChnParam));
        STCHECKRESULT(MI_ISP_StartChannel(IspDevId, IspChnId));

        switch(pCameraBootSetting->u8DayNightMode)
        {
            case 0: //WUDayMod:
                CamOsSprintf(&stPatch[0], "%s/%s%u%s", application_selector_get_rofile_path(), _ISP_API, u8SnrPad, _EXT_BIN);
                break;
            case 1: //WUNightMod:
                CamOsSprintf(&stPatch[0], "%s/%s%u%s", application_selector_get_rofile_path(), _ISP_API, u8SnrPad, _EXT_BIN);
                break;
            case 2: //WUColorNightMod:
                CamOsSprintf(&stPatch[0], "%s/%s%u%s", application_selector_get_rofile_path(), _ISP_API, u8SnrPad, _EXT_BIN);
                break;
            case 3: //HDR:
                CamOsSprintf(&stPatch[0], "%s/%s%u%s", application_selector_get_rofile_path(), _ISP_API_HDR, u8SnrPad, _EXT_BIN);
                break;
            case 4: //HDR_3f:
                CamOsSprintf(&stPatch[0], "%s/%s%u%s", application_selector_get_rofile_path(), _ISP_API_HDR_3F, u8SnrPad, _EXT_BIN);
                break;
            default:
                break;
        }
        MI_ISP_ApiCmdLoadBinFile(IspDevId, IspChnId, &stPatch[0], 1234);
        if (i == u32SensorNum-1)
            CamOsTsemUp(&tIspReadFileTsem);
        BootTimestampRecord(__LINE__,"MI_ISP_ApiCmdLoadBinFile");

       /************************************************
        Step3:  init SCL
        *************************************************/
#if (defined MULTIRING_SUPPORT) && (MULTIRING_SUPPORT == 1)
        if (i == 0 && pstStreamAttr[i].eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
        {
            MI_SYS_GlobalPrivPoolConfig_t stPrivPoolCfg;
            memset(&stPrivPoolCfg, 0x0, (size_t)sizeof(MI_SYS_GlobalPrivPoolConfig_t));

            stPrivPoolCfg.bCreate  = TRUE;
            stPrivPoolCfg.eConfigType  = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.eModule = E_MI_MODULE_ID_SCL;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u32Devid = SclDevId;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth = g_stStreamAttr[0].u32MaxWidth;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = g_stStreamAttr[0].u32MaxHeight;
            stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16RingLine = g_stStreamAttr[0].u32MaxHeight/4;
            MI_SYS_ConfigPrivateMMAPool(0, &stPrivPoolCfg);
        }
#endif
        memset(&stSclDevAttr, 0x0, (size_t)sizeof(MI_SCL_DevAttr_t));
        memset(&stSclChnAttr, 0x0, (size_t)sizeof(MI_SCL_ChannelAttr_t));
        memset(&stSclChnParam, 0x0, (size_t)sizeof(MI_SCL_ChnParam_t));

    #if defined(CONFIG_SIGMASTAR_CHIP_IFORD)
        stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL0 | E_MI_SCL_HWSCL1 | E_MI_SCL_HWSCL2;
    #else
        stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL0 | E_MI_SCL_HWSCL1 | E_MI_SCL_HWSCL2 | E_MI_SCL_HWSCL3;// | E_MI_SCL_HWSCL4 | E_MI_SCL_HWSCL5;
    #endif
        STCHECKRESULT(MI_SCL_CreateDevice(SclDevId, &stSclDevAttr));

        STCHECKRESULT(MI_SCL_CreateChannel(SclDevId, SclChnId, &stSclChnAttr));
        STCHECKRESULT(MI_SCL_SetChnParam(SclDevId, SclChnId, &stSclChnParam));
        STCHECKRESULT(MI_SCL_StartChannel(SclDevId, SclChnId));

        /************************************************
        Step5:  Bind ISP->SCL
        *************************************************/
        memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));

        stSrcChnPort.eModId = E_MI_MODULE_ID_ISP;
        stSrcChnPort.u32DevId = IspDevId;
        stSrcChnPort.u32ChnId = IspChnId;
        stSrcChnPort.u32PortId = 0;
        stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId = SclDevId;
        stDstChnPort.u32ChnId = SclChnId;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate = u32snr_fps;
        u32DstFrmrate = u32snr_fps;
        eBindType = E_MI_SYS_BIND_TYPE_REALTIME;
        //	CamOsPrintf("isp scl bind :%d\n", E_MI_SYS_BIND_TYPE_REALTIME);
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, 0));
        STCHECKRESULT(MI_ISP_EnableOutputPort(IspDevId, IspChnId, 0));
    }
#if (defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)) && INTERFACE_RGN
    if(TRUE == pCameraBootSetting->u8RegionsCun)
    {
        ST_OSD_Init();
        memset(&stRgnAttr, 0, (size_t)sizeof(MI_RGN_Attr_t));
        stRgnAttr.eType = E_MI_RGN_TYPE_OSD;
        stRgnAttr.stOsdInitParam.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_ARGB1555;
        stRgnAttr.stOsdInitParam.stSize.u32Width = RGN_OSD_TIME_WIDTH;
        stRgnAttr.stOsdInitParam.stSize.u32Height = RGN_OSD_TIME_HEIGHT;
        STCHECKRESULT(ST_OSD_Create(VPE_PORT0_OSD_FOR_TIME_HANDLE, &stRgnAttr));
        memset(&stRgnAttr, 0, (size_t)sizeof(MI_RGN_Attr_t));
        stRgnAttr.eType = E_MI_RGN_TYPE_OSD;
        stRgnAttr.stOsdInitParam.ePixelFmt = g_stFileInfo[0].ePixelFmt;
        stRgnAttr.stOsdInitParam.stSize.u32Width = g_stFileInfo[0].u16RgnWidth;
        stRgnAttr.stOsdInitParam.stSize.u32Height = g_stFileInfo[0].u16RgnHeight;
        STCHECKRESULT(ST_OSD_Create(VPE_PORT0_OSD_FOR_PIC_HANDLE, &stRgnAttr));
    }
#endif

#if INTERFACE_VDISP
    if(2<=u32SensorNum && 1==u8VideoNum)
    {
        /**Only scenarios: multiple sensors images into one canvas**/
        ST_StartPipeLine_Multi2One(u32SensorNum);
    }
    else
#endif
    {
        for(i = 0; i < u8VideoNum; i ++)
        {
            if(pstStreamAttr[i].enInput == ST_Sys_Input_SCL)
            {
                ST_StartPipeLine(i, pstStreamAttr[i].u32Width, pstStreamAttr[i].u32Height, pstStreamAttr[i].u32CropWidth, pstStreamAttr[i].u32CropHeight, pstStreamAttr[i].u32CropX, pstStreamAttr[i].u32CropY);
            }
        }
    }

    for(i=0;i<u32SensorNum;++i)
    {
        
        u8SnrPad = i*2;//2+2lane
        IspChnId = i;
        SclChnId = i;
        
#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
        if(earlyinit_enable && pearlyinit_cfg)
        {
            u8SnrPad = pearlyinit_cfg->ChCfg[i].u8SnrPad;
        }
#endif

        switch(u8SnrPad)
        {
            case 0:
                u32VifDevId = 0;
                break;
            case 1:
                u32VifDevId = 8;
                break;
            case 2:
                u32VifDevId = 4;
                break;
            case 3:
                u32VifDevId = 12;
                break;
            default:
                return -2;
        }
        u32VifChnId = 0;
        u32VifPortId = 0;

        /************************************************
        Step4:  Bind VIF->ISP
        *************************************************/
        memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId = E_MI_MODULE_ID_VIF;
        stSrcChnPort.u32DevId = u32VifDevId;
        stSrcChnPort.u32ChnId = u32VifChnId;
        stSrcChnPort.u32PortId = u32VifPortId;
        stDstChnPort.eModId = E_MI_MODULE_ID_ISP;
        stDstChnPort.u32DevId = IspDevId;
        stDstChnPort.u32ChnId = IspChnId;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate = u32snr_fps;
        u32DstFrmrate = u32snr_fps;
#if defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_EARLYINIT_SUPPORT)
        if(earlyinit_enable && pearlyinit_cfg)
        {
            eBindType = (MI_SYS_BindType_e)pearlyinit_cfg->ChCfg[i].eBindMode;
        }
#elif defined(_LPC_SUPPORT_)
        eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
#else
        eBindType = E_MI_SYS_BIND_TYPE_REALTIME;
#endif
         STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, 0));
     }

     BootTimestampRecord(__LINE__,"ST_StartPipeLine");
    if(pCameraBootSetting->u8enableUserYUV || pCameraBootSetting->u8enableSED)
    {
        MI_U32 u32BufQueueDepth = 0;
        MI_SCL_OutPortParam_t  stSclOutputParam;
        memset(&stSclOutputParam, 0x0, (size_t)sizeof(MI_SCL_OutPortParam_t));
        stSclOutputParam.stSCLOutCropRect.u16X = 0;
        stSclOutputParam.stSCLOutCropRect.u16Y = 0;
        stSclOutputParam.stSCLOutCropRect.u16Width = _stVpeResolution[0].u32OutWidth;
        stSclOutputParam.stSCLOutCropRect.u16Height = _stVpeResolution[0].u32OutHeight;
        stSclOutputParam.stSCLOutputSize.u16Width = 352;
        stSclOutputParam.stSCLOutputSize.u16Height = 288;
        stSclOutputParam.bMirror = FALSE;
        stSclOutputParam.bFlip = FALSE;
        stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_NONE;
        stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        STCHECKRESULT(MI_SCL_SetOutputPortParam(0, 0, 1, &stSclOutputParam));
        STCHECKRESULT(MI_SCL_EnableOutputPort(0, 0, 1));

        memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
        stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId = 0;
        stDstChnPort.u32ChnId = 0;
        stDstChnPort.u32PortId = 1;
        u32BufQueueDepth = pCameraBootSetting->u32PreloadYUVFrameDepth;
#if defined(CONFIG_VDF_IN_RTOS_ENABLE)
        if(TRUE == pCameraBootSetting->u8enableVDF)
        {
            u32BufQueueDepth = pCameraBootSetting->u32PreloadYUVFrameDepth + 2;
        }
#endif
#if defined(CONFIG_IPU_IN_RTOS_ENABLE)
        if(TRUE == pCameraBootSetting->u8enableIPU)
        {
            u32BufQueueDepth = pCameraBootSetting->u32PreloadYUVFrameDepth + 2;
        }
#endif
#if defined(_SED_IN_RTOS_ENABLE_)
        if(TRUE == pCameraBootSetting->u8enableSED)
        {
            u32BufQueueDepth = pCameraBootSetting->u32PreloadYUVFrameDepth + 2;
        }
#endif
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, pCameraBootSetting->u32PreloadYUVFrameDepth, u32BufQueueDepth));
    }

#ifdef CONFIG_SED_ENABLE
    if(pCameraBootSetting->u8enableSED)
    {
        MI_S32 ret;
        MI_SED_DetectorAttr_t sedStAttr;

        //SED Moudle
        sedStAttr.stAlgoAttr.eType = E_MI_IVEOBJDETECT_ALGOPARAM ;
        sedStAttr.stAlgoAttr.stIveObjDetectAlgo.u32VdfChn  = 4; //0formd 1forod 2forvg
        sedStAttr.stAlgoAttr.stIveObjDetectAlgo.u8Sensitivity = 80;
        sedStAttr.stTargetAttr.s32RltQp = -7;
        sedStAttr.stInputAttr.u32Width = 352;
        sedStAttr.stInputAttr.u32Height = 288;
        sedStAttr.stInputAttr.u32FrameRateDen = 1;
        sedStAttr.stInputAttr.u32FrameRateNum = 10;
        sedStAttr.stInputAttr.stInputPort.eModId = E_MI_MODULE_ID_SCL;
        sedStAttr.stInputAttr.stInputPort.u32ChnId = 0;
        sedStAttr.stInputAttr.stInputPort.u32DevId = 0;
        sedStAttr.stInputAttr.stInputPort.u32PortId = 1;
        //CamOsPrintf("[SED_DEMO] check QP = %d\n", sedStAttr.stTargetAttr.s32RltQp);
        ret = MI_SED_CreateChn(0,&sedStAttr);
        if (ret != MI_SUCCESS)
        {
            CamOsPrintf("[SED_DEMO] MI_SED_CreateChn return %d\n",ret);
        }

        ret = MI_SED_AttachToVencChn(0, 0);
        if (ret != MI_SUCCESS)
        {
            CamOsPrintf("[SED_DEMO] MI_SED_AttachToChn 0 return %d\n",ret);
        }

        ret = MI_SED_StartDetector(0);
        if (ret != MI_SUCCESS)
        {
            CamOsPrintf("[SED_DEMO] MI_SED_StartDetector return %d\n",ret);
        }
        else
        {
            CamOsPrintf("[SED_DEMO] MI_SED_StartDetector success return %d\n",ret);
        }
    }
#endif
#ifdef CONFIG_PANEL_IN_RTOS_ENABLE
    {
        MI_DISP_Interface_e eInterface = E_MI_DISP_INTF_MIPIDSI;
        MI_DISP_OutputTiming_e eOutputTiming = E_MI_DISP_OUTPUT_USER;
        MI_DISP_RotateMode_e eRotateMode = E_MI_DISP_ROTATE_NONE;
        MI_SYS_PixelFormat_e ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        MI_PANEL_IntfType_e eIntfType = E_MI_PNL_INTF_MIPI_DSI;
        MI_U8 u8ChnNum = 1;
        MI_U8 u8LayerId = 0;
        MI_U8 u8PortId = 0;
        MI_U32 u32SrcWidth = 720;
        MI_U32 u32SrcHeight = 1280;
        MI_U32 u32Crop_x = 0;
        MI_U32 u32Crop_y = 0;
        MI_U32 u32Crop_w = 0;
        MI_U32 u32Crop_h = 0;
        MI_U32 u32Show_x = 0;
        MI_U32 u32Show_y = 0;
        MI_U32 u32Show_w = 720;
        MI_U32 u32Show_h = 1280;
        MI_U8 u8ChnIndex = 0;
        MI_DISP_DEV DispDev = 0;
        MI_SYS_ChnPort_t stSrcChnPort;
        MI_SYS_ChnPort_t stDstChnPort;

        memset(g_astDispUtDev, 0, (size_t)sizeof(g_astDispUtDev));
        memset(g_astDispUtLayer, 0, (size_t)sizeof(g_astDispUtLayer));
        memset(g_astDispUtPort, 0, (size_t)sizeof(g_astDispUtPort));

        g_astDispUtDev[0].stPubAttr.eIntfSync = eOutputTiming;
        g_astDispUtDev[0].stPubAttr.eIntfType = eInterface;
        g_astDispUtDev[0].eIntfType = eIntfType;
        g_astDispUtLayer[0].eRotateMode = eRotateMode;

        for(u8ChnIndex = 0; u8ChnIndex < u8ChnNum; u8ChnIndex++){
        u8PortId = u8ChnIndex;
        g_astDispUtPort[0][u8PortId].stInputPortAttr.u16SrcWidth = u32SrcWidth;
        g_astDispUtPort[0][u8PortId].stInputPortAttr.u16SrcHeight = u32SrcHeight;

        if(u32Show_x || u32Show_y || u32Show_w || u32Show_h){
                g_astDispUtPort[0][u8PortId].stInputPortAttr.stDispWin.u16X = u32Show_x;
                g_astDispUtPort[0][u8PortId].stInputPortAttr.stDispWin.u16Y = u32Show_y;
                g_astDispUtPort[0][u8PortId].stInputPortAttr.stDispWin.u16Width = u32Show_w;
                g_astDispUtPort[0][u8PortId].stInputPortAttr.stDispWin.u16Height = u32Show_h;
        }
        g_astDispUtPort[0][u8PortId].stCropWin.u16X = u32Crop_x;
        g_astDispUtPort[0][u8PortId].stCropWin.u16Y = u32Crop_y;
        g_astDispUtPort[0][u8PortId].stCropWin.u16Width = u32Crop_w?u32Crop_w:u32SrcWidth;
        g_astDispUtPort[0][u8PortId].stCropWin.u16Height = u32Crop_h?u32Crop_h:u32SrcHeight;
        g_astDispUtPort[0][u8PortId].ePixFormat = ePixelFormat;
        }

        MDrv_GPIO_Init();
        camdriver_gpio_request(NULL, 138);
        camdriver_gpio_direction_output(NULL, 138, 1);
        camdriver_gpio_request(NULL, 135);
        camdriver_gpio_direction_output(NULL, 135, 1);
        //set disp pub
        disp_ut_setdev(DispDev);
        MI_PANEL_Init(E_MI_PNL_INTF_MIPI_DSI);
        //set layer
        disp_ut_setlayer(DispDev, u8LayerId);
        //set inputport
        disp_ut_portshowsize(u8ChnNum, g_astDispUtDev[DispDev].stPubAttr.eIntfSync);
        disp_ut_setport(u8LayerId, u8PortId);

        MI_SCL_OutPortParam_t  stSclOutputParam;
        memset(&stSclOutputParam, 0x0, (size_t)sizeof(MI_SCL_OutPortParam_t));
        stSclOutputParam.stSCLOutCropRect.u16X = 0;
        stSclOutputParam.stSCLOutCropRect.u16Y = 0;
        stSclOutputParam.stSCLOutCropRect.u16Width = _stVpeResolution[0].u32OutWidth;
        stSclOutputParam.stSCLOutCropRect.u16Height = _stVpeResolution[0].u32OutHeight;
        stSclOutputParam.stSCLOutputSize.u16Width = 1280;
        stSclOutputParam.stSCLOutputSize.u16Height = 720;
        stSclOutputParam.bMirror = FALSE;
        stSclOutputParam.bFlip = FALSE;
        stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_NONE;
        stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        STCHECKRESULT(MI_SCL_SetOutputPortParam(0, 0, 5, &stSclOutputParam));
        STCHECKRESULT(MI_SCL_EnableOutputPort(0, 0, 5));

        stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId = 0;
        stSrcChnPort.u32ChnId = 0;
        stSrcChnPort.u32PortId = 5;

        stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
        stDstChnPort.u32DevId = DispDev;
        stDstChnPort.u32ChnId = 0;
        stDstChnPort.u32PortId = u8PortId;
        u32SrcFrmrate = u32snr_fps;
        u32DstFrmrate = u32snr_fps;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, -1, -1, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0));
   }
#endif //#ifndef CONFIG_PANEL_IN_RTOS_ENABLE
    return MI_SUCCESS;
}

static MI_S32 Test_vif_vpe_venc(CLI_t *pCli, char *p)
{
#ifdef _LPC_SUPPORT_
    extern int MHalLpc_RpcInit(void);
    /*Set PM_GPIO5 = HIGH, for test*/
    camdriver_gpio_request(NULL, PAD_PM_GPIO5);
    camdriver_gpio_direction_output(NULL, PAD_PM_GPIO5, 1);
    /*-----------------------------*/
    MHalLpc_RpcInit();
#endif
    CamOsPrintf("Func %s start\n", __FUNCTION__);
    ST_ResetArgs();
    STCHECKRESULT(STUB_BaseModuleInit_vif_vpe_venc());
#ifdef _LPC_SUPPORT_
    STCHECKRESULT(lpcPipelineInit());
#endif
    CamOsPrintf("Func %s end\n", __FUNCTION__);
    return 0;
}

#if defined(CONFIG_VDF_IN_RTOS_ENABLE)
static int Rtk_VDF_Autorun(CLI_t *pCli, char *p)
{
    ST_ModuleInit_VDF();
    ST_VdfStart();
    return 0;
}
#endif

#if INTERFACE_AI
static int ST_AI_Autorun(void)
{
    MI_AUDIO_DEV AiDevId = 0;
    MI_AI_Attr_t stAiSetAttr = {0};
    MI_AI_Attr_t stAiGetAttr = {0};
    MI_AI_If_e enAiIf[] = {E_MI_AI_IF_ADC_AB};
    MI_U8 u8ChnGrpId = 0;
    MI_S16 s8DpgaGain[] = {-10};
    MI_U32 u32FrameCnt = 0;
    MI_AUDIO_SampleRate_e eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_16000;
    //set output port buffer depth
    MI_SYS_ChnPort_t stAiChnOutputPort;
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    AiDevId = pCameraBootSetting->u32AIDevId;

    if (RtosPreloadIsUseReduceMemCfg())
        u32FrameCnt = 5;
    else
        u32FrameCnt = pCameraBootSetting->u32PreloadAudioFrame;

    CamOsPrintf("Func %s start, line %d\n", __FUNCTION__, __LINE__);
    memset(&stAiSetAttr, 0, (size_t)sizeof(MI_AI_Attr_t));
    memset(&stAiGetAttr, 0, (size_t)sizeof(MI_AI_Attr_t));

    switch(pCameraBootSetting->u32AISamplerate)
    {
        case 8000:
            eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_8000;
            break;
        case 11025:
            eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_11025;
            break;
        case 12000:
            eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_12000;
            break;
        case 16000:
            eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_16000;
            break;
        case 22050:
            eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_22050;
            break;
        case 24000:
            eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_24000;
            break;
        case 32000:
            eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_32000;
            break;
        case 44100:
            eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_44100;
            break;
        case 48000:
            eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_48000;
            break;
        case 96000:
            eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_96000;
            break;
        default:
            CamOsPrintf("Func %s:CameraBootSetting.u32AISamplerate ng\n", __FUNCTION__);
            eAISetSamplerate = E_MI_AUDIO_SAMPLE_RATE_16000;
            break;
    }

    stAiSetAttr.enFormat = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    stAiSetAttr.enSoundMode = E_MI_AUDIO_SOUND_MODE_STEREO;
    stAiSetAttr.enSampleRate = eAISetSamplerate;
    stAiSetAttr.u32PeriodSize = AI_PERIOD_SIZE(eAISetSamplerate);
    stAiSetAttr.bInterleaved = TRUE;
    /* open ai device */
    STCHECKRESULT(MI_AI_Open(AiDevId, &stAiSetAttr));

    /* get ai device */
    STCHECKRESULT(MI_AI_GetAttr(AiDevId, &stAiGetAttr));

    /* attach ai interface */
    STCHECKRESULT(MI_AI_AttachIf(AiDevId, enAiIf, sizeof(enAiIf) / sizeof(enAiIf[0])));

    /* set ai interface gain */
    STCHECKRESULT(MI_AI_SetIfGain(E_MI_AI_IF_ADC_AB, 18, 18));

    /* set output depth */
    memset(&stAiChnOutputPort, 0, (size_t)sizeof(stAiChnOutputPort));
    stAiChnOutputPort.eModId = E_MI_MODULE_ID_AI;
    stAiChnOutputPort.u32DevId = AiDevId;
    stAiChnOutputPort.u32ChnId = u8ChnGrpId;
    stAiChnOutputPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stAiChnOutputPort, u32FrameCnt, u32FrameCnt));

    /* set ai dpga gain */
    STCHECKRESULT(MI_AI_SetGain(AiDevId, u8ChnGrpId, s8DpgaGain, sizeof(s8DpgaGain) / sizeof(s8DpgaGain[0])));

    /* enable ai device channel */
    STCHECKRESULT(MI_AI_EnableChnGroup(AiDevId, u8ChnGrpId));

    /* get ai data */
    //{
    //    MI_AI_Data_t stAiChFrame;
    //    MI_AI_Data_t stAecFrame;
    //    STCHECKRESULT(MI_AI_Read(AiDevId, u8ChnGrpId, &stAiChFrame, &stAecFrame, -1));
    //}
    CamOsPrintf("Func %s end\n", __FUNCTION__);
    return 0;
}
#endif

#if INTERFACE_AO
static int ST_AO_Autorun(void)
{
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MI_AUDIO_DEV AoDevId = pCameraBootSetting->u32AODevId;
    MI_S8 s8LeftVolume, s8RightVolume;
    MI_AO_GainFading_e eGainFading;
    MI_AUDIO_SampleRate_e eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;
    MI_AO_Attr_t stAoSetAttr, stAoGetAttr;
    MI_S32 s32AoVolume = pCameraBootSetting->u32AOVol;

    CamOsPrintf("Func %s start\n", __FUNCTION__);
    memset(&stAoSetAttr, 0x0, (size_t)sizeof(MI_AO_Attr_t));

    switch(pCameraBootSetting->u32AOSamplerate)
    {
        case 8000:
            eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_8000;
            break;
        case 11025:
            eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_11025;
            break;
        case 12000:
            eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_12000;
            break;
        case 16000:
            eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;
            break;
        case 22050:
            eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_22050;
            break;
        case 24000:
            eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_24000;
            break;
        case 32000:
            eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_32000;
            break;
        case 44100:
            eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_44100;
            break;
        case 48000:
            eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_48000;
            break;
        case 96000:
            eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_96000;
            break;
        default:
            CamOsPrintf("Func %s:CameraBootSetting.eOutSampleRate ng\n", __FUNCTION__);
            eOutSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;
            break;
    }

    stAoSetAttr.enFormat = E_MI_AUDIO_FORMAT_PCM_S16_LE;
    stAoSetAttr.enSoundMode = E_MI_AUDIO_SOUND_MODE_STEREO;
    stAoSetAttr.enSampleRate = eOutSampleRate;
    stAoSetAttr.u32PeriodSize = AO_PERIOD_SIZE(eOutSampleRate);
    stAoSetAttr.enChannelMode = E_MI_AO_CHANNEL_MODE_STEREO;
    /* open ao device */
    STCHECKRESULT(MI_AO_Open(AoDevId, &stAoSetAttr));
    /* get ao device */
    STCHECKRESULT(MI_AO_GetAttr(AoDevId, &stAoGetAttr));
    /* attach ao device interface */
    STCHECKRESULT(MI_AO_AttachIf(AoDevId, E_MI_AO_IF_DAC_AB, 0));

    //STCHECKRESULT(MI_AO_SetChannelMode(AoDevId, E_MI_AO_CHANNEL_MODE_ONLY_LEFT));
    s8LeftVolume = s32AoVolume;
    s8RightVolume = s32AoVolume;
    eGainFading = E_MI_AO_GAIN_FADING_OFF;
    /* set ao device volume */
    STCHECKRESULT(MI_AO_SetVolume(AoDevId, s8LeftVolume, s8RightVolume, eGainFading));
    CamOsPrintf("Func %s end\n", __FUNCTION__);
    return 0;
}
#endif

static void ST_SetLinux2RtosData(char *pdata)
{
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();

    if (!strncmp(pdata,"--dlaosdoff", 11))
    {
        g_u8dlaosdoff = 1;
        CamOsPrintf("%s g_u8dlaosdoff:%d\n",__FUNCTION__, g_u8dlaosdoff);
    }
    else if (!strncmp(pdata,"--timeosdoff", 12))
    {
        g_u8timeosdoff = 1;
        CamOsPrintf("%s g_u8timeosdoff:%d\n",__FUNCTION__, g_u8timeosdoff);
    }
    else if (!strncmp(pdata,"--sedoff", 8))
    {
        CamOsPrintf("%s sedoff\n",__FUNCTION__);
        if(pCameraBootSetting->u8enableSED)
        {
            #ifdef CONFIG_SED_ENABLE
            MI_SED_DetachFromVencChn(0, 0);
            MI_SED_StopDetector(0);
            MI_SED_DestroyChn(0);
            #endif
        }
    }
    else if (!strncmp(pdata,"--vdfoff", 8))
    {
        CamOsPrintf("%s vdfoff\n",__FUNCTION__);
        if(pCameraBootSetting->u8enableVDF)
        {
            #if defined(CONFIG_VDF_IN_RTOS_ENABLE)
            for(MI_U8 j = 0 ; j < MAX_FULL_RGN_NULL ; j ++)
            {
                g_stVdfThreadArgs[j].bRunFlag = FALSE;
            }
            #endif
        }
    }
    else if (!strncmp(pdata,"--stoppipe", 10))
    {
        g_u8stoppipe = 1;
        CamOsPrintf("%s g_u8stoppipe:%d\n",__FUNCTION__, g_u8stoppipe);
        ST_ExitPipeLine();
    }
    else if (!strncmp(pdata,"--deintmoudle", 13))
    {
        g_u8deintmoudle = 1;
        CamOsPrintf("%s g_u8deintmoudle:%d\n",__FUNCTION__, g_u8deintmoudle);
        ST_DeinitMoudle();
    }
    else
    {
        CamOsPrintf("%s data[%s] available\n",__FUNCTION__, pdata);
    }
}
#if INTERFACE_RGN

#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP) || defined(CONFIG_IPU_IN_RTOS_ENABLE) || defined(CONFIG_VDF_IN_RTOS_ENABLE)
static DMF_BitMapFile_S* _OSD_BitmapFileInfo(DMF_Font_Type_E enType, DMF_Font_Size_E enSize)
{
    if (enType < DMF_Font_Type_ASCII || enType >= DMF_Font_Type_BUTT)
    {
        return NULL;
    }

    if (enSize < DMF_Font_Size_16x16 || enSize >= DMF_Font_Size_BUTT)
    {
        return NULL;
    }

    return &g_dmf_bitmapfile[enType][enSize];
}
static void _OSD_BitmapFile_UnInit(void)
{
    int i = 0, j = 0;
    DMF_BitMapFile_S *pstDMFBitMapFile = NULL;

    for (i = 0; i < (int)DMF_Font_Type_BUTT; i ++)
    {
        for (j = 0; j < (int)DMF_Font_Size_BUTT; j ++)
        {
            pstDMFBitMapFile = &g_dmf_bitmapfile[i][j];

            if (pstDMFBitMapFile->pBitMapAddr != NULL)
            {
                MI_SYS_MMA_Free(0, pstDMFBitMapFile->pBitMapAddr);
                pstDMFBitMapFile->pBitMapAddr = NULL;
            }
        }
    }
}

static int _dmf_GetUtf8Length(const uint8_t *src)
{
    switch (*src)
    {
    case 0x0 ... 0x7f:
        return 1;
    case 0xC0 ... 0xDF:
        return 2;
    case 0xE0 ... 0xEF:
        return 3;
    case 0xF0 ... 0xF7:
        return 4;
    default:
        return -1;
    }
}
static int _dmf_IsUtf8(unsigned char *string, int len)
{
    size_t i = 0;
    size_t continuation_bytes = 0;
    int quick_flag = 0;
    int count = 0;

    while (i < len)
    {
        // printf("%s %d, 0x%X, len:%d\n", __func__, __LINE__, string[i], len);
        switch (string[i])
        {
        case 0x0 ... 0x7f:
            continuation_bytes = 0;
            break;
        case 0xC0 ... 0xDF:
            continuation_bytes = 1;
            break;
        case 0xE0 ... 0xEF:
            continuation_bytes = 2;
            break;
        case 0xF0 ... 0xF4: /* Cause of RFC 3629 */
            continuation_bytes = 3;
            quick_flag = 1;
            break;
        default:
            return i + 1;
        }
        i += 1;
        while (i < len && continuation_bytes > 0
            && string[i] >= 0x80
            && string[i] <= 0xBF)
        {
            i += 1;
            continuation_bytes -= 1;

            count ++;
        }

        if (continuation_bytes != 0)
            return i + 1;
        else if (quick_flag)
            return -1;
    }

    return count;
}
static uint32_t _dmf_Gb2312codeToFontoffset(uint32_t gb2312code, uint32_t font_height)
{
    uint32_t fontoffset;

    fontoffset = (gb2312code % 0x100 - 0xA1) * 94
                    + (gb2312code / 0x100 - 0xA1);
    fontoffset *= (font_height * font_height / 8);

    return fontoffset;
}
static uint32_t _dmf_AsciiToFontoffset(uint32_t ascii, uint32_t width, uint32_t height)
{
    uint32_t size = 0;

    size = (width / 8) * height;

    return ascii * size;
    // return (ascii)* 16;
    // return (ascii * 16) + 1;
}


static int _OSD_Utf8ToUnicode(const uint8_t *src, uint8_t *dst)
{
    int length;
    uint8_t unicode[2] = {0};

    length = _dmf_GetUtf8Length(src);
    if (length < 0)
    {
        return -1;
    }

    switch (length)
    {
    case 1:
        *dst = *src;
        *(dst + 1) = 0;
        return 1;
    case 2:
        unicode[0] = *(src + 1) & 0x3f;
        unicode[0] += (*src & 0x3) << 6;
        unicode[1] = (*src & 0x7 << 2) >> 2;
        break;
    case 3:
        unicode[0] = *(src + 2) & 0x3f;
        unicode[0] += (*(src + 1) & 0x3) << 6;
        unicode[1] = (*(src + 1) & 0xF << 2) >> 2;
        unicode[1] += (*src & 0xf) << 4;
        break;
    case 4:
        /* not support now */
        return -1;
    }

    *dst = unicode[0];
    *(dst + 1) = unicode[1];

    return length;
}

static int _OSD_UnicodeToGb2312(uint16_t unicode, const uint16_t *mem_gb2312, int gb2312_num)
{
    int i = 0;

    for (i = 0; i < gb2312_num; i++)
    {
        if (mem_gb2312[2 * i] == unicode)
        {
            return mem_gb2312[2 * i + 1];
        }
    }

    return -1;
}

static DMF_Encoding_Type_E _OSD_DetectEncodingType(const char *string)
{
    DMF_Encoding_Type_E enEncodIngType = UTF8_NO_BOM;
    int len = 0;
    int ret = 0;

    if (string == NULL)
    {
        return Encoding_Butt;
    }

    len = strlen(string);

    // printf("%s %d, len:%d\n", __func__, __LINE__, len);

    if (len >= 3 && string[0] == 0xef && string[1] == 0xbb && string[2] == 0xbf)
    {
        enEncodIngType = UTF8_WITH_BOM;
    }
    else
    {
        // printf("%s %d\n", __func__, __LINE__);
        ret = _dmf_IsUtf8((unsigned char *)string, len);
        if (ret > 0)
        {
            enEncodIngType = GBK;
        }
        else if (ret == -1)
        {
            enEncodIngType = UTF8_NO_BOM;
        }
    }

    return enEncodIngType;
}

static int _OSD_CalcCharNumGetGb2312Code(const char *string, int *charTotalNum, uint8_t *gb2312buf, int bufLen)
{
    DMF_Encoding_Type_E enEncodIngType = UTF8_NO_BOM;
    int charNum = 0, strLen = 0, dealLen = 0;
    int ret = 0;
    uint8_t unicode[2] = {0};
    const uint8_t *ptr = NULL;
    uint16_t gb2312_code;
    int gb2312codeLen = sizeof(gb2312code) / sizeof(gb2312code[0]) / 3;
    uint8_t *ptr_gb2312;

    if (string == NULL)
    {
        return 0;
    }

    strLen = strlen(string);
    ptr = (uint8_t *)string;
    ptr_gb2312 = gb2312buf;

    enEncodIngType = _OSD_DetectEncodingType(string);

    // ST_DBG("enEncodIngType:%d\n", enEncodIngType);

    if (enEncodIngType == UTF8_NO_BOM ||
        enEncodIngType == UTF8_WITH_BOM)
    {
        do
        {
            ret = _OSD_Utf8ToUnicode(ptr, unicode);
            // printf("%s %d, 0x%X, 0x%X\n", __func__, __LINE__, unicode[0], unicode[1]);
            if (ret < 0)
            {
                return -1;
            }

            ptr += ret;
            dealLen += ret;
            charNum ++;

            gb2312_code = _OSD_UnicodeToGb2312(unicode[0] +
                                    unicode[1] * 0x100, gb2312code, gb2312codeLen);

            // printf("%s %d, 0x%X, %p\n", __func__, __LINE__, gb2312_code, ptr_gb2312);
            ptr_gb2312[0] = gb2312_code % 0x100;
            if (gb2312_code / 0x100 > 0)
            {
                ptr_gb2312[1] = gb2312_code / 0x100;
                ptr_gb2312 += 2;
            }
            else
            {
                ptr_gb2312 += 1;
            }
        } while (dealLen < strLen);
    }
    else if (enEncodIngType == GBK)
    {
        charNum += 2;
        dealLen += 2;
        ptr += 2;
        do
        {
            enEncodIngType = _OSD_DetectEncodingType((const char *)ptr);
            if (enEncodIngType == GBK)
            {
                charNum += 2;
                dealLen += 2;
                ptr += 2;
            }
            else
            {
                charNum ++;
                dealLen ++;
                ptr ++;
            }
        } while (dealLen < strLen);

        charNum = snprintf((char *)gb2312buf, bufLen - 1, "%s", string);
    }

    *charTotalNum = charNum;

    return 0;
}

static void _OSD_CalcBMPWH(int charTotalNum, int *bmpWidth, int *bmpHeight, uint8_t *pGb2312)
{
    DMF_BitMapFile_S *pstDMFBitMapFile = &g_dmf_bitmapfile[DMF_Font_Type_ASCII][DMF_Font_Size_16x16];
    DMF_BitMapAttr_S *pstDMFBitMapAttr = &g_dmf_bitmapattr;

    int width = 0, height = 0;
    int lines = 0, i = 0, offset = 0;
    int charNumPerLine = 0;

    charNumPerLine = MIN(charTotalNum, pstDMFBitMapAttr->charNumPerLine);

    lines = ALIGN_MULTI(charTotalNum, pstDMFBitMapAttr->charNumPerLine);

    offset = 0;
    if (pstDMFBitMapAttr->verticalFlag == 0)
    {
        for (i = 0; i < charNumPerLine; i ++)
        {
            if (pGb2312[offset] > 0xA0 &&
                pGb2312[offset]  < 0xff)
            {
                width += pstDMFBitMapFile->width;

                offset += 2;
            }
            else if (pGb2312[offset] > 0x1f &&
                    pGb2312[offset] < 0x80)
            {
                width += pstDMFBitMapFile->width;

                offset ++;
            }
        }

        for (i = 0; i < lines; i ++)
        {
            height += pstDMFBitMapFile->height;
        }
    }
    else if (pstDMFBitMapAttr->verticalFlag == 1)
    {
        for (i = 0; i < charNumPerLine; i ++)
        {
            height += pstDMFBitMapFile->height;
        }

        for (i = 0; i < lines; i ++)
        {
            width += pstDMFBitMapFile->width;
        }
    }

    if (pstDMFBitMapAttr->verticalFlag == 0)
    {
        width += pstDMFBitMapAttr->leftMargin +
                pstDMFBitMapAttr->charSpace * (charTotalNum - 1) +
                pstDMFBitMapAttr->rightMargin;

        height += pstDMFBitMapAttr->upMargin +
                pstDMFBitMapAttr->lineSpace * (lines - 1) +
                pstDMFBitMapAttr->downMargin;
    }
    else if (pstDMFBitMapAttr->verticalFlag == 1)
    {
        width += pstDMFBitMapAttr->leftMargin +
                pstDMFBitMapAttr->lineSpace * (lines - 1) +
                pstDMFBitMapAttr->rightMargin;

        height += pstDMFBitMapAttr->upMargin +
                pstDMFBitMapAttr->charSpace * (charTotalNum - 1) +
                pstDMFBitMapAttr->downMargin;
    }

    *bmpWidth = width;
    *bmpHeight = height;
}

static void _OSD_FontDataToCanvas(const uint8_t *pFontdata, int x, int y, int width,
                        int height, MI_RGN_HANDLE hHandle, MI_U32 u32Color)
{
    int i = 0, j = 0;
    int char_num;
    int char_bit;
    char bit;
    uint8_t *pFontdataTemp = NULL;
    ST_Point_T stPoint;

    if ((pFontdata == NULL))
    {
        return;
    }

#if 0
    for (i = 0; i < height; i ++)
#else
    for (i = height - 1; i >= 0; i--)
    {
        pFontdataTemp = (uint8_t *)pFontdata + (width + 7) / 8 * i;

        for (j = 0; j < width; j ++)
        {
            char_num = j / 8;
            char_bit = 7 - j % 8;
            bit = pFontdataTemp[char_num] & (1 << char_bit);

            stPoint.u32X = x + j;
            stPoint.u32Y = y + i;

            if (bit)
            {
                ST_OSD_DrawPoint(hHandle, stPoint, u32Color);
            }
            else
            {
            }
        }
    }
#endif
}

static void _OSD_DrawTextToCanvas(MI_RGN_HANDLE hHandle, ST_Point_T stPoint, const char *szString, MI_U32 u32Color, DMF_Font_Size_E enSize)
{
    DMF_BitMapFile_S *pstDMFBitMapFile = NULL;
    DMF_BitMapAttr_S *pstDMFBitMapAttr = &g_dmf_bitmapattr;

    int charTotalNum = 0;
    uint8_t gb2312buf[MAX_BUF_LEN * 2];
    int totalLines = 0;
    int charNumPerLine[MAX_LINES];
    int charRemainNum = 0;
    int i = 0;
    int j = 0;
    int bmpWidth = 0;
    int bmpHeight = 0;
    int gb2312Offset = 0;
    int gb2312LineOffset = 0;
    uint8_t *pGb2312Line = NULL;
    int fontoffset = 0;
    int fontWidth = 0;
    int fontHeight = 0;
    uint8_t *fontAddr = NULL;
    int fontTotalWidth = 0;
    int fontTotalHeight = 0;
    int xpos = 0;
    int ypos = 0;

    if (szString == NULL)
    {
        return;
    }

    memset(gb2312buf, 0, (size_t)sizeof(gb2312buf));
    _OSD_CalcCharNumGetGb2312Code(szString, &charTotalNum, gb2312buf, MAX_BUF_LEN * 2);

    totalLines = ALIGN_MULTI(charTotalNum, pstDMFBitMapAttr->charNumPerLine);
    totalLines = MIN(totalLines, MAX_LINES);

    memset(&charNumPerLine, 0, (size_t)sizeof(charNumPerLine));
    charRemainNum = charTotalNum;
    for (i = 0; i < totalLines; i ++)
    {
        charNumPerLine[i] = MIN(charRemainNum, pstDMFBitMapAttr->charNumPerLine);

        charRemainNum -= charNumPerLine[i];
    }

    _OSD_CalcBMPWH(charTotalNum, &bmpWidth, &bmpHeight, gb2312buf);

#if 0
    printf("string:\t%s\n", szString);
    printf("charTotalNum:\t%d\n", charTotalNum);
    printf("charNumPerLine:\t");
    for (i = 0; i < totalLines; i ++)
    {
        printf("%d ", charNumPerLine[i]);
    }
    printf("\n");
    printf("totalLines:\t%d\n", totalLines);
    printf("bmpWidth:\t%d\n", bmpWidth);
    printf("bmpHeight:\t%d\n", bmpHeight);;
#endif

    gb2312Offset = 0;
    for (i = 0; i < totalLines; i ++)
    {
        pGb2312Line = gb2312buf + gb2312Offset;
        gb2312LineOffset = 0;

        // printf("%s %d, %p\n", __func__, __LINE__, pGb2312Line);

        for (j = 0; j < charNumPerLine[i]; j ++)
        {
            if (pGb2312Line[gb2312LineOffset] > 0xA0 &&
                pGb2312Line[gb2312LineOffset]  < 0xff)
            {
                pstDMFBitMapFile = _OSD_BitmapFileInfo(DMF_Font_Type_HZ, enSize);
                if (pstDMFBitMapFile == NULL)
                {
                    continue;
                }

                fontoffset = _dmf_Gb2312codeToFontoffset(pGb2312Line[gb2312LineOffset] +
                                0x100 * pGb2312Line[gb2312LineOffset + 1],
                                pstDMFBitMapFile->height);

                fontWidth = pstDMFBitMapFile->width;
                fontHeight = pstDMFBitMapFile->height;

                gb2312LineOffset += 2;

                fontAddr = mi_sys_Vmap(pstDMFBitMapFile->pBitMapAddr,0,TRUE);
            }
            else if (pGb2312Line[gb2312LineOffset] > 0x1f &&
                        pGb2312Line[gb2312LineOffset] < 0x80)
            {
                pstDMFBitMapFile = _OSD_BitmapFileInfo(DMF_Font_Type_ASCII, enSize);
                if (pstDMFBitMapFile == NULL)
                {
                    continue;
                }

                fontWidth = pstDMFBitMapFile->width;
                fontHeight = pstDMFBitMapFile->height;

                fontoffset = _dmf_AsciiToFontoffset(pGb2312Line[gb2312LineOffset], fontWidth, fontHeight);

                gb2312LineOffset ++;

                fontAddr = mi_sys_Vmap(pstDMFBitMapFile->pBitMapAddr,0,TRUE);
            }
            else
            {
                continue;
            }

            if (pstDMFBitMapAttr->verticalFlag == 0)
            {
                xpos = pstDMFBitMapAttr->leftMargin + fontTotalWidth +
                        j * pstDMFBitMapAttr->lineSpace;
                ypos = pstDMFBitMapAttr->upMargin + i * pstDMFBitMapFile->height +
                        i * pstDMFBitMapAttr->lineSpace;

                fontTotalWidth += fontWidth;
            }
            else if (pstDMFBitMapAttr->verticalFlag == 1)
            {
                xpos = pstDMFBitMapAttr->leftMargin + i * pstDMFBitMapFile->width +
                        i * pstDMFBitMapAttr->lineSpace;
                ypos = pstDMFBitMapAttr->upMargin + fontTotalHeight +
                        j * pstDMFBitMapAttr->lineSpace;

                fontTotalHeight += fontHeight;
            }

            //CamOsPrintf("xpos:%d, ypos:%d, gb2312LineOffset:%d, fontoffset:%d, fontWidth:%d,fontHeight:%d, 0x%x\n",
            //            xpos, ypos, gb2312LineOffset, fontoffset, fontWidth, fontHeight, (fontAddr + fontoffset)[0]);
            if (fontAddr != NULL)
                _OSD_FontDataToCanvas(fontAddr + fontoffset,
                                xpos + stPoint.u32X, ypos + stPoint.u32Y, fontWidth, fontHeight, hHandle, u32Color);
        }

        fontTotalWidth = 0;
        fontTotalHeight = 0;

        gb2312Offset += gb2312LineOffset;
    }
}

static MI_S32 ST_OSD_Init(void)
{
    MI_U32 i = 0;

    CamOsMutexInit(&g_stRNGOsdMutex);
    CamOsMutexLock(&g_stRNGOsdMutex);
    if (g_bInit == FALSE)
    {
        g_bInit = TRUE;
        for (i = 0; i < MAX_RGN_NUM; i ++)
        {
            g_stRgnInfo[i].hHandle = -1;
        }
        MI_RGN_Init(0, &_gstPaletteTable);
    }
    CamOsMutexUnlock(&g_stRNGOsdMutex);

    return MI_RGN_OK;
}
static MI_S32 ST_OSD_Deinit(void)
{
    MI_U32 i = 0;
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();

    CamOsMutexLock(&g_stRNGOsdMutex);
    if (g_bInit == TRUE)
    {
        g_bInit = FALSE;
        for (i = 0; i < MAX_RGN_NUM; i ++)
        {
            g_stRgnInfo[i].hHandle = -1;
        }
        if(TRUE == pCameraBootSetting->u8RegionsCun)
        {
            _OSD_BitmapFile_UnInit();
        }
        MI_RGN_DeInit(0);
    }
    CamOsMutexUnlock(&g_stRNGOsdMutex);
    CamOsMutexDestroy(&g_stRNGOsdMutex);

    return MI_RGN_OK;
}
static MI_S32 ST_OSD_Create(MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion)
{
    STCHECKRESULT(MI_RGN_Create(0, hHandle, pstRegion));
    g_stRgnInfo[hHandle].hHandle = hHandle;
    g_stRgnInfo[hHandle].ePixelFmt = pstRegion->stOsdInitParam.ePixelFmt;
    memset(&g_stRgnInfo[hHandle].stCanvasInfo, 0, (size_t)sizeof(MI_RGN_CanvasInfo_t));

    return MI_RGN_OK;
}
static MI_S32 ST_OSD_Destroy(MI_RGN_HANDLE hHandle)
{
    STCHECKRESULT(MI_RGN_Destroy(0, hHandle));
    memset(&g_stRgnInfo[hHandle], 0, (size_t)sizeof(ST_RGN_Info_T));
    g_stRgnInfo[hHandle].hHandle = MI_RGN_HANDLE_NULL;

    return MI_RGN_OK;
}
static MI_S32 ST_OSD_DrawPoint(MI_RGN_HANDLE hHandle, ST_Point_T stPoint, MI_U32 u32Color)
{
    MI_U16 *pDst = NULL;
    MI_U32 u32Stride = 0;
    MI_U8 u8Value = 0;

    ST_OSD_INIT_CHECK(hHandle);

    // ST_DBG("point(%d,%d), size(%dx%d)\n", stPoint.u32X, stPoint.u32Y,
    //    g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width, g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height);

    if (stPoint.u32X < 0 || stPoint.u32X >= g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width ||
        stPoint.u32Y < 0 || stPoint.u32Y >= g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height)
    {
        return 1;
    }

    pDst = (MI_U16 *)g_stRgnInfo[hHandle].stCanvasInfo.virtAddr;
    u32Stride = g_stRgnInfo[hHandle].stCanvasInfo.u32Stride;

    //CamOsPrintf("pDst:%p, u32Stride:%d, point(%d,%d)\n", pDst, u32Stride, stPoint.u32X, stPoint.u32Y);

    if (g_stRgnInfo[hHandle].ePixelFmt == E_MI_RGN_PIXEL_FORMAT_ARGB1555)
    {
        *(pDst + (u32Stride / 2 * stPoint.u32Y) + stPoint.u32X) = u32Color & 0xffff;
    }
    else if (g_stRgnInfo[hHandle].ePixelFmt == E_MI_RGN_PIXEL_FORMAT_I4)
    {
        // *(pDst + (u32Stride / 2 * stPoint.u32Y) + stPoint.u32X) = u32Color & 0xffff;
        //*(pDst + (u32Stride / 2 * stPoint.u32Y) + stPoint.u32X) = u32Color & 0x0f;
        if (stPoint.u32X % 2)
        {
            u8Value = (*((MI_U8 *)pDst + (u32Stride * stPoint.u32Y) + stPoint.u32X / 2) & 0x0F) | ((u32Color & 0x0f) << 4);
            // *((MI_U8 *)pDst + (u32Stride * stPoint.u32Y) + stPoint.u32X / 2) |= u32Color & 0x0f;
            // *((MI_U8 *)pDst + (u32Stride * stPoint.u32Y) + stPoint.u32X / 2) |= (u32Color & 0x0f) << 4;
            *((MI_U8 *)pDst + (u32Stride * stPoint.u32Y) + stPoint.u32X / 2) = u8Value;
        }
        else
        {
            // u8Value = ;
            // *((MI_U8 *)pDst + (u32Stride * stPoint.u32Y) + stPoint.u32X / 2) |= (u32Color & 0x0f) << 4;
            // *((MI_U8 *)pDst + (u32Stride * stPoint.u32Y) + stPoint.u32X / 2) |= u32Color & 0x0f;
            u8Value = (*((MI_U8 *)pDst + (u32Stride * stPoint.u32Y) + stPoint.u32X / 2) & 0xF0) | (u32Color & 0x0f);
            *((MI_U8 *)pDst + (u32Stride * stPoint.u32Y) + stPoint.u32X / 2) = u8Value;
        }
    }

    return MI_RGN_OK;
}

static MI_S32 ST_OSD_DrawLine(MI_RGN_HANDLE hHandle, ST_Point_T stPoint0, ST_Point_T stPoint1, MI_U8 u8BorderWidth, MI_U32 u32Color)
{
    int x, y;
    int i = 0, j = 0;
    float k, e, dx, dy;
    ST_Point_T stPoint;

    // ST_DBG("point0(%d,%d), point1(%d,%d)\n", stPoint0.u32X, stPoint0.u32Y, stPoint1.u32X, stPoint1.u32Y);

    ST_OSD_INIT_CHECK(hHandle);

    stPoint0.u32X = (stPoint0.u32X < 0) ? 0 : stPoint0.u32X;
    stPoint0.u32X = (stPoint0.u32X > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width) ?
                        g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width : stPoint0.u32X;
    stPoint0.u32Y = (stPoint0.u32Y < 0) ? 0 : stPoint0.u32Y;
    stPoint0.u32Y = (stPoint0.u32Y > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height) ?
                        g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height: stPoint0.u32Y;

    stPoint1.u32X = (stPoint1.u32X < 0) ? 0 : stPoint1.u32X;
    stPoint1.u32X = (stPoint1.u32X > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width) ?
                        g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width : stPoint1.u32X;
    stPoint1.u32Y = (stPoint1.u32Y < 0) ? 0 : stPoint1.u32Y;
    stPoint1.u32Y = (stPoint1.u32Y > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height) ?
                        g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height: stPoint1.u32Y;

    if (stPoint1.u32X > stPoint0.u32X)
    {
        dx = stPoint1.u32X - stPoint0.u32X;
        x = stPoint0.u32X;
    }
    else
    {
        dx = stPoint0.u32X - stPoint1.u32X;
        x = stPoint1.u32X;
    }

    dy = (float)(stPoint1.u32Y - stPoint0.u32Y);

    if (stPoint1.u32Y > stPoint0.u32Y)
    {
        dy = stPoint1.u32Y - stPoint0.u32Y;
        y = stPoint0.u32Y;
    }
    else
    {
        dy = stPoint0.u32Y - stPoint1.u32Y;
        y = stPoint1.u32Y;
    }

    // draw v line
    if (dx == 0)
    {
        for (j = 0; j < u8BorderWidth; j ++)
        {
            for (i = 0; i < dy; i ++)
            {
                stPoint.u32X = x + j;
                stPoint.u32Y = y + i;
                ST_OSD_DrawPoint(hHandle, stPoint, u32Color);
            }
        }

        return MI_RGN_OK;
    }

    // draw h line
    if (dy == 0)
    {
        for (j = 0; j < u8BorderWidth; j ++)
        {
            for (i = 0; i < dx; i ++)
            {
                stPoint.u32X = x + i;
                stPoint.u32Y = y + j;
                ST_OSD_DrawPoint(hHandle, stPoint, u32Color);
            }
        }

        return MI_RGN_OK;
    }

    // draw slant line
    e = -0.5;
    k = (float)(dy /dx);

    // for (j = 0; j < u8BorderWidth; j ++)
    {
        for (i = 0; i < dx; i ++)
        {
            stPoint.u32X = x + j;
            stPoint.u32Y = y + j;

            ST_OSD_DrawPoint(hHandle, stPoint, u32Color);
            x ++;
            e += k;
            if (e >= 0)
            {
                y ++;
                e --;
            }
        }
    }

    return MI_RGN_OK;
}

static MI_S32 ST_OSD_Update(MI_RGN_HANDLE hHandle)
{
    ST_OSD_INIT_CHECK(hHandle);

    // ExecFunc(MI_RGN_UpdateCanvas(hHandle), MI_RGN_OK);
    memset(&g_stRgnInfo[hHandle].stCanvasInfo, 0, (size_t)sizeof(MI_RGN_CanvasInfo_t));
    MI_RGN_UpdateCanvas(0, hHandle);

    return MI_RGN_OK;
}

static MI_S32 ST_OSD_GetCanvasInfo(MI_RGN_HANDLE hHandle, MI_RGN_CanvasInfo_t** ppstCanvasInfo)
{
    MI_S32 sRet = MI_RGN_OK;

    ST_OSD_INIT_CHECK(hHandle);
    sRet = MI_RGN_GetCanvasInfo(0, hHandle, &g_stRgnInfo[hHandle].stCanvasInfo);
    if (sRet == MI_RGN_OK)
    {
        *ppstCanvasInfo = &g_stRgnInfo[hHandle].stCanvasInfo;
    }

    return sRet;
}
static void ST_OSD_PrepareLine(MI_U8 u8BorderWidth, MI_U16 u16PixW, MI_U8 u8Color, MI_U8 *pu8Data)
{
    MI_U32 i = 0;

    for (i = 0; i < u8BorderWidth/2; i++)
    {
        pu8Data[i] = u8Color;
    }
    i += (u16PixW >> 1) - i * 2;
    memcpy(&pu8Data[i], pu8Data, u8BorderWidth / 2);
}

static MI_S32 ST_OSD_ClearRectFast(MI_RGN_HANDLE hHandle, ST_Rect_T stRect)
{
    MI_U8 *pu8StartAddr = NULL;
    MI_U32 i = 0;

    stRect.u32X = (stRect.u32X < 0) ? 0 : stRect.u32X;
    stRect.u32Y = (stRect.u32Y < 0) ? 0 : stRect.u32Y;
    stRect.u16PicW = ((stRect.u32X + stRect.u16PicW) > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width) ?
                        (g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width - stRect.u32X) : stRect.u16PicW;
    stRect.u16PicH = ((stRect.u32Y + stRect.u16PicH) > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height) ?
                        (g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height - stRect.u32Y) : stRect.u16PicH;
    pu8StartAddr = (MI_U8 *)(g_stRgnInfo[hHandle].stCanvasInfo.virtAddr + stRect.u32Y * g_stRgnInfo[hHandle].stCanvasInfo.u32Stride + (stRect.u32X >> 1));
    for (i = 0; i < stRect.u16PicH; i++)
    {
        memset(pu8StartAddr, 0, (stRect.u16PicW >> 1));
        pu8StartAddr += g_stRgnInfo[hHandle].stCanvasInfo.u32Stride;
    }

    return MI_RGN_OK;
}
static MI_S32 ST_OSD_DrawRectFast(MI_RGN_HANDLE hHandle, ST_Rect_T stRect, MI_U8 u8BorderWidth, MI_U32 u32Color)
{
    MI_U32 i = 0;
    MI_U8 u8Color = 0;
    MI_U8 *pu8StartAddr = NULL;

    ST_OSD_INIT_CHECK(hHandle);

    stRect.u32X = (stRect.u32X < 0) ? 0 : stRect.u32X;
    stRect.u32Y = (stRect.u32Y < 0) ? 0 : stRect.u32Y;
    stRect.u16PicW = ((stRect.u32X + stRect.u16PicW) > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width) ?
                        (g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width - stRect.u32X) : stRect.u16PicW;
    stRect.u16PicH = ((stRect.u32Y + stRect.u16PicH) > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height) ?
                        (g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height - stRect.u32Y) : stRect.u16PicH;
    u8Color = (u32Color & 0xF) | ((u32Color & 0xF) << 4);
    pu8StartAddr = (MI_U8 *)(g_stRgnInfo[hHandle].stCanvasInfo.virtAddr + stRect.u32Y * g_stRgnInfo[hHandle].stCanvasInfo.u32Stride + (stRect.u32X >> 1));
    for (i = 0; i < u8BorderWidth; i++)
    {
        memset(pu8StartAddr, u8Color, (stRect.u16PicW >> 1));
        pu8StartAddr += g_stRgnInfo[hHandle].stCanvasInfo.u32Stride;
    }
    for (i = 0; i < stRect.u16PicH - u8BorderWidth * 2; i++)
    {
        ST_OSD_PrepareLine(u8BorderWidth, stRect.u16PicW, u8Color, pu8StartAddr);
        pu8StartAddr += g_stRgnInfo[hHandle].stCanvasInfo.u32Stride;
    }
    for (i = 0; i < u8BorderWidth; i++)
    {
        memset(pu8StartAddr, u8Color, (stRect.u16PicW >> 1));
        pu8StartAddr += g_stRgnInfo[hHandle].stCanvasInfo.u32Stride;
    }

    return MI_RGN_OK;
}

static MI_S32 ST_OSD_DrawRect(MI_RGN_HANDLE hHandle, ST_Rect_T stRect, MI_U8 u8BorderWidth, MI_U32 u32Color)
{
    ST_Point_T stPoint0, stPoint1;

    ST_OSD_INIT_CHECK(hHandle);

    stRect.u32X = (stRect.u32X < 0) ? 0 : stRect.u32X;
    stRect.u32Y = (stRect.u32Y < 0) ? 0 : stRect.u32Y;
    stRect.u16PicW = ((stRect.u32X + stRect.u16PicW) > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width) ?
                        (g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width - stRect.u32X) : stRect.u16PicW;
    stRect.u16PicH = ((stRect.u32Y + stRect.u16PicH) > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height) ?
                        (g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height - stRect.u32Y) : stRect.u16PicH;

    stPoint0.u32X = stRect.u32X;
    stPoint0.u32Y = stRect.u32Y;

    stPoint1.u32X = stRect.u32X + stRect.u16PicW;
    stPoint1.u32Y = stRect.u32Y;
    ST_OSD_DrawLine(hHandle, stPoint0, stPoint1, u8BorderWidth, u32Color);

    stPoint0.u32X = stRect.u32X + stRect.u16PicW;
    stPoint0.u32Y = stRect.u32Y;

    stPoint1.u32X = stRect.u32X + stRect.u16PicW;
    stPoint1.u32Y = stRect.u32Y + stRect.u16PicH;
    ST_OSD_DrawLine(hHandle, stPoint0, stPoint1, u8BorderWidth, u32Color);

    stPoint0.u32X = stRect.u32X + stRect.u16PicW;
    stPoint0.u32Y = stRect.u32Y + stRect.u16PicH;

    stPoint1.u32X = stRect.u32X;
    stPoint1.u32Y = stRect.u32Y + stRect.u16PicH;
    ST_OSD_DrawLine(hHandle, stPoint1, stPoint0, u8BorderWidth, u32Color);

    stPoint0.u32X = stRect.u32X;
    stPoint0.u32Y = stRect.u32Y + stRect.u16PicH;

    stPoint1.u32X = stRect.u32X;
    stPoint1.u32Y = stRect.u32Y;
    ST_OSD_DrawLine(hHandle, stPoint1, stPoint0, u8BorderWidth, u32Color);

    return MI_RGN_OK;
}

static MI_S32 ST_OSD_FillRect(MI_RGN_HANDLE hHandle, ST_Rect_T stRect, MI_U32 u32Color)
{
    int i = 0;
    ST_Point_T stPoint0, stPoint1;

    ST_OSD_INIT_CHECK(hHandle);

    stRect.u32X = (stRect.u32X < 0) ? 0 : stRect.u32X;
    stRect.u32Y = (stRect.u32Y < 0) ? 0 : stRect.u32Y;
    stRect.u16PicW = ((stRect.u32X + stRect.u16PicW) > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width) ?
                        (g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Width - stRect.u32X) : stRect.u16PicW;
    stRect.u16PicH = ((stRect.u32Y + stRect.u16PicH) > g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height) ?
                        (g_stRgnInfo[hHandle].stCanvasInfo.stSize.u32Height - stRect.u32Y) : stRect.u16PicH;

    for (i = 0; i < stRect.u16PicH; i ++)
    {
        stPoint0.u32X = stRect.u32X;
        stPoint0.u32Y = stRect.u32Y + i;

        stPoint1.u32X = stRect.u32X + stRect.u16PicW;
        stPoint1.u32Y = stRect.u32Y + i;

        ST_OSD_DrawLine(hHandle, stPoint0, stPoint1, 1, u32Color);
    }

    return MI_RGN_OK;
}

static MI_S32 ST_OSD_DrawText(MI_RGN_HANDLE hHandle, ST_Point_T stPoint, const char *szString, MI_U32 u32Color, DMF_Font_Size_E enSize)
{
    ST_OSD_INIT_CHECK(hHandle);

    _OSD_DrawTextToCanvas(hHandle, stPoint, szString, u32Color, enSize);

    return MI_RGN_OK;
}

static MI_S32 ST_OSD_Clear(MI_RGN_HANDLE hHandle, ST_Rect_T *pstRect)
{
    MI_RGN_CanvasInfo_t *pstRgnCanvasInfo = NULL;

    ST_OSD_INIT_CHECK(hHandle);

    pstRgnCanvasInfo = &g_stRgnInfo[hHandle].stCanvasInfo;

    if (g_stRgnInfo[hHandle].ePixelFmt == E_MI_RGN_PIXEL_FORMAT_ARGB1555)
    {
        mi_sys_MemsetPa(pstRgnCanvasInfo->phyAddr, 0x23, pstRgnCanvasInfo->stSize.u32Height*pstRgnCanvasInfo->u32Stride );
    }
    else if (g_stRgnInfo[hHandle].ePixelFmt == E_MI_RGN_PIXEL_FORMAT_I4)
    {
        mi_sys_MemsetPa(pstRgnCanvasInfo->phyAddr, 0, pstRgnCanvasInfo->stSize.u32Height*pstRgnCanvasInfo->u32Stride);
    }

    return MI_RGN_OK;
}

#if defined(CONFIG_OSD_USE_BMP)
typedef int LONG;
typedef unsigned int DWORD;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int Uint32;

typedef struct __attribute__ ((__packed__)){
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
}BMPFILEHEADER_T;

typedef struct __attribute__ ((__packed__)){
    DWORD      biSize;
    LONG       biWidth;
    LONG       biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    LONG       biXPelsPerMeter;
    LONG       biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;
}BMPINFOHEADER_T;
typedef struct __attribute__ ((__packed__))
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved; // 0
}RGBQUAD_T;
static BMPFILEHEADER_T bmpFileInfo;
static BMPINFOHEADER_T bmpInfoheadr;

static MI_S32 bmp_to_argb1555(unsigned char **ppBMPaddr, MI_S32 s32Len)
{
    MI_U32 source_pos = 0, dest_pos = 0;
    unsigned char red = 0, green = 0, blue = 0;
    unsigned int cnt = 0;
    unsigned int color_data = 0;
    unsigned short out_data = 0;
    MI_PHY phyOSDPicFile1;
    unsigned char *paddr_tmp;
    MI_S32 s32Ret = MI_SUCCESS;

    memcpy(&bmpFileInfo, *ppBMPaddr+source_pos, (size_t)sizeof(BMPFILEHEADER_T));
    source_pos += sizeof(BMPFILEHEADER_T);
    memcpy(&bmpInfoheadr, *ppBMPaddr+source_pos, (size_t)sizeof(BMPINFOHEADER_T));
    source_pos += sizeof(BMPINFOHEADER_T);
    CamOsPrintf("bmp width %d  height %d\n", bmpInfoheadr.biWidth, bmpInfoheadr.biHeight);

    s32Len = bmpInfoheadr.biWidth * bmpInfoheadr.biHeight *2;

    s32Ret = MI_SYS_MMA_Alloc(0, NULL,s32Len,&phyOSDPicFile1);
    if(s32Ret != MI_SUCCESS){
        return -1;
    }
    s32Ret = MI_SYS_Mmap(phyOSDPicFile1, s32Len, (void **)&paddr_tmp, TRUE);
    if(s32Ret != MI_SUCCESS){
        return -1;
    }

    dest_pos = bmpInfoheadr.biWidth * (bmpInfoheadr.biHeight - 1) * 2;

    while (1)
    {
        memcpy(&color_data, *ppBMPaddr+source_pos , 3);
        source_pos+=3;
        cnt++;
        blue = color_data & 0xFF;
        red = (color_data & 0xFF0000) >> 16;
        green = (color_data & 0xFF00) >> 8;
        out_data = 0x8000 | ((red & 0xF8) << 7) | ((green & 0xF8) << 2) | ((blue & 0xF8) >> 3);
        memcpy(paddr_tmp+dest_pos, &out_data, 2);
        dest_pos+=2;

        if (cnt == bmpInfoheadr.biHeight * bmpInfoheadr.biWidth)
        {
            MI_SYS_MMA_Free(0, phyOSDPicFile);
            phyOSDPicFile = phyOSDPicFile1;
            *ppBMPaddr = paddr_tmp;
            break;
        }
        if (cnt / bmpInfoheadr.biWidth != (cnt - 1) / bmpInfoheadr.biWidth)
        {
            dest_pos = bmpInfoheadr.biWidth * (bmpInfoheadr.biHeight - (cnt / bmpInfoheadr.biWidth + 1)) * 2;
        }
    }

    return 0;
}
#endif

static void ST_PreReadOSDPicFile(void)
{
    CamFsRet_e eRet = CAM_FS_OK;
    CamFsFd tFD;
    MI_S32 s32FileLen = 0;
    ST_TestFileInfo_t *pstTestFileInfo;
    MI_S32 s32Ret = MI_SUCCESS;

    pstTestFileInfo = &g_stFileInfo[0];
    eRet = CamFsOpen(&tFD, pstTestFileInfo->fileName, O_RDONLY, 0777);
    if(CAM_FS_OK == eRet)
    {
        s32FileLen = CamFsSeek(tFD, 0, SEEK_END);
        if((s32FileLen != -1) && (CamFsSeek(tFD, 0, SEEK_SET) != -1) )
        {
                s32Ret = MI_SYS_MMA_Alloc(0, NULL,s32FileLen,&phyOSDPicFile);
                if(s32Ret != MI_SUCCESS){
                    return;
                }
                s32Ret = MI_SYS_Mmap(phyOSDPicFile, s32FileLen, (void **)&pstTestFileInfo->pTestFileAddr, TRUE);
                if(s32Ret != MI_SUCCESS){
                    MI_SYS_MMA_Free(0, phyOSDPicFile);
                    return;
                }
                if(pstTestFileInfo->pTestFileAddr != NULL)
                {
                    memset(pstTestFileInfo->pTestFileAddr, 0, s32FileLen);
                    CamFsRead(tFD, pstTestFileInfo->pTestFileAddr, s32FileLen);
                    eRet = CAM_FS_OK;
                    CamOsPrintf("open:%s ok,filesize:%d \n",pstTestFileInfo->fileName, s32FileLen);
                    MI_SYS_FlushInvCache(pstTestFileInfo->pTestFileAddr, s32FileLen);
#if defined(CONFIG_OSD_USE_BMP)
                    s32Ret = bmp_to_argb1555(&pstTestFileInfo->pTestFileAddr, s32FileLen);
                    if(s32Ret != MI_SUCCESS){
                        CamOsPrintf("bmp_to_argb1555 fail!! \n");
                        CamFsClose(tFD);
                        return;
                    }
#endif
                }
                else
                {
                    CamOsPrintf("%s open:%s ok, but MemAlloc[%d]filesize fail \n",__FUNCTION__, pstTestFileInfo->fileName, s32FileLen);
                }
        }
        else
        {
            eRet = CAM_FS_FAIL;
            CamOsPrintf("%s seek file[%s] fail \n",__FUNCTION__, pstTestFileInfo->fileName);
        }
        CamFsClose(tFD);
    }
    else
    {
        eRet = CAM_FS_FAIL;
        CamOsPrintf("%s open %s fail \n",__FUNCTION__, pstTestFileInfo->fileName);
    }
}
static void ST_PreReadOSDBitmapFile(void)
{
    int i = 0, j = 0;
    MI_PHY pPhyAddr = NULL;
    void * pBitMapAddr = NULL;
    DMF_BitMapFile_S *pstDMFBitMapFile = NULL;
    CamFsRet_e eRet = CAM_FS_OK;
    CamFsFd tFD;
    char szBitmapFile[128];
    MI_S32 s32FileLen = 0;
    MI_S32 s32Ret = MI_SUCCESS;

    CamOsSprintf(&szBitmapFile[0], "%s/%s", application_selector_get_rofile_path(), pstDMFBitMapFile->szFile);

    for (i = 0; i < (int)DMF_Font_Type_BUTT; i ++)
    {
        for (j = 0; j < (int)DMF_Font_Size_BUTT; j ++)
        {
            pstDMFBitMapFile = &g_dmf_bitmapfile[i][j];
            eRet = CamFsOpen(&tFD, &szBitmapFile[0], O_RDONLY, 0777);
            if(CAM_FS_OK == eRet)
            {
                s32FileLen = CamFsSeek(tFD, 0, SEEK_END);
                if((s32FileLen != -1) && (CamFsSeek(tFD, 0, SEEK_SET) != -1) )
                {
                    s32Ret = MI_SYS_MMA_Alloc(0, NULL,s32FileLen,&pPhyAddr);
                    if(s32Ret == MI_SUCCESS)
                    {
                        pBitMapAddr = mi_sys_Vmap(pPhyAddr,s32FileLen,TRUE);
                        memset(pBitMapAddr,0,s32FileLen);
                        CamFsRead(tFD, pBitMapAddr, s32FileLen);
                        MI_SYS_FlushInvCache(pBitMapAddr, s32FileLen);
                        pstDMFBitMapFile->pBitMapAddr = pPhyAddr;
                        eRet = CAM_FS_OK;
                        CamOsPrintf("%s open:%s ok,filesize:%d \n",__FUNCTION__, &szBitmapFile[0], s32FileLen);
                    }
                    else
                    {
                        CamOsPrintf("%s open:%s ok, but MemAlloc[%d]filesize fail \n",__FUNCTION__, &szBitmapFile[0], s32FileLen);
                    }
                }
                else
                {
                    eRet = CAM_FS_FAIL;
                    CamOsPrintf("%s seek file[%s] fail \n",__FUNCTION__, &szBitmapFile[0]);
                }
                CamFsClose(tFD);
            }
            else
            {
                eRet = CAM_FS_FAIL;
                CamOsPrintf("%s open %s fail \n",__FUNCTION__, &szBitmapFile[0]);
            }
        }
    }
}
static static MI_S32 CreatePicOsd(MI_RGN_HANDLE handle, ST_TestFileInfo_t *pstTestFileInfo)
{
    MI_RGN_CanvasInfo_t stCanvasInfo;
    MI_U32 u32FileSize = 0;
    MI_U16 u16CopyLineBytes = 0;
    MI_S32 i = 0;

    // read pic data
    switch (pstTestFileInfo->ePixelFmt)
    {
        case E_MI_RGN_PIXEL_FORMAT_ARGB1555:
        case E_MI_RGN_PIXEL_FORMAT_ARGB4444:
        case E_MI_RGN_PIXEL_FORMAT_RGB565:
            u16CopyLineBytes = pstTestFileInfo->u16RgnWidth*2;
            break;
        case E_MI_RGN_PIXEL_FORMAT_I2:
            u16CopyLineBytes = pstTestFileInfo->u16RgnWidth/4;
            break;
        case E_MI_RGN_PIXEL_FORMAT_I4:
            u16CopyLineBytes = pstTestFileInfo->u16RgnWidth/2;
            break;
        case E_MI_RGN_PIXEL_FORMAT_I8:
            u16CopyLineBytes = pstTestFileInfo->u16RgnWidth;
            break;
        case E_MI_RGN_PIXEL_FORMAT_ARGB8888 :
            u16CopyLineBytes = pstTestFileInfo->u16RgnWidth*4;
            break;
        default:
            CamOsPrintf("pixel format is not supported\n");
            return -1;
    }

    u32FileSize = u16CopyLineBytes * pstTestFileInfo->u16RgnHeight;
    CamOsPrintf("file fmt:%d, w:%d, h:%d,FileSize:%d\n", pstTestFileInfo->ePixelFmt, pstTestFileInfo->u16RgnWidth, pstTestFileInfo->u16RgnHeight,u32FileSize);

    // copy data to bitmap struct
    CamOsMutexLock(&g_stRNGOsdMutex);
    memset(&stCanvasInfo, 0, (size_t)sizeof(MI_RGN_CanvasInfo_t));
    STCHECKRESULT(MI_RGN_GetCanvasInfo(0, handle, &stCanvasInfo));
    for (i = 0; i < pstTestFileInfo->u16RgnHeight; i++)
        memcpy((MI_U8*)stCanvasInfo.virtAddr+i*stCanvasInfo.u32Stride, pstTestFileInfo->pTestFileAddr+i*u16CopyLineBytes, u16CopyLineBytes);
    STCHECKRESULT(MI_RGN_UpdateCanvas(0, handle));
    CamOsMutexUnlock(&g_stRNGOsdMutex);

    MI_SYS_MMA_Free(0, phyOSDPicFile);
    pstTestFileInfo->pTestFileAddr = NULL;

    return MI_SUCCESS;
}

#if defined(CONFIG_IPU_IN_RTOS_ENABLE)
static int _GetFileSize(const char *file)
{
    CamFsFd tFD;
    MI_U32 filelen;
    MI_S32 s32Ret;

    s32Ret = CamFsOpen(&tFD, file, O_RDONLY, 0644);
    if (CAM_FS_OK != s32Ret)
    {
        CamOsPrintf("Open %s FAIL\n", file);
        return s32Ret;
    }
    else
    {
        CamOsPrintf("Open %s Success\n", file);
        filelen = CamFsSeek(tFD, 0, SEEK_END);
        CamFsClose(tFD);
        return filelen;
    }
}

static void *Rtos_DIVPCheckFD_Func(void* p)
{
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE stBufHandle;
    MI_S32 s32Ret;

    stChnPort.eModId = E_MI_MODULE_ID_SCL;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = 0;
    stChnPort.u32PortId = 3;

    while (0 == g_u8dlaosdoff)
    {
        CamOsMsSleep(10);
        if(g_u8dlaosdoff)
          break;
        s32Ret = MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &stBufHandle);
        if(MI_SUCCESS != s32Ret)
        {
            u32waitflag = u32waitflag +1;
            //CamOsPrintf("dla wait MI_SYS_ChnOutputPortGetBuf err,:%x,u32waitflag:%d\n",s32Ret ,u32waitflag);
            if(u32waitflag <= 300)
                continue;
            else if(u32waitflag > 300)
                break;
        }
        else
        {
           u32waitflag = 0;
        }
        //CamOsPrintf("[%s]channelId[%u] portlId[%u] type[%d] [%dx%d] format[%d] layout[%d]\n",__func__, stChnPort.u32ChnId, stChnPort.u32PortId, stBufInfo.eBufType,
        //    stBufInfo.stFrameData.u16Width, stBufInfo.stFrameData.u16Height, stBufInfo.stFrameData.ePixelFormat, stBufInfo.stFrameData.ePhylayoutType);
        if (dla_ready) {
            dla_InferenceFD(channel, &dla_desc, &stBufInfo);
        }
        MI_SYS_ChnOutputPortPutBuf(stBufHandle);
    }
    return NULL;
}

static int _MI_IPU_MemoryReadFunc(void *dst_buf, int offset, int size, char *ctx)
{
    void *ptr = (void*)ctx;
    memcpy(dst_buf, ptr + offset, size);
    return 0;
}

static int _MI_IPU_MemoryReadFunc2(void *dst_buf, int offset, int size, char *ctx)
{
    void *ptr = (void*)ctx;
    memcpy(dst_buf + offset, ptr, size);
    return 0;
}


extern int MI_IPU_Scaling_Freq(unsigned int freq_index);
static void ReleasePreRes(void);
static int dla_init(char *model, MI_IPU_SubNet_InputOutputDesc_t *desc)
{
    MI_S32 s32Ret, buf_depth = 1;
	MI_IPU_CHN channel;
	MI_IPU_DevAttr_t stDevAttr;
	MI_IPUChnAttr_t chnAttr;
	MI_IPU_OfflineModelStaticInfo_t stStaticInfo;
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
	s32Ret = MI_IPU_GetOfflineModeStaticInfo(_MI_IPU_MemoryReadFunc, model, &stStaticInfo);
	if (s32Ret != MI_SUCCESS)
    {
		CamOsPrintf("fail to get %s static info\n", model);
		return s32Ret;
	}

	stDevAttr.u32MaxVariableBufSize = stStaticInfo.u32VariableBufferSize;

	s32Ret = MI_IPU_CreateDevice(&stDevAttr, NULL, NULL, 0);
	if (s32Ret != MI_SUCCESS)
    {
		CamOsPrintf("fail to create ipu device\n");
		return s32Ret;
	}

	memset(&chnAttr, 0, (size_t)sizeof(chnAttr));
	chnAttr.u32InputBufDepth = buf_depth;
	chnAttr.u32OutputBufDepth = buf_depth;
	s32Ret = MI_IPU_CreateCHN(&channel, &chnAttr, _MI_IPU_MemoryReadFunc2, model);
	if (s32Ret != MI_SUCCESS)
    {
		CamOsPrintf("fail to create ipu channel%d\n", channel);
		MI_IPU_DestroyDevice();
		return s32Ret;
	}

    MI_IPU_Scaling_Freq(pCameraBootSetting->u32IPUClk);

	s32Ret = MI_IPU_GetInOutTensorDesc(channel, desc);
	if (s32Ret)
    {
		CamOsPrintf("fail to get network(%d) description\n", channel);
		MI_IPU_DestroyCHN(channel);
		MI_IPU_DestroyDevice();
		return s32Ret;
	}
    CamOsPrintf("dla_init success\n");
    return MI_SUCCESS;
}

static int dla_exit(MI_IPU_CHN chn)
{
    MI_IPU_DestroyCHN(chn);
    MI_IPU_DestroyDevice();
    ReleasePreRes();
    return MI_SUCCESS;
}
static MI_S32 ST_CompareRect (ST_Rect_T stRectArea)
{
    MI_S32 s32Ret = MI_SUCCESS;
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();


    if(((pCameraBootSetting->u32IPUActiveAreaX + pCameraBootSetting->u32IPUActiveAreaW) > _stVpeResolution[0].u32OutWidth) \
        || ((pCameraBootSetting->u32IPUActiveAreaY + pCameraBootSetting->u32IPUActiveAreaH) > _stVpeResolution[0].u32OutHeight))
    {
        s32Ret = 1;
        CamOsPrintf("%s input error:pCameraBootSetting.IPUActiveAreaX[%d]+IPUActiveAreaW[%d]>OutWidth[%d],IPUActiveAreaY[%d]+IPUActiveAreaH[%d]>OutHeight[%d] \n", \
                    __FUNCTION__, pCameraBootSetting->u32IPUActiveAreaX, pCameraBootSetting->u32IPUActiveAreaW, _stVpeResolution[0].u32OutWidth, \
                    pCameraBootSetting->u32IPUActiveAreaY, pCameraBootSetting->u32IPUActiveAreaH, _stVpeResolution[0].u32OutHeight);
    }
    else
    {
        #if 0
        CamOsPrintf("%s input:pCameraBootSetting.IPUActiveAreaX[%d].IPUActiveAreaW[%d].IPUActiveAreaY[%d].IPUActiveAreaH[%d], stRectArea.u32X[%d].u32Y[%d].u32W[%d].u32H[%d] and OutWidth[%d] OutHeight[%d] \n", \
                    __FUNCTION__, pCameraBootSetting->u32IPUActiveAreaX, pCameraBootSetting->u32IPUActiveAreaW, \
                    pCameraBootSetting->u32IPUActiveAreaY, pCameraBootSetting->u32IPUActiveAreaH, stRectArea.u32X, stRectArea.u32Y, stRectArea.u16PicW, stRectArea.u16PicH, _stVpeResolution[0].u32OutWidth, _stVpeResolution[0].u32OutHeight);
        #endif
        if((pCameraBootSetting->u32IPUActiveAreaX > stRectArea.u32X) || (pCameraBootSetting->u32IPUActiveAreaY > stRectArea.u32Y))
        {
            s32Ret = 1;
        }
        else if((pCameraBootSetting->u32IPUActiveAreaX <= stRectArea.u32X)&&(stRectArea.u16PicW <= (pCameraBootSetting->u32IPUActiveAreaW - (stRectArea.u32X - pCameraBootSetting->u32IPUActiveAreaX))) \
            && (pCameraBootSetting->u32IPUActiveAreaY <= stRectArea.u32Y)&&(stRectArea.u16PicH <= (pCameraBootSetting->u32IPUActiveAreaH - (stRectArea.u32Y - pCameraBootSetting->u32IPUActiveAreaY))))
        {
            s32Ret = MI_SUCCESS;
        }
        else
        {
            s32Ret = 1;
        }
    }

    return s32Ret;
}

static MI_S32 send_FD_result(FD_coordinate_t *stCoordinate, MI_S32 u32Cnt)
{
    MI_S32 s32Ret = MI_RGN_OK;
    MI_RGN_CanvasInfo_t *pCanvasInfo;
    ST_Rect_T stRectArea;
    MI_U32 u32Color = 0;
    MI_U32 u32i = 0;

    if ((NULL == stCoordinate) || (0 == u32Cnt))
    {
        CamOsMutexLock(&g_stRNGOsdMutex);
        s32Ret = ST_OSD_GetCanvasInfo(VPE_PORT0_OSD_FOR_FD_HANDLE, &pCanvasInfo);
        if (s32Ret == MI_RGN_OK)
        {
            ST_OSD_Clear(VPE_PORT0_OSD_FOR_FD_HANDLE, NULL);
        }
        //CamOsPrintf("ST_OSD_DrawRectFast OSD_Clear\n");
        ST_OSD_Update(VPE_PORT0_OSD_FOR_FD_HANDLE);
        CamOsMutexUnlock(&g_stRNGOsdMutex);
    }
    else
    {
        CamOsMutexLock(&g_stRNGOsdMutex);
        s32Ret = ST_OSD_GetCanvasInfo(VPE_PORT0_OSD_FOR_FD_HANDLE, &pCanvasInfo);
        if (s32Ret == MI_RGN_OK)
        {
            ST_OSD_Clear(VPE_PORT0_OSD_FOR_FD_HANDLE, NULL);
            for(u32i = 0; u32i < u32Cnt; u32i++)
            {
                stRectArea.u32X = (MI_U32)(stCoordinate[u32i].x1*_stVpeResolution[0].u32OutWidth/SGS_IMG_WIDTH);
                stRectArea.u32Y = (MI_U32)(stCoordinate[u32i].y1*_stVpeResolution[0].u32OutHeight/SGS_IMG_HEIGHT);
                stRectArea.u16PicW = (MI_U32)((stCoordinate[u32i].x2-stCoordinate[u32i].x1)*_stVpeResolution[0].u32OutWidth/SGS_IMG_WIDTH);
                stRectArea.u16PicH = (MI_U32)((stCoordinate[u32i].y2-stCoordinate[u32i].y1)*_stVpeResolution[0].u32OutHeight/SGS_IMG_HEIGHT);
                u32Color =RGB2PIXEL1555(0, 255, 0);
                if(MI_SUCCESS == ST_CompareRect(stRectArea))
                {
                    ST_OSD_DrawRect(VPE_PORT0_OSD_FOR_FD_HANDLE, stRectArea, OSD_RECT_BORDERWIDTH, u32Color);
                    CamOsPrintf("[IPU] DrawRectFast[%d] x=%d, y=%d, w=%d, h=%d\n", u32i,stRectArea.u32X, stRectArea.u32Y, stRectArea.u16PicW, stRectArea.u16PicH);
                }
                else
                {
                    //CamOsPrintf("%s [%d] overrun\n",__FUNCTION__ , u32i);
                }
            }
        }
        ST_OSD_Update(VPE_PORT0_OSD_FOR_FD_HANDLE);
        CamOsMutexUnlock(&g_stRNGOsdMutex);
        if(1 == inttestflag)
        {
            BootTimestampRecord(__LINE__, "send_FD_result fist");
        }
    }
    return s32Ret;
}

extern int postprocess_c(const ProcessInfo_t *ptr, BoxInfo (*pBoxInfo)[AI_MAX_RESULT]);
static int dla_InferenceFD(MI_IPU_CHN chn, MI_IPU_SubNet_InputOutputDesc_t *desc, MI_SYS_BufInfo_t *stBufInfo)
{

	MI_S32 s32Ret;
	MI_IPU_TensorVector_t inputV, outputV;

    memset(&inputV, 0, (size_t)sizeof(MI_IPU_TensorVector_t));
	inputV.u32TensorCount = desc->u32InputTensorCount;
    memset(&outputV, 0, (size_t)sizeof(MI_IPU_TensorVector_t));
    if (MI_SUCCESS != (s32Ret = MI_IPU_GetOutputTensors(chn, &outputV)))
    {
        CamOsPrintf("MI_IPU_GetOutputTensors error, ret[0x%x], %d.\n", s32Ret, chn);
        return -1;
    }

	if (stBufInfo->eBufType == E_MI_SYS_BUFDATA_RAW)
    {
		//CamOsPrintf("InferenceFDANetwork E_MI_SYS_BUFDATA_RAW\n");
		inputV.astArrayTensors[0].phyTensorAddr[0] = stBufInfo->stRawData.phyAddr;
		inputV.astArrayTensors[0].ptTensorData[0] = stBufInfo->stRawData.pVirAddr;
	}
    else if (stBufInfo->eBufType == E_MI_SYS_BUFDATA_FRAME)
    {
		//CamOsPrintf("InferenceFDANetwork E_MI_SYS_BUFDATA_FRAME\n");
		inputV.astArrayTensors[0].phyTensorAddr[0] = stBufInfo->stFrameData.phyAddr[0];
		inputV.astArrayTensors[0].ptTensorData[0] = stBufInfo->stFrameData.pVirAddr[0];
		inputV.astArrayTensors[0].phyTensorAddr[1] = stBufInfo->stFrameData.phyAddr[1];
		inputV.astArrayTensors[0].ptTensorData[1] = stBufInfo->stFrameData.pVirAddr[1];
	}

	s32Ret = MI_IPU_Invoke(chn, &inputV, &outputV);
	if (s32Ret != MI_SUCCESS)
    {
		CamOsPrintf("[channel%u]IPU invoke error:%d\n", chn, s32Ret);
		// send data
		// fake interface
		send_FD_result(NULL, 0);
	}
    else
    {
        MI_S32 loop;
        ProcessInfo_t p_info_t;
        //CamOsPrintf("[OutputTensorCnt=%d][channel%u]IPU invoke success:%d\n",desc->u32OutputTensorCount, chn, s32Ret);
        for (int j = 0; j < desc->u32OutputTensorCount; j++)
        {
            p_info_t.data[j] = (float *)(outputV.astArrayTensors[j].ptTensorData[0]);
            //CamOsPrintf("ipuinfo data: %d: %d, %d, %d, %d\n", j, p_info_t.data[j][0]*10000, p_info_t.data[j][1]*10000, p_info_t.data[j][2]*10000, p_info_t.data[j][3]*10000);
        }
        p_info_t.frame_size_w = SGS_IMG_WIDTH;
        p_info_t.frame_size_h = SGS_IMG_HEIGHT;
        p_info_t.net_size_w = SGS_IMG_WIDTH;
        p_info_t.net_size_h = SGS_IMG_HEIGHT;
        p_info_t.num_class = 1;
        p_info_t.threshold = 0.6;
        p_info_t.nms_threshold = 0.5;
        BoxInfo result_c[AI_MAX_RESULT];
        FD_coordinate_t result[AI_MAX_RESULT];

        //BootTimestampRecord(__LINE__, "dla1");
        int count = postprocess_c(&p_info_t, &result_c);
        //BootTimestampRecord(__LINE__, "dla2");

        for (loop = 0;loop < count;loop++)
        {
            result[loop].x1 = result_c[loop].x1;
            result[loop].x2 = result_c[loop].x2;
            result[loop].y1 = result_c[loop].y1;
            result[loop].y2 = result_c[loop].y2;
            //CamOsPrintf("dla_InferenceFD[%d] x1=%f, y1=%f, x2=%f, y2=%f\n", loop,result[loop].x1, result[loop].y1, result[loop].x2,result[loop].y2);
        }

        // send data
        if ((0==inttestflag) && (0 != count))
        {
            BootTimestampRecord(__LINE__, "dla_InferenceFD fist");
            inttestflag = 1;
        }
        else if ((1==inttestflag) && (0 != count))
        {
            inttestflag = 2;
        }
        send_FD_result(result, count);
    }
    MI_IPU_PutOutputTensors(chn, &outputV);
    return s32Ret;
}

static int _MI_IPU_DefaultReadFunc(void *dst_buf, int offset, int size, char *ctx)
{
    MI_S32 s32Ret, len = 0, read_size = 0;
    const char *filename = (const char*)ctx;
    CamFsFd tFD;

    s32Ret = CamFsOpen(&tFD, filename, O_RDONLY, 0644);
    if (CAM_FS_OK != s32Ret)
    {
        CamOsPrintf("Open %s FAIL\n", filename);
        return s32Ret;
    }

    if (offset > 0)
        CamFsSeek(tFD, offset, SEEK_SET);

    while (len < size)
    {
        read_size = CamFsRead(tFD, dst_buf+len, size-len);
        if (read_size < 0)
        {
            CamOsPrintf("read error in %s\n", filename);
            CamFsClose(tFD);
            return read_size;
        }
        else if (!read_size)
        {
            CamFsClose(tFD);
            return 0;
        }
        len += read_size;
    }
    CamFsClose(tFD);
    return 0;
}

static int ST_PreReadIPUFile(void)
{
    char network_file[128] = {0};
#ifdef PERSON_DETECT
    char *network_name = "sypdy5.3603013_fixed.sim_sgsimg.img";//"sypdy2.3603013_fixed.sim_sgsimg_yuv.img";
#else
    char *network_name = "caffe_fda_fixed.tflite_sgsimg.img";
#endif

    MI_S32 s32Ret;
    MI_S32 imgSize = 0;

    //BootTimestampRecord(__LINE__, "pre-read0");
    memIPUModel = NULL;

    CamOsSprintf(network_file, "%s/%s", application_selector_get_rofile_path(), network_name);
    /* read model file */
    imgSize = _GetFileSize(network_file);
    if (imgSize < 0)
    {
        return FALSE;
    }

    s32Ret = MI_SYS_MMA_Alloc(0, NULL, imgSize, &phyModel);
    if (s32Ret != MI_SUCCESS)
    {
        MI_SYS_MMA_Free(0, phyModel);
        memIPUModel = NULL;
        CamOsPrintf("_MI_IPU_MMA_Alloc fail\n");
        return E_IPU_ERR_NOMEM;
    }
    s32Ret = MI_SYS_Mmap(phyModel, imgSize, &memIPUModel, TRUE);
    if (s32Ret != MI_SUCCESS)
    {
        MI_SYS_MMA_Free(0, phyModel);
        memIPUModel = NULL;
        CamOsPrintf("MI_SYS_Mmap fail\n");
        return E_IPU_ERR_MAP;
    }
    s32Ret = _MI_IPU_DefaultReadFunc(memIPUModel, 0, imgSize, network_file);
    if (s32Ret)
    {
        MI_SYS_Munmap(memIPUModel, imgSize);
        MI_SYS_MMA_Free(0, phyModel);
        memIPUModel = NULL;
        CamOsPrintf("pre-read %s fail\n", network_file);
        return E_IPU_ERR_FILE_OPERATION;
    }

    MI_SYS_FlushInvCache(memIPUModel, imgSize);
    modelSize = imgSize;
    CamOsPrintf(" modelSize=%u\n", modelSize);
    //BootTimestampRecord(__LINE__, "pre-read1");
    return TRUE;
}

static void ReleasePreRes(void)
{
    if (memIPUModel)
    {
        MI_SYS_Munmap(memIPUModel, modelSize);
        MI_SYS_MMA_Free(0, phyModel);
        memIPUModel = NULL;
    }
}

static void FD_init(void)
{
    if (memIPUModel)
    {
        if (dla_init(memIPUModel, &dla_desc) == MI_SUCCESS)
            dla_ready = 1;
    }
}

static void FD_deinit(void)
{
    if (g_u8dlaosdoff == 1 && dla_ready == 1)
    {
#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)
        ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
        CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
        if(TRUE == pCameraBootSetting->u8RegionsCun)
        {
            MI_RGN_ChnPort_t stBufRNGInfo;
            memset(&stBufRNGInfo, 0, (size_t)sizeof(MI_RGN_ChnPort_t));
            stBufRNGInfo.eModId = E_MI_MODULE_ID_SCL;
            stBufRNGInfo.s32DevId = 0;
            stBufRNGInfo.s32ChnId = pstStreamAttr[0].u32InputChn;
            stBufRNGInfo.s32PortId = pstStreamAttr[0].u32InputPort;
            MI_RGN_DetachFromChn(0, VPE_PORT0_OSD_FOR_FD_HANDLE, &stBufRNGInfo);
            ST_OSD_Destroy(VPE_PORT0_OSD_FOR_FD_HANDLE);
        }
#endif
        dla_exit(channel);
        dla_ready = 0;
        if (threadid != NULL)
        {
            CamOsThreadStop(threadid);
            threadid = NULL;
        }
    }
}

static int Rtk_DLA_Autorun(CLI_t *pCli, char *p)
{
    MI_SYS_ChnPort_t stDstChnPort;
    MI_RGN_Attr_t stRgnAttr;
    MI_RGN_ChnPort_t stAttachChnPort;
    MI_RGN_ChnPortParam_t stChnPortParam;
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MI_SCL_OutPortParam_t  stSclOutputParam;
    MI_ISP_OutPortParam_t  stIspOutputParam;

    BootTimestampRecord(__LINE__, "dla");
    CamOsPrintf("Func :%s\n", __FUNCTION__);
    if(TRUE == pCameraBootSetting->u8enableIPU)
    {
        FD_init();
        if (!dla_ready)
            return -1;
        memset(&stIspOutputParam, 0x0, (size_t)sizeof(MI_ISP_OutPortParam_t));
        memset(&stSclOutputParam, 0x0, (size_t)sizeof(MI_SCL_OutPortParam_t));
        STCHECKRESULT(MI_ISP_GetInputPortCrop(0, 0, &stIspOutputParam.stCropRect));
        stSclOutputParam.stSCLOutCropRect.u16X = 0;
        stSclOutputParam.stSCLOutCropRect.u16Y = 0;
        stSclOutputParam.stSCLOutCropRect.u16Width = stIspOutputParam.stCropRect.u16Width;
        stSclOutputParam.stSCLOutCropRect.u16Height = stIspOutputParam.stCropRect.u16Height;
        stSclOutputParam.stSCLOutputSize.u16Width = SGS_IMG_WIDTH;
        stSclOutputParam.stSCLOutputSize.u16Height = SGS_IMG_HEIGHT;
        stSclOutputParam.bMirror = FALSE;
        stSclOutputParam.bFlip = FALSE;
        stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_NONE;
        stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        STCHECKRESULT(MI_SCL_SetOutputPortParam(0, 0, 3, &stSclOutputParam));
        STCHECKRESULT(MI_SCL_EnableOutputPort(0, 0, 3));

        stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
        stDstChnPort.u32DevId = 0;
        stDstChnPort.u32ChnId = 0;
        stDstChnPort.u32PortId = 3;
        STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stDstChnPort, 3, 4));

#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)
        if(TRUE == pCameraBootSetting->u8RegionsCun)
        {
            memset(&stRgnAttr, 0, (size_t)sizeof(MI_RGN_Attr_t));
            stRgnAttr.eType = E_MI_RGN_TYPE_OSD;
            stRgnAttr.stOsdInitParam.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_ARGB1555;
            stRgnAttr.stOsdInitParam.stSize.u32Width = _stVpeResolution[0].u32OutWidth;
            stRgnAttr.stOsdInitParam.stSize.u32Height = _stVpeResolution[0].u32OutHeight;
            STCHECKRESULT(ST_OSD_Create(VPE_PORT0_OSD_FOR_FD_HANDLE, &stRgnAttr));

            // attach
            memset(&stAttachChnPort, 0, (size_t)sizeof(MI_RGN_ChnPort_t));
            stAttachChnPort.eModId = E_MI_MODULE_ID_SCL;
            stAttachChnPort.s32DevId = 0;
            stAttachChnPort.s32ChnId =  pstStreamAttr[0].u32InputChn;
            stAttachChnPort.s32PortId = pstStreamAttr[0].u32InputPort;
            memset(&stChnPortParam, 0, (size_t)sizeof(MI_RGN_ChnPortParam_t));
            stChnPortParam.bShow = TRUE;
            stChnPortParam.stPoint.u32X = 0;
            stChnPortParam.stPoint.u32Y = 0;
            stChnPortParam.unPara.stOsdChnPort.u32Layer = VPE_PORT0_OSD_FOR_FD_HANDLE;
            stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
            stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;
            stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 0xFF;
            STCHECKRESULT(MI_RGN_AttachToChn(0, VPE_PORT0_OSD_FOR_FD_HANDLE, &stAttachChnPort, &stChnPortParam));
        }
#endif
        STCHECKRESULT(CamOsThreadCreate(&threadid, &threadAttr_FD, Rtos_DIVPCheckFD_Func, &threadid));
    }
    return 0;
}
#endif

static void ST_OSDTimer_Exit(void)
{
    if (g_u8timeosdoff == 1)
    {
        MI_RGN_ChnPort_t stBufRNGInfo;
        ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
        CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
        MI_U32 u32i = 0;
        MI_U32 u32SensorNum = 1;
        MI_U8 u8VideoNum = 1;

        u32SensorNum = pCameraBootSetting->u8SensorNum;
        u8VideoNum = pCameraBootSetting->u8VideoNum;
        if(u8VideoNum < u32SensorNum)
        {
            u8VideoNum = u32SensorNum;
            CamOsPrintf("%s:VideoNum[%d]<SensorNum[%d],set VideoNum=SensorNum\n",__FUNCTION__, u8VideoNum,u32SensorNum);

        }
        CamOsPrintf(">>>exit Rtos_UpdateOSDTimer_Func>>>\n");
        for(u32i = 0; u32i < u8VideoNum; u32i ++)
        {
            if(pstStreamAttr[u32i].enInput == ST_Sys_Input_SCL)
            {
                // detach osd
                memset(&stBufRNGInfo, 0, (size_t)sizeof(MI_RGN_ChnPort_t));
                stBufRNGInfo.eModId = E_MI_MODULE_ID_SCL;
                stBufRNGInfo.s32DevId = 0;
                stBufRNGInfo.s32ChnId = pstStreamAttr[0].u32InputChn;
                stBufRNGInfo.s32PortId = pstStreamAttr[0].u32InputPort;
                MI_RGN_DetachFromChn(0, VPE_PORT0_OSD_FOR_TIME_HANDLE, &stBufRNGInfo);
                MI_RGN_DetachFromChn(0, VPE_PORT0_OSD_FOR_PIC_HANDLE, &stBufRNGInfo);
            }
        }
        ST_OSD_Destroy(VPE_PORT0_OSD_FOR_TIME_HANDLE);
        ST_OSD_Destroy(VPE_PORT0_OSD_FOR_PIC_HANDLE);
        if (threadOSDTimer != NULL)
        {
            CamOsThreadStop(threadOSDTimer);
            threadOSDTimer = NULL;
        }
    }
}

static void *Rtos_UpdateOSDTimer_Func(void* p)
{
    MI_S32 s32Ret;
    CamOsTimespec_t stSystemTimetemp;
    struct tm *tm = NULL;
    char szTime[64] = {0};
    MI_U32 u32Len = 0;
    MI_RGN_CanvasInfo_t *pstCanvasInfo;
    ST_Point_T stPoint;
    time_t timeDiffStart = 0, timeStart = 0;
    time_t timeDiffEnd = 0;
    time_t timeInterval = 1000*1000;

    memset(&stSystemTimetemp, 0, (size_t)sizeof(CamOsTimespec_t));

    while (0 == g_u8timeosdoff)
    {
        CamOsUsSleep(timeInterval);

        CamOsGetTimeOfDay(&stSystemTimetemp);
        timeDiffStart= stSystemTimetemp.nSec*1000000ULL + stSystemTimetemp.nNanoSec/1000;//us
        timeStart = (time_t)stSystemTimetemp.nSec;
        //CamOsPrintf("%s timeStart:%llu, Sec:%lu, nNanoSec:%lu\n",__FUNCTION__, timeStart, stSystemTimetemp.nSec, stSystemTimetemp.nNanoSec);
        if((tm = localtime(&timeStart)) == NULL)
        {
            CamOsPrintf("localtime error\n");
            return NULL;
        }
        u32Len = 0;
        memset(szTime, 0, (size_t)sizeof(szTime));
        u32Len += sprintf(szTime + u32Len, "%d-", tm->tm_year + 1900);
        u32Len += sprintf(szTime + u32Len, "%02d-", tm->tm_mon + 1);
        u32Len += sprintf(szTime + u32Len, "%02d ", tm->tm_mday);
        u32Len += sprintf(szTime + u32Len, "%02d:", tm->tm_hour);
        u32Len += sprintf(szTime + u32Len, "%02d:", tm->tm_min);
        u32Len += sprintf(szTime + u32Len, "%02d", tm->tm_sec);
        stPoint.u32X = 0;
        stPoint.u32Y = 0;

        CamOsMutexLock(&g_stRNGOsdMutex);
        s32Ret = ST_OSD_GetCanvasInfo(VPE_PORT0_OSD_FOR_TIME_HANDLE, &pstCanvasInfo);
        if(MI_RGN_OK == s32Ret)
        {
            ST_OSD_Clear(VPE_PORT0_OSD_FOR_TIME_HANDLE, NULL);
        }
        ST_OSD_DrawText(VPE_PORT0_OSD_FOR_TIME_HANDLE, stPoint, szTime, RGB2PIXEL1555(255, 0, 0), DMF_Font_Size_16x16);
        ST_OSD_Update(VPE_PORT0_OSD_FOR_TIME_HANDLE);
        CamOsMutexUnlock(&g_stRNGOsdMutex);

        CamOsGetTimeOfDay(&stSystemTimetemp);
        timeDiffEnd= stSystemTimetemp.nSec*1000000ULL + stSystemTimetemp.nNanoSec/1000;//us

        if(timeDiffEnd >= timeDiffStart)//1s=1000 * 1000us
        {
            if(1000 * 1000 > (timeDiffEnd - timeDiffStart))
            {
                timeInterval = 1000 * 1000 - (timeDiffEnd - timeDiffStart);
                //CamOsPrintf("%s %s 0timeInterval:%llu,timeDiffStart:%llu,timeDiffEnd:%llu\n",__FUNCTION__, szTime, timeInterval, timeDiffStart, timeDiffEnd);
            }
            else
            {
                timeInterval = 1000;
                //CamOsPrintf("%s %s timeInterval:%llu,timeDiffStart:%llu,timeDiffEnd:%llu\n",__FUNCTION__, szTime, timeInterval, timeDiffStart, timeDiffEnd);
            }
        }
        else
        {
            timeInterval = 1000 * 1000;
            //CamOsPrintf("%s %s 2timeInterval:%llu,timeDiffStart:%llu,timeDiffEnd:%llu\n",__FUNCTION__, szTime, timeInterval, timeDiffStart, timeDiffEnd);
        }
    }
    return NULL;
}

static int Rtk_OSD_Autorun(CLI_t *pCli, char *p)
{
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();

    BootTimestampRecord(__LINE__, "osd");
    CamOsPrintf("Func :%s\n", __FUNCTION__);
    if(TRUE == pCameraBootSetting->u8RegionsCun)
    {
        STCHECKRESULT(CamOsThreadCreate(&threadOSDTimer, &threadAttr_OSDTimer, Rtos_UpdateOSDTimer_Func, &threadOSDTimer));
        CreatePicOsd(VPE_PORT0_OSD_FOR_PIC_HANDLE, &g_stFileInfo[0]);
    }
    return 0;
}

#if defined(CONFIG_VDF_IN_RTOS_ENABLE)
static MI_S32 ST_ModuleInit_VDF_MDOD_Rect(void)
{
    ST_VdfSetting_Attr_T *pstVdfSettingAttr = g_stVdfSettingAttr;
    MI_U32 u32ArraySize = ARRAY_SIZE(g_stVdfSettingAttr);
    ST_VDF_OSD_Info_T *pstVDFOsdInfo = g_stVDFOsdInfo;
    //ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MI_U32 i,j = 0;
    MI_U32 u32Chn = 0;

    ST_Rect_T stArea ={
        .u32X = 0,
        .u32Y = 0,
        .u16PicW = 384,
        .u16PicH = 288,
    };

    if(E_MI_VDF_WORK_MODE_MD == pCameraBootSetting->u8VDFWorkMode)
    {
        pstVdfSettingAttr[0].stVdfArgs.u16MdNum = 1;
        pstVdfSettingAttr[0].stVdfArgs.u16OdNum = 0;
        pstVdfSettingAttr[0].stVdfArgs.u16VgNum = 0;
    }
    else if(E_MI_VDF_WORK_MODE_OD == pCameraBootSetting->u8VDFWorkMode)
    {
        pstVdfSettingAttr[0].stVdfArgs.u16MdNum = 0;
        pstVdfSettingAttr[0].stVdfArgs.u16OdNum = 1;
        pstVdfSettingAttr[0].stVdfArgs.u16VgNum = 0;
    }
    else if(E_MI_VDF_WORK_MODE_VG == pCameraBootSetting->u8VDFWorkMode)
    {
        pstVdfSettingAttr[0].stVdfArgs.u16MdNum = 0;
        pstVdfSettingAttr[0].stVdfArgs.u16OdNum = 0;
        pstVdfSettingAttr[0].stVdfArgs.u16VgNum = 1;
    }
    else
    {
        pstVdfSettingAttr[0].stVdfArgs.u16MdNum = 0;
        pstVdfSettingAttr[0].stVdfArgs.u16OdNum = 0;
        pstVdfSettingAttr[0].stVdfArgs.u16VgNum = 0;
        CamOsPrintf("%s input u8VDFWorkMode[%d] error\n",pCameraBootSetting->u8VDFWorkMode);
    }

    for (i = 0; i < u32ArraySize; i++)
    {
        for (j = 0; j < pstVdfSettingAttr[i].stVdfArgs.u16MdNum; j++)
        {
            pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].u32Chn = u32Chn;
            u32Chn++;
            memcpy(&pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea, &stArea, (size_t)sizeof(ST_Rect_T));
        }

        for (j = 0; j < pstVdfSettingAttr[i].stVdfArgs.u16OdNum; j++)
        {
            pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].u32Chn = u32Chn;
            u32Chn++;
            memcpy(&pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea, &stArea, (size_t)sizeof(ST_Rect_T));
        }

        for (j = 0; j < pstVdfSettingAttr[i].stVdfArgs.u16VgNum; j++)
        {
            pstVdfSettingAttr[i].stVdfArgs.stVgArea[j].u32Chn = u32Chn;
            u32Chn++;
            memcpy(&pstVdfSettingAttr[i].stVdfArgs.stVgArea[j].stArea, &stArea, (size_t)sizeof(ST_Rect_T));
        }
    }
    for (i = 0; i < MAX_FULL_RGN_NULL; i ++)
    {
        pstVDFOsdInfo[i].hHandle = ST_OSD_HANDLE_INVALID;
    }
    pstVDFOsdInfo[0].hHandle = VPE_PORT0_OSD_FOR_VDF_HANDLE;
    pstVDFOsdInfo[0].eModId = E_MI_MODULE_ID_SCL;
    pstVDFOsdInfo[0].u32Chn = 0;
    pstVDFOsdInfo[0].u32Port = 2;
    return MI_SUCCESS;
}

static MI_S32 ST_ModuleInit_VDF(void)
{
    MI_VDF_ChnAttr_t stVdfAttr;
    MI_VDF_CHANNEL vdfChn = 0;
    MI_U32 i = 0, j = 0;
    //int mdTotalNum = 0, odTotalNum = 0;
    MI_S32 s32Ret = 0;
    MI_U32 version;
    ST_VdfSetting_Attr_T *pstVdfSettingAttr = g_stVdfSettingAttr;
    MI_U32 u32ArraySize = ARRAY_SIZE(g_stVdfSettingAttr);

    memset(g_stRect_Bak, 0, (size_t)sizeof(g_stRect_Bak));
    memset(g_md_detect_cnt_bak, 0, (size_t)sizeof(g_md_detect_cnt_bak));

    for (i = 0; i < u32ArraySize; i++)
    {
        // create md chn
        for (j = 0; j < pstVdfSettingAttr[i].stVdfArgs.u16MdNum; j ++)
        {
            vdfChn = pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].u32Chn;

            memset(&stVdfAttr, 0, (size_t)sizeof(MI_VDF_ChnAttr_t));
            stVdfAttr.enWorkMode = E_MI_VDF_WORK_MODE_MD;
            stVdfAttr.stMdAttr.u8Enable    = 1;
            stVdfAttr.stMdAttr.u8MdBufCnt  = 4;
            stVdfAttr.stMdAttr.u8VDFIntvl  = 0;
            stVdfAttr.stMdAttr.ccl_ctrl.u16InitAreaThr = 8;
            stVdfAttr.stMdAttr.ccl_ctrl.u16Step = 2;
            stVdfAttr.stMdAttr.stMdDynamicParamsIn.sensitivity = 80;
            stVdfAttr.stMdAttr.stMdDynamicParamsIn.learn_rate = 2000;
            stVdfAttr.stMdAttr.stMdDynamicParamsIn.md_thr = 16;
            stVdfAttr.stMdAttr.stMdDynamicParamsIn.obj_num_max = 0;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.width =pstVdfSettingAttr[i].stVdfArgs.u16VdfInWidth;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.height =pstVdfSettingAttr[i].stVdfArgs.u16VdfInHeight;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.stride = pstVdfSettingAttr[i].stVdfArgs.u16stride;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.color = 1;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.mb_size = MDMB_MODE_MB_8x8;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.sad_out_ctrl = MDSAD_OUT_CTRL_8BIT_SAD;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.md_alg_mode  = MDALG_MODE_SAD;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.num      = 4;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].x = pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u32X;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].y = pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u32Y;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].x = pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u32X + pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u16PicW - 1;
            //720 - 1;//g_width - 1;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].y = pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u32Y;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].x = pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u32X + pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u16PicW - 1;
            // 720 - 1;//g_width - 1;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].y = pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u32Y + pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u16PicH - 1;
            //576 - 1;//g_height - 1;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].x = pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u32X;
            //0;
            stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].y = pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u32Y + pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].stArea.u16PicH - 1;
            //576 - 1;//g_height - 1;
            CamOsPrintf("MD width:%d,height:%d, 0(%dx%d), 1(%dx%d), 2(%dx%d), 3(%dx%d)\n",
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.width,
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.height,
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].x,
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].y,
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].x,
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].y,
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].x,
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].y,
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].x,
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].y);

            if (MI_SUCCESS != (s32Ret = MI_VDF_CreateChn(vdfChn, &stVdfAttr)))
            {
                CamOsPrintf("MI_VDF_CreateChn err, chn %d, %x\n", vdfChn, s32Ret);
                return 1;
            }

            MI_VDF_GetLibVersion(vdfChn, &version);

            CamOsPrintf("MD MI_VDF_CreateChn success, chn %d\n", vdfChn);
        }
        //mdTotalNum += g_stVdfSettingAttr[i].stVdfArgs.u16MdNum;

        for (j = 0; j < pstVdfSettingAttr[i].stVdfArgs.u16OdNum; j ++)
        {
            vdfChn = pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].u32Chn;

            stVdfAttr.enWorkMode = E_MI_VDF_WORK_MODE_OD;
            stVdfAttr.stOdAttr.u8OdBufCnt  = 4;
            stVdfAttr.stOdAttr.u8VDFIntvl  = 0;
            stVdfAttr.stOdAttr.stOdDynamicParamsIn.thd_tamper     = 3;
            stVdfAttr.stOdAttr.stOdDynamicParamsIn.tamper_blk_thd = 1;
            stVdfAttr.stOdAttr.stOdDynamicParamsIn.min_duration   = 15;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.inImgW = pstVdfSettingAttr[i].stVdfArgs.u16VdfInWidth;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.inImgH = pstVdfSettingAttr[i].stVdfArgs.u16VdfInHeight;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.inImgStride = pstVdfSettingAttr[i].stVdfArgs.u16stride;;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.nClrType = (ODColor_e) 1;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.div = OD_WINDOW_3X3;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.alpha = 2;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.M = 120;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.MotionSensitivity = 0;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.num = 4;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[0].x = pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u32X;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[0].y = pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u32Y;//0;

            stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[1].x = pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u32X + pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u16PicW - 1;//g_width - 1;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[1].y = pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u32Y;//0;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[2].x = pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u32X + pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u16PicW - 1;//g_width - 1;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[2].y = pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u32Y + pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u16PicH - 1;//g_height - 1;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[3].x = pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u32X;//0;
            stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[3].y = pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u32Y + pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].stArea.u16PicH - 1;//g_height - 1;
            CamOsPrintf("OD width:%d,height:%d, 0(%dx%d), 1(%dx%d), 2(%dx%d), 3(%dx%d)\n",
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.width,
                   stVdfAttr.stMdAttr.stMdStaticParamsIn.height,
                   stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[0].x,
                   stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[0].y,
                   stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[1].x,
                   stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[1].y,
                   stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[2].x,
                   stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[2].y,
                   stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[3].x,
                   stVdfAttr.stOdAttr.stOdStaticParamsIn.roi_od.pnt[3].y);

            if (MI_SUCCESS != (s32Ret = MI_VDF_CreateChn(vdfChn, &stVdfAttr)))
            {
                CamOsPrintf("MI_VDF_CreateChn err, chn %d, %x\n", vdfChn, s32Ret);
                return 1;
            }
            MI_VDF_GetLibVersion(vdfChn, &version);

            CamOsPrintf("OD MI_VDF_CreateChn success, chn %d\n", vdfChn);

        }

        for (j = 0; j < pstVdfSettingAttr[i].stVdfArgs.u16VgNum; j ++)
        {
            vdfChn = pstVdfSettingAttr[i].stVdfArgs.stVgArea[j].u32Chn;

            stVdfAttr.enWorkMode = E_MI_VDF_WORK_MODE_VG;
            stVdfAttr.stVgAttr.u8VgBufCnt  = 4;
            stVdfAttr.stVgAttr.u8VDFIntvl  = 0;

            stVdfAttr.stVgAttr.height = pstVdfSettingAttr[i].stVdfArgs.u16VdfInHeight;
            stVdfAttr.stVgAttr.width = pstVdfSettingAttr[i].stVdfArgs.u16VdfInWidth;
            stVdfAttr.stVgAttr.stride = pstVdfSettingAttr[i].stVdfArgs.u16stride;

            stVdfAttr.stVgAttr.object_size_thd = 3;
            stVdfAttr.stVgAttr.indoor = 1;
            stVdfAttr.stVgAttr.function_state = VG_VIRTUAL_GATE;
            stVdfAttr.stVgAttr.line_number = 2;

            if( stVdfAttr.stVgAttr.function_state == VG_VIRTUAL_GATE )
            {
                if( stVdfAttr.stVgAttr.line_number >= 1 )
                {
                    //First Line
                    stVdfAttr.stVgAttr.line[0].px.x = 15;
                    stVdfAttr.stVgAttr.line[0].px.y = 95;
                    stVdfAttr.stVgAttr.line[0].py.x = 115;
                    stVdfAttr.stVgAttr.line[0].py.y = 95;
                    stVdfAttr.stVgAttr.line[0].pdx.x = 50;
                    stVdfAttr.stVgAttr.line[0].pdx.y = 150;
                    stVdfAttr.stVgAttr.line[0].pdy.x = 50;
                    stVdfAttr.stVgAttr.line[0].pdy.y = 15;
                }

                if( stVdfAttr.stVgAttr.line_number == 2 )
                {
                    //Second Line
                    stVdfAttr.stVgAttr.line[1].px.x = 220;
                    stVdfAttr.stVgAttr.line[1].px.y = 100;
                    stVdfAttr.stVgAttr.line[1].py.x = 225;
                    stVdfAttr.stVgAttr.line[1].py.y = 50;
                    stVdfAttr.stVgAttr.line[1].pdx.x = 220;
                    stVdfAttr.stVgAttr.line[1].pdx.y = 50;
                    stVdfAttr.stVgAttr.line[1].pdy.x = 230;
                    stVdfAttr.stVgAttr.line[1].pdy.y = 50;
                }
            }
            else  //VG_REGION_INVASION
            {
                stVdfAttr.stVgAttr.vg_region.p_one.x = 15;
                stVdfAttr.stVgAttr.vg_region.p_one.y = 20;
                stVdfAttr.stVgAttr.vg_region.p_two.x = 115;
                stVdfAttr.stVgAttr.vg_region.p_two.y = 20;
                stVdfAttr.stVgAttr.vg_region.p_three.x = 115;
                stVdfAttr.stVgAttr.vg_region.p_three.y = 95;
                stVdfAttr.stVgAttr.vg_region.p_four.x = 15;
                stVdfAttr.stVgAttr.vg_region.p_four.y = 95;

                //Set region direction
                stVdfAttr.stVgAttr.vg_region.region_dir = VG_REGION_ENTER;
                //vg_region.region_dir = VG_REGION_LEAVING;
                //vg_region.region_dir = VG_REGION_CROSS;
            }

            if (MI_SUCCESS != (s32Ret = MI_VDF_CreateChn(vdfChn, &stVdfAttr)))
            {
                CamOsPrintf("MI_VDF_CreateChn err, chn %d, %x\n", vdfChn, s32Ret);
                return 1;
            }
            MI_VDF_GetLibVersion(vdfChn, &version);

            CamOsPrintf("OD MI_VDF_CreateChn success, chn %d\n", vdfChn);

        }
        //odTotalNum += g_stVdfSettingAttr[i].stVdfArgs.u16OdNum;
    }
    if(pstVdfSettingAttr[0].stVdfArgs.u16MdNum)
    {
        STCHECKRESULT(MI_VDF_Run(E_MI_VDF_WORK_MODE_MD));
    }
    if(pstVdfSettingAttr[0].stVdfArgs.u16OdNum)
    {
        STCHECKRESULT(MI_VDF_Run(E_MI_VDF_WORK_MODE_OD));
    }
    if(pstVdfSettingAttr[0].stVdfArgs.u16VgNum)
    {
        STCHECKRESULT(MI_VDF_Run(E_MI_VDF_WORK_MODE_VG));
    }
    return MI_SUCCESS;
}
static int ST_VDFMDSadMdNumCal(MI_U8 *pu8MdRstData, int i, int j, int col, int row, int cusCol, int cusRow)
{
    int c, r;
    int rowIdx = 0;
    int sad8BitThr = 20;
    int mdNum = 0;

    for(r = 0; r < cusRow; r++)
    {
        rowIdx = (i + r) * col + j;

        for(c = 0; c < cusCol; c++)
        {
            if(pu8MdRstData[rowIdx + c] > sad8BitThr) mdNum ++;
        }
    }

    return mdNum;
}

/*
 * backup the prev detect result
 */

static int ST_VDFMDToRectBase(MI_U8 *pu8MdRstData, int col, int row, MI_U32 u32baseWidth, MI_U32 u32baseHeight, ST_Rect_T *pstRect)
{
    int i, j;
    MI_S32 md_detect_cnt = 0;

    int cusCol = 4;     // 4 macro block Horizontal
    int cusRow = 2;     // 3 macro block vertical

    if(pu8MdRstData)
    {
        for(i = 0; i < row; i += cusRow)
        {
            for(j = 0; j < col; j += cusCol)
            {
                // clac all macro block result
                if(ST_VDFMDSadMdNumCal(pu8MdRstData, i, j, col, row, cusCol, cusRow) > cusRow * cusCol / 2)
                {
                    pstRect[md_detect_cnt].u32X = (j * u32baseWidth / col) & 0xFFFE;
                    pstRect[md_detect_cnt].u32Y = (i * u32baseHeight / row) & 0xFFFE;
                    pstRect[md_detect_cnt].u16PicW = (cusCol * u32baseWidth / col) & 0xFFFE;
                    pstRect[md_detect_cnt].u16PicH = (cusRow * u32baseHeight / row) & 0xFFFE;
                    md_detect_cnt++;
                }
            }
        }
    }

    return md_detect_cnt;
}

static int ST_VDFMDtoRECT_SAD(MI_U8 *pu8MdRstData, int col, int row)
{
    MI_U32 i, j;
    MI_U32 md_detect_cnt = 0;

    MI_RGN_CanvasInfo_t *pstCanvasInfo;
    ST_VDF_OSD_Info_T *pstVDFOsdInfo = g_stVDFOsdInfo;

    for(i = 0; i < MAX_FULL_RGN_NULL; i ++)
    {
        if(ST_OSD_HANDLE_INVALID == pstVDFOsdInfo[i].hHandle)
        {
            continue;
        }

        memset(&g_stRect[0], 0, (size_t)sizeof(g_stRect));
        CamOsMutexLock(&g_stRNGOsdMutex);
        if (MI_RGN_OK == ST_OSD_GetCanvasInfo(pstVDFOsdInfo[i].hHandle, &pstCanvasInfo))
        {
            md_detect_cnt = ST_VDFMDToRectBase(pu8MdRstData, col, row, pstCanvasInfo->stSize.u32Width, pstCanvasInfo->stSize.u32Height, &g_stRect[0]);
            for(j = 0; j < g_md_detect_cnt_bak[i]; j++)
            {
                if( g_stRect_Bak[j][i].u32X == g_stRect[j].u32X &&
                    g_stRect_Bak[j][i].u32Y == g_stRect[j].u32Y &&
                    g_stRect_Bak[j][i].u16PicW == g_stRect[j].u16PicW &&
                    g_stRect_Bak[j][i].u16PicH == g_stRect[j].u16PicH )
                {
                    continue;
                }
                if( g_stRect_Bak[j][i].u16PicH != 0 && g_stRect_Bak[j][i].u16PicW != 0)
                {
                    ST_OSD_ClearRectFast(pstVDFOsdInfo[i].hHandle, g_stRect_Bak[j][i]);
                    memset(&g_stRect_Bak[j][i], 0, (size_t)sizeof(ST_Rect_T));
                }
            }

            for(j = 0; j < md_detect_cnt; j++)
            {
                //CamOsPrintf("handle:%d, chn:%d, port:%d, rect(%d, %d, %dx%d)\n", pstVDFOsdInfo[i].hHandle,
                //    pstVDFOsdInfo[i].u32Chn, pstVDFOsdInfo[i].u32Port,
                //   g_stRect[j].u32X, g_stRect[j].u32Y, g_stRect[j].u16PicW, g_stRect[j].u16PicH);
                if( g_stRect_Bak[j][i].u32X == g_stRect[j].u32X &&
                    g_stRect_Bak[j][i].u32Y == g_stRect[j].u32Y &&
                    g_stRect_Bak[j][i].u16PicW == g_stRect[j].u16PicW &&
                    g_stRect_Bak[j][i].u16PicH == g_stRect[j].u16PicH )
                {
                    continue;
                }
                if( g_stRect[j].u16PicH != 0 && g_stRect[j].u16PicW != 0)
                {
                    ST_OSD_DrawRectFast(pstVDFOsdInfo[i].hHandle, g_stRect[j], 2, I4_RED);
                    g_stRect_Bak[j][i] = g_stRect[j];
                }
            }
            ST_OSD_Update(pstVDFOsdInfo[i].hHandle);
            g_md_detect_cnt_bak[i] = md_detect_cnt;
        }

        CamOsMutexUnlock(&g_stRNGOsdMutex);
    }

    return 0;
}

static MI_S32 ST_ModuleDeinit_VDF(void)
{
    STCHECKRESULT(MI_VDF_Stop(E_MI_VDF_WORK_MODE_MD));
    STCHECKRESULT(MI_VDF_Stop(E_MI_VDF_WORK_MODE_OD));

    MI_U32 u32ArraySize = ARRAY_SIZE(g_stVdfSettingAttr);
    MI_VDF_CHANNEL vdfChn = 0;
    MI_U32 i = 0, j = 0;

    for (i = 0; i < u32ArraySize; i++)
    {
        for(j = 0; j < g_stVdfSettingAttr[i].stVdfArgs.u16MdNum; j ++)
        {
            vdfChn = g_stVdfSettingAttr[i].stVdfArgs.stMdArea[j].u32Chn;
            STCHECKRESULT(MI_VDF_DestroyChn(vdfChn));
        }

        for(j = 0; j < g_stVdfSettingAttr[i].stVdfArgs.u16MdNum; j ++)
        {
            vdfChn = g_stVdfSettingAttr[i].stVdfArgs.stOdArea[j].u32Chn;
            STCHECKRESULT(MI_VDF_DestroyChn(vdfChn));
        }

        for (j = 0; j < g_stVdfSettingAttr[i].stVdfArgs.u16MdNum; j ++)
        {
            vdfChn = g_stVdfSettingAttr[i].stVdfArgs.stVgArea[j].u32Chn;
            STCHECKRESULT(MI_VDF_DestroyChn(vdfChn));
        }
    }

    STCHECKRESULT(MI_VDF_Uninit());

    return MI_SUCCESS;
}

static void *ST_VDFGetResult(void *args)
{
    VDF_Thread_Args_t *pstArgs = (VDF_Thread_Args_t *)args;
    MI_VDF_Result_t stVdfResult;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_U8 *pu8MdRstData = NULL;
    MI_VDF_CHANNEL vdfChn = pstArgs->vdfChn;
    MI_U32 buffer_size = 0;
    MI_U32 col = 0;
    MI_U32 row = 0;
    MI_RGN_ChnPort_t stBufRNGInfo;
    ST_VDF_OSD_Info_T *pstVDFOsdInfo = g_stVDFOsdInfo;
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();

    if(pstArgs->enWorkMode == E_MI_VDF_WORK_MODE_MD)
    {
        CamOsPrintf("Get md result, chn:%d\n", vdfChn);
        // hard code, MDMB_MODE_MB_8x8
        col = pstArgs->u16Width >> 3;    // 48
        row = pstArgs->u16Height >> 3;    // 36
        buffer_size = col * row;    // MDSAD_OUT_CTRL_8BIT_SAD
    }
    else
    {
        CamOsPrintf("Get od result, chn:%d\n", vdfChn);
    }

    while(pstArgs->bRunFlag)
    {
        memset(&stVdfResult, 0x00, (size_t)sizeof(stVdfResult));
        memset(&g_stSadDataArry, 0, (size_t)sizeof(g_stSadDataArry));

        stVdfResult.enWorkMode = pstArgs->enWorkMode;
        s32Ret = MI_VDF_GetResult(vdfChn, &stVdfResult, 0);

        if((0 == s32Ret) &&((1 == stVdfResult.stMdResult.u8Enable) || (1 == stVdfResult.stOdResult.u8Enable) || (stVdfResult.stVgResult.alarm_cnt > 0)))
        {
            if(pstArgs->enWorkMode == E_MI_VDF_WORK_MODE_MD)
            {
                pu8MdRstData = (MI_U8 *)stVdfResult.stMdResult.pstMdResultSad->paddr;
#if 0
                CamOsPrintf("[MD_TEST][HDL=0xA0] pts=0x%llx [WorkMode=%d, Enable=%d] Get MD-Rst data:\n",
                       stVdfResult.stMdResult.u64Pts,      \
                       stVdfResult.enWorkMode,                         \
                       stVdfResult.stMdResult.u8Enable);

                // col = (pstArgs->u16Width >> 1) >> (1 + 2);
                // row = pstArgs->u16Height >> (1 + 2);
                // buffer_size = col * row * (2 - 1);


                // printf("buffer_size:%d\n", buffer_size);
#endif
                memcpy(g_stSadDataArry, pu8MdRstData, buffer_size);
                MI_VDF_PutResult(vdfChn, &stVdfResult);
                ST_VDFMDtoRECT_SAD(g_stSadDataArry, col, row);
            }
            else if(pstArgs->enWorkMode == E_MI_VDF_WORK_MODE_OD)
            {
#if 1
                CamOsPrintf("[OD_TEST][HDL=02] pts=0x%llx [WorkMode=%d, Enable=%d, RmsData=%u (%d, %d)] Get OD-Rst data: ",
                       stVdfResult.stOdResult.u64Pts,      \
                       stVdfResult.enWorkMode,                         \
                       stVdfResult.stOdResult.u8Enable,    \
                       stVdfResult.stOdResult.u8DataLen,   \
                       stVdfResult.stOdResult.u8WideDiv,   \
                       stVdfResult.stOdResult.u8HightDiv);
                CamOsPrintf("{%u %u %u  %u %u %u  %u %u %u}\n",
                       stVdfResult.stOdResult.u8RgnAlarm[0][0],
                       stVdfResult.stOdResult.u8RgnAlarm[0][1],
                       stVdfResult.stOdResult.u8RgnAlarm[0][2],
                       stVdfResult.stOdResult.u8RgnAlarm[1][0],
                       stVdfResult.stOdResult.u8RgnAlarm[1][1],
                       stVdfResult.stOdResult.u8RgnAlarm[1][2],
                       stVdfResult.stOdResult.u8RgnAlarm[2][0],
                       stVdfResult.stOdResult.u8RgnAlarm[2][1],
                       stVdfResult.stOdResult.u8RgnAlarm[2][2]);
#endif
                MI_VDF_PutResult(vdfChn, &stVdfResult);
            }
            else if(stVdfResult.stVgResult.alarm_cnt > 0)
            {
                CamOsPrintf("[VG_TEST][WorkMode=%d, MdEnable=%d, OdEnable=%d, VGalarm_cnt=%d] Get data[0-3]:[%d,%d,%d,%d] \n",
                       stVdfResult.enWorkMode,
                       stVdfResult.stMdResult.u8Enable,
                       stVdfResult.stOdResult.u8Enable,
                       stVdfResult.stVgResult.alarm_cnt,
                       stVdfResult.stVgResult.alarm[0],
                       stVdfResult.stVgResult.alarm[1],
                       stVdfResult.stVgResult.alarm[2],
                       stVdfResult.stVgResult.alarm[3]);
                MI_VDF_PutResult(vdfChn, &stVdfResult);
            }
         }
        else if(0 == s32Ret)
        {
           MI_VDF_PutResult(vdfChn, &stVdfResult);
        }
        else
        {
            // CamOsPrintf("[MD_TEST][HDL=0x0] line%d call MI_VDF_GetResult() fail(%d)\n\n",__LINE__, s32Ret);
        }

        CamOsUsSleep(30 * 1000);
    }

    CamOsPrintf(">>>exit ST_VDFGetResult<<<\n");
    ST_ModuleDeinit_VDF();

    // detach osd
    memset(&stBufRNGInfo, 0, (size_t)sizeof(MI_RGN_ChnPort_t));
    stBufRNGInfo.eModId = pstVDFOsdInfo[0].eModId;
    stBufRNGInfo.s32DevId = 0;
    stBufRNGInfo.s32ChnId = pstVDFOsdInfo[0].u32Chn;
    stBufRNGInfo.s32PortId = pstVDFOsdInfo[0].u32Port;
    MI_RGN_DetachFromChn(0, pstVDFOsdInfo[0].hHandle, &stBufRNGInfo);

    ST_OSD_Destroy(pstVDFOsdInfo[0].hHandle);
    if(FALSE == pCameraBootSetting->u8RegionsCun)
    {
        ST_OSD_Deinit();
    }
    CamOsThreadStop(pstArgs->pThreadId);

    return NULL;
}

static MI_S32 ST_VdfStart(void)
{
    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;
    MI_U32 u32SrcFrmrate;
    MI_U32 u32DstFrmrate;
    MI_S32 s32Ret = MI_SUCCESS;
    ST_VdfSetting_Attr_T *pstVdfSettingAttr = g_stVdfSettingAttr;
    MI_U32 u32ArraySize = ARRAY_SIZE(g_stVdfSettingAttr);
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MI_VDF_CHANNEL vdfChn = 0;
    MI_U32 i = 0,j = 0;
    MI_U8 u8BufTemp[32] = {0};
    MI_RGN_Attr_t stRgnAttr;
    MI_RGN_ChnPort_t stAttachChnPort;
    MI_RGN_ChnPortParam_t stChnPortParam;
    ST_VDF_OSD_Info_T *pstVDFOsdInfo = g_stVDFOsdInfo;
    MI_SCL_OutPortParam_t  stSclOutputParam;

    memset(&stRgnAttr, 0, (size_t)sizeof(MI_RGN_Attr_t));
    stRgnAttr.eType = E_MI_RGN_TYPE_OSD;
    stRgnAttr.stOsdInitParam.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_I4;
    stRgnAttr.stOsdInitParam.stSize.u32Width = pstVdfSettingAttr[0].stVdfArgs.u16VdfInWidth;
    stRgnAttr.stOsdInitParam.stSize.u32Height = pstVdfSettingAttr[0].stVdfArgs.u16VdfInHeight;
    STCHECKRESULT(ST_OSD_Create(pstVDFOsdInfo[0].hHandle, &stRgnAttr));
    memset(&stAttachChnPort, 0, (size_t)sizeof(MI_RGN_ChnPort_t));
    stAttachChnPort.eModId = pstVDFOsdInfo[0].eModId;
    stAttachChnPort.s32DevId = 0;
    stAttachChnPort.s32ChnId =  pstVDFOsdInfo[0].u32Chn;
    stAttachChnPort.s32PortId = pstVDFOsdInfo[0].u32Port;
    memset(&stChnPortParam, 0, (size_t)sizeof(MI_RGN_ChnPortParam_t));
    stChnPortParam.bShow = TRUE;
    stChnPortParam.stPoint.u32X = 0;
    stChnPortParam.stPoint.u32Y = 0;
    stChnPortParam.unPara.stOsdChnPort.u32Layer = pstVDFOsdInfo[0].hHandle;
    stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
    stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;
    stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 0xFF;
    STCHECKRESULT(MI_RGN_AttachToChn(0, pstVDFOsdInfo[0].hHandle, &stAttachChnPort, &stChnPortParam));

        memset(&stSclOutputParam, 0x0, (size_t)sizeof(MI_SCL_OutPortParam_t));
        stSclOutputParam.stSCLOutCropRect.u16X = 0;
        stSclOutputParam.stSCLOutCropRect.u16Y = 0;
        stSclOutputParam.stSCLOutCropRect.u16Width = _stVpeResolution[0].u32OutWidth;
        stSclOutputParam.stSCLOutCropRect.u16Height = _stVpeResolution[0].u32OutHeight;
        stSclOutputParam.stSCLOutputSize.u16Width = pstVdfSettingAttr[0].stVdfArgs.u16VdfInWidth;
        stSclOutputParam.stSCLOutputSize.u16Height = pstVdfSettingAttr[0].stVdfArgs.u16VdfInHeight;
        stSclOutputParam.bMirror = FALSE;
        stSclOutputParam.bFlip = FALSE;
        stSclOutputParam.eCompressMode= E_MI_SYS_COMPRESS_MODE_NONE;
        stSclOutputParam.ePixelFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        STCHECKRESULT(MI_SCL_SetOutputPortParam(0, 0, 2, &stSclOutputParam));
        STCHECKRESULT(MI_SCL_EnableOutputPort(0, 0, 2));


    /************************************************
    Step1:  start scl port ,bind scl -> vdf, enable vdf
    *************************************************/

    for (i = 0; i < u32ArraySize; i ++)
    {
        for(j = 0; j < pstVdfSettingAttr[i].stVdfArgs.u16MdNum; j++)
        {
            vdfChn = pstVdfSettingAttr[i].stVdfArgs.stMdArea[j].u32Chn;
            memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
            stSrcChnPort.u32DevId = 0;
            stSrcChnPort.u32ChnId = pstVdfSettingAttr[i].u32InputChn;
            stSrcChnPort.u32PortId = pstVdfSettingAttr[i].u32InputPort;

            stDstChnPort.eModId = E_MI_MODULE_ID_VDF;
            stDstChnPort.u32DevId = 0;
            stDstChnPort.u32ChnId = vdfChn;
            stDstChnPort.u32PortId = 0;
            u32SrcFrmrate = pCameraBootSetting->u8SensorFrameRate;
            u32DstFrmrate = 6;
            CamOsPrintf("MD:divp chn-port:(%d %d) vdf chn-port:(%d %d)\n", pstVdfSettingAttr[i].u32InputChn, pstVdfSettingAttr[i].u32InputPort, vdfChn, 0);
            STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0));

            if(MI_SUCCESS != (s32Ret = MI_VDF_EnableSubWindow(vdfChn, 0, 0, 1)))
            {
                CamOsPrintf("MI_VDF_EnableSubWindow err, chn %d, %x\n", vdfChn, s32Ret);
                return 1;
            }
            else
            {
                CamOsPrintf("MI_VDF_EnableSubWindow ok, chn %d, %x\n", vdfChn, s32Ret);
            }

            memset(u8BufTemp, 0, (size_t)sizeof(u8BufTemp));
            sprintf((char *)u8BufTemp, "%s%sCh%d", "UpVDF", "MD", vdfChn);
            g_stVdfThreadArgs[vdfChn].enWorkMode = E_MI_VDF_WORK_MODE_MD;
            g_stVdfThreadArgs[vdfChn].vdfChn = vdfChn;
            g_stVdfThreadArgs[vdfChn].bRunFlag = TRUE;
            g_stVdfThreadArgs[vdfChn].u16Width = pstVdfSettingAttr[i].stVdfArgs.u16VdfInWidth;
            g_stVdfThreadArgs[vdfChn].u16Height = pstVdfSettingAttr[i].stVdfArgs.u16VdfInHeight;
            g_stVdfThreadArgs[vdfChn].stThreadAttrb.nPriority = 0;
            g_stVdfThreadArgs[vdfChn].stThreadAttrb.szName = (char *)u8BufTemp;
            g_stVdfThreadArgs[vdfChn].stThreadAttrb.nStackSize = 8192;
            STCHECKRESULT(CamOsThreadCreate(&g_stVdfThreadArgs[vdfChn].pThreadId , &g_stVdfThreadArgs[vdfChn].stThreadAttrb, ST_VDFGetResult, &g_stVdfThreadArgs[vdfChn]));
        }

        for(j = 0; j < pstVdfSettingAttr[i].stVdfArgs.u16OdNum; j++)
        {
            vdfChn = pstVdfSettingAttr[i].stVdfArgs.stOdArea[j].u32Chn;
            memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
            stSrcChnPort.u32DevId = 0;
            stSrcChnPort.u32ChnId = pstVdfSettingAttr[i].u32InputChn;
            stSrcChnPort.u32PortId = pstVdfSettingAttr[i].u32InputPort;

            stDstChnPort.eModId = E_MI_MODULE_ID_VDF;
            stDstChnPort.u32DevId = 0;
            stDstChnPort.u32ChnId = vdfChn;
            stDstChnPort.u32PortId = 0;
            u32SrcFrmrate = pCameraBootSetting->u8SensorFrameRate;
            u32DstFrmrate = 12;
            STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0));
            CamOsPrintf("OD:divp chn-port:(%d %d) vdf chn-port:(%d %d)\n", pstVdfSettingAttr[i].u32InputChn, pstVdfSettingAttr[i].u32InputPort, vdfChn, 0);

            if(MI_SUCCESS != (s32Ret = MI_VDF_EnableSubWindow(vdfChn, 0, 0, 1)))
            {
                CamOsPrintf("MI_VDF_EnableSubWindow err, chn %d, %x\n", vdfChn, s32Ret);
                return 1;
            }
            else
            {
                CamOsPrintf("MI_VDF_EnableSubWindow ok, chn %d, %x\n", vdfChn, s32Ret);
            }

            memset(u8BufTemp, 0, (size_t)sizeof(u8BufTemp));
            sprintf((char *)u8BufTemp, "%s%sCh%d", "UpVDF", "OD", vdfChn);
            g_stVdfThreadArgs[vdfChn].enWorkMode = E_MI_VDF_WORK_MODE_OD;
            g_stVdfThreadArgs[vdfChn].vdfChn = vdfChn;
            g_stVdfThreadArgs[vdfChn].bRunFlag = TRUE;
            g_stVdfThreadArgs[vdfChn].u16Width = pstVdfSettingAttr[i].stVdfArgs.u16VdfInWidth;
            g_stVdfThreadArgs[vdfChn].u16Height = pstVdfSettingAttr[i].stVdfArgs.u16VdfInHeight;
            g_stVdfThreadArgs[vdfChn].stThreadAttrb.nPriority = 0;
            g_stVdfThreadArgs[vdfChn].stThreadAttrb.szName = (char *)u8BufTemp;
            g_stVdfThreadArgs[vdfChn].stThreadAttrb.nStackSize = 8192;
            STCHECKRESULT(CamOsThreadCreate(&g_stVdfThreadArgs[vdfChn].pThreadId , &g_stVdfThreadArgs[vdfChn].stThreadAttrb, ST_VDFGetResult, &g_stVdfThreadArgs[vdfChn]));
        }
        for(j = 0; j < pstVdfSettingAttr[i].stVdfArgs.u16VgNum; j++)
        {
            vdfChn = pstVdfSettingAttr[i].stVdfArgs.stVgArea[j].u32Chn;
            memset(&stSrcChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            memset(&stDstChnPort, 0x0, (size_t)sizeof(MI_SYS_ChnPort_t));
            stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
            stSrcChnPort.u32DevId = 0;
            stSrcChnPort.u32ChnId = pstVdfSettingAttr[i].u32InputChn;
            stSrcChnPort.u32PortId = pstVdfSettingAttr[i].u32InputPort;

            stDstChnPort.eModId = E_MI_MODULE_ID_VDF;
            stDstChnPort.u32DevId = 0;
            stDstChnPort.u32ChnId = vdfChn;
            stDstChnPort.u32PortId = 0;
            u32SrcFrmrate = pCameraBootSetting->u8SensorFrameRate;
            u32DstFrmrate = 12;
            STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, E_MI_SYS_BIND_TYPE_FRAME_BASE, 0));
            CamOsPrintf("VG:divp chn-port:(%d %d) vdf chn-port:(%d %d)\n", pstVdfSettingAttr[i].u32InputChn, pstVdfSettingAttr[i].u32InputPort, vdfChn, 0);

            if(MI_SUCCESS != (s32Ret = MI_VDF_EnableSubWindow(vdfChn, 0, 0, 1)))
            {
                CamOsPrintf("MI_VDF_EnableSubWindow err, chn %d, %x\n", vdfChn, s32Ret);
                return 1;
            }
            else
            {
                CamOsPrintf("MI_VDF_EnableSubWindow ok, chn %d, %x\n", vdfChn, s32Ret);
            }

            memset(u8BufTemp, 0, (size_t)sizeof(u8BufTemp));
            sprintf((char *)u8BufTemp, "%s%sCh%d", "UpVDF", "VG", vdfChn);
            g_stVdfThreadArgs[vdfChn].enWorkMode = E_MI_VDF_WORK_MODE_VG;
            g_stVdfThreadArgs[vdfChn].vdfChn = vdfChn;
            g_stVdfThreadArgs[vdfChn].bRunFlag = TRUE;
            g_stVdfThreadArgs[vdfChn].u16Width = pstVdfSettingAttr[i].stVdfArgs.u16VdfInWidth;
            g_stVdfThreadArgs[vdfChn].u16Height = pstVdfSettingAttr[i].stVdfArgs.u16VdfInHeight;
            g_stVdfThreadArgs[vdfChn].stThreadAttrb.nPriority = 0;
            g_stVdfThreadArgs[vdfChn].stThreadAttrb.szName = (char *)u8BufTemp;
            g_stVdfThreadArgs[vdfChn].stThreadAttrb.nStackSize = 8192;
            STCHECKRESULT(CamOsThreadCreate(&g_stVdfThreadArgs[vdfChn].pThreadId , &g_stVdfThreadArgs[vdfChn].stThreadAttrb, ST_VDFGetResult, &g_stVdfThreadArgs[vdfChn]));
        }
    }

    return MI_SUCCESS;
}
#endif
#endif
#endif

#ifdef CONFIG_PANEL_IN_RTOS_ENABLE
extern void LoadConfig(void*  PanelIniAddr,MI_U32 PanelIniSize);

static int ST_PreReadPANELFile(void)
{
    CamFsRet_e eRet = CAM_FS_OK;
    CamFsFd tFD;
    MI_S32 s32FileLen = 0;
    MI_S32 s32Ret = MI_SUCCESS;

    eRet = CamFsOpen(&tFD, PANEL_ini_PATH, O_RDONLY, 0777);
    if(CAM_FS_OK == eRet)
    {
        s32FileLen = CamFsSeek(tFD, 0, SEEK_END);
        if((s32FileLen != -1) && (CamFsSeek(tFD, 0, SEEK_SET) != -1))
        {
                s32Ret = MI_SYS_MMA_Alloc(0, NULL,s32FileLen,&phyPanelIni);
                if(s32Ret != MI_SUCCESS){
                    CamFsClose(tFD);
                    return -1;
                }
                s32Ret = MI_SYS_Mmap(phyPanelIni, s32FileLen, &g_PanelIniAddr, TRUE);
                if(s32Ret != MI_SUCCESS){
                    MI_SYS_MMA_Free(0, phyPanelIni);
                    CamFsClose(tFD);
                    return -1;
                }
                if(g_PanelIniAddr != NULL)
                {
                    memset(g_PanelIniAddr, 0, s32FileLen);
                    CamFsRead(tFD, g_PanelIniAddr, s32FileLen);
                    g_u32PanelIniSize = s32FileLen;
                    eRet = CAM_FS_OK;
                    CamOsPrintf("open:%s ok,filesize:%d \n",PANEL_ini_PATH, s32FileLen);
                }
                else
                {
                    CamOsPrintf("%s open:%s ok, but MemAlloc[%d]filesize fail \n",__FUNCTION__, PANEL_ini_PATH, s32FileLen);
                    CamFsClose(tFD);
                    return -1;
                }
        }
        else
        {
            eRet = CAM_FS_FAIL;
            CamOsPrintf("%s seek file[%s] fail \n",__FUNCTION__, PANEL_ini_PATH);
            CamFsClose(tFD);
            return -1;
        }
        CamFsClose(tFD);
    }
    else
    {
        eRet = CAM_FS_FAIL;
        CamOsPrintf("%s open %s fail \n",__FUNCTION__, PANEL_ini_PATH);
        return -1;
    }
    return MI_SUCCESS;
}
#endif

CamOsTsem_t tPreloadFileTsem;
CamOsTsem_t tIspReadFileTsem;

static void* MI_PreloadFile(void* p)
{
#if (INTERFACE_LDC) || defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)
    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
#endif
    BootTimestampRecord(__LINE__, "PreloadFile Start");

#if INTERFACE_LDC
    if(pCameraBootSetting->u8enableLDC)
    {
        ST_PreReadLDCCfgFile();
        CamOsTsemUp(&tPreloadFileTsem);
    }
#endif // INTERFACE_LDC
#if defined(CONFIG_IPU_IN_RTOS_ENABLE)
    if(pCameraBootSetting->u8enableIPU)
        ST_PreReadIPUFile();
#endif
#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)
    if(pCameraBootSetting->u8RegionsCun)
    {
        ST_PreReadOSDBitmapFile();
        ST_PreReadOSDPicFile();
    }
#endif
#ifdef CONFIG_PANEL_IN_RTOS_ENABLE
    if(ST_PreReadPANELFile() == MI_SUCCESS)
    {
        LoadConfig(g_PanelIniAddr,g_u32PanelIniSize);
        MI_SYS_MMA_Free(0, phyPanelIni);
        CamOsTsemUp(&tPreloadFileTsem);
    }
#endif
    CamOsTsemUp(&tPreloadFileTsem);
    CamOsTsemDown(&tIspReadFileTsem);//wait Isp Load api bin

    BootTimestampRecord(__LINE__, "PreloadFile end");

    if(application_selector_retreat() != 0)
    {
        CamOsPrintf("application_selector_stop failed.\n");
    }
    return 0;
}

static void* MI_PreloadMiPipe(void* p)
{
    CamOsTimespec_t stSystemTimetemp;
    MI_U64 u64PtsBase;
    int s32PreloadChnFlag = 0;
    int s32PreloadAI = 0;
    int s32PreloadAO = 0;

    CameraBootSetting_t* pCameraBootSetting = CameraBootSettingGetHandle();
    MI_SYS_Init(0);
    mi_debug_init();
    CamOsGetTimeOfDay(&stSystemTimetemp);
    u64PtsBase = stSystemTimetemp.nSec*1000000ULL+stSystemTimetemp.nNanoSec/1000;
    MI_SYS_InitPtsBase(0, u64PtsBase);

    MI_COMMON_MODPARAM_GETVALUE("E_MI_MODULE_ID_VENC", "s32PreloadChnFlag", E_MI_MODPARAMTYPE_MI_S32, &s32PreloadChnFlag);
    MI_COMMON_MODPARAM_GETVALUE("E_MI_MODULE_ID_AI", "s32PreloadAI", E_MI_MODPARAMTYPE_MI_S32, &s32PreloadAI);
    MI_COMMON_MODPARAM_GETVALUE("E_MI_MODULE_ID_AO", "s32PreloadAO", E_MI_MODPARAMTYPE_MI_S32, &s32PreloadAO);

    if(s32PreloadChnFlag != 0)
    {
        BootTimestampRecord(__LINE__, "Test_vif_vpe_venc Start");
        Test_vif_vpe_venc(NULL,NULL);
        BootTimestampRecord(__LINE__, "Test_vif_vpe_venc Done");
    }
    else
    {
        CamOsTsemUp(&tIspReadFileTsem);
    }

    if(s32PreloadAI)
    {
#if INTERFACE_AI
        ST_AI_Autorun();
#endif
    }
    if(s32PreloadAO)
    {
#if INTERFACE_AO
        ST_AO_Autorun();
#endif
    }

    CamOsTsemDown(&tPreloadFileTsem);
    if(pCameraBootSetting->u8enableIPU)
    {
#if defined(CONFIG_IPU_IN_RTOS_ENABLE)
        Rtk_DLA_Autorun(NULL,NULL);
#endif
    }
    if(pCameraBootSetting->u8RegionsCun)
    {
#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)
        Rtk_OSD_Autorun(NULL,NULL);
#endif
    }
    if(TRUE == pCameraBootSetting->u8enableVDF)
    {
#if defined(CONFIG_VDF_IN_RTOS_ENABLE)
        Rtk_VDF_Autorun(NULL,NULL);
#endif
    }
    return 0;
}

static void MI_PreloadTask(void)
{
    CamOsThreadAttrb_t threadPreloadFileAttr = {.nPriority = 10,.szName = "MI_PreloadFile",.nStackSize = 3072};
    CamOsThreadAttrb_t threadPreloadMiPipeAttr = {.nPriority = 99,.szName = "MI_PreloadMiPipe",.nStackSize = 6144};

    CamOsTsemInit(&tPreloadFileTsem,0);
    CamOsTsemInit(&tIspReadFileTsem,0);
    CamOsThreadCreate(&PreloadMiPipe_tid, &threadPreloadMiPipeAttr, MI_PreloadMiPipe, NULL);
    CamOsThreadCreate(&PreloadFile_tid, &threadPreloadFileAttr, MI_PreloadFile, NULL);
#if defined(CONFIG_USB_GADGET_UVC_SUPPORT)
    composite_thread_exit = 0;
    CamOsThreadAttrb_t task_attr = {.nPriority = TASK_USB_APP_PRIORITY,
                                    .szName = TASK_USB_APP_NAME,
                                    .nStackSize = TASK_USB_APP_STACK_SIZE};
    CamOsTsemInit(&composite_sem, 0);
    CamOsThreadCreate(&composite_thread, &task_attr,
                             composite_thread_entry, NULL);
#if defined(CONFIG_USB_GADGET_VBUS_DETECT)
    usb_vbus_init();
#else
    // wake up composite thread to pull up the composite device
    composite_thread_wakeup();
#endif

#endif

}

static int RtosAppMainEntry(int argc, char **argv)
{
    MI_DEVICE_Init(NULL);

    #if defined(CONFIG_USB_GADGET_UVC_SUPPORT) || !defined(CONFIG_SYNC_FROM_RTC)
    CameraBootSetting_t* pCameraBootSetting = NULL;
    pCameraBootSetting = CameraBootSettingGetHandle();
    #endif

    #if defined(CONFIG_SYNC_FROM_RTC)
    SysSetTimeFromRtc();
    #else
    // set time.
    CamOsSetTimeOfDay(&pCameraBootSetting->stSystemTime);
    #endif

    #if defined(CONFIG_USB_GADGET_UVC_SUPPORT) && defined(EN_FIXED_SAMPLE_PRCS) && (EN_FIXED_SAMPLE_PRCS == 1)
    /* NOP when UVC streaming by fixed sample instead of sensor/pipeine */
    /* CAUTION: enable sensor initialize (sys_drv_init) here will cause rtos binary > 2MB for this case (EN_FIXED_SAMPLE_PRCS == 1) */

    #else // defined(CONFIG_USB_GADGET_UVC_SUPPORT) && defined(EN_FIXED_SAMPLE_PRCS) && (EN_FIXED_SAMPLE_PRCS == 1)

    #if defined(CONFIG_USB_GADGET_UVC_SUPPORT)

    #endif

    MI_PreloadTask();

    #endif // // defined(CONFIG_USB_GADGET_UVC_SUPPORT) && defined(EN_FIXED_SAMPLE_PRCS) && (EN_FIXED_SAMPLE_PRCS == 1)
    return 0;
}
#else // if defined(CONFIG_MI_SDK_SUPPORT) && (CONFIG_ENABLE_MI_SDK_PIPELINE_FLOW == 1)

CamOsTsem_t tPreloadFileTsem;
CamOsThread PreloadMiPipe_tid;

static void* MI_PreloadMiPipe(void* p)
{
    MI_SYS_Init(0);
    mi_debug_init();
    CamOsPrintf("[%s,%d] \n", __FUNCTION__, __LINE__);
    if(application_selector_retreat() != 0)
    {
        CamOsPrintf("application_selector_stop failed.\n");
    }
    CamOsTsemDown(&tPreloadFileTsem);
    return 0;
}

static void MI_PreloadTask(void)
{
    CamOsThreadAttrb_t threadPreloadMiPipeAttr = {.nPriority = 99,.szName = "MI_PreloadMiPipe",.nStackSize = 6144};

    CamOsTsemInit(&tPreloadFileTsem,0);
    CamOsThreadCreate(&PreloadMiPipe_tid, &threadPreloadMiPipeAttr, MI_PreloadMiPipe, NULL);
}

static int RtosAppMainEntry(int argc, char **argv)
{
    CamOsPrintf("[%s,%d] MI_DEVICE_Init begin\n", __FUNCTION__, __LINE__);
    MI_DEVICE_Init(NULL);
    CamOsPrintf("[%s,%d] MI_DEVICE_Init done\n", __FUNCTION__, __LINE__);

    MI_PreloadTask();
#if 1
    MI_SYS_Init(0);
#endif
    return 0;
}
#endif
rtos_application_selector_initcall(gadget_preload, RtosAppMainEntry);

#if defined(CONFIG_MI_SDK_SUPPORT) && (CONFIG_ENABLE_MI_SDK_PIPELINE_FLOW == 1)
static int _L2RData(CLI_t * pCli, char * p)
{
    if (CliTokenCount(pCli) == 0)
    {
        goto HELP_EXIT;
    }
    else if (CliTokenCount(pCli) == 1)
    {
        char *pCmd;

        pCmd = CliTokenPop(pCli);
        if (!strncmp(pCmd,"--dlaosdoff", 11))
        {
            ST_SetLinux2RtosData(pCmd);
        }
        else if (!strncmp(pCmd,"--timeosdoff", 12))
        {
            ST_SetLinux2RtosData(pCmd);
        }
        else if (!strncmp(pCmd,"--sedoff", 8))
        {
            ST_SetLinux2RtosData(pCmd);
        }
        else if (!strncmp(pCmd,"--vdfoff", 8))
        {
            ST_SetLinux2RtosData(pCmd);
        }
        else if (!strncmp(pCmd,"--stoppipe", 10))
        {
            ST_SetLinux2RtosData(pCmd);
        }
        else if (!strncmp(pCmd,"--deintmoudle", 13))
        {
            ST_SetLinux2RtosData(pCmd);
        }
        else
        {
            goto HELP_EXIT;
        }
    }
    else
    {
        goto HELP_EXIT;
    }

    return eCLI_PARSE_OK;

HELP_EXIT:
    return eCLI_PARSE_ERROR;
}
SS_RTOS_CLI_CMD(l2rdata,
        "get linux 2 rtos data",
        "Usage: memstat [options]\n"
        "   --dlaosdoff  set g_u8dlaosdoff=1 \n"
        "   --timeosdoff set g_u8timeosdoff=1 \n"
        "   --sedoff deint sed \n"
        "   --vdfoff deint vdf&osd \n"
        "   --stoppipe stop video pipeLine \n"
        "   --deintmoudle deint moudle \n",
        _L2RData);
#endif
