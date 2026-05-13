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
#ifndef __RTOS_PRELOAD_COMMON_H__
#define __RTOS_PRELOAD_COMMON_H__

#include <stdio.h>

#if defined(CONFIG_FIRMWAREFS_SUPPORT)
#define MISC_FILESYSTEM CAM_FS_FMT_FIRMWAREFS
#elif defined(CONFIG_LITTLEFS_SUPPORT)
#define MISC_FILESYSTEM CAM_FS_FMT_LITTLEFS
#elif defined(CONFIG_LWFS_SUPPORT)
#define MISC_FILESYSTEM CAM_FS_FMT_LWFS
#else
#error no supported fs for misc partition
#endif
/*For rtos preload use*/
typedef enum
{
    E_INIT_SR0 = 0,
    E_INIT_SR1 = 1,
    E_INIT_SR_MAX_NUM
}INIT_SR_ID_e;

typedef enum
{
    E_HDR_FUSION_TYPE_NONE,
    E_VIF_HDR_FUSION_TYPE_2T1,
    E_VIF_HDR_FUSION_TYPE_3T1,
    E_VIF_HDR_FUSION_TYPE_MAX
} Init_HDRFusionType_e;

typedef enum
{
    E_BIND_TYPE_FRAME    = 0x1,
    E_BIND_TYPE_REALTIME = 0x4,
} Init_BindType_e;

typedef struct
{
    unsigned char u8SnrPad;                    //sensor pad id
    unsigned char bStreamOnoff;                //senssor streamonoff
    unsigned char bMirror;                     //sensor mirror
    unsigned char bFlip;                       //sensor flip
    unsigned long u32SensorFrameRate;          //sensor fps x 1000
    unsigned char u8ResIdx;                    //sensor resolution id
    unsigned long eWorkMode;                   //fixed E_MI_VIF_WORK_MODE_1MULTIPLEX
    unsigned char bHDREn;                      //HDR mode enable
    Init_HDRFusionType_e eHDRFusoinType;  //HDR fusion mode
    unsigned short u16SnrEarlyFlicker;         //AE de-flicker frequency
    unsigned long u32ShutterLEF;               //sensor long exposure frame shutter in us
    unsigned long u32ShutterMEF;               //sensor median exposure frame shutter in us
    unsigned long u32ShutterSEF;               //sensor short exposure frame shutter in us
    unsigned long u32SensorGainLEFx1024;       //sensor long exposure frame gain in 1024 base
    unsigned long u32SensorGainMEFx1024;       //sensor median exposure frame gain in 1024 base
    unsigned long u32SensorGainSEFx1024;       //sensor short exposure frame gain in 1024 base
    unsigned long u32IspGainLEFx1024;          //isp long exposure frame gain in 1024 base
    unsigned long u32IspGainMEFx1024;          //isp median exposure frame gain in 1024 base
    unsigned long u32IspGainSEFx1024;          //isp short exposure frame gain in 1024 base
    Init_BindType_e eBindMode;            //vif->isp bind mode
}InitPreloadChCfg_t;

typedef struct
{
    unsigned long long u64Magic;
    unsigned long u32NumSnr;
    InitPreloadChCfg_t ChCfg[E_INIT_SR_MAX_NUM];
}InitPreloadCfg_t;

char* ST_RtosPreloadGetRoFilePath(void);
char* ST_RtosPreloadGetRwFilePath(void);
int ST_RtosPreloadinit(void);
int ST_RtosPreloadExit(void);
int ST_GetRtosPreloadInitParam(InitPreloadCfg_t *pstInit);
#endif //__RTOS_PRELOAD_COMMON_H__