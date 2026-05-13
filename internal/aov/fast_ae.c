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
#include "st_common.h"
#include "st_common_isp.h"
#include "st_common_vif.h"
#include "st_common_venc.h"
#include "mi_common_datatype.h"
#include "mi_isp_awb.h"
#include "mi_isp_cus3a_api.h"
#include "fast_ae.h"

MI_S32 ST_Common_FastAE_CheckDNChange(MI_U32 u32IspDevId, MI_U32 u32IspChnId, ST_EnvBrightnessType_e *peCurrentLight,
                                      char *pu8IqApiBinDarkPath, char *pu8IqApiBinBrightPath, MI_BOOL *pbDNChange)
{
    ST_CHECK_POINTER(peCurrentLight);
    ST_CHECK_POINTER(pu8IqApiBinDarkPath);
    ST_CHECK_POINTER(pu8IqApiBinBrightPath);

    MI_ISP_IQ_ColorToGrayType_t  stColorToGray;
    MI_ISP_IQ_DaynightInfoType_t stDayNightInfo;

    memset(&stColorToGray, 0x0, sizeof(MI_ISP_IQ_ColorToGrayType_t));
    memset(&stDayNightInfo, 0x0, sizeof(MI_ISP_IQ_DaynightInfoType_t));

    MI_ISP_IQ_QueryDayNightInfo(u32IspDevId, u32IspChnId, &stDayNightInfo);
    if (E_ST_LIGHT_BRIGHT == *peCurrentLight)
    {
        printf("Check BV: stDayNightInfo.bD2N = ");
        if (TRUE == stDayNightInfo.bD2N)
        {
            *pbDNChange = TRUE;
            printf("TRUE\n");
            *peCurrentLight = E_ST_LIGHT_DARK;

            ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId, pu8IqApiBinDarkPath);

            stColorToGray.bEnable = TRUE;
            MI_ISP_IQ_SetColorToGray(u32IspDevId, u32IspChnId, &stColorToGray);

            // Add light control logic here if necessary
        }
        else
        {
            *pbDNChange = FALSE;
            printf("FALSE\n");
            goto EXIT;
        }
    }
    else
    {
        printf("Check AWB: stDayNightInfo.bN2D = ");
        if (TRUE == stDayNightInfo.bN2D)
        {
            *pbDNChange = TRUE;
            printf("TRUE\n");
            *peCurrentLight = E_ST_LIGHT_BRIGHT;

            if (strlen(pu8IqApiBinBrightPath) != 0)
            {
                ST_Common_IspSetIqBin(u32IspDevId, u32IspChnId, pu8IqApiBinBrightPath);
            }

            stColorToGray.bEnable = FALSE;
            MI_ISP_IQ_SetColorToGray(u32IspDevId, u32IspChnId, &stColorToGray);

            // Add light control logic here if necessary
        }
        else
        {
            *pbDNChange = FALSE;
            printf("FALSE\n");
            goto EXIT;
        }
    }

EXIT:
    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_DisableSensorSleepMode(MI_U32 u32VifDevId)
{
    ST_Common_SNRSleepParam_t stSNRSleepParam;

    stSNRSleepParam.bSleepEnable = FALSE;
    MI_VIF_CustFunction(u32VifDevId, E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(ST_Common_SNRSleepParam_t),
                        &stSNRSleepParam);

    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_EnableSensorSleepMode(MI_U32 u32VifDevId, MI_U32 u32FrameCntBeforeSleep)
{
    ST_Common_SNRSleepParam_t stSNRSleepParam;

    stSNRSleepParam.bSleepEnable           = TRUE;
    stSNRSleepParam.u32FrameCntBeforeSleep = u32FrameCntBeforeSleep;
    MI_VIF_CustFunction(u32VifDevId, E_MI_VIF_CUSTCMD_SLEEPPARAM_SET, sizeof(ST_Common_SNRSleepParam_t),
                        &stSNRSleepParam);

    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_QueryExpoInfo(MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_ISP_AE_ExpoInfoType_t *pstAeExpoInfo)
{
    ST_CHECK_POINTER(pstAeExpoInfo);

    MI_ISP_AE_QueryExposureInfo(u32IspDevId, u32IspChnId, pstAeExpoInfo);

    PRINT_FAST_AE_DEBUG(
        "==> stAeExpoInfo :\nLumY = %d\nSceneTarget = %d\nbStable = %d\nbIsReachBoundary = %d\nExpoValueLong.u32US = "
        "%d\nExpoValueLong.u32SensorGain = %d\nExpoValueLong.u32ISPGain = %d\nstHistWeightY.u32AvgY = %d\n",
        pstAeExpoInfo->stHistWeightY.u32LumY, pstAeExpoInfo->u32SceneTarget, pstAeExpoInfo->bIsStable,
        pstAeExpoInfo->bIsReachBoundary, pstAeExpoInfo->stExpoValueLong.u32US,
        pstAeExpoInfo->stExpoValueLong.u32SensorGain, pstAeExpoInfo->stExpoValueLong.u32ISPGain,
        pstAeExpoInfo->stHistWeightY.u32AvgY);

    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_WaitIspAeDone(MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_U32 u32IntervalTimeMS,
                                      MI_U32 u32TimeoutMS)
{
    MI_S32         s32Ret           = -1;
    static MI_U32  gu32CurrentAeCnt = 0;
    static MI_U32  gu32LastAECnt    = 0;
    struct timeval stBefore         = {0, 0};
    struct timeval stAfter          = {0, 0};

    gu32LastAECnt = gu32CurrentAeCnt;
    PRINT_FAST_AE_DEBUG("LastAECnt = %d\n", gu32LastAECnt);

    // wait isp ae update done every u32IntervalTimeMS ms. timeout is u32TimeoutMS ms.
    gettimeofday(&stBefore, NULL);
    do
    {
        /*CHECK_FAST_AE_RESULT(MI_ISP_CUS3A_GetDoAeCount(u32IspDevId, u32IspChnId, &gu32CurrentAeCnt), s32Ret, EXIT);
        if (gu32CurrentAeCnt != gu32LastAECnt)
        {
            PRINT_FAST_AE_DEBUG("CurrentAECnt = %d\n", gu32CurrentAeCnt);
            break;
        }*/

        gettimeofday(&stAfter, NULL);

        if (ST_Common_CalcDiffTime_MS(&stBefore, &stAfter) > u32TimeoutMS)
        {
            s32Ret = -1;
            printf("ST_Common_FastAE_WaitIspAeDone timeout\n");
            break;
        }

        usleep(1000 * u32IntervalTimeMS);

    } while (1);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_WaitSensorSleep(MI_U32 u32VifDevId, MI_SNR_PADID u32PADId, MI_U32 u32IntervalTimeMS,
                                        MI_U32 u32TimeoutMS)
{
    MI_S32              s32Ret = -1;
    ST_SensorCusSleep_t stSensorSleep;
    struct timeval      stBefore = {0, 0};
    struct timeval      stAfter  = {0, 0};

    // wait sensor entry sleep mode every u32IntervalTimeMS ms. timeout is u32TimeoutMS ms.
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_EnableSensorSleepMode(u32VifDevId, 2), s32Ret, EXIT);
    gettimeofday(&stBefore, NULL);
    do
    {
        CHECK_FAST_AE_RESULT(MI_SNR_CustFunction(u32PADId, E_ST_CMDID_FAE_GET_SENSOR_SLEEPMODE,
                                                 sizeof(ST_SensorCusSleep_t), &(stSensorSleep),
                                                 E_MI_SNR_CUSTDATA_TO_USER),
                             s32Ret, EXIT);

        if (stSensorSleep.sleepmode == 1)
        {
            PRINT_FAST_AE_DEBUG("Sensor is in sleep mode\n");
            break;
        }

        gettimeofday(&stAfter, NULL);

        if (ST_Common_CalcDiffTime_MS(&stBefore, &stAfter) > u32TimeoutMS)
        {
            s32Ret = -1;
            printf("ST_Common_FastAE_WaitSensorSleep timeout\n");
            break;
        }

        usleep(1000 * u32IntervalTimeMS);

    } while (1);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_WaitIspConvToStable(MI_U32 u32IspDevId, MI_U32 u32IspChnId,
                                            MI_U32 u32KeepStableCntWhileReachBoundary, MI_U32 u32WaitIspStableCnt,
                                            MI_ISP_AE_ExpoInfoType_t *pstAeExpoInfo)
{
    ST_CHECK_POINTER(pstAeExpoInfo);

    MI_S32               s32Ret = -1;
    MI_ISP_AE_ModeType_e eAEMode;

    // wait isp conv bstable to true
    PRINT_FAST_AE_DEBUG("==> Wait ISP stable :\n");

    for (MI_U8 i = 0; i < u32WaitIspStableCnt; i++)
    {
        // wait isp ae update done
        CHECK_FAST_AE_RESULT(ST_Common_FastAE_WaitIspAeDone(u32IspDevId, u32IspChnId, 2, 1000), s32Ret, EXIT);

        if (i == 0)
        {
            eAEMode = E_SS_AE_OP_TYP_AUTO;
            MI_ISP_AE_SetExpoMode(u32IspDevId, u32IspChnId, &eAEMode);
        }

        MI_ISP_AE_QueryExposureInfo(u32IspDevId, u32IspChnId, pstAeExpoInfo);

        PRINT_FAST_AE_DEBUG(
            "LumY = %d\nSceneTarget = %d\nbStable = %d\nbIsReachBoundary = %d\nExpoValueLong.u32US = "
            "%d\nExpoValueLong.u32SensorGain = %d\nExpoValueLong.u32ISPGain = %d\nstHistWeightY.u32AvgY = "
            "%d\n",
            pstAeExpoInfo->stHistWeightY.u32LumY, pstAeExpoInfo->u32SceneTarget, pstAeExpoInfo->bIsStable,
            pstAeExpoInfo->bIsReachBoundary, pstAeExpoInfo->stExpoValueLong.u32US,
            pstAeExpoInfo->stExpoValueLong.u32SensorGain, pstAeExpoInfo->stExpoValueLong.u32ISPGain,
            pstAeExpoInfo->stHistWeightY.u32AvgY);

        if (TRUE == pstAeExpoInfo->bIsStable)
        {
            // If beyond the AE adjustment range
            if ((TRUE == pstAeExpoInfo->bIsReachBoundary) && (u32KeepStableCntWhileReachBoundary > 0))
            {
                u32KeepStableCntWhileReachBoundary--;
                continue;
            }

            break;
        }
        else
        {
            PRINT_FAST_AE_DEBUG("bIsStable = %d\n", pstAeExpoInfo->bIsStable);
        }
    }

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_PresetForSwitchOnFastAe(MI_U32 u32IspDevId, MI_U32 u32IspChnId,
                                                MI_ISP_AE_ExpoInfoType_t *    pstExpoInfo,
                                                ST_Sensor_OS04D10_AeSwitch_t *pstAeSwitch)
{
    ST_CHECK_POINTER(pstExpoInfo);
    ST_CHECK_POINTER(pstAeSwitch);

    pstAeSwitch->SensorAEswitch = 1;
    pstAeSwitch->SensorAEfpsmax = 48;

    CusAEInfo_t      stCusAeInfo;
    CusAEInitParam_t stCusAeInitParam;

    MI_ISP_CUS3A_GetAeStatus(u32IspDevId, u32IspChnId, &stCusAeInfo);
    MI_ISP_CUS3A_GetAeInitStatus(u32IspDevId, u32IspChnId, &stCusAeInitParam);

    // cus3a
    //  MI_U64 U64AETH = (MI_U64)pstExpoInfo->stExpoValueLong.u32US * pstExpoInfo->stExpoValueLong.u32SensorGain
    //                   * pstExpoInfo->stExpoValueLong.u32ISPGain * pstExpoInfo->u32SceneTarget
    //                   / pstExpoInfo->stHistWeightY.u32LumY;

    // sensor
    MI_U64 U64AETH = (MI_U64)stCusAeInitParam.shutter * stCusAeInitParam.sensor_gain * stCusAeInfo.IspGain
                     * pstExpoInfo->u32SceneTarget / pstExpoInfo->stHistWeightY.u32LumY;

    MI_U32 u32SNRShutterMax = (1022 * 22629) / 1000; // get from sensor
    MI_U32 u32SNRShutterMin = 100;                   // get from sensor
    MI_U32 u32SNRAgainMin   = 1024;                  // get from sensor
    MI_U32 u32SNRAgainMax   = 49725;                 // get from sensor
    MI_U32 u32SNRDgainMin   = 1024;                  // get from sensor
    MI_U32 u32SNRDgainMax   = 32 * 1024;             // get from sensor
    MI_U64 u64SNRTH1        = (MI_U64)u32SNRShutterMax * u32SNRAgainMin * u32SNRDgainMin;
    MI_U64 u64SNRTH2        = (MI_U64)u32SNRShutterMax * u32SNRAgainMax * u32SNRDgainMin;

    if (U64AETH >= u64SNRTH1)
    {
        pstAeSwitch->SensorAEshutter = MAX(u32SNRShutterMin, u32SNRShutterMax / 4);
        pstAeSwitch->SensorAEdgain =
            MINMAX(4 * u32SNRDgainMin * U64AETH / u64SNRTH2, 4 * u32SNRDgainMin, u32SNRDgainMax);
        pstAeSwitch->SensorAEagain =
            MINMAX(U64AETH / pstAeSwitch->SensorAEshutter / pstAeSwitch->SensorAEdgain, u32SNRAgainMin, u32SNRAgainMax);
    }
    else
    {
        pstAeSwitch->SensorAEagain   = u32SNRAgainMin;
        pstAeSwitch->SensorAEdgain   = 4 * u32SNRDgainMin;
        pstAeSwitch->SensorAEshutter = MINMAX(U64AETH / pstAeSwitch->SensorAEagain / pstAeSwitch->SensorAEdgain,
                                              u32SNRShutterMin, u32SNRShutterMax);
    }

    pstAeSwitch->SensorAEtarget = pstExpoInfo->u32SceneTarget / 10;

    PRINT_FAST_AE_DEBUG(
        "==> ST_Common_FastAE_PresetForSwitchOnFastAe :\npstAeSwitch->SensorAEswitch = %d\npstAeSwitch->SensorAEtarget "
        "= %d\npstAeSwitch->SensorAEshutter = %d\npstAeSwitch->SensorAEagain = %d\npstAeSwitch->SensorAEdgain = "
        "%d\npstAeSwitch->SensorAEled = %d\npstAeSwitch->SensorAEfpsmax = %d\n",
        pstAeSwitch->SensorAEswitch, pstAeSwitch->SensorAEtarget, pstAeSwitch->SensorAEshutter,
        pstAeSwitch->SensorAEagain, pstAeSwitch->SensorAEdgain, pstAeSwitch->SensorAEled, pstAeSwitch->SensorAEfpsmax);

    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_PresetForSwitchOffFaseAe(MI_U32 u32SensorFps, ST_Sensor_OS04D10_CusAeState_t *pstAeStateFull,
                                                 ST_Sensor_OS04D10_AeSwitch_t *pstAeSwitch)
{
    ST_CHECK_POINTER(pstAeStateFull);
    ST_CHECK_POINTER(pstAeSwitch);

    pstAeSwitch->SensorAEswitch  = 0;
    pstAeSwitch->SensorAEshutter = pstAeStateFull->shutter;
    pstAeSwitch->SensorAEagain   = pstAeStateFull->again;
    pstAeSwitch->SensorAEfpsmax  = u32SensorFps;

    PRINT_FAST_AE_DEBUG(
        "==> ST_Common_FastAE_PresetForSwitchOffFaseAe :\npstAeSwitch->SensorAEswitch = "
        "%d\npstAeSwitch->SensorAEtarget = "
        "%d\npstAeSwitch->SensorAEshutter = %d\npstAeSwitch->SensorAEagain = "
        "%d\npstAeSwitch->SensorAEdgain = "
        "%d\npstAeSwitch->SensorAEled = %d\npstAeSwitch->SensorAEfpsmax = %d\n",
        pstAeSwitch->SensorAEswitch, pstAeSwitch->SensorAEtarget, pstAeSwitch->SensorAEshutter,
        pstAeSwitch->SensorAEagain, pstAeSwitch->SensorAEdgain, pstAeSwitch->SensorAEled, pstAeSwitch->SensorAEfpsmax);

    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_ConvertFastAeResult(MI_U32 u32SensorFps, MI_ISP_AE_ExpoInfoType_t *pstExpoInfo,
                                            ST_Sensor_OS04D10_CusAeState_t *pstAeState, CusAEResult_t *pstAeResult,
                                            ST_Sensor_OS04D10_CusAeState_t *pstAeStateFull)
{
    ST_CHECK_POINTER(pstExpoInfo);
    ST_CHECK_POINTER(pstAeState);
    ST_CHECK_POINTER(pstAeResult);
    ST_CHECK_POINTER(pstAeStateFull);

    // u32MaxShutter : max shutter in us. It will be set by user app.
    MI_U32 u32IspGainMin = 1024;
    MI_U32 u32MaxShutter = 1000000 / u32SensorFps;
    MI_U32 u32SNRGainMin = 1024;       // get from sensor
    MI_U32 u32SNRGainMax = 49725 * 32; // get from sensor
    MI_U64 TH            = u32IspGainMin * (MI_U64)pstAeState->shutter * pstAeState->again * pstExpoInfo->u32SceneTarget
                / MAX((pstAeState->ymean * 10), 1);

    if (TH >= (MI_U64)u32MaxShutter * u32SNRGainMin * u32IspGainMin)
    {
        pstAeStateFull->shutter = u32MaxShutter;
        pstAeStateFull->dgain   = MAX(1024, TH / ((MI_U64)u32MaxShutter * u32SNRGainMax));
        pstAeStateFull->again   = TH / ((MI_U64)u32MaxShutter * pstAeStateFull->dgain);
    }
    else
    {
        pstAeStateFull->again   = u32SNRGainMin;
        pstAeStateFull->dgain   = u32IspGainMin;
        pstAeStateFull->shutter = TH / ((MI_U64)u32SNRGainMin * u32IspGainMin);
    }

    if ((0 != pstAeState->ymean) || (0 != pstAeState->again) || (0 != pstAeState->shutter))
    {
        pstAeResult->Size               = sizeof(CusAEResult_t);
        pstAeResult->Change             = 2;
        pstAeResult->Shutter            = pstAeStateFull->shutter;
        pstAeResult->SensorGain         = pstAeStateFull->again;
        pstAeResult->IspGain            = pstAeStateFull->dgain;
        pstAeResult->ShutterHdrShort    = 100;
        pstAeResult->SensorGainHdrShort = 1024;
        pstAeResult->IspGainHdrShort    = 1024;
        pstAeResult->i4BVx16384         = 0;
        pstAeResult->AvgY               = pstExpoInfo->u32SceneTarget;
        pstAeResult->HdrRatio           = 1024;
        //pstAeResult->HdrRatio1          = 1024;
        pstAeResult->FNx10              = pstExpoInfo->stExpoValueLong.u32FNx10;
        pstAeResult->DebandFPS          = 30000;
        pstAeResult->WeightY            = pstExpoInfo->u32SceneTarget;
        pstAeResult->GMBlendRatio       = 512;
    }
    else
    {
        printf("fast ae result is 0, will use the last AE parameter\n");

        pstAeResult->Size               = sizeof(CusAEResult_t);
        pstAeResult->Change             = 2;
        pstAeResult->Shutter            = pstExpoInfo->stExpoValueLong.u32US;
        pstAeResult->SensorGain         = pstExpoInfo->stExpoValueLong.u32SensorGain;
        pstAeResult->IspGain            = pstExpoInfo->stExpoValueLong.u32ISPGain;
        pstAeResult->ShutterHdrShort    = 100;
        pstAeResult->SensorGainHdrShort = 1024;
        pstAeResult->IspGainHdrShort    = 1024;
        pstAeResult->i4BVx16384         = 0;
        pstAeResult->AvgY               = pstExpoInfo->u32SceneTarget;
        pstAeResult->HdrRatio           = 1024;
        //pstAeResult->HdrRatio1          = 1024;
        pstAeResult->FNx10              = pstExpoInfo->stExpoValueLong.u32FNx10;
        pstAeResult->DebandFPS          = 30000;
        pstAeResult->WeightY            = pstExpoInfo->u32SceneTarget;
        pstAeResult->GMBlendRatio       = 512;
    }

    PRINT_FAST_AE_DEBUG(
        "==> ST_Common_FastAE_ConvertFastAeResult :\npstAeResult->Size = %d\nAEResult.Change = %d\nAEResult.Shutter = "
        "%d\nAEResult.SensorGain = %d\npstAeResult->IspGain = %d\npstAEResult->ShutterHdrShort = "
        "%d\npstAEResult->SensorGainHdrShort = %d\npstAeResult->IspGainHdrShort = %d\npstAEResult->i4BVx16384 = "
        "%d\npstAEResult->AvgY = "
        "%d\npstAEResult->HdrRatio = %d\npstAeResult->HdrRatio1 = %d\npstAEResult->FNx10 = %d\npstAEResult->DebandFPS "
        "= %d\npstAEResult->WeightY = "
        "%d\npstAeResult->GMBlendRatio = %d\n",
        pstAeResult->Size, pstAeResult->Change, pstAeResult->Shutter, pstAeResult->SensorGain, pstAeResult->IspGain,
        pstAeResult->ShutterHdrShort, pstAeResult->SensorGainHdrShort, pstAeResult->IspGainHdrShort,
        pstAeResult->i4BVx16384, pstAeResult->AvgY, pstAeResult->HdrRatio, pstAeResult->HdrRatio1, pstAeResult->FNx10,
        pstAeResult->DebandFPS, pstAeResult->WeightY, pstAeResult->GMBlendRatio);

    return MI_SUCCESS;
}

MI_S32 ST_Common_FastAE_SetToISP(MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_ISP_AE_ExpoInfoType_t *pstExpoInfo,
                                 CusAEResult_t *pstAeResult)
{
    ST_CHECK_POINTER(pstExpoInfo);

    MI_S32                    s32Ret = -1;
    MI_ISP_AE_ModeType_e      eAEMode;
    MI_ISP_AE_ExpoValueType_t stExpoVal;

    CHECK_FAST_AE_RESULT(MI_ISP_CUS3A_SetAeParam(u32IspDevId, u32IspChnId, pstAeResult), s32Ret, EXIT);

    eAEMode = E_SS_AE_MODE_M;
    CHECK_FAST_AE_RESULT(MI_ISP_AE_SetExpoMode(u32IspDevId, u32IspChnId, &eAEMode), s32Ret, EXIT);

    stExpoVal.u32FNx10      = pstExpoInfo->stExpoValueLong.u32FNx10;
    stExpoVal.u32SensorGain = pstAeResult->SensorGain;
    stExpoVal.u32ISPGain    = pstAeResult->IspGain;
    stExpoVal.u32US         = pstAeResult->Shutter;
    CHECK_FAST_AE_RESULT(MI_ISP_AE_SetManualExpo(u32IspDevId, u32IspChnId, &stExpoVal), s32Ret, EXIT);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_ConfigToSmallPic(MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_SNR_PADID u32PADId,
                                         MI_ISP_AE_ExpoInfoType_t *pstExpoInfo)
{
    ST_CHECK_POINTER(pstExpoInfo);

    MI_S32                       s32Ret = -1;
    ST_Sensor_OS04D10_AeSwitch_t stFastAeSwitch;

    // preset parameter for fast ae switch on (samll pic)
    ST_Common_FastAE_PresetForSwitchOnFastAe(u32IspDevId, u32IspChnId, pstExpoInfo, &stFastAeSwitch);

    // Configure Sensor to switch on fast ae
    CHECK_FAST_AE_RESULT(MI_SNR_CustFunction(u32PADId, E_ST_CMDID_FAE_SWITCH_SENSOR_AE,
                                             sizeof(ST_Sensor_OS04D10_AeSwitch_t), &stFastAeSwitch,
                                             E_MI_SNR_CUSTDATA_TO_DRIVER),
                         s32Ret, EXIT);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_ConfigToFullPic(MI_SNR_PADID u32PADId, MI_U32 u32SensorFps,
                                        ST_Sensor_OS04D10_CusAeState_t *pstAeStateFull)
{
    ST_CHECK_POINTER(pstAeStateFull);

    MI_S32                       s32Ret = -1;
    ST_Sensor_OS04D10_AeSwitch_t stFastAeSwitch;

    // Configure Sensor to switch off fast ae
    ST_Common_FastAE_PresetForSwitchOffFaseAe(u32SensorFps, pstAeStateFull, &stFastAeSwitch);
    CHECK_FAST_AE_RESULT(MI_SNR_CustFunction(u32PADId, E_ST_CMDID_FAE_SWITCH_SENSOR_AE,
                                             sizeof(ST_Sensor_OS04D10_AeSwitch_t), &stFastAeSwitch,
                                             E_MI_SNR_CUSTDATA_TO_DRIVER),
                         s32Ret, EXIT);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_GetSmallPicAeResult(MI_SNR_PADID u32PADId, MI_U32 u32SensorFps,
                                            ST_Sensor_OS04D10_CusAeState_t *pstAeState,
                                            MI_ISP_AE_ExpoInfoType_t *pstExpoInfo, CusAEResult_t *pstAeResult,
                                            MI_U32 u32IntervalTimeMS, MI_U32 u32TimeoutMS)
{
    MI_S32                s32Ret   = -1;
    struct timeval        stBefore = {0, 0};
    struct timeval        stAfter  = {0, 0};
    ST_SensorCusAeAbort_t stSensorCusAbort;

    // wait fast ae done every u32IntervalTimeMS ms. timeout is u32TimeoutMS ms.
    gettimeofday(&stBefore, NULL);
    do
    {
        CHECK_FAST_AE_RESULT(MI_SNR_CustFunction(u32PADId, E_ST_CMDID_FAE_GET_SENSOR_AE_STATE,
                                                 sizeof(ST_Sensor_OS04D10_CusAeState_t), pstAeState,
                                                 E_MI_SNR_CUSTDATA_TO_USER),
                             s32Ret, EXIT);

        if (pstAeState->state == 1)
        {
            PRINT_FAST_AE_DEBUG("Run fast ae done\n");
            break;
        }

        gettimeofday(&stAfter, NULL);

        if (ST_Common_CalcDiffTime_MS(&stBefore, &stAfter) > u32TimeoutMS)
        {
            printf("abort fast ae while run timeout\n");

            // abort while fast ae timeout
            stSensorCusAbort.abort = 1;
            CHECK_FAST_AE_RESULT(MI_SNR_CustFunction(u32PADId, E_ST_CMDID_FAE_GET_ABORT_SENSOR_AE,
                                                     sizeof(ST_SensorCusAeAbort_t), &stSensorCusAbort,
                                                     E_MI_SNR_CUSTDATA_TO_DRIVER),
                                 s32Ret, EXIT);

            break;
        }

        usleep(1000 * u32IntervalTimeMS);

    } while (1);

    PRINT_FAST_AE_DEBUG(
        "==> ST_Common_FastAE_GetSmallPicAeResult :\npstAeState->state = %d\npstAeState->again = %d\npstAeState->dgain "
        "= %d\npstAeState->shutter = %d\npstAeState->ymean = %d\n",
        pstAeState->state, pstAeState->again, pstAeState->dgain, pstAeState->shutter, pstAeState->ymean);

EXIT:
    return s32Ret;
}

MI_S32 ST_Common_FastAE_Run(MI_U32 u32VifDevId, MI_U32 u32IspDevId, MI_U32 u32IspChnId, MI_SNR_PADID u32PADId,
                            MI_U32 u32SensorFps, MI_U32 u32VencDevId, MI_U32 u32VencChnId, MI_VENC_ModType_e eVencType,
                            MI_U32 u32GOP, ST_EnvBrightnessType_e *peCurrentLight, char *pu8IqApiBinDarkPath,
                            char *pu8IqApiBinBrightPath, MI_BOOL *pbIsDoFastAE)
{
    MI_S32                         s32Ret = MI_SUCCESS;
    MI_ISP_AE_ExpoInfoType_t       stAeExpoInfo;
    ST_Sensor_OS04D10_AeSwitch_t   stFastAESwitch;
    ST_Sensor_OS04D10_CusAeState_t stAeStateFull;
    CusAEResult_t                  stAeResult;
    ST_Sensor_OS04D10_CusAeState_t stAeState;

#if DO_CHECK_DNCHANGE
    MI_BOOL bDNChange;
#endif

    memset(&stAeExpoInfo, 0x0, sizeof(MI_ISP_AE_ExpoInfoType_t));
    memset(&stFastAESwitch, 0x00, sizeof(ST_Sensor_OS04D10_AeSwitch_t));
    memset(&stAeStateFull, 0x00, sizeof(ST_Sensor_OS04D10_CusAeState_t));
    memset(&stAeState, 0x00, sizeof(ST_Sensor_OS04D10_CusAeState_t));

    // wait isp ae update done
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_WaitIspAeDone(u32IspDevId, u32IspChnId, 2, 1000), s32Ret, EXIT);

    // check whether ae is stable
    CHECK_FAST_AE_RESULT(ST_Common_FastAE_QueryExpoInfo(u32IspDevId, u32IspChnId, &stAeExpoInfo), s32Ret, EXIT);

    if (FALSE == stAeExpoInfo.bIsStable)
    {
        printf("ae bstable is false, will do fast ae\n");

        *pbIsDoFastAE = TRUE;

        // request IDR to ensure better coding effect
        CHECK_FAST_AE_RESULT(MI_VENC_RequestIdr(u32VencDevId, u32VencChnId, 0), s32Ret, EXIT);

        // wait sensor entry sleep mode
        CHECK_FAST_AE_RESULT(ST_Common_FastAE_WaitSensorSleep(u32VifDevId, u32PADId, 10, 1000), s32Ret, EXIT);

        // config to small pic (switch on fast ae)
        CHECK_FAST_AE_RESULT(ST_Common_FastAE_ConfigToSmallPic(u32IspDevId, u32IspChnId, u32PADId, &stAeExpoInfo),
                             s32Ret, EXIT);

        // configure sensor to disable sleep mode
        CHECK_FAST_AE_RESULT(ST_Common_FastAE_DisableSensorSleepMode(u32VifDevId), s32Ret, EXIT);

        // get small pic (fast ae) result
        CHECK_FAST_AE_RESULT(ST_Common_FastAE_GetSmallPicAeResult(u32PADId, u32SensorFps, &stAeState, &stAeExpoInfo,
                                                                  &stAeResult, 10, 2000),
                             s32Ret, EXIT);

        // convert small pic parameters to full pic parameters
        CHECK_FAST_AE_RESULT(
            ST_Common_FastAE_ConvertFastAeResult(u32SensorFps, &stAeExpoInfo, &stAeState, &stAeResult, &stAeStateFull),
            s32Ret, EXIT);

        // set fast ae result to isp
        CHECK_FAST_AE_RESULT(ST_Common_FastAE_SetToISP(u32IspDevId, u32IspChnId, &stAeExpoInfo, &stAeResult), s32Ret,
                             EXIT);

        // config to full pic (switch off fast ae)
        CHECK_FAST_AE_RESULT(ST_Common_FastAE_ConfigToFullPic(u32PADId, u32SensorFps, &stAeStateFull), s32Ret, EXIT);

        // wait isp conv bstable to true
        CHECK_FAST_AE_RESULT(ST_Common_FastAE_WaitIspConvToStable(u32IspDevId, u32IspChnId, 5, 10, &stAeExpoInfo),
                             s32Ret, EXIT);

        // configure sensor to enable sleep mode & consecutive frames cnt (to 2)
        CHECK_FAST_AE_RESULT(ST_Common_FastAE_EnableSensorSleepMode(u32VifDevId, 2), s32Ret, EXIT);

#if DO_CHECK_DNCHANGE
        // check D2N or N2D
        ST_Common_FastAE_CheckDNChange(u32IspDevId, u32IspChnId, peCurrentLight, pu8IqApiBinDarkPath,
                                       pu8IqApiBinBrightPath, &bDNChange);

        if (TRUE == bDNChange)
        {
            // request IDR to ensure better coding effect
            CHECK_FAST_AE_RESULT(MI_VENC_RequestIdr(u32VencDevId, u32VencChnId, 0), s32Ret, EXIT);

            // wait sensor entry sleep mode
            CHECK_FAST_AE_RESULT(ST_Common_FastAE_WaitSensorSleep(u32VifDevId, u32PADId, 10, 1000), s32Ret, EXIT);

            // config to small pic (switch on fast ae)
            CHECK_FAST_AE_RESULT(ST_Common_FastAE_ConfigToSmallPic(u32IspDevId, u32IspChnId, u32PADId, &stAeExpoInfo),
                                 s32Ret, EXIT);

            // configure sensor to disable sleep mode
            CHECK_FAST_AE_RESULT(ST_Common_FastAE_DisableSensorSleepMode(u32VifDevId), s32Ret, EXIT);

            // get small pic (fast ae) result
            CHECK_FAST_AE_RESULT(ST_Common_FastAE_GetSmallPicAeResult(u32PADId, u32SensorFps, &stAeState, &stAeExpoInfo,
                                                                      &stAeResult, 10, 2000),
                                 s32Ret, EXIT);

            // convert small pic parameters to full pic parameters
            CHECK_FAST_AE_RESULT(ST_Common_FastAE_ConvertFastAeResult(u32SensorFps, &stAeExpoInfo, &stAeState,
                                                                      &stAeResult, &stAeStateFull),
                                 s32Ret, EXIT);

            // set fast ae result to isp
            CHECK_FAST_AE_RESULT(ST_Common_FastAE_SetToISP(u32IspDevId, u32IspChnId, &stAeExpoInfo, &stAeResult),
                                 s32Ret, EXIT);

            // config to full pic (switch off fast ae)
            CHECK_FAST_AE_RESULT(ST_Common_FastAE_ConfigToFullPic(u32PADId, u32SensorFps, &stAeStateFull), s32Ret,
                                 EXIT);

            // configure sensor to enable sleep mode & consecutive frames cnt (to 2)
            CHECK_FAST_AE_RESULT(ST_Common_FastAE_EnableSensorSleepMode(u32VifDevId, 2), s32Ret, EXIT);
        }
#endif
    }
    else
    {
        *pbIsDoFastAE = FALSE;
    }

EXIT:
    return s32Ret;
}