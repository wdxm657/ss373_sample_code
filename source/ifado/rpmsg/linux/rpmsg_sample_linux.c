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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#include "sstar_rpmsg.h"
#include "rpmsg_dualos_common.h"

#define MODULE_NAME "RPMSG"

uint32_t u32TimeoutSeconds = 10;
uint32_t u32TestCount      = 0;

int32_t ST_RPMSG_GetCmdlineParam(int argc, char **argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "timeout_seconds"))
        {
            u32TimeoutSeconds = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "test_count"))
        {
            u32TestCount = atoi(argv[i + 1]);
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    struct ss_rpmsg_endpoint_info info;
    char                          devPath[64];
    int32_t                       s32Ret            = 0;
    int32_t                       s32RpmsgFd        = 0;
    int32_t                       s32RpmsgCtrlFd    = 0;
    char                          send_common[128]  = {MODULE_NAME};
    char                          send_data[256]    = {0};
    char                          recv_data[256]    = {0};
    struct timeval                timeout;
    fd_set                        readfds;

    ST_RPMSG_GetCmdlineParam(argc, argv);

    // set timeout
    timeout.tv_sec  = u32TimeoutSeconds;
    timeout.tv_usec = 0;

    memset(&info, 0, sizeof(info));
    info.src =
        EPT_ADDR_PREFIX(EPT_TYPE_SIGMASTAR, EPT_SOC_DEFAULT, EPT_OS_LINUX) | EPT_CLASS(EPT_CLASS_RESERVED) | 0x66;
    info.dst = EPT_ADDR_PREFIX(EPT_TYPE_SIGMASTAR, EPT_SOC_DEFAULT, EPT_OS_RTOS) | EPT_CLASS(EPT_CLASS_RESERVED) | 0x20;
    snprintf(info.name, sizeof(info.name), "rpmsg_sample_linux");

    info.mode      = 5;
    info.target_id = 0;

    s32RpmsgFd = open("/dev/rpmsg_ctrl0", O_RDWR);
    if (s32RpmsgFd < 0)
    {
        printf("RPMSG CASE FAIL: open /dev/rpmsg_ctrl0 failed");
        return 0;
    }

    if (ioctl(s32RpmsgFd, SS_RPMSG_CREATE_EPT_IOCTL, &info) < 0)
    {
        printf("RPMSG CASE FAIL: ioctl SS_RPMSG_CREATE_EPT_IOCTL failed");
        return 0;
    }
    // waiting for Rpmsg Init
    usleep(100 * 1000);

    snprintf(devPath, sizeof(devPath), "/dev/rpmsg%d", info.id);
    s32RpmsgCtrlFd = open(devPath, O_RDWR);
    if (s32RpmsgCtrlFd < 0)
    {
        printf("RPMSG CASE FAIL: Failed to open %s %d!\n", devPath, s32RpmsgCtrlFd);
        return 0;
    }

    memset(send_data, 0, sizeof(send_data));
    if(u32TestCount > 0)
    {
        sprintf(send_data, "%s %d",send_common, u32TestCount);
    }
    else
    {
        strcpy(send_data, send_common);
    }

    s32Ret = write(s32RpmsgCtrlFd, send_data, strlen(send_data) + 1);
    if (s32Ret < 0)
    {
        printf("RPMSG CASE FAIL: Failed to write endpoint!\n");
        return 0;
    }

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(s32RpmsgCtrlFd, &readfds);

        s32Ret = select(s32RpmsgCtrlFd + 1, &readfds, NULL, NULL, &timeout);

        if (s32Ret == -1)
        {
            printf("RPMSG CASE FAIL: select fail");
        }
        else if (s32Ret == 0)
        {
            printf("RPMSG CASE FAIL: Timeout \n");
            break;
        }
        else
        {
            memset(recv_data, 0, sizeof(recv_data));
            s32Ret = read(s32RpmsgCtrlFd, recv_data, sizeof(recv_data));
            if (s32Ret > 0)
            {
                if (strcmp("pass", recv_data) == 0)
                {
                    printf("RPMSG CASE PASS\n");
                }
                else if (strcmp("fail", recv_data) == 0)
                {
                    printf("RPMSG CASE FAIL: RPMSG CASE FAIL \n");
                }
                else
                {
                    printf("RPMSG CASE FAIL: read date not find \n");
                }
            }
            else
            {
                printf("RPMSG CASE FAIL: read ept fail:%d\n", s32Ret);
            }
            break;
        }
    }
    close(s32RpmsgCtrlFd);
    close(s32RpmsgFd);
    return 0;
}