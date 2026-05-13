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
#ifndef ST_SMARTFACEAE_H
#define ST_SMARTFACEAE_H
#ifdef __cplusplus

extern "C"
{
#endif
#include "mi_common.h"

#ifdef DBG_EN
#define ISP_INFO(fmt, args...) \
    do{ \
        printf(fmt, ##args); \
    }while(0)
#define ISP_LOG(fmt,args...) \
    do{ \
        printf(fmt, ##args); \
    }while(0)
#else
#define ISP_INFO(fmt, args...)
#define ISP_LOG(fmt,args...)
#endif

#define RGB_AE_CHN 0

#define OVER_LIGHT_RGB 155//300
#define OVER_DARK_RGB 90

#define TARGET_MIN_RGB 400
#define TARGET_NORMAL_RGB 550
#define TARGET_MAX_RGB 650

/****************************
 ****      DARK  Light
 ****        |    |
 ****        V    V
 ****    过暗|正常|过亮
 ****    BaseTargetStep:Target变化的基本步进，和人脸亮度和TH的距离，最终决定一次跑多少。
 ****    u32AjustmentAeSpeed：变化过程，以及恢复瞬间的速度；
 ****    u32AjustmentAeCov：变化过程，以及恢复瞬间的区间；
 ****
 *****************************/
typedef struct ST_SMARTFACEAE_Config_s
{
    MI_BOOL bEnable;
    MI_U32 u32OverDarkValue;
    MI_U32 u32OverLightValue;
    MI_U32 u32BaseTargetStep;
    MI_U32 u32AjustmentAeSpeed[4];
    MI_U32 u32AjustmentAeCov[2];
} ST_SMARTFACEAE_Config_t;

void ST_InitSmartFace(ST_SMARTFACEAE_Config_t *pstConfig);
void ST_SetAEData(MI_U32 u32DevId, MI_U32 u32ChnId, MI_U32 u32Lightness);
void ST_SaveOriAEData(MI_U32 u32DevId);
void ST_RestoreAEData(MI_U32 u32DevId);
void ST_RestoreAESpeedData(MI_U32 u32DevId);

#ifdef __cplusplus
}
#endif
#endif
