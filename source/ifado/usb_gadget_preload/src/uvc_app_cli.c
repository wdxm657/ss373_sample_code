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

#include <string.h>
#include "sys_sys_isw_cli.h"
#include "sys_sys_console.h"
#include "cam_os_wrapper.h"

#include "usb_gadget_preload.h"

#if (VID_LATENCY_MEASURE)
static char uvc_latency_help_txt[] = "Show UVC stream latency";
static char uvc_latency_usage_txt[] = "";

void uvc_stream_latency_show(void)
{
    u32 strm_no = 0;
    unsigned int vid_idx = 0;
    u32 frame_time_tx = 0, frame_size_tx = 0, frame_interval = 0;

    if(!usb_vs_get_latency_measurement_tx_enable())
    {
        CamOsPrintf("UVC Latency is disabled.\r\n");
        return;
    }

    for(vid_idx = 0; vid_idx < MAX_VS_IF; ++vid_idx)
    {
        if(!usb_vs_get_latency_measurement_tx_start(vid_idx))
        {
            continue;
        }

        //strm_no = ST_UVC_GetStreamIDFromEPID(vid_idx); //TBD
        strm_no = vid_idx; //TBD
        frame_time_tx = usb_vs_get_one_frame_time_tx(vid_idx);
        frame_size_tx = usb_vs_get_one_frame_size_tx(vid_idx);
        frame_interval = usb_vs_get_average_frame_time_interval(vid_idx);

        CamOsPrintf("Video ID: %d, Stream No: %d\r\n", vid_idx, strm_no);
        CamOsPrintf("MI PTS -> Get frame: %d us\r\n", usb_vs_get_stream_data_pts_average(strm_no));
        CamOsPrintf("UVC Tx per frame: %d us\r\n", frame_time_tx);
        CamOsPrintf("One frame size: %d kBytes\r\n", frame_size_tx);
        CamOsPrintf("Bandwidth: %d (kBytes/Sec)\r\n", (frame_time_tx > 0) ? (u32)(((u64)frame_size_tx * 1000000) / frame_time_tx) : 0);
        if(frame_interval > 0)
        {
            CamOsPrintf("Real FPS: %d.%02d\r\n", (1000000 / frame_interval), (100 * 1000000 / frame_interval) % 100);
        }
        CamOsPrintf("\r\n");
    }
}

static s32 uvc_latency_handle(CLI_t * pCli, char * p)
{
    char *pCmd;
    int itoken_cnt = 0;

    itoken_cnt = CliTokenCount(pCli);

    if(itoken_cnt == 0)
    {
        CamOsPrintf("== uvc_latency help ==\r\n");
        CamOsPrintf("uvc_latency enable\r\n");
        CamOsPrintf("uvc_latency disable\r\n");
        CamOsPrintf("uvc_latency show\r\n");
    }
    else if(itoken_cnt == 1)
    {
        pCmd = CliTokenPop(pCli);

        if (!strncmp(pCmd,"enable", 6))
        {
            usb_vs_set_latency_measurement_tx_enable(1);
        }
        else if (!strncmp(pCmd,"disable", 7))
        {
            usb_vs_set_latency_measurement_tx_enable(0);
        }
        else if (!strncmp(pCmd,"show", 4))
        {
            uvc_stream_latency_show();
        }
        return eCLI_PARSE_OK;
    }
    else if(itoken_cnt == 2)
    {
#if 0
        //TBD
        char *pParameter;

        pCmd = CliTokenPop(pCli);
        pParameter = CliTokenPop(pCli);
#endif
        return eCLI_PARSE_OK;
    }
    else
    {
        return eCLI_PARSE_ERROR;
    }

    return eCLI_PARSE_OK;
}

SS_RTOS_CLI_CMD(uvc_latency, uvc_latency_help_txt, uvc_latency_usage_txt, uvc_latency_handle);
#endif

