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
#include "CameraSetting.h"
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"
#include "env_util.h"
#include "sys_sys_boot_timestamp.h"

/////////////////////////////////////////////////////////

#define CamSetting_LOGINFO(fmt, args...)                            //CamOsPrintf(fmt, ##args)
#define CamSetting_LOGERR(fmt, args...)                             CamOsPrintf(fmt, ##args)

#if defined(CONFIG_LOAD_CAMERA_SETTING_FROM_BOOTENV)
#define PRELOAD_SETTING_FROM                                        "BOOTENV"
#else
#define PRELOAD_SETTING_FROM                                        "MISC"
#endif
#define PRELOAD_SETTING_TXT_PATH                                    "/misc/PreloadSetting.txt"
#define PRELOAD_SETTING_TXT_READ_SIZE                               3072 // 3*1024

/////////////////////////////////////////////////////////

static u8 g_u8uLoadMiscPartitionBufFilled                           = 0;
static u8 g_au8LoadMiscPartitionBuf[PRELOAD_SETTING_TXT_READ_SIZE]  = {0};
static char *pPartitionBuf = NULL;
static u8 g_u8EnableLoadCamSetting = 0;

QpMapping_t gQpMapping[8] =
{
  {1,1},
  {4,5},
  {3,5},
  {2,5},
  {1,5},
  {1,15},
  {1,60},
  {1,120}
};

CameraBootSetting_t gCameraBootSetting =
{
    30,                 // u8SensorFrameRate
    2000,               // u32Lux
    0,                  // u8DayNightMode
    1,                  // u8AntiFlicker
    24978,              // u32shutter
    1280,               // u32SensorGain
    1024,               // u32DigitalGain
    1405,               // u16AWBRGain
    1024,               // u16AWBGGain
    2317,               // u16AWBBGain
    0,                  // u8Orientation
    0,                  // u32ExposureLevel     //no use
    0,                  // u8HDRMode            //no use
    1,                  // u8SensorNum
    {1546300800, 0},    // stSystemTime
#if defined(CONFIG_OSD_USE_ARGB1555) || defined(CONFIG_OSD_USE_BMP)
    1,                  // u8RegionsCun
#else
    0,                  // u8RegionsCun
#endif
    {0, 0},             // u16Regions_x[2]
    {0, 0},             // u16Regions_y[2]
    {1, 1},             // u16Regions_w[2]
    {1, 1},             // u16Regions_h[2]
    5,                  // u8IspClkIndex
    0,                  // u8SclClkIndex
    216000000,          // u32DivpClk
#if defined(__INFINITY6C__)
    384000000,          // u32VencClk
    320000000,          // u32Venc2ndpuClk
#elif defined(__INFINITY7__)
    600000000,          // u32VencClk
    600000000,          // u32Venc2ndpuClk
#elif defined(__MERCURY6P__)
    480000000,          // u32VencClk
    384000000,          // u32Venc2ndpuClk
#else
    432000000,          // u32VencClk
    384000000,          // u32Venc2ndpuClk
#endif
#if defined(__PIONEER5__)
    216000000,          // u32JpeClk
#elif defined(__INFINITY7__) || defined(__MERCURY6P__)
    480000000,          // u32JpeClk
#elif defined(__INFINITY6C__)
    320000000,          // u32JpeClk
#else
    320000000,          // u32JpeClk
#endif
    1920,               // u32VencMhalMaxWidth
    1088,               // u32VencMhalMaxHeight
    1,                  // u8enableUserYUV
    5,                  // u32PreloadYUVFrameRate
    15,                 // u32PreloadYUVFrameDepth
    100,                // u32PreloadVideoFrame
    120,                // u32PreloadAudioFrame
#if defined(CONFIG_IPU_IN_RTOS_ENABLE)
    1,                  // u8enableIPU
#else
    0,                  // u8enableIPU
#endif
    800,                // u32IPUClk
    0,                  // u32IPUFwHeapSize
    0,                  // u8IPUDynamicSwitch;
    0,                  // u32IPUSensitivity;
    0,                  // u32IPUActiveAreaX;
    0,                  // u32IPUActiveAreaY;
    1920,               // u32IPUActiveAreaW;
    1080,               // u32IPUActiveAreaH;
#if defined(CONFIG_VDF_IN_RTOS_ENABLE)
    1,                  // u8enableVDF;
#else
    0,                  // u8enableVDF;
#endif
    0,                  // u8VDFWorkMode;
#if defined(CONFIG_SED_ENABLE)
    1,                  // u8enableSED;
#else
    0,                  // u8enableSED;
#endif
#if defined(CONFIG_LDC_IN_RTOS_ENABLE)
    1,                  // u8enableLDC;
#else
    0,                  // u8enableLDC;
#endif
    0,                  // u32AIDevId;
    16000,              // u32AISamplerate;
    14,                 // u32AIVol;
    0,                  // u32AODevId
    0,                  // u32AoBitwidth
    1,                  // u32AoWorkmode
    16000,              // u32AOSamplerate
    14,                 // u32AOVol
    1,                  // u8VideoNum;
    0,                  // u8Video0Format;
    30,                 // u8Video0EncFrameRate;
    0,                  // u8Video0Input;       //no use
    0,                  // u32Video0InputChn;   //no use
    1,                  // u32Video0InputPort;  //no use
    0,                  // u8Video0VencChn;     //no use
    1,                  // u8Video0BindType;
    1920,               // u16Video0Width;
    1080,               // u16Video0Height;
    1920,               // u16Video0MaxWidth
    1088,               // u16Video0MaxHeight
    2097152,            // u32Video0Bitrate;
    1,                  // u32Video0Profile;
    12,                 // u32Video0Minqp;
    48,                 // u32Video0Maxqp;
    60,                 // u16Video0Gop;
    0,                  // u8Video0H264RcMode;
    2,                  // u8Video0H265RcMode;
    0,                  // u8Video0Metadata;    //no use
    0,                  // u16Video0VPECropx;
    0,                  // u16Video0VPECropy;
    0,                  // u16Video0VPECropw;
    0,                  // u16Video0VPECroph;
    0,                  // u8Video1Format;
    30,                 // u8Video1EncFrameRate;
    1,                  // u8Video1Input;       //no use
    0,                  // u32Video1InputChn;   //no use
    2,                  // u32Video1InputPort;  //no use
    1,                  // u8Video1VencChn;     //no use
    640,                // u16Video1Width;
    360,                // u16Video1Height;
    2097152,            // u32Video1Bitrate;
    1,                  // u32Video1Profile;
    12,                 // u32Video1Minqp;
    48,                 // u32Video1Maxqp;
    60,                 // u16Video1Gop;
    0,                  // u8Video1H264RcMode;
    2,                  // u8Video1H265RcMode;
    0,                  // u8Video1Metadata;    //no use
    0,                  // u16Video1VPECropx;
    0,                  // u16Video1VPECropy;
    0,                  // u16Video1VPECropw;
    0,                  // u16Video1VPECroph;
    0,                  // u8Video2Format;
    30,                 // u8Video2EncFrameRate;
    1,                  // u8Video2Input;       //no use
    0,                  // u32Video2InputChn;   //no use
    2,                  // u32Video2InputPort;  //no use
    2,                  // u8Video2VencChn;     //no use
    384,                // u16Video2Width;
    256,                // u16Video2Height;
    2097152,            // u32Video2Bitrate;
    1,                  // u32Video2Profile;
    12,                 // u32Video2Minqp;
    48,                 // u32Video2Maxqp;
    60,                 // u16Video2Gop;
    0,                  // u8Video2H264RcMode;
    2,                  // u8Video2H265RcMode;
    0,                  // u8Video2Metadata;    //no use
    0,                  // u16Video2VPECropx;
    0,                  // u16Video2VPECropy;
    0,                  // u16Video2VPECropw;
    0,                  // u16Video2VPECroph;
    0,                  // u16alsName           //no use
    0,                  // u16AlsDayThr         //no use
    0,                  // u16AlsNightThr       //no use
    0,                  // u16IRCut             //no use
    0,                  // u16IRLed             //no use
    0,                  // u16ExtDacName        //no use
    0,                  // u32ExtDacGain        //no use
};

/*
for Uboot ENV example:
setenv r_SensorFrameRate 30
setenv r_Lux 2000
setenv r_DayNightMode 0
setenv r_AntiFlicker 1
setenv r_shutter 24978
setenv r_SensorGain 1280
setenv r_DigitalGain 1024
setenv r_AWBRGain 1405
setenv r_AWBGGain 1024
setenv r_AWBBGain 2317
setenv r_Orientation 0
setenv r_ExposureLevel 0
setenv r_HDRMode 0
setenv r_SensorNum 1
setenv r_stSystemTime.nSec 1546300800
setenv r_stSystemTime.nNanoSec 0
setenv r_RegionsCun 0
setenv r_Regions_x0 0
setenv r_Regions_x1 0
setenv r_Regions_y0 0
setenv r_Regions_y1 0
setenv r_Regions_w0 1
setenv r_Regions_w1 1
setenv r_Regions_h0 1
setenv r_Regions_h1 1
setenv r_IspClkIndex 0
setenv r_SclClkIndex 2
setenv r_DivpClk 384000000
setenv r_VencClk 384000000
setenv r_Venc2ndpuClk 320000000
setenv r_JpeClk 480000000
setenv r_VencMhalMaxWidth 3840
setenv r_VencMhalMaxHeight 2160
setenv r_PreloadYUVFrameRate 5
setenv r_PreloadYUVFrameDepth 25
setenv r_PreloadVideoFrame 120
setenv r_PreloadAudioFrame 150
setenv r_enableIPU 0
setenv r_IPUClk 1
setenv r_IPUDynamicSwitch 0
setenv r_IPUSensitivity 0
setenv r_IPUActiveAreaX 0
setenv r_IPUActiveAreaY 0
setenv r_IPUActiveAreaW 0
setenv r_IPUActiveAreaH 0
setenv r_enableVDF 0
setenv r_VDFWorkMode 0
setenv r_enableSED 0
setenv r_enableLDC 0
setenv r_AIDevId 0
setenv r_AISamplerate 16000
setenv r_AIVol 14
setenv r_AODevId 0
setenv r_AOSamplerate 16000
setenv r_AOVol 14
setenv r_VideoNum 1
setenv r_Video0Format 0
setenv r_Video0EncFrameRate 30
setenv r_Video0Input 0
setenv r_Video0InputChn 0
setenv r_Video0InputPort 0
setenv r_Video0VencChn 0
setenv r_Video0Width 1920
setenv r_Video0Height 1080
setenv r_Video0Bitrate 2097152
setenv r_Video0Profile 1
setenv r_Video0Minqp 0
setenv r_Video0Maxqp 51
setenv r_Video0Gop 60
setenv r_Video0H264RcMode 0
setenv r_Video0H265RcMode 2
setenv r_Video0Metadata 0
setenv r_Video0VPECropx 0
setenv r_Video0VPECropy 0
setenv r_Video0VPECropw 0
setenv r_Video0VPECroph 0
setenv r_Video1Format 0
setenv r_Video1EncFrameRate 30
setenv r_Video1Input 1
setenv r_Video1InputChn 0
setenv r_Video1InputPort 2
setenv r_Video1VencChn 1
setenv r_Video1Width 640
setenv r_Video1Height 360
setenv r_Video1Bitrate 2097152
setenv r_Video1Profile 1
setenv r_Video1Minqp 0
setenv r_Video1Maxqp 51
setenv r_Video1Gop 60
setenv r_Video1H264RcMode 0
setenv r_Video1H265RcMode 2
setenv r_Video1Metadata 0
setenv r_Video1VPECropx 0
setenv r_Video1VPECropy 0
setenv r_Video1VPECropw 0
setenv r_Video1VPECroph 0
setenv r_Video2Format 0
setenv r_Video2EncFrameRate 30
setenv r_Video2Input 1
setenv r_Video2InputChn 0
setenv r_Video2InputPort 2
setenv r_Video2VencChn 2
setenv r_Video2Width 384
setenv r_Video2Height 216
setenv r_Video2Bitrate 2097152
setenv r_Video2Profile 1
setenv r_Video2Minqp 0
setenv r_Video2Maxqp 51
setenv r_Video2Gop 60
setenv r_Video2H264RcMode 0
setenv r_Video2H265RcMode 2
setenv r_Video2Metadata 0
setenv r_Video2VPECropx 0
setenv r_Video2VPECropy 0
setenv r_Video2VPECropw 0
setenv r_Video2VPECroph 0
setenv r_alsName 0
setenv r_AlsDayThr 0
setenv r_AlsNightThr 0
setenv r_IRCut 0
setenv r_IRLed 0
setenv r_ExtDacName 0
setenv r_ExtDacGain 0
saveenv

for PreloadSetting.txt example:
r_SensorFrameRate=30
r_Lux=2000
r_DayNightMode=0
r_AntiFlicker=1
r_shutter=24978
r_SensorGain=1280
r_DigitalGain=1024
r_AWBRGain=1405
r_AWBGGain=1024
r_AWBBGain=2317
r_Orientation=0
r_ExposureLevel=0
r_HDRMode=0
r_SensorNum=1
r_stSystemTime.nSec=1546300800
r_stSystemTime.nNanoSec=0
r_RegionsCun=0
r_Regions_x0=0
r_Regions_x1=0
r_Regions_y0=0
r_Regions_y1=0
r_Regions_w0=1
r_Regions_w1=1
r_Regions_h0=1
r_Regions_h1=1
r_IspClkIndex=0
r_SclClkIndex=2
r_DivpClk=384000000
r_VencClk=384000000
r_Venc2ndpuClk=320000000
r_PreloadYUVFrameRate=5
r_PreloadYUVFrameDepth=25
r_PreloadVideoFrame=120
r_PreloadAudioFrame=150
r_enableIPU=0
r_IPUClk=1
r_IPUDynamicSwitch=0
r_IPUSensitivity=0
r_IPUActiveAreaX=0
r_IPUActiveAreaY=0
r_IPUActiveAreaW=0
r_IPUActiveAreaH=0
r_enableVDF=0
r_VDFWorkMode 0
r_enableSED=0
r_enableLDC=0
r_JpeClk=480000000
r_AIDevId=0
r_AISamplerate=16000
r_AIVol=14
r_AODevId=0
r_AOSamplerate=16000
r_AOVol=14
r_VideoNum=1
r_Video0Format=0
r_Video0EncFrameRate=30
r_Video0Input=0
r_Video0InputChn=0
r_Video0InputPort=0
r_Video0VencChn=0
r_Video0Width=1920
r_Video0Height=1080
r_Video0Bitrate=2097152
r_Video0Profile=1
r_Video0Minqp=0
r_Video0Maxqp=51
r_Video0Gop=60
r_Video0H264RcMode=0
r_Video0H265RcMode=2
r_Video0Metadata=0
r_Video0VPECropx=0
r_Video0VPECropy=0
r_Video0VPECropw=0
r_Video0VPECroph=0
r_Video1Format=0
r_Video1EncFrameRate=30
r_Video1Input=1
r_Video1InputChn=0
r_Video1InputPort=2
r_Video1VencChn=1
r_Video1Width=640
r_Video1Height=360
r_Video1Bitrate=2097152
r_Video1Profile=1
r_Video1Minqp=0
r_Video1Maxqp=51
r_Video1Gop=60
r_Video1H264RcMode=0
r_Video1H265RcMode=2
r_Video1Metadata=0
r_Video1VPECropx=0
r_Video1VPECropy=0
r_Video1VPECropw=0
r_Video1VPECroph=0
r_Video2Format=0
r_Video2EncFrameRate=30
r_Video2Input=1
r_Video2InputChn=0
r_Video2InputPort=2
r_Video2VencChn=2
r_Video2Width=384
r_Video2Height=216
r_Video2Bitrate=2097152
r_Video2Profile=1
r_Video2Minqp=0
r_Video2Maxqp=51
r_Video2Gop=60
r_Video2H264RcMode=0
r_Video2H265RcMode=2
r_Video2Metadata=0
r_Video2VPECropx=0
r_Video2VPECropy=0
r_Video2VPECropw=0
r_Video2VPECroph=0
r_alsName=0
r_AlsDayThr=0
r_AlsNightThr=0
r_IRCut=0
r_IRLed=0
r_ExtDacName=0
r_ExtDacGain=0
*/

/////////////////////////////////////////////////////////

static s32 LoadMiscPartition(void)
{
    CamFsRet_e eRet = CAM_FS_OK;
    CamFsFd tFD;
    s32 s32FileLen = 0;

    if(0 == g_u8uLoadMiscPartitionBufFilled)
    {
        eRet = CamFsOpen(&tFD, PRELOAD_SETTING_TXT_PATH, CAM_FS_O_RDONLY, 0777);
        if(CAM_FS_OK == eRet)
        {
            s32FileLen = CamFsSeek(tFD, 0, CAM_FS_SEEK_END);
            if((s32FileLen != -1) && (CamFsSeek(tFD, 0, CAM_FS_SEEK_SET) != -1) )
            {
                if(PRELOAD_SETTING_TXT_READ_SIZE >= s32FileLen)
                {
                    memset(g_au8LoadMiscPartitionBuf, 0, PRELOAD_SETTING_TXT_READ_SIZE);
                    CamFsRead(tFD, g_au8LoadMiscPartitionBuf, s32FileLen);
                    eRet = CAM_FS_OK;
                    CamSetting_LOGINFO("open:%s ok,filesize:%d \n",PRELOAD_SETTING_TXT_PATH, s32FileLen);
                }
                else
                {
                    eRet = CAM_FS_FAIL;
                    CamSetting_LOGERR("%s open:%s ok,but filesize[%d]>3M,fail \n",__FUNCTION__, PRELOAD_SETTING_TXT_PATH, s32FileLen);
                }
            }
            else
            {
                eRet = CAM_FS_FAIL;
                CamSetting_LOGERR("%s seek file[%s] fail \n",__FUNCTION__, PRELOAD_SETTING_TXT_PATH);
            }
            CamFsClose(tFD);
        }
        else
        {
            eRet = CAM_FS_FAIL;
            CamSetting_LOGERR("%s open %s fail \n",__FUNCTION__, PRELOAD_SETTING_TXT_PATH);
        }
    }

    g_u8uLoadMiscPartitionBufFilled = 1;
    return eRet;
}

static char* FindCharPtr(char *pVar)
{
    char *pCh = NULL;
    char *pStr = NULL;
    u32 u32SkipLen = 0;

    if(pPartitionBuf != NULL)
    {
        pStr = pPartitionBuf;
    }
    else
    {
        pStr = (char *)g_au8LoadMiscPartitionBuf;
    }
    u32SkipLen = strlen(pVar) + 1;

    if((pCh = strstr(pStr, pVar)) != NULL)
    {
        pCh += u32SkipLen;
        pPartitionBuf = pCh;
    }
    else
    {
        pStr = (char *)g_au8LoadMiscPartitionBuf;
        u32SkipLen = strlen(pVar) + 1;

        if((pCh = strstr(pStr, pVar)) != NULL)
        {
            pCh += u32SkipLen;
            pPartitionBuf = pCh;
        }
        else
        {
            CamSetting_LOGERR("%s not find %s\n",__FUNCTION__, pVar);
        }
    }

    return pCh;
}

static s32 GetMiscPartitionU32(char *pVar, u32 *pValue)
{
    char *pCh = NULL;
    char aChartemp[80] = {0};

    if(!pVar || !pValue)
    {
        CamSetting_LOGERR("%s input pVar and pValue ng %s\n",__FUNCTION__);
        return -1;
    }

    if(LoadMiscPartition() != 0)
    {
        CamSetting_LOGERR("%s LoadMiscPartition ng \n",__FUNCTION__);
        return -1;
    }

    pCh = FindCharPtr(pVar);

    if (NULL != pCh)
    {
        sscanf(pCh, "%s", aChartemp);
        sscanf(aChartemp, "%u", pValue);
        //CamSetting_LOGINFO("%s:[%s][%d][%s]:0x%x\n",__FUNCTION__, pVar, strlen(aChartemp), aChartemp, *pValue);
    }
    else
    {
        CamSetting_LOGERR("%s not find %s\n",__FUNCTION__, pVar);
        return -1;
    }

    return 0;
}

static s32 GetCameraSettingValue(char *pVar, u32 *pValue)
{
#if defined(CONFIG_LOAD_CAMERA_SETTING_FROM_BOOTENV)
    if(env_get_u32(pVar, pValue) == -1)
    {
        //not found env, retry from misc
        return GetMiscPartitionU32(pVar, pValue);
    }
    return 0;
#else
    return GetMiscPartitionU32(pVar, pValue);
#endif
}

/*
r_USBPwrSrc=0
r_USBHiber=1
r_USB2Mode=0
r_SnrPowerGpio 8
r_VbusGpio 11
r_ISOCPostEn 0
r_RemoteWakeup 0
*/

static u8 g_u8UsbParserDn = 0;

UsbBootCust_t gUsbBootSetting =
{
    1,                //u8ISOCPostEn; //0: ISOC post mechanism disable. 1: ISOC post mechanism enable.
    0,                //u16USBPwrSrc; //0: USB3_BUS_POWER 1: USB3_SELF_POWER
    2,                //u16USBHiber; //0: Disable USB3 hibernation. 1: Enable USB3 hibernation. 2: Enable USB3 hibernation without system suspend.
    0,                //u16USB2Mode; //0: USB3.0 mode. 1: Force USB2.0 mode.
    0,                //u16RemoteWakeupEn; //0: Disable remote wakeup. 1: Enable remote wakeup.
#if defined(CONFIG_USB_GADGET_SUPPORT)
    (u16)(~0U),       //u16VbusGpio; // default: Max. value of u16
#endif
    0,                //0:TxCurrentTabIdx
    0x07,             //0x07:Ibias x1523_26[B3:B1]  /* dem current, default 3'b111 as index 3 */
    0x2E,             //0x2E:Idrv  x1523_40[B11:B6] /* driver current, default 6'b101110 as index 3 */
    0x13,             //0x13:Idem  x1523_41[B1:B0] x1523_40[B15:B12] /* dem current, default 6'b010011 as index 3 */
};

UsbBootCust_t* UsbBootSettingGetHandle(void)
{
    u32 u32GetSettingVolTemp = 0;

    if (g_u8UsbParserDn)
    {
        return &gUsbBootSetting;
    }

    if(GetCameraSettingValue("r_USBPwrSrc",&u32GetSettingVolTemp) == 0)
    {
        gUsbBootSetting.u16USBPwrSrc = u32GetSettingVolTemp;
        CamSetting_LOGINFO("get r_USBPwrSrc =%d \n", gUsbBootSetting.u16USBPwrSrc);
    }
    if(GetCameraSettingValue("r_USBHiber",&u32GetSettingVolTemp) == 0)
    {
        gUsbBootSetting.u16USBHiber = u32GetSettingVolTemp;
        CamSetting_LOGINFO("get r_USBHiber =%d \n", gUsbBootSetting.u16USBHiber);
    }
    if(GetCameraSettingValue("r_USB2Mode",&u32GetSettingVolTemp) == 0)
    {
        gUsbBootSetting.u16USB2Mode = u32GetSettingVolTemp;
        CamSetting_LOGINFO("get r_USB2Mode =%d \n", gUsbBootSetting.u16USB2Mode);
    }
    if(GetCameraSettingValue("r_RemoteWakeup",&u32GetSettingVolTemp) == 0)
    {
        gUsbBootSetting.u16RemoteWakeupEn = u32GetSettingVolTemp;
        CamSetting_LOGINFO("get r_RemoteWakeup =%d \n", gUsbBootSetting.u16RemoteWakeupEn);
    }
//#if defined(CONFIG_USB_GADGET_SUPPORT)
//    if(GetCameraSettingValue("r_VbusGpio",&u32GetSettingVolTemp) == 0)
//    {
//        gUsbBootSetting.u16VbusGpio = u32GetSettingVolTemp;
//        CamSetting_LOGINFO("get r_VbusGpio =%d \n", gUsbBootSetting.u16VbusGpio);
//    }
//#endif
    if (GetCameraSettingValue("r_ISOCPostEn",&u32GetSettingVolTemp) == 0) {
        gUsbBootSetting.u8ISOCPostEn = u32GetSettingVolTemp;
        CamSetting_LOGINFO("get r_ISOCPostEn =%d \n", gUsbBootSetting.u8ISOCPostEn);
    }
    if (GetCameraSettingValue("r_TxCurIdx",&u32GetSettingVolTemp) == 0) {
        gUsbBootSetting.u8TxCurrentIdx = u32GetSettingVolTemp;
        CamSetting_LOGINFO("get r_TxCurIdx =%d \n", gUsbBootSetting.u8TxCurrentIdx);
    }
    if (GetCameraSettingValue("r_VcmCur",&u32GetSettingVolTemp) == 0) {
        gUsbBootSetting.u8VcmCur = u32GetSettingVolTemp;
        CamSetting_LOGINFO("get r_VcmCur =%d \n", gUsbBootSetting.u8VcmCur);

    }
    if (GetCameraSettingValue("r_DrvCur",&u32GetSettingVolTemp) == 0) {
        gUsbBootSetting.u8DrvCur = u32GetSettingVolTemp;
        CamSetting_LOGINFO("get r_DrvCur =%d \n", gUsbBootSetting.u8DrvCur);
    }
    if (GetCameraSettingValue("r_DemCur",&u32GetSettingVolTemp) == 0) {
        gUsbBootSetting.u8DemCur = u32GetSettingVolTemp;
        CamSetting_LOGINFO("get r_DemCur =%d \n", gUsbBootSetting.u8DemCur);
    }

    g_u8UsbParserDn = 1;

    return &gUsbBootSetting;
}

//ENV>litesfs>hard code
CameraBootSetting_t* CameraBootSettingGetHandle(void)
{
    u32 u32GetSettingVolTemp = 0;

    if(0 == g_u8EnableLoadCamSetting)
    {
        //BootTimestampRecord(__LINE__, "cambootset.LoadCameraSetting.start");
        CamSetting_LOGINFO("before read from %s:CameraBootSetting:(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)",
                            PRELOAD_SETTING_FROM,
                            gCameraBootSetting.u8SensorFrameRate,
                            gCameraBootSetting.u32Lux,
                            gCameraBootSetting.u8DayNightMode,
                            gCameraBootSetting.u8AntiFlicker,
                            gCameraBootSetting.u32shutter,
                            gCameraBootSetting.u32SensorGain,
                            gCameraBootSetting.u32DigitalGain,
                            gCameraBootSetting.u16AWBRGain,
                            gCameraBootSetting.u16AWBGGain,
                            gCameraBootSetting.u16AWBBGain,
                            gCameraBootSetting.u8Orientation,
                            gCameraBootSetting.u32ExposureLevel,
                            gCameraBootSetting.u8HDRMode,
                            gCameraBootSetting.u8SensorNum,
                            gCameraBootSetting.stSystemTime.nSec,
                            gCameraBootSetting.stSystemTime.nNanoSec,
                            gCameraBootSetting.u8RegionsCun,
                            gCameraBootSetting.u16Regions_x[0],
                            gCameraBootSetting.u16Regions_x[1],
                            gCameraBootSetting.u16Regions_y[0],
                            gCameraBootSetting.u16Regions_y[1],
                            gCameraBootSetting.u16Regions_w[0],
                            gCameraBootSetting.u16Regions_w[1],
                            gCameraBootSetting.u16Regions_h[0],
                            gCameraBootSetting.u16Regions_h[1],
                            gCameraBootSetting.u8IspClkIndex,
                            gCameraBootSetting.u8SclClkIndex,
                            gCameraBootSetting.u32DivpClk,
                            gCameraBootSetting.u32VencClk,
                            gCameraBootSetting.u32Venc2ndpuClk,
                            gCameraBootSetting.u32VencMhalMaxWidth,
                            gCameraBootSetting.u32VencMhalMaxHeight,
                            gCameraBootSetting.u8enableUserYUV,
                            gCameraBootSetting.u32PreloadYUVFrameRate,
                            gCameraBootSetting.u32PreloadYUVFrameDepth,
                            gCameraBootSetting.u32PreloadVideoFrame,
                            gCameraBootSetting.u32PreloadAudioFrame,
                            gCameraBootSetting.u8enableIPU,
                            gCameraBootSetting.u32IPUClk,
                            gCameraBootSetting.u32IPUFwHeapSize,
                            gCameraBootSetting.u8IPUDynamicSwitch,
                            gCameraBootSetting.u32IPUSensitivity,
                            gCameraBootSetting.u32IPUActiveAreaX,
                            gCameraBootSetting.u32IPUActiveAreaY,
                            gCameraBootSetting.u32IPUActiveAreaW,
                            gCameraBootSetting.u32IPUActiveAreaH,
                            gCameraBootSetting.u8enableVDF,
                            gCameraBootSetting.u8VDFWorkMode,
                            gCameraBootSetting.u8enableSED,
                            gCameraBootSetting.u8enableLDC,
                            gCameraBootSetting.u32JpeClk);
        CamSetting_LOGINFO("(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)",
                            gCameraBootSetting.u32AIDevId,
                            gCameraBootSetting.u32AISamplerate,
                            gCameraBootSetting.u32AIVol,
                            gCameraBootSetting.u32AODevId,
                            gCameraBootSetting.u32AOSamplerate,
                            gCameraBootSetting.u32AOVol,
                            gCameraBootSetting.u8VideoNum,
                            gCameraBootSetting.u8Video0Format,
                            gCameraBootSetting.u8Video0EncFrameRate,
                            gCameraBootSetting.u8Video0Input,
                            gCameraBootSetting.u32Video0InputChn,
                            gCameraBootSetting.u32Video0InputPort,
                            gCameraBootSetting.u8Video0VencChn,
                            gCameraBootSetting.u8Video0BindType,
                            gCameraBootSetting.u16Video0Width,
                            gCameraBootSetting.u16Video0Height,
                            gCameraBootSetting.u16Video0MaxWidth,
                            gCameraBootSetting.u16Video0MaxHeight,
                            gCameraBootSetting.u32Video0Bitrate,
                            gCameraBootSetting.u32Video0Profile,
                            gCameraBootSetting.u32Video0Minqp,
                            gCameraBootSetting.u32Video0Maxqp,
                            gCameraBootSetting.u16Video0Gop,
                            gCameraBootSetting.u8Video0H264RcMode,
                            gCameraBootSetting.u8Video0H265RcMode,
                            gCameraBootSetting.u8Video0Metadata,
                            gCameraBootSetting.u16Video0VPECropx,
                            gCameraBootSetting.u16Video0VPECropy,
                            gCameraBootSetting.u16Video0VPECropw,
                            gCameraBootSetting.u16Video0VPECroph,
                            gCameraBootSetting.u8Video1Format,
                            gCameraBootSetting.u8Video1EncFrameRate,
                            gCameraBootSetting.u8Video1Input,
                            gCameraBootSetting.u32Video1InputChn,
                            gCameraBootSetting.u32Video1InputPort,
                            gCameraBootSetting.u8Video1VencChn,
                            gCameraBootSetting.u16Video1Width,
                            gCameraBootSetting.u16Video1Height,
                            gCameraBootSetting.u32Video1Bitrate,
                            gCameraBootSetting.u32Video1Profile,
                            gCameraBootSetting.u32Video1Minqp,
                            gCameraBootSetting.u32Video1Maxqp,
                            gCameraBootSetting.u16Video1Gop,
                            gCameraBootSetting.u8Video1H264RcMode,
                            gCameraBootSetting.u8Video1H265RcMode,
                            gCameraBootSetting.u8Video1Metadata,
                            gCameraBootSetting.u16Video1VPECropx,
                            gCameraBootSetting.u16Video1VPECropy,
                            gCameraBootSetting.u16Video1VPECropw,
                            gCameraBootSetting.u16Video1VPECroph);
        CamSetting_LOGINFO("(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)\n",
                            gCameraBootSetting.u8Video2Format,
                            gCameraBootSetting.u8Video2EncFrameRate,
                            gCameraBootSetting.u8Video2Input,
                            gCameraBootSetting.u32Video2InputChn,
                            gCameraBootSetting.u32Video2InputPort,
                            gCameraBootSetting.u8Video2VencChn,
                            gCameraBootSetting.u16Video2Width,
                            gCameraBootSetting.u16Video2Height,
                            gCameraBootSetting.u32Video2Bitrate,
                            gCameraBootSetting.u32Video2Profile,
                            gCameraBootSetting.u32Video2Minqp,
                            gCameraBootSetting.u32Video2Maxqp,
                            gCameraBootSetting.u16Video2Gop,
                            gCameraBootSetting.u8Video2H264RcMode,
                            gCameraBootSetting.u8Video2H265RcMode,
                            gCameraBootSetting.u8Video2Metadata,
                            gCameraBootSetting.u16Video2VPECropx,
                            gCameraBootSetting.u16Video2VPECropy,
                            gCameraBootSetting.u16Video2VPECropw,
                            gCameraBootSetting.u16Video2VPECroph,
                            gCameraBootSetting.u16alsName,
                            gCameraBootSetting.u16AlsDayThr,
                            gCameraBootSetting.u16AlsNightThr,
                            gCameraBootSetting.u16IRCut,
                            gCameraBootSetting.u16IRLed,
                            gCameraBootSetting.u16ExtDacName,
                            gCameraBootSetting.u32ExtDacGain);
        if(GetCameraSettingValue("r_SensorFrameRate",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8SensorFrameRate = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_SensorFrameRate =%d \n",gCameraBootSetting.u8SensorFrameRate);
        }
        if(GetCameraSettingValue("r_Lux",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Lux = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Lux =%d \n",gCameraBootSetting.u32Lux);
        }
        if(GetCameraSettingValue("r_DayNightMode",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8DayNightMode = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_DayNightMode =%d \n",gCameraBootSetting.u8DayNightMode);
        }
        if(GetCameraSettingValue("r_AntiFlicker",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8AntiFlicker = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AntiFlicker =%d \n",gCameraBootSetting.u8AntiFlicker);
        }
        if(GetCameraSettingValue("r_shutter",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32shutter = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_shutter =%d \n",gCameraBootSetting.u32shutter);
        }
        if(GetCameraSettingValue("r_SensorGain",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32SensorGain = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_SensorGain =%d \n",gCameraBootSetting.u32SensorGain);
        }
        if(GetCameraSettingValue("r_DigitalGain",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32DigitalGain = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_DigitalGain =%d \n",gCameraBootSetting.u32DigitalGain);
        }
        if(GetCameraSettingValue("r_AWBRGain",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16AWBRGain = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AWBRGain =%d \n",gCameraBootSetting.u16AWBRGain);
        }
        if(GetCameraSettingValue("r_AWBGGain",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16AWBGGain = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AWBGGain =%d \n",gCameraBootSetting.u16AWBGGain);
        }
        if(GetCameraSettingValue("r_AWBBGain",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16AWBBGain = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AWBBGain =%d \n",gCameraBootSetting.u16AWBBGain);
        }
        if(GetCameraSettingValue("r_Orientation",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Orientation = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Orientation =%d \n",gCameraBootSetting.u8Orientation);
        }
        if(GetCameraSettingValue("r_ExposureLevel",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32ExposureLevel = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_ExposureLevel =%d \n",gCameraBootSetting.u32ExposureLevel);
        }
        if(GetCameraSettingValue("r_HDRMode",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8HDRMode = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_HDRMode =%d \n",gCameraBootSetting.u8HDRMode);
        }
        if(GetCameraSettingValue("r_SensorNum",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8SensorNum = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_SensorNum =%d \n",gCameraBootSetting.u8SensorNum);
        }
        if(GetCameraSettingValue("r_stSystemTime.nSec",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.stSystemTime.nSec = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_stSystemTime.nSec =%d \n",gCameraBootSetting.stSystemTime.nSec);
        }
        if(GetCameraSettingValue("r_stSystemTime.nNanoSec",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.stSystemTime.nNanoSec = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_stSystemTime.nNanoSec =%d \n",gCameraBootSetting.stSystemTime.nNanoSec);
        }
        if(GetCameraSettingValue("r_RegionsCun",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8RegionsCun = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_RegionsCun =%d \n",gCameraBootSetting.u8RegionsCun);
        }
        if(GetCameraSettingValue("r_Regions_x0",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Regions_x[0] = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Regions_x0 =%d \n",gCameraBootSetting.u16Regions_x[0]);
        }
        if(GetCameraSettingValue("r_Regions_x1",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Regions_x[1] = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Regions_x1 =%d \n",gCameraBootSetting.u16Regions_x[1]);
        }
        if(GetCameraSettingValue("r_Regions_y0",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Regions_y[0] = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Regions_y0 =%d \n",gCameraBootSetting.u16Regions_y[0]);
        }
        if(GetCameraSettingValue("r_Regions_y1",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Regions_y[1] = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Regions_y1 =%d \n",gCameraBootSetting.u16Regions_y[1]);
        }
        if(GetCameraSettingValue("r_Regions_w0",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Regions_w[0] = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Regions_w0 =%d \n",gCameraBootSetting.u16Regions_w[0]);
        }
        if(GetCameraSettingValue("r_Regions_w1",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Regions_w[1] = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Regions_w1 =%d \n",gCameraBootSetting.u16Regions_w[1]);
        }
        if(GetCameraSettingValue("r_Regions_h0",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Regions_h[0] = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Regions_h0 =%d \n",gCameraBootSetting.u16Regions_h[0]);
        }
        if(GetCameraSettingValue("r_Regions_h1",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Regions_h[1] = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Regions_h1 =%d \n",gCameraBootSetting.u16Regions_h[1]);
        }
        if(GetCameraSettingValue("r_IspClkIndex",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8IspClkIndex = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_IspClkIndex =%d \n",gCameraBootSetting.u8IspClkIndex);
        }
        if(GetCameraSettingValue("r_SclClkIndex",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8SclClkIndex = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_SclClkIndex =%d \n",gCameraBootSetting.u8SclClkIndex);
        }
        if(GetCameraSettingValue("r_DivpClk",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32DivpClk = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_DivpClk =%d \n",gCameraBootSetting.u32DivpClk);
        }
        if(GetCameraSettingValue("r_VencClk",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32VencClk = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_VencClk =%d \n",gCameraBootSetting.u32VencClk);
        }
        if(GetCameraSettingValue("r_Venc2ndpuClk",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Venc2ndpuClk = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Venc2ndpuClk =%d \n",gCameraBootSetting.u32Venc2ndpuClk);
        }
        if(GetCameraSettingValue("r_VencMhalMaxWidth",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32VencMhalMaxWidth = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_VencMhalMaxWidth =%d \n",gCameraBootSetting.u32VencMhalMaxWidth);
        }
        if(GetCameraSettingValue("r_VencMhalMaxHeight",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32VencMhalMaxHeight = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_VencMhalMaxHeight =%d \n",gCameraBootSetting.u32VencMhalMaxHeight);
        }
        if(GetCameraSettingValue("r_enableUserYUV",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8enableUserYUV = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_enableUserYUV =%d \n",gCameraBootSetting.u8enableUserYUV);
        }
        if(GetCameraSettingValue("r_PreloadYUVFrameRate",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32PreloadYUVFrameRate = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_PreloadYUVFrameRate =%d \n",gCameraBootSetting.u32PreloadYUVFrameRate);
        }
        if(GetCameraSettingValue("r_PreloadYUVFrameDepth",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32PreloadYUVFrameDepth = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_PreloadYUVFrameDepth =%d \n",gCameraBootSetting.u32PreloadYUVFrameDepth);
        }
        if(GetCameraSettingValue("r_PreloadVideoFrame",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32PreloadVideoFrame = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_PreloadVideoFrame =%d \n",gCameraBootSetting.u32PreloadVideoFrame);
        }
        if(GetCameraSettingValue("r_PreloadAudioFrame",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32PreloadAudioFrame = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_PreloadAudioFrame =%d \n",gCameraBootSetting.u32PreloadAudioFrame);
        }
        if(GetCameraSettingValue("r_enableIPU",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8enableIPU = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_enableIPU =%d \n",gCameraBootSetting.u8enableIPU);
        }
        if(GetCameraSettingValue("r_IPUClk",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32IPUClk = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_IPUClk =%d \n",gCameraBootSetting.u32IPUClk);
        }
        if(GetCameraSettingValue("r_IPUFwHeapSize",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32IPUFwHeapSize = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_IPUFwHeapSize =%d \n",gCameraBootSetting.u32IPUFwHeapSize);
        }
        if(GetCameraSettingValue("r_IPUDynamicSwitch",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8IPUDynamicSwitch = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_IPUDynamicSwitch =%d \n",gCameraBootSetting.u8IPUDynamicSwitch);
        }
        if(GetCameraSettingValue("r_IPUSensitivity",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32IPUSensitivity = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_IPUSensitivity =%d \n",gCameraBootSetting.u32IPUSensitivity);
        }
        if(GetCameraSettingValue("r_IPUActiveAreaX",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32IPUActiveAreaX = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_IPUActiveAreaX =%d \n",gCameraBootSetting.u32IPUActiveAreaX);
        }
        if(GetCameraSettingValue("r_IPUActiveAreaY",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32IPUActiveAreaY = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_IPUActiveAreaY =%d \n",gCameraBootSetting.u32IPUActiveAreaY);
        }
        if(GetCameraSettingValue("r_IPUActiveAreaW",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32IPUActiveAreaW = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_IPUActiveAreaW =%d \n",gCameraBootSetting.u32IPUActiveAreaW);
        }
        if(GetCameraSettingValue("r_IPUActiveAreaH",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32IPUActiveAreaH = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_IPUActiveAreaH =%d \n",gCameraBootSetting.u32IPUActiveAreaH);
        }
        if(GetCameraSettingValue("r_enableVDF",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8enableVDF = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_enableVDF =%d \n",gCameraBootSetting.u8enableVDF);
        }
        if(GetCameraSettingValue("r_VDFWorkMode",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8VDFWorkMode = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_VDFWorkMode =%d \n",gCameraBootSetting.u8VDFWorkMode);
        }
        if(GetCameraSettingValue("r_enableSED",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8enableSED = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_enableSED =%d \n",gCameraBootSetting.u8enableSED);
        }
        if(GetCameraSettingValue("r_enableLDC",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8enableLDC = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_enableLDC from MISC=%d \n",gCameraBootSetting.u8enableLDC);
        }
        if(GetCameraSettingValue("r_JpeClk",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32JpeClk = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_JpeClk =%d \n",gCameraBootSetting.u32JpeClk);
        }
        if(GetCameraSettingValue("r_AIDevId",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32AIDevId = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AIDevId =%d \n",gCameraBootSetting.u32AIDevId);
        }
        if(GetCameraSettingValue("r_AISamplerate",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32AISamplerate = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AISamplerate =%d \n",gCameraBootSetting.u32AISamplerate);
        }
        if(GetCameraSettingValue("r_AIVol",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32AIVol = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AIVol =%d \n",gCameraBootSetting.u32AIVol);
        }
        if(GetCameraSettingValue("r_AODevId",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32AODevId = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AODevId =%d \n",gCameraBootSetting.u32AODevId);
        }
        if(GetCameraSettingValue("r_AOBitwidth",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32AOBitwidth = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AOBitwidth =%d \n",gCameraBootSetting.u32AODevId);
        }
        if(GetCameraSettingValue("r_AOWorkmode",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32AOWorkmode = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AOWorkmode =%d \n",gCameraBootSetting.u32AODevId);
        }
        if(GetCameraSettingValue("r_AOSamplerate",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32AOSamplerate = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AOSamplerate =%d \n",gCameraBootSetting.u32AOSamplerate);
        }
        if(GetCameraSettingValue("r_AOVol",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32AOVol = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AOVol =%d \n",gCameraBootSetting.u32AOVol);
        }
        if(GetCameraSettingValue("r_VideoNum",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8VideoNum = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_VideoNum =%d \n",gCameraBootSetting.u8VideoNum);
        }
        if(GetCameraSettingValue("r_Video0Format",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video0Format = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0Format =%d \n",gCameraBootSetting.u8Video0Format);
        }
        if(GetCameraSettingValue("r_Video0EncFrameRate",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video0EncFrameRate = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0EncFrameRate =%d \n",gCameraBootSetting.u8Video0EncFrameRate);
        }
        if(GetCameraSettingValue("r_Video0Input",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video0Input = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0Input =%d \n",gCameraBootSetting.u8Video0Input);
        }
        if(GetCameraSettingValue("r_Video0InputChn",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video0InputChn = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0InputChn =%d \n",gCameraBootSetting.u32Video0InputChn);
        }
        if(GetCameraSettingValue("r_Video0InputPort",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video0InputPort = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0InputPort =%d \n",gCameraBootSetting.u32Video0InputPort);
        }
        if(GetCameraSettingValue("r_Video0VencChn",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video0VencChn = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0VencChn =%d \n",gCameraBootSetting.u8Video0VencChn);
        }
        if(GetCameraSettingValue("r_Video0BindType",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video0BindType = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0BindType =%d \n",gCameraBootSetting.u8Video0BindType);
        }
        if(GetCameraSettingValue("r_Video0Width",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video0Width = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0Width =%d \n",gCameraBootSetting.u16Video0Width);
        }
        if(GetCameraSettingValue("r_Video0Height",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video0Height = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0Height =%d \n",gCameraBootSetting.u16Video0Height);
        }
        if(GetCameraSettingValue("r_Video0MaxWidth",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video0MaxWidth = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0MaxWidth =%d \n",gCameraBootSetting.u16Video0MaxWidth);
        }
        if(GetCameraSettingValue("r_Video0MaxHeight",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video0MaxHeight = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0MaxHeight =%d \n",gCameraBootSetting.u16Video0MaxHeight);
        }
        if(GetCameraSettingValue("r_Video0Bitrate",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video0Bitrate = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0Bitrate =%d \n",gCameraBootSetting.u32Video0Bitrate);
        }
        if(GetCameraSettingValue("r_Video0Profile",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video0Profile = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0Profile =%d \n",gCameraBootSetting.u32Video0Profile);
        }
        if(GetCameraSettingValue("r_Video0Minqp",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video0Minqp = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0Minqp =%d \n",gCameraBootSetting.u32Video0Minqp);
        }
        if(GetCameraSettingValue("r_Video0Maxqp",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video0Maxqp = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0Maxqp =%d \n",gCameraBootSetting.u32Video0Maxqp);
        }
        if(GetCameraSettingValue("r_Video0Gop",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video0Gop = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0Gop =%d \n",gCameraBootSetting.u16Video0Gop);
        }
        if(GetCameraSettingValue("r_Video0H264RcMode",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video0H264RcMode = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0H264RcMode =%d \n",gCameraBootSetting.u8Video0H264RcMode);
        }
        if(GetCameraSettingValue("r_Video0H265RcMode",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video0H265RcMode = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0H265RcMode =%d \n",gCameraBootSetting.u8Video0H265RcMode);
        }
        if(GetCameraSettingValue("r_Video0Metadata",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video0Metadata = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0Metadata =%d \n",gCameraBootSetting.u8Video0Metadata);
        }
        if(GetCameraSettingValue("r_Video0VPECropx",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video0VPECropx = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0VPECropx =%d \n",gCameraBootSetting.u16Video0VPECropx);
        }
        if(GetCameraSettingValue("r_Video0VPECropy",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video0VPECropy = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0VPECropy =%d \n",gCameraBootSetting.u16Video0VPECropy);
        }
        if(GetCameraSettingValue("r_Video0VPECropw",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video0VPECropw = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0VPECropw =%d \n",gCameraBootSetting.u16Video0VPECropw);
        }
        if(GetCameraSettingValue("r_Video0VPECroph",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video0VPECroph = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video0VPECroph =%d \n",gCameraBootSetting.u16Video0VPECroph);
        }
        if(GetCameraSettingValue("r_Video1Format",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video1Format = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1Format =%d \n",gCameraBootSetting.u8Video1Format);
        }
        if(GetCameraSettingValue("r_Video1EncFrameRate",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video1EncFrameRate = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1EncFrameRate =%d \n",gCameraBootSetting.u8Video1EncFrameRate);
        }
        if(GetCameraSettingValue("r_Video1Input",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video1Input = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1Input =%d \n",gCameraBootSetting.u8Video1Input);
        }
        if(GetCameraSettingValue("r_Video1InputChn",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video1InputChn = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1InputChn =%d \n",gCameraBootSetting.u32Video1InputChn);
        }
        if(GetCameraSettingValue("r_Video1InputPort",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video1InputPort = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1InputPort =%d \n",gCameraBootSetting.u32Video1InputPort);
        }
        if(GetCameraSettingValue("r_Video1VencChn",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video1VencChn = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1VencChn =%d \n",gCameraBootSetting.u8Video1VencChn);
        }
        if(GetCameraSettingValue("r_Video1Width",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video1Width = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1Width =%d \n",gCameraBootSetting.u16Video1Width);
        }
        if(GetCameraSettingValue("r_Video1Height",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video1Height = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1Height =%d \n",gCameraBootSetting.u16Video1Height);
        }
        if(GetCameraSettingValue("r_Video1Bitrate",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video1Bitrate = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1Bitrate =%d \n",gCameraBootSetting.u32Video1Bitrate);
        }
        if(GetCameraSettingValue("r_Video1Profile",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video1Profile = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1Profile =%d \n",gCameraBootSetting.u32Video1Profile);
        }
        if(GetCameraSettingValue("r_Video1Minqp",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video1Minqp = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1Minqp =%d \n",gCameraBootSetting.u32Video1Minqp);
        }
        if(GetCameraSettingValue("r_Video1Maxqp",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video1Maxqp = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1Maxqp =%d \n",gCameraBootSetting.u32Video1Maxqp);
        }
        if(GetCameraSettingValue("r_Video1Gop",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video1Gop = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1Gop =%d \n",gCameraBootSetting.u16Video1Gop);
        }
        if(GetCameraSettingValue("r_Video1H264RcMode",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video1H264RcMode = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1H264RcMode =%d \n",gCameraBootSetting.u8Video1H264RcMode);
        }
        if(GetCameraSettingValue("r_Video1H265RcMode",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video1H265RcMode = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1H265RcMode =%d \n",gCameraBootSetting.u8Video1H265RcMode);
        }
        if(GetCameraSettingValue("r_Video1Metadata",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video1Metadata = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1Metadata =%d \n",gCameraBootSetting.u8Video1Metadata);
        }
        if(GetCameraSettingValue("r_Video1VPECropx",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video1VPECropx = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1VPECropx =%d \n",gCameraBootSetting.u16Video1VPECropx);
        }
        if(GetCameraSettingValue("r_Video1VPECropy",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video1VPECropy = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1VPECropy =%d \n",gCameraBootSetting.u16Video1VPECropy);
        }
        if(GetCameraSettingValue("r_Video1VPECropw",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video1VPECropw = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1VPECropw =%d \n",gCameraBootSetting.u16Video1VPECropw);
        }
        if(GetCameraSettingValue("r_Video1VPECroph",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video1VPECroph = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video1VPECroph =%d \n",gCameraBootSetting.u16Video1VPECroph);
        }
        if(GetCameraSettingValue("r_Video2Format",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video2Format = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2Format =%d \n",gCameraBootSetting.u8Video2Format);
        }
        if(GetCameraSettingValue("r_Video2EncFrameRate",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video2EncFrameRate = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2EncFrameRate =%d \n",gCameraBootSetting.u8Video2EncFrameRate);
        }
        if(GetCameraSettingValue("r_Video2Input",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video2Input = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2Input =%d \n",gCameraBootSetting.u8Video2Input);
        }
        if(GetCameraSettingValue("r_Video2InputChn",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video2InputChn = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2InputChn =%d \n",gCameraBootSetting.u32Video2InputChn);
        }
        if(GetCameraSettingValue("r_Video2InputPort",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video2InputPort = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2InputPort =%d \n",gCameraBootSetting.u32Video2InputPort);
        }
        if(GetCameraSettingValue("r_Video2VencChn",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video2VencChn = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2VencChn =%d \n",gCameraBootSetting.u8Video2VencChn);
        }
        if(GetCameraSettingValue("r_Video2Width",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video2Width = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2Width =%d \n",gCameraBootSetting.u16Video2Width);
        }
        if(GetCameraSettingValue("r_Video2Height",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video2Height = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2Height =%d \n",gCameraBootSetting.u16Video2Height);
        }
        if(GetCameraSettingValue("r_Video2Bitrate",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video2Bitrate = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2Bitrate =%d \n",gCameraBootSetting.u32Video2Bitrate);
        }
        if(GetCameraSettingValue("r_Video2Profile",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video2Profile = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2Profile =%d \n",gCameraBootSetting.u32Video2Profile);
        }
        if(GetCameraSettingValue("r_Video2Minqp",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video2Minqp = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2Minqp =%d \n",gCameraBootSetting.u32Video2Minqp);
        }
        if(GetCameraSettingValue("r_Video2Maxqp",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32Video2Maxqp = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2Maxqp =%d \n",gCameraBootSetting.u32Video2Maxqp);
        }
        if(GetCameraSettingValue("r_Video2Gop",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video2Gop = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2Gop =%d \n",gCameraBootSetting.u16Video2Gop);
        }
        if(GetCameraSettingValue("r_Video2H264RcMode",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video2H264RcMode = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2H264RcMode =%d \n",gCameraBootSetting.u8Video2H264RcMode);
        }
        if(GetCameraSettingValue("r_Video2H265RcMode",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video2H265RcMode = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2H265RcMode =%d \n",gCameraBootSetting.u8Video2H265RcMode);
        }
        if(GetCameraSettingValue("r_Video2Metadata",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u8Video2Metadata = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2Metadata =%d \n",gCameraBootSetting.u8Video2Metadata);
        }
        if(GetCameraSettingValue("r_Video2VPECropx",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video2VPECropx = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2VPECropx =%d \n",gCameraBootSetting.u16Video2VPECropx);
        }
        if(GetCameraSettingValue("r_Video2VPECropy",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video2VPECropy = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2VPECropy =%d \n",gCameraBootSetting.u16Video2VPECropy);
        }
        if(GetCameraSettingValue("r_Video2VPECropw",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video2VPECropw = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2VPECropw =%d \n",gCameraBootSetting.u16Video2VPECropw);
        }
        if(GetCameraSettingValue("r_Video2VPECroph",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16Video2VPECroph = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_Video2VPECroph =%d \n",gCameraBootSetting.u16Video2VPECroph);
        }
        if(GetCameraSettingValue("r_alsName",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16alsName = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_alsName =%d \n",gCameraBootSetting.u16alsName);
        }
        if(GetCameraSettingValue("r_AlsDayThr",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16AlsDayThr = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AlsDayThr =%d \n",gCameraBootSetting.u16AlsDayThr);
        }
        if(GetCameraSettingValue("r_AlsNightThr",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16AlsNightThr = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_AlsNightThr =%d \n",gCameraBootSetting.u16AlsNightThr);
        }
        if(GetCameraSettingValue("r_IRCut",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16IRCut = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_IRCut =%d \n",gCameraBootSetting.u16IRCut);
        }
        if(GetCameraSettingValue("r_IRLed",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16IRLed = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_IRLed =%d \n",gCameraBootSetting.u16IRLed);
        }
        if(GetCameraSettingValue("r_ExtDacName",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u16ExtDacName = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_ExtDacName =%d \n",gCameraBootSetting.u16ExtDacName);
        }
        if(GetCameraSettingValue("r_ExtDacGain",&u32GetSettingVolTemp) == 0)
        {
            gCameraBootSetting.u32ExtDacGain = u32GetSettingVolTemp;
            CamSetting_LOGINFO("get r_ExtDacGain =%d \n",gCameraBootSetting.u32ExtDacGain);
        }
        CamSetting_LOGINFO("after read from %s:CameraBootSetting:(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)",
                            PRELOAD_SETTING_FROM,
                            gCameraBootSetting.u8SensorFrameRate,
                            gCameraBootSetting.u32Lux,
                            gCameraBootSetting.u8DayNightMode,
                            gCameraBootSetting.u8AntiFlicker,
                            gCameraBootSetting.u32shutter,
                            gCameraBootSetting.u32SensorGain,
                            gCameraBootSetting.u32DigitalGain,
                            gCameraBootSetting.u16AWBRGain,
                            gCameraBootSetting.u16AWBGGain,
                            gCameraBootSetting.u16AWBBGain,
                            gCameraBootSetting.u8Orientation,
                            gCameraBootSetting.u32ExposureLevel,
                            gCameraBootSetting.u8HDRMode,
                            gCameraBootSetting.u8SensorNum,
                            gCameraBootSetting.u8PreloadVideo,
                            gCameraBootSetting.u8PreloadAI,
                            gCameraBootSetting.u8PreloadAO,
                            gCameraBootSetting.stSystemTime.nSec,
                            gCameraBootSetting.stSystemTime.nNanoSec,
                            gCameraBootSetting.u8RegionsCun,
                            gCameraBootSetting.u16Regions_x[0],
                            gCameraBootSetting.u16Regions_x[1],
                            gCameraBootSetting.u16Regions_y[0],
                            gCameraBootSetting.u16Regions_y[1],
                            gCameraBootSetting.u16Regions_w[0],
                            gCameraBootSetting.u16Regions_w[1],
                            gCameraBootSetting.u16Regions_h[0],
                            gCameraBootSetting.u16Regions_h[1],
                            gCameraBootSetting.u8IspClkIndex,
                            gCameraBootSetting.u8SclClkIndex,
                            gCameraBootSetting.u32DivpClk,
                            gCameraBootSetting.u32VencClk,
                            gCameraBootSetting.u32Venc2ndpuClk,
                            gCameraBootSetting.u32VencMhalMaxWidth,
                            gCameraBootSetting.u32VencMhalMaxHeight,
                            gCameraBootSetting.u8enableUserYUV,
                            gCameraBootSetting.u32PreloadYUVFrameRate,
                            gCameraBootSetting.u32PreloadYUVFrameDepth,
                            gCameraBootSetting.u32PreloadVideoFrame,
                            gCameraBootSetting.u32PreloadAudioFrame,
                            gCameraBootSetting.u8enableIPU,
                            gCameraBootSetting.u32IPUClk,
                            gCameraBootSetting.u32IPUFwHeapSize,
                            gCameraBootSetting.u8IPUDynamicSwitch,
                            gCameraBootSetting.u32IPUSensitivity,
                            gCameraBootSetting.u32IPUActiveAreaX,
                            gCameraBootSetting.u32IPUActiveAreaY,
                            gCameraBootSetting.u32IPUActiveAreaW,
                            gCameraBootSetting.u32IPUActiveAreaH,
                            gCameraBootSetting.u8enableVDF,
                            gCameraBootSetting.u8VDFWorkMode,
                            gCameraBootSetting.u8enableSED,
                            gCameraBootSetting.u8enableLDC,
                            gCameraBootSetting.u32JpeClk);
        CamSetting_LOGINFO("(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)",
                            gCameraBootSetting.u32AIDevId,
                            gCameraBootSetting.u32AISamplerate,
                            gCameraBootSetting.u32AIVol,
                            gCameraBootSetting.u32AODevId,
                            gCameraBootSetting.u32AOSamplerate,
                            gCameraBootSetting.u32AOVol,
                            gCameraBootSetting.u8VideoNum,
                            gCameraBootSetting.u8Video0Format,
                            gCameraBootSetting.u8Video0EncFrameRate,
                            gCameraBootSetting.u8Video0Input,
                            gCameraBootSetting.u32Video0InputChn,
                            gCameraBootSetting.u32Video0InputPort,
                            gCameraBootSetting.u8Video0VencChn,
                            gCameraBootSetting.u8Video0BindType,
                            gCameraBootSetting.u16Video0Width,
                            gCameraBootSetting.u16Video0Height,
                            gCameraBootSetting.u16Video0MaxWidth,
                            gCameraBootSetting.u16Video0MaxHeight,
                            gCameraBootSetting.u32Video0PreloadBufSize,
                            gCameraBootSetting.u32Video0Bitrate,
                            gCameraBootSetting.u32Video0Profile,
                            gCameraBootSetting.u32Video0Minqp,
                            gCameraBootSetting.u32Video0Maxqp,
                            gCameraBootSetting.u16Video0Gop,
                            gCameraBootSetting.u8Video0H264RcMode,
                            gCameraBootSetting.u8Video0H265RcMode,
                            gCameraBootSetting.u8Video0Metadata,
                            gCameraBootSetting.u16Video0VPECropx,
                            gCameraBootSetting.u16Video0VPECropy,
                            gCameraBootSetting.u16Video0VPECropw,
                            gCameraBootSetting.u16Video0VPECroph,
                            gCameraBootSetting.u8Video1Format,
                            gCameraBootSetting.u8Video1EncFrameRate,
                            gCameraBootSetting.u8Video1Input,
                            gCameraBootSetting.u32Video1InputChn,
                            gCameraBootSetting.u32Video1InputPort,
                            gCameraBootSetting.u8Video1VencChn,
                            gCameraBootSetting.u16Video1Width,
                            gCameraBootSetting.u16Video1Height,
                            gCameraBootSetting.u32Video1PreloadBufSize,
                            gCameraBootSetting.u32Video1Bitrate,
                            gCameraBootSetting.u32Video1Profile,
                            gCameraBootSetting.u32Video1Minqp,
                            gCameraBootSetting.u32Video1Maxqp,
                            gCameraBootSetting.u16Video1Gop,
                            gCameraBootSetting.u8Video1H264RcMode,
                            gCameraBootSetting.u8Video1H265RcMode,
                            gCameraBootSetting.u8Video1Metadata,
                            gCameraBootSetting.u16Video1VPECropx,
                            gCameraBootSetting.u16Video1VPECropy,
                            gCameraBootSetting.u16Video1VPECropw,
                            gCameraBootSetting.u16Video1VPECroph);
        CamSetting_LOGINFO("(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)\n",
                            gCameraBootSetting.u8Video2Format,
                            gCameraBootSetting.u8Video2EncFrameRate,
                            gCameraBootSetting.u8Video2Input,
                            gCameraBootSetting.u32Video2InputChn,
                            gCameraBootSetting.u32Video2InputPort,
                            gCameraBootSetting.u8Video2VencChn,
                            gCameraBootSetting.u16Video2Width,
                            gCameraBootSetting.u16Video2Height,
                            gCameraBootSetting.u32Video2PreloadBufSize,
                            gCameraBootSetting.u32Video2Bitrate,
                            gCameraBootSetting.u32Video2Profile,
                            gCameraBootSetting.u32Video2Minqp,
                            gCameraBootSetting.u32Video2Maxqp,
                            gCameraBootSetting.u16Video2Gop,
                            gCameraBootSetting.u8Video2H264RcMode,
                            gCameraBootSetting.u8Video2H265RcMode,
                            gCameraBootSetting.u8Video2Metadata,
                            gCameraBootSetting.u16Video2VPECropx,
                            gCameraBootSetting.u16Video2VPECropy,
                            gCameraBootSetting.u16Video2VPECropw,
                            gCameraBootSetting.u16Video2VPECroph,
                            gCameraBootSetting.u16alsName,
                            gCameraBootSetting.u16AlsDayThr,
                            gCameraBootSetting.u16AlsNightThr,
                            gCameraBootSetting.u16IRCut,
                            gCameraBootSetting.u16IRLed,
                            gCameraBootSetting.u16ExtDacName,
                            gCameraBootSetting.u32ExtDacGain,
                            );

        g_u8EnableLoadCamSetting = 1;
        //BootTimestampRecord(__LINE__, "cambootset.LoadCameraSetting.end");
    }
    return &gCameraBootSetting;
}

CameraBootSetting_t* CameraBootSettingGetHandle_HardCode(void)
{
    return &gCameraBootSetting;
}
