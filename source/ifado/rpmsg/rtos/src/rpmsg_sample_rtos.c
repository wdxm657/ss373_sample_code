/* SigmaStar trade secret */
/* Copyright (c) [2019~2022] SigmaStar Technology.
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
#include <stdlib.h>
#include <string.h>

#include "cam_os_wrapper.h"
#include "mi_common_datatype.h"
#include "rpmsg_dualos.h"
#include "cam_fs_wrapper.h"
#include "sys_memmap.h"

#if defined(CONFIG_APPLICATION_SELECTOR)
#include "application_selector.h"
#endif

#define MODULE_NAME "RPMSG"

static struct rpmsg_lite_instance *rpmsg_instance;
static CamOsThread                 rpmsg_sample_demo_thread;

static rpmsg_queue_handle          stRpmsgQueueHandle;
static struct rpmsg_lite_endpoint *stRpmsgLiteEndpoint;

int ST_RPMSG_Init(void)
{
    int EndpointAddr;

    CamOsPrintf("Running rpmsg_init ...\n");
    // get rpmsg instance
    while (1)
    {
        rpmsg_instance = rpmsg_dualos_get_instance(EPT_SOC_DEFAULT, EPT_OS_LINUX);
        if (rpmsg_instance)
            break;
        CamOsPrintf("Ru ...\n");
        CamOsMsSleep(1);
    }
    // create rpmsg queue
    stRpmsgQueueHandle = rpmsg_queue_create(rpmsg_instance);
    if (stRpmsgQueueHandle == NULL)
    {
        CamOsPrintf("rpmsg: failed to create queue\n");
        return -1;
    }

    EndpointAddr =
        EPT_ADDR_PREFIX(EPT_TYPE_SIGMASTAR, EPT_SOC_DEFAULT, EPT_OS_RTOS) | EPT_CLASS(EPT_CLASS_RESERVED) | 0x20;
    stRpmsgLiteEndpoint = rpmsg_lite_create_ept(rpmsg_instance, EndpointAddr, rpmsg_queue_rx_cb, stRpmsgQueueHandle);
    if (stRpmsgLiteEndpoint == NULL)
    {
        CamOsPrintf("rpmsg: failed to create ept\n");
        rpmsg_queue_destroy(rpmsg_instance, stRpmsgQueueHandle);
        return -1;
    }

    return 0;
}

void ST_RPMSG_Deinit(void)
{
    rpmsg_lite_destroy_ept(rpmsg_instance, stRpmsgLiteEndpoint);
    rpmsg_queue_destroy(rpmsg_instance, stRpmsgQueueHandle);
}

static void *ST_RPMSG_Thread(void *arg)
{
    int                   recved = 0;
    int                   ret    = 0;
    char                  buf_rec[256];
    unsigned long         src;

    if(ST_RPMSG_Init() != 0)
    {
        CamOsPrintf("rpmsg_init failed!\n");
        return NULL;
    }
    CamOsPrintf("rpmsg_init ok\n");

    while (1)
    {
        recved    = 0x0;
        memset(buf_rec, 0, sizeof(buf_rec));
        rpmsg_queue_recv(rpmsg_instance, stRpmsgQueueHandle, &src, (char *)buf_rec, 256, &recved, RL_BLOCK);
        if (recved > 0)
        {
            CamOsPrintf("rtos get data from linux: %s\n", buf_rec);
            ret = rpmsg_lite_send(rpmsg_instance, stRpmsgLiteEndpoint, src, "pass", 4, 5 * 1000);
            if (ret != RL_SUCCESS)
            {
                CamOsPrintf("rpmsg_send rpmsg_lite_send return %d\n", ret);
            }
            break;
        }
    }
    ST_RPMSG_Deinit();

    return NULL;
}

extern int    MI_DEVICE_Init(void* pMmaConfig);

static int ST_RPMSG_Main(int menu_argc, char** menu_argv)
{
    if (application_selector_retreat() != 0)
    {
        CamOsPrintf("application selector stop failed.\n");
    }

    CamOsThreadAttrb_t attr_read = {.nStackSize = 40960};
    attr_read.szName             = "Rpmsg_Thread";
    attr_read.nPriority          = 95;
    CamOsThreadCreate(&rpmsg_sample_demo_thread, &attr_read, ST_RPMSG_Thread, NULL);
    CamOsPrintf("rpmsg_sample_demo create ok ...\n");

    return 0;
}

rtos_application_selector_initcall(rpmsg, ST_RPMSG_Main);
