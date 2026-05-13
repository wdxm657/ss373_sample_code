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

#ifndef __RTOS_PRELOAD_H__
#define __RTOS_PRELOAD_H__

#include "mi_common_internal.h"
#include "mi_common_modparam.h"

#define AOV_GET_STATUS      "Aov_Get_Status"
#define AOV_GET_SENSORNUM   "Aov_Get_SensorNum"
#define MAX_FRAME_HANDLE    (16)
#define _EXT_BIN            ".bin"
#define _ISP_API            "isp_api"
#define MI_CLICMD_PRELOAD   0
#define AOV_MAX_SENSOR_NUM  2

#define CHECK_DLA_RESULT(_func_, _ret_)                                                   \
    {                                                                                     \
        _ret_ = _func_;                                                                   \
        if (_ret_ != MI_SUCCESS)                                                          \
        {                                                                                 \
            printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, _ret_); \
            continue;                                                                     \
        }                                                                                 \
    }

#define STCHECKRESULT(result)\
    if (result != MI_SUCCESS)\
    {\
        CamOsPrintf("[%s %d]exec function failed\n", __FUNCTION__, __LINE__);\
        return 1;\
    }

enum{
    AOV_DEFAULT_STATUS,
    AOV_IPU_GET_PEOPLE,
    AOV_POWER_DOWN,
    AOV_UNKONW_STATUS,
};

typedef struct ST_Stream_Attr_S
{
    MI_BOOL             bEnable;
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
    MI_SYS_BindType_e   eBindType;
    MI_U32              u32BindPara;
 }ST_Stream_Attr_T;

struct MMA_BootArgs_Config stMmaConfig[MAX_MMA_AREAS]=
{
    {0, CONFIG_MMA_HEAP_SIZE, "mma_heap_name0", 0, CONFIG_MMA_HEAP_ADDR},
};

typedef struct _MaxFrameSize {
    MI_U32 frameSizeI;
    MI_U32 frameSizeP;
} MaxFrameSize;

extern CamOsTsem_t tPreloadFileTsem;
extern CamOsTsem_t tIspReadFileTsem;
extern int    MI_DEVICE_Init(void* pMmaConfig);
extern int    mi_debug_init(void);

void *ST_DetectThread(void *p);

#endif /*__RTOS_PRELOAD_H__*/
