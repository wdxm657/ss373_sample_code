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
#ifndef _FAST_AE_H_
#define _FAST_AE_H_

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

#include "mi_isp_ae.h"

#define FAST_AE_DEBUG 0
#if FAST_AE_DEBUG
#define PRINT_FAST_AE_DEBUG(...) printf(__VA_ARGS__)
#else
#define PRINT_FAST_AE_DEBUG(...)
#endif

// When the brightness changes (such as turning the light on and off), loading different iqbin, switches that control
// the auxiliary lights, whether to use color to gray, etc.
#define DO_CHECK_DNCHANGE 0

#ifdef MINMAX
#undef MINMAX
#endif
#define MINMAX(v, a, b)       (((v) < (a)) ? (a) : ((v) > (b)) ? (b) : (v))
#define FASTAE_SENSOR_TIMEOUT 20 // The unit is 10 millisecond; The value 20 mean 200 millisecond.

#define CHECK_FAST_AE_RESULT(_func_, _ret_, _exit_label_)                                 \
    do                                                                                    \
    {                                                                                     \
        _ret_ = _func_;                                                                   \
        if (_ret_ != MI_SUCCESS)                                                          \
        {                                                                                 \
            printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, _ret_); \
            goto _exit_label_;                                                            \
        }                                                                                 \
    } while (0)

    typedef enum
    {
        E_ST_CMDID_FAE_SWITCH_SENSOR_AE     = 0,
        E_ST_CMDID_FAE_GET_SENSOR_AE_STATE  = 1,
        E_ST_CMDID_FAE_GET_ABORT_SENSOR_AE  = 2,
        E_ST_CMDID_FAE_GET_SENSOR_SLEEPMODE = 3
    } ST_Sensor_OS04D10_CMDID_e;

    typedef struct
    {
        MI_U32 SensorAEswitch;
        MI_U32 SensorAEled;
        MI_U32 SensorAEfpsmax;
        MI_U32 SensorAEdgain;
        MI_U32 SensorAEagain;
        MI_U32 SensorAEshutter;
        MI_U32 SensorAEtarget;
    } ST_Sensor_OS04D10_AeSwitch_t;

    typedef struct
    {
        MI_U32 state;   // 0 : AE working, 1 : AE done,
        MI_U32 shutter; // us
        MI_U32 again;   // 1X : 1024 (include hcg)
        MI_U32 dgain;   // 1X : 1024
        MI_U32 ymean;   // 0~255 : AE ROI average Y value
    } ST_Sensor_OS04D10_CusAeState_t;

    typedef struct
    {
        MI_BOOL sleepmode; // 0 : sensor working, 1 : sensor sleep,
    } ST_SensorCusSleep_t;

    typedef struct
    {
        MI_BOOL abort; // 0 : AE working, 1 : AE abort,
    } ST_SensorCusAeAbort_t;

    MI_S32 ST_Common_FastAE_CheckDNChange(MI_U32 u32IspDevId, MI_U32 u32IspChnId,
                                          ST_EnvBrightnessType_e *peCurrentLight, char *pu8IqApiBinDarkPath,
                                          char *pu8IqApiBinBrightPath, MI_BOOL *pbDNChange);
    MI_S32 ST_Common_FastAE_EnableSensorSleepMode(MI_U32 u32VifDevId, MI_U32 u32FrameCntBeforeSleep);
    MI_S32 ST_Common_FastAE_DisableSensorSleepMode(MI_U32 u32VifDevId);
    MI_S32 ST_Common_FastAE_QueryExpoInfo(MI_U32 u32IspDevId, MI_U32 u32IspChnId,
                                          MI_ISP_AE_ExpoInfoType_t *pstAeExpoInfo);
    MI_S32 ST_Common_FastAE_WaitIspAeDone(MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_U32 u32IntervalTimeMS,
                                          MI_U32 u32TimeoutMS);
    MI_S32 ST_Common_FastAE_WaitSensorSleep(MI_U32 u32VifDevId, MI_SNR_PADID u32PADId, MI_U32 u32IntervalTimeMS,
                                            MI_U32 u32TimeoutMS);
    MI_S32 ST_Common_FastAE_WaitIspConvToStable(MI_U32 u32IspDevId, MI_U32 u32IspChnId,
                                                MI_U32 u32KeepStableCntWhileReachBoundary, MI_U32 u32WaitIspStableCnt,
                                                MI_ISP_AE_ExpoInfoType_t *pstAeExpoInfo);
    MI_S32 ST_Common_FastAE_PresetForSwitchOnFastAe(MI_U32 u32IspDevId, MI_U32 u32IspChnId,
                                                    MI_ISP_AE_ExpoInfoType_t *    pstExpoInfo,
                                                    ST_Sensor_OS04D10_AeSwitch_t *pstAeSwitch);
    MI_S32 ST_Common_FastAE_PresetForSwitchOffFaseAe(MI_U32                          u32SensorFps,
                                                     ST_Sensor_OS04D10_CusAeState_t *pstAeStateFull,
                                                     ST_Sensor_OS04D10_AeSwitch_t *  pstAeSwitch);
    MI_S32 ST_Common_FastAE_ConvertFastAeResult(MI_U32 u32SensorFps, MI_ISP_AE_ExpoInfoType_t *pstExpoInfo,
                                                ST_Sensor_OS04D10_CusAeState_t *pstAeState, CusAEResult_t *pstAeResult,
                                                ST_Sensor_OS04D10_CusAeState_t *pstAeStateFull);
    MI_S32 ST_Common_FastAE_SetToISP(MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_ISP_AE_ExpoInfoType_t *pstExpoInfo,
                                     CusAEResult_t *pstAeResult);
    MI_S32 ST_Common_FastAE_ConfigToSmallPic(MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_SNR_PADID u32PADId,
                                             MI_ISP_AE_ExpoInfoType_t *pstExpoInfo);
    MI_S32 ST_Common_FastAE_ConfigToFullPic(MI_SNR_PADID u32PADId, MI_U32 u32SensorFps,
                                            ST_Sensor_OS04D10_CusAeState_t *pstAeStateFull);
    MI_S32 ST_Common_FastAE_GetSmallPicAeResult(MI_SNR_PADID u32PADId, MI_U32 u32SensorFps,
                                                ST_Sensor_OS04D10_CusAeState_t *pstAeState,
                                                MI_ISP_AE_ExpoInfoType_t *pstExpoInfo, CusAEResult_t *pstAeResult,
                                                MI_U32 u32IntervalTimeMS, MI_U32 u32TimeoutMS);
    MI_S32 ST_Common_FastAE_Run(MI_U32 u32VifDevId, MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_SNR_PADID u32PADId,
                                MI_U32 u32SensorFps, MI_U32 u32VencDevId, MI_U32 u32VencChnId,
                                MI_VENC_ModType_e eVencType, MI_U32 u32GOP, ST_EnvBrightnessType_e *peCurrentLight,
                                char *pu8IqApiBinDarkPath, char *pu8IqApiBinBrightPath, MI_BOOL *pbIsDoFastAE);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_FAST_AE_H_
