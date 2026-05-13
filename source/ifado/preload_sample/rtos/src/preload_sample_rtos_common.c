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
#include <stdbool.h>
#include "initcall.h"
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"

#if defined(CONFIG_SENSOR_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT)
#include "earlyinit_preload_api.h"
#endif
#include "preload_sample_rtos_common.h"

// below path don't export to user directly
#define MISC_NAME        "MISC"
#define MISC_RTOS_NAME   "MISC_RTOS"
#define MISC_PATH        "/misc"
#define ROFILES_PART_NAME     "RO_FILES"
#define ROFILES_PATH     "/rofiles"


static bool fs_mounted = false;
static bool rofiles_mounted = false;

static int DEVICE_Mount_Fs(void)
{
    CamFsRet_e eRet = CAM_FS_FAIL;

    if (fs_mounted == false)
    {
        eRet = CamFsMount(MISC_FILESYSTEM, MISC_RTOS_NAME, MISC_PATH);
        if (eRet != CAM_FS_OK)
        {
            eRet = CamFsMount(MISC_FILESYSTEM, MISC_NAME, MISC_PATH);
            if (eRet != CAM_FS_OK)
            {
                CamOsPrintf("Failed to Mount MISC Partition\n");
                return -1;
            }
        }

#if defined(CONFIG_LWFS_SUPPORT)
        eRet = CamFsMount(CAM_FS_FMT_LWFS, ROFILES_PART_NAME, ROFILES_PATH);
        if (eRet == CAM_FS_OK)
            rofiles_mounted = true;
#endif
        fs_mounted = true;
    }
    CamFsShowMount();

    return 0;
}

static int DEVICE_Unmount_Fs(void)
{
    CamFsRet_e eRet = CAM_FS_FAIL;

    if (fs_mounted)
    {
        if (rofiles_mounted)
        {
            eRet = CamFsUnmount(ROFILES_PATH);
            if (eRet != CAM_FS_OK)
                return -1;
            rofiles_mounted = false;
        }

        eRet = CamFsUnmount(MISC_PATH);
        if (eRet != CAM_FS_OK)
            return -1;
        fs_mounted = false;
    }

    return 0;
}

// for big file such as ipu model, isp iq bin etc..
char* ST_RtosPreloadGetRoFilePath(void)
{
    if (fs_mounted)
    {
        if (rofiles_mounted)
        {
            return ROFILES_PATH;
        }
        else
        {
            return MISC_PATH;
        }
    }
    else
    {
        return NULL;
    }
}

// for small file such as modparam.json, PreloadSetting.txt etc..
char* ST_RtosPreloadGetRwFilePath(void)
{
    if (fs_mounted)
    {
        return MISC_PATH;
    }
    else
    {
        return NULL;
    }

}

#ifdef CONFIG_SENSOR_EARLYINIT_SUPPORT
void earlyinit_main(void);
#ifndef CONFIG_EARLYINIT_SETTING_FS
//premain_2nd_initcall(earlyinit_main);
#endif
#endif

int ST_RtosPreloadinit(void)
{
    int ret = 0;

    ret = DEVICE_Mount_Fs();
    if (ret)
    {
        return ret;
    }
#ifdef CONFIG_EARLYINIT_SETTING_FS
    earlyinit_main();
#endif
    return ret;
}

int ST_RtosPreloadExit(void)
{
    if(DEVICE_Unmount_Fs() != 0)
    {
        CamOsPrintf("DEVICE_Unmount_Fs failed.\n");
    }
    return 0;
}

int ST_GetRtosPreloadInitParam(InitPreloadCfg_t *pstInit)
{
#if defined(CONFIG_SENSOR_EARLYINIT_SUPPORT) || defined(CONFIG_SENSOR_IPL_EARLYINIT_SUPPORT)
    unsigned char earlyinit_enable = 0;
    EarlyInitPreloadCfg_t *pearlyinit = DrvEarlyInitGetPreloadCfg();
    pstInit->u32NumSnr = pearlyinit->u32NumSnr;
    for(int dev = 0; dev < pearlyinit->u32NumSnr; ++dev)
    {
        earlyinit_enable = DrvEarlyInitForPreloadIsEnabled(dev);
        if(!earlyinit_enable)
        {
            pstInit->ChCfg[dev].u8SnrPad = -1;
            continue;
        }
        else
        {
            pstInit->ChCfg[dev].u16SnrEarlyFlicker = pearlyinit->ChCfg[dev].u16SnrEarlyFlicker;
            pstInit->ChCfg[dev].u32ShutterLEF = pearlyinit->ChCfg[dev].u32ShutterLEF;
            pstInit->ChCfg[dev].u32SensorGainSEFx1024 = pearlyinit->ChCfg[dev].u32SensorGainSEFx1024;
            pstInit->ChCfg[dev].u32IspGainLEFx1024 = pearlyinit->ChCfg[dev].u32IspGainLEFx1024;
            pstInit->ChCfg[dev].eBindMode = pearlyinit->ChCfg[dev].eBindMode;
            pstInit->ChCfg[dev].u32SensorFrameRate = pearlyinit->ChCfg[dev].u32SensorFrameRate;
            pstInit->ChCfg[dev].u8SnrPad = pearlyinit->ChCfg[dev].u8SnrPad;
            pstInit->ChCfg[dev].u8ResIdx = pearlyinit->ChCfg[dev].u8ResIdx;
        }
        CamOsPrintf("[%d] u32NumSnr:%d u8SnrPad:%d  eBindMode:%d\n",dev,pstInit->u32NumSnr, pstInit->ChCfg[dev].u8SnrPad, pstInit->ChCfg[dev].eBindMode);
    }
#else
    for (int dev = 0; dev < E_INIT_SR_MAX_NUM; ++dev)
    {
        pstInit->ChCfg[dev].u16SnrEarlyFlicker = 1;
        pstInit->ChCfg[dev].u32ShutterLEF = 24978;
        pstInit->ChCfg[dev].u32SensorGainSEFx1024 = 1280;
        pstInit->ChCfg[dev].u32IspGainLEFx1024 = 1024;
        pstInit->ChCfg[dev].eBindMode = E_BIND_TYPE_FRAME;
        pstInit->ChCfg[dev].u32SensorFrameRate = 15000;
        pstInit->ChCfg[dev].u8SnrPad  = -1;
        pstInit->ChCfg[dev].u8ResIdx  = 0;

#if defined(CONFIG_SENSOR_SUPPORT_AOV_DUALSNR_RT)
        pstInit->u32NumSnr = 2;
        pstInit->ChCfg[dev].u8SnrPad  = dev*2;
        pstInit->ChCfg[dev].eBindMode = E_BIND_TYPE_REALTIME;
        CamOsPrintf("[%d]u8SnrPad:%d  eBindMode:%d\n",dev, pstInit->ChCfg[dev].u8SnrPad, pstInit->ChCfg[dev].eBindMode);
#else
       pstInit->ChCfg[dev].u8SnrPad  = dev*2;
       if(pstInit->u32NumSnr < 2)
       {
          pstInit->ChCfg[dev].eBindMode = E_BIND_TYPE_REALTIME;
          return 0;
       }
#endif
    }
    CamOsPrintf("snr num:%u\n",pstInit->u32NumSnr);
#endif
    return 0;
}