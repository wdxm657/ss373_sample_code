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
#ifndef _ST_COMMON_AOV_H_
#define _ST_COMMON_AOV_H_

#include "mi_common_datatype.h"
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#include "list.h"
#include "st_common.h"
#include "st_common_font.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_venc.h"
#include "st_common_rgn.h"
#include "st_common_dla_det.h"
#include "st_common_audio.h"
#include "st_common_vdf.h"
#include "st_common_rtsp_video.h"
#include "light_misc_control_datatype.h"
#include "fast_ae.h"

//#define _MCU_UART_

#define AOV_DEBUG 0
#if AOV_DEBUG
#define PRINT_AOV_DEBUG(...) printf(__VA_ARGS__)
#else
#define PRINT_AOV_DEBUG(...)
#endif

#define FPS_HIGH                    15
#define FPS_LOW                     1
#define MAX_FRAME_HANDLE            (16)
#define DET_THRESHOLD               0.7
#define DET_TARGETID                0
#define RTC_DEVICE_NAME             "/dev/rtc0"
#define WAKEUP_EVENT_FILE_NAME      "/sys/devices/virtual/sstar/rtcpwc/wakeup_event"
#define WAKEUP_EVENT_TIMER          "rtc_alarm"
#define WAKEUP_EVENT_PREVIEW        "rtc_io0"
#define WAKEUP_EVENT_PIR            "rtc_io1"
#define IS_DUALOS                   "/proc/dualos"
#define ALARM_DEVICE_NAME           "/sys/devices/virtual/sstar/rtcpwc/alarm_timer"
#define ALARM_THRESHOLD_DEVICE_NAME "/sys/devices/virtual/sstar/rtcpwc/alarm_threshold"
#define POWER_STATE                 "/sys/power/state"
#define PM_FREEZE_TIMEOUT           "/sys/power/pm_freeze_timeout"
#define PM_FREEZE_EXPECT_TIME       "300"
#define FAILED_FREEZE               "/sys/power/suspend_stats/failed_freeze"
#define RTCPWC_FOR_OS               "/sys/class/sstar/rtcpwc/save_in_sw"
#define GPIO_EXPORT_NAME            "/sys/class/gpio/export"
#define ALARM_THRESHOLD_VALUE       500
#define AOV_MAX_SENSOR_NUM          2
#define SENSOR_0                    0
#define SENSOR_1                    1
#define PIPE_VENC_GOP               30
#define VENC_STREAM_POOL_BUFFER     50
#define CMDID_SLEEP_MODE            (0x01)
#define AO_WRITE_SIZE               2048
#define DAYNIGHT_BV_THRESHOLD       -10000
#define DAYNIGHT_LUX_THRESHOLD      10
#define STABLEFRAMESBEFORETURNIR    2
#define STABLEFRAMESAFTERTURNIR     6
#define DELAY_CONFIRM_FRAME         10
#define BUF_POOL_MAX                60
#define LIGHT_SENSOR_MANUAL_AE_TIMES (0x3)
#define LIGHT_SENSOR_MANUAL_AE_DIFF_LUX (20)

#ifdef _MCU_UART_
#define SOC_UART_DEV_NAME           "/dev/ttyS1"
#define SOC_UART_BAUDRATE           9600
#define SOC_UART_DATA_BIT           8
#define SOC_UART_PARITY             'n'
#define SOC_UART_STOP               1
#define SOC_UART_CTSRTS             0

#endif

#define CHECK_AOV_RESULT(_func_, _ret_, _exit_label_)                                     \
    do                                                                                    \
    {                                                                                     \
        _ret_ = _func_;                                                                   \
        if (_ret_ != MI_SUCCESS)                                                          \
        {                                                                                 \
            printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, _ret_); \
            goto _exit_label_;                                                            \
        }                                                                                 \
    } while (0)

#define DIFF(a, b)             (((a) > (b)) ? (a-b) : (b-a))

#define CMDID_EARLYINIT_SHUTTER_GAIN (0x03)
#define EARLYINIT_PARAM_TYPE_MASTER  (0x1)
    typedef enum{
        eUartPowerHigh = 0,
        eUartPowerLow,
        eUartMaxPowerCount
    }eUartPowerState;

    typedef enum{
        eUartWakeSrcTimer = 0,
        eUartWakeSrcWifi,
        eUartWakeSrcPir,
        eUartMaxWakeSrcCount
    }eUartWakeSrcType;

    typedef enum{
        eUartCMDReboot = 0,
        eUartCMDSwitchOs,
        eUartCMDGetOs,
        eUartCMDPurelinux,
        eUartCMDDualos,
        eUartCMDSuspend,
        eUartCMDPowerHigh,
        eUartCMDPowerLow,
        eUartCMDWakeSrcTimer,
        eUartCMDWakeSrcWifi,
        eUartCMDWakeSrcPir,
        eUartCMDAck,
        eUartCMDSrc
    }eUartCMD;
#define UART_CMD_POWER_STATE_OFFSET    (6)
#define UART_CMD_WAKEUP_SRC_OFFSET    (UART_CMD_POWER_STATE_OFFSET + eUartMaxPowerCount)

    typedef enum
    {
        E_ST_WAKEUP_TIMER = 0,
        E_ST_WAKEUP_PREVIEW,
        E_ST_WAKEUP_PIR,
        E_ST_WAKEUP_EMPTY
    } ST_WakeupType_e;

    typedef enum
    {
        E_ST_OS_PURELINUX = 0,
        E_ST_OS_DUALOS
    } ST_OSType_e;

    typedef enum
    {
        E_ST_BATTERYLEVEL_NORMAL = 0,
        E_ST_BATTERYLEVEL_LOW
    } ST_BatteryLevel_e;

    typedef enum
    {
        E_ST_REMOTE_CONNECTTING = 0,
        E_ST_REMOTE_DISCONNECT
    } ST_RemoteStatus_e;

    typedef enum
    {
        E_ST_DET_DETECTED = 0,
        E_ST_DET_UNDETECTED,
        E_ST_DET_DELAYCONFIRM
    } ST_DetResult_e;

    typedef enum
    {
        E_ST_FPS_LOW,
        E_ST_FPS_HIGH
    } ST_FpsType_e;

    typedef enum
    {
        E_ST_SYSOFF_POWEROFF,
        E_ST_SYSOFF_REBOOT
    } ST_SysoffType_e;

    typedef struct
    {
        unsigned short u16SnrEarlyFps;
        unsigned short u16SnrEarlyFlicker;
        unsigned long  u32SnrEarlyShutter;
        unsigned long  u32SnrEarlyGainX1024;
        unsigned long  u32SnrEarlyDGain;
        unsigned long  u32SnrEarlyShutterShort;
        unsigned long  u32SnrEarlyGainX1024Short;
        unsigned long  u32SnrEarlyDGainShort;
        unsigned short u16SnrEarlyAwbRGain;
        unsigned short u16SnrEarlyAwbGGain;
        unsigned short u16SnrEarlyAwbBGain;
        unsigned long  u32SnrEarlyShutterMedium;
        unsigned long  u32SnrEarlyGainX1024Medium;
        unsigned long  u32SnrEarlyDGainMedium;
    } __attribute__((packed)) MasterEarlyInitParam_t;

    typedef struct ST_Common_DetBufHandle_s
    {
        MI_SYS_BufInfo_t  stDetBufInfo;
        MI_SYS_BUF_HANDLE hDetHandle;
    } ST_Common_DetBufHandle_t;

    typedef struct ST_Common_AovDetParam_s
    {
        MI_BOOL                    bIsDetected;
        MI_BOOL                    bIsDetectDone;
        pthread_t                  pDetectThread;
        MI_BOOL                    bDetectThreadExit;
        void *                     pDetHandle;
        ST_Common_DetBufHandle_t * pstDetBufHandle;
    } ST_Common_AovDetParam_t;

    typedef struct ST_Common_AovFrameSize_s
    {
        MI_U16 u16Width;
        MI_U16 u16Height;
    } ST_Common_AovFrameSize_t;

    typedef struct ST_Common_AovStreamNode_s
    {
        MI_VENC_Stream_t stStream;
        MI_U32           u32Fps;
        struct list_head list;
        MI_U32           u32Index;
    } ST_Common_AovStreamNode_t;

    typedef struct ST_Common_AovStreamHandle_s
    {
        MI_U32                    u32ListCnt;
        MI_U32                    u32StreamSize;
        pthread_mutex_t           ListMutex;
        ST_Common_AovStreamNode_t stStreamNode;
    } ST_Common_AovStreamHandle_t;

    typedef struct ST_Common_AovLightTable_s
    {
        MI_S32 lux;
        MI_U32 u32Shutter;
        MI_U32 u32Sensorgain;
        MI_U32 u32Ispgain;
    } ST_Common_AovLightTable_t;

    typedef struct ST_Common_AovSWLightSensorParam_s
    {
        MI_BOOL bEnableSWLightSensor;
        MI_BOOL bKeep;
        MI_U32  u32StableFramesBeforeTurnIr;
        MI_U32  u32StableFramesAfterTurnIr;
        MI_BOOL bCurrentRegardedStable;
        MI_BOOL bLastRegardedStable;
        MI_BOOL bFlagTurnIRLedCut;
        MI_U32  au32LastAECnt[AOV_MAX_SENSOR_NUM];
        MI_U32  au32CurrentAECnt[AOV_MAX_SENSOR_NUM];
    } ST_Common_AovSWLightSensorParam_t;

    typedef struct ST_Common_AovLightCtlParam_s
    {
        SSTAR_Light_Ctl_Attr_t stLightCtlAttr;
        SSTAR_Switch_State_e   eSwitchState;
    } ST_Common_AovLightCtlParam_t;

    typedef struct ST_Common_AovHWLightSensorParam_s
    {
        MI_BOOL bEnableHWLightSensor;
        MI_BOOL bUpdateLight;
        MI_S32  lastLux;
        MI_S32  s32TigMode;
        MI_U8   u8LastIndex;
        MI_U32  u32DiffLux;
        MI_U8   u8LeftSetManualAETimes;
    } ST_Common_AovHWLightSensorParam_t;

    typedef struct ST_Common_AovLightMiscCtl_s
    {
        MI_S32                            fd;
        ST_Common_AovLightCtlParam_t      stAovLightCtlParam;
        ST_Common_AovHWLightSensorParam_t stAovHWLightSensorParam;
    } ST_Common_AovLightMiscCtlParam_t;

    typedef struct ST_Common_AovAutoTestParam_s
    {
        MI_BOOL        bAutoTest;
        struct timeval stTargetDetected;
        struct timeval stTargetBufferGet;
        char           au8KeyBoardInput[256];
    } ST_Common_AovAutoTestParam_t;

    typedef struct ST_Common_AovAudioParam_s
    {
        MI_U8        stAiChnGrpId;
        MI_AUDIO_DEV stAiDevId;
        MI_AUDIO_DEV stAoDevId;
        MI_BOOL      bEnable;
        MI_BOOL      bAoEnable;
        MI_BOOL      bAudioRun;
        MI_AI_If_e   enAiIf[2];
        MI_AO_If_e   aenAoIf[2];
        char         au8AiFile[256];
        char         au8AoFile[256];
    } ST_Common_AovAudioParam_t;

    typedef struct ST_Common_AovFastAE_s
    {
        MI_BOOL bEnableFastAE;
        MI_BOOL bIsDoFastAE;
    } ST_Common_AovFastAE_t;

    typedef struct ST_Common_AovVifParam_s
    {
        MI_U32 u32SnrPadId;
        MI_U8  u8SensorIndex;
        MI_U32 u32VifGroupId;
        MI_U32 u32VifDevId;
        MI_U32 u32VifPortId;
    } ST_Common_AovVifParam_t;

    typedef struct ST_Common_AovRtspInfo_s
    {
        void                      *pool_handle;
        ss_rtsp_handle            rtsphandle;
        pthread_mutex_t           Outputmutex;
        char                      url[16];
        struct rtsp_connection_fp fp;
        MI_U32                    u32RefCnt;
    }ST_Common_AovRtspInfo_t;

    typedef struct ST_Common_AovPipeAttr_s
    {
        MI_U32                       u32SensorNum;
        ST_Common_AovVifParam_t      stVifParam[AOV_MAX_SENSOR_NUM];
        MI_SYS_ChnPort_t             stChnPortIsp[AOV_MAX_SENSOR_NUM];
        MI_SYS_ChnPort_t             stChnPortSclPreview[AOV_MAX_SENSOR_NUM];
        MI_SYS_ChnPort_t             stChnPortSclDetect[AOV_MAX_SENSOR_NUM];
        MI_SYS_ChnPort_t             stChnPortVenc[AOV_MAX_SENSOR_NUM];
        MI_RGN_ChnPort_t             stChnPortRgnAttached[AOV_MAX_SENSOR_NUM];
        MI_U32                       u32MaxStreamBufferNum;
        MI_U32                       u32MaxStreamBufferSize;
        MI_U32                       u32MaxFrameHandleNum;
        MI_U32                       u32MaxFrameHandleNumForChn;
        MI_VENC_ModType_e            eVencType;
        MI_VDF_CHANNEL               s32VdfMdChn[AOV_MAX_SENSOR_NUM];
        MI_VDF_ChnAttr_t             stVdfChnAttr;
        MI_RGN_HANDLE                ahFrame[AOV_MAX_SENSOR_NUM][MAX_FRAME_HANDLE];
        MI_RGN_HANDLE                hOsd;
        MI_RGN_ChnPortParam_t        stChnPortFrameParam;
        MI_RGN_ChnPortParam_t        stChnPortOsdParam;
        ST_Common_OsdDrawText_Attr_t stDrawTextAttr;
        InputAttr_t                  stDetModelAttr;
        ST_Common_AovDetParam_t      stDetParam[AOV_MAX_SENSOR_NUM];
        char                         au8IqApiBinBrightPath[256];
        char                         au8IqApiBinDarkPath[256];
        ST_FpsType_e                 eCurrentFps;
        ST_FpsType_e                 eLastFps;
        ST_EnvBrightnessType_e       eCurrentLight;
        ST_EnvBrightnessType_e       eLastLight;
        MI_S32                       fdRtc;
        ST_Common_AovAudioParam_t    stAudioParam;
        ST_Common_AovRtspInfo_t      stRtspInfo[AOV_MAX_SENSOR_NUM];
    } ST_Common_AovPipeAttr_t;

    typedef struct ST_Common_AovParam_s
    {
        MI_U32                   u32DelayConfirmFrameNum;
        MI_U32                   u32SuspendTime;
        char                     au8StreamDumpPath[256];
        char                     au8DetectModelPath[256];
        MI_U32                   u32WriteStreamTriggerNum;
        ST_Common_AovFrameSize_t stPreviewSize;
    } ST_Common_AovParam_t;

    typedef struct ST_Common_AovHandle_s
    {
        ST_Common_AovParam_t              stAovParam;
        ST_Common_AovPipeAttr_t           stAovPipeAttr;
        ST_WakeupType_e                   eCurrentWakeupType;
        ST_WakeupType_e                   eLastWakeupType;
        eUartPowerState                   eCurrentPowerState;
        eUartWakeSrcType                  eNextWakeupSrcToMcu;

        ST_DetResult_e                    eCurrentDetResult;
        ST_DetResult_e                    eLastDetResult;
        ST_Common_SNRSleepParam_t         stSNRSleepParam;
        ST_Common_AovAutoTestParam_t      stAutoTestParam;
        ST_Common_AovFastAE_t             stFastAEParam;
        ST_Common_AovLightMiscCtlParam_t  stLightMiscCtlParam;
        ST_Common_AovSWLightSensorParam_t stAovSWLightSensorParam;
        MI_BOOL                           bDemonstrate;
        MI_BOOL                           bStressTest;
        MI_BOOL                           bUseLastFrameDet;
        MI_BOOL                           bUseMdBeforeDet;
        MI_BOOL                           bUseFrameMode;
        MI_BOOL                           bRtspEnable;
        struct timeval                    stResumeTime;
    } ST_Common_AovHandle_t;

    typedef struct ST_Common_AovDetHandle_s
    {
        ST_Common_AovHandle_t * stAovHandle;
        MI_U32                  u32DetIndex;
    } ST_Common_AovDetHandle_t;

    MI_U32 __AudioDumpAIData(FILE *AiChnFd, MI_U32 *u32MicDumpSize, MI_AUDIO_DEV AiDevId, MI_U8 u8ChnGrpId);
    MI_U32 __AudioPlayAoData(MI_U32 playfd, char *pTmpBuf, MI_U32 writeBufferSize, MI_AUDIO_DEV AoDevId);
    MI_S32 __AudioAddWaveHeader(WaveFileHeader_t *tWavHead, SoundMode_e enSoundMode, SampleRate_e enSampleRate,
                                MI_U32 u32Len);

    MI_S32 __PrintToKmsg(char *buf);
    MI_S32 __PowerOff(ST_SysoffType_e stSysoffType, MI_BOOL bAutoTest);
    MI_S32 __AovAttachTimestamp(ST_Common_AovHandle_t *pstAovHandle);

    MI_S32            ST_Common_AovGetDefaultAttr(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32            ST_Common_AovSetSuspendTime(MI_U32 u32SuspendSec);
    MI_S32            ST_Common_AovEnterSuspend(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32            ST_Common_AovEnterLowPowerMode(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32            ST_Common_AovEnterNormalPowerMode();
    MI_S32            ST_Common_AovSetLowFps(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32            ST_Common_AovSetHighFps(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32            ST_Common_AovISPAdjust_HWLightSensor(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32            ST_Common_AovISPAdjust_SWLightSensor(ST_Common_AovHandle_t *pstAovHandle);
    ST_OSType_e       ST_Common_AovOSCheck();
    ST_WakeupType_e   ST_Common_AovWakeupCheck(ST_Common_AovHandle_t *pstAovHandle);
    ST_BatteryLevel_e ST_Common_AovBatteryLevelCheck(MI_U8 *pu8SimulateCmd);
    ST_RemoteStatus_e ST_Common_AovRemoteStatusCheck(ST_Common_AovHandle_t *pstAovHandle, MI_U8 *pu8SimulateCmd);
    MI_S32            ST_Common_AovSetQos(ST_Common_AovHandle_t *pstAovHandle);

    MI_S32 ST_Common_AovStreamCreate(ST_Common_AovStreamHandle_t *pstStreamHandle);
    MI_S32 ST_Common_AovStreamProduce(ST_Common_AovHandle_t *      pstAovHandle,
                                      ST_Common_AovStreamHandle_t *pstStreamHandle);
    MI_S32 ST_Common_AovStreamConsume(ST_Common_AovHandle_t *      pstAovHandle,
                                      ST_Common_AovStreamHandle_t *pstStreamHandle);
    MI_S32 ST_Common_AovDetect(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32 ST_Common_AovDoAudio(ST_Common_AovHandle_t *pstAovHandle);

    MI_S32 ST_Common_AovPipeInit(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32 ST_Common_AovPipeStart(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32 ST_Common_AovPipeStop(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32 ST_Common_AovPipeDeInit(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32 ST_Common_AovRtspServerStartVideo(ST_Common_AovHandle_t *pstAovHandle);
    MI_S32 ST_Common_AovRtspServerStopVideo(ST_Common_AovHandle_t *pstAovHandle);
    void *ST_Common_AovDetectThread(void *args);
#ifdef _MCU_UART_
    MI_S32 ST_Common_AovMCUUartInit(void);
    void ST_Common_AovSetNextWakeupState(ST_Common_AovHandle_t *pstAovHandle, eUartWakeSrcType eWakeupSrc);
    eUartPowerState ST_Common_AovGetPowerState();
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_AOV_H_
