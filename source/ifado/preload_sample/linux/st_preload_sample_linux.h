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

#ifndef __ST_SAMPLE_AOV_PRELOAD_H__
#define __ST_SAMPLE_AOV_PRELOAD_H__

#include "mi_common_datatype.h"

#define MI_CLICMD_PRELOAD               0
#define IOCTL_PRELOAD_DATA_L2R          _IOW('i', MI_CLICMD_PRELOAD, char *)
#define AOV_GET_STATUS                  "Aov_Get_Status"
#define AOV_GET_SENSORNUM               "Aov_Get_SensorNum"
#define DEVICE_FILE                     "/dev/mi_cli"
#define MSG_THEADNAME                   "msg_thread"
#define AI_THEADNAME                    "ai_thread"
#define AO_THEADNAME                    "ao_thread"
#define MI_VENC_WIDTH                   2560
#define MI_VENC_HEIGHT                  1440
#define SENSOR_0                        0
#define SENSOR_1                        1


#define CHECK_L2R_RESULT(_func_, _ret_)      \
    {                                        \
        _ret_ = _func_;                      \
        if (_ret_ < MI_SUCCESS)              \
        {                                    \
            perror("IOCTL_DATA_L2R failed"); \
            close(fd);                       \
            return -1;                       \
        }                                    \
    }

#ifdef __MUSL__
#include <linux/ioctl.h>
#endif
#define MI_CLI_IOCTL(__fd, __cmd, __p)                                                       \
    ({                                                                                       \
        struct                                                                               \
        {                                                                                    \
            unsigned short     __socid;                                                      \
            int                __len;                                                        \
            unsigned long long __ptr;                                                        \
        } __tr     = {0, _IOC_SIZE(__cmd), (unsigned long)__p};                              \
        int __rval = ioctl(__fd, __cmd, &__tr);                                              \
        if (__rval == -1)                                                                    \
        {                                                                                    \
            printf("failed to ioctl 0x%08lx!(%s)\n", (unsigned long)__cmd, strerror(errno)); \
        }                                                                                    \
        __rval;                                                                              \
    })

enum
{
    AOV_DEFAULT_STATUS,
    AOV_IPU_GET_PEOPLE,
    AOV_POWER_DOWN,
    AOV_UNKONW_STATUS,
};

typedef struct
{
    pthread_t   pthread;
    int         thread_exit;
    int         exit;
    const char *threadname;
} ST_pthreadHandel_T;

int ST_MICLIRtosQuit(void);

#endif