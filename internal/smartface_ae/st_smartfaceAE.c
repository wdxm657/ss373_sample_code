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
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include "st_smartfaceAE.h"
#include "mi_isp.h"
#include "mi_isp_ae.h"

static MI_ISP_AE_IntpLutType_t g_stLutTarget;
static MI_ISP_AE_IntpLutType_t g_stLutTmpTarget;
static MI_ISP_AE_ExpoInfoType_t g_stExpoInfo;
static MI_ISP_AE_ConvConditonType_t g_stRgbAeConvCondition;
static MI_BOOL g_bIqEnable = 1;
static MI_U32 g_au32Thr[2] = {OVER_DARK_RGB,OVER_LIGHT_RGB};

static MI_U32 g_au32AdjustmentAeSpeed[4] = {50,50,50,50};
static MI_U32 g_au32AdjustmentAeCov[2]   = {40,60};

static MI_S32 g_s32Target = 0;
static MI_U32 g_u32BaseTarget   = 300;
static MI_U32 g_u32TargetStep   = 100;
static MI_U32 g_u32TargetPre    = 0;
static MI_U32 g_u32Preu32Lightness =0;

MI_S32 g_s32NoFaceCount        = 0;
MI_S32 g_s32IfLightnessIsEqual = 0;
MI_S32 g_s32IfInTh0Th1         = 0;
MI_S32 g_s32IfConutC1C2        = 0;

void ST_InitSmartFace(ST_SMARTFACEAE_Config_t *pstConfig)
{
    //这里要加防呆,传参可能不在允许范围
    g_bIqEnable = pstConfig->bEnable;

    g_au32Thr[0] = pstConfig->u32OverDarkValue;
    g_au32Thr[1] = pstConfig->u32OverLightValue;

    g_u32TargetStep = pstConfig->u32BaseTargetStep;

    g_au32AdjustmentAeSpeed[0] = pstConfig->u32AjustmentAeSpeed[0];
    g_au32AdjustmentAeSpeed[1] = pstConfig->u32AjustmentAeSpeed[1];
    g_au32AdjustmentAeSpeed[2] = pstConfig->u32AjustmentAeSpeed[2];
    g_au32AdjustmentAeSpeed[3] = pstConfig->u32AjustmentAeSpeed[3];
    g_au32AdjustmentAeCov[0]   = pstConfig->u32AjustmentAeCov[0];
    g_au32AdjustmentAeCov[1]   = pstConfig->u32AjustmentAeCov[1];
}

void ST_SetAEData(MI_U32 u32DevId, MI_U32 u32ChnId,MI_U32 u32Lightness)
{
    MI_ISP_AE_ConvConditonType_t stAeConvCondition = {};

    if(g_bIqEnable != 1)
    {
        return;
    }

    //---------Recovery-----------------
    if(-1 == u32ChnId && -1 == u32Lightness)
    {
        g_u32TargetPre           = 0;
        g_s32Target              = 0;
        g_u32Preu32Lightness     = 0;
        g_s32NoFaceCount         = 1;
        g_s32IfLightnessIsEqual = 0;
        g_s32IfInTh0Th1          = 0;
        g_s32IfConutC1C2         = 0;
        return;
    }

    /*
     * 考虑客户在call SetAEData的时候，一帧会内会多次call,但是AE下target，到生效需要3帧之后，期间会出现即使下更新target的状况后
     * 智能那边算出来的人脸亮度u32Lightness会保持很多帧不变的情况。这样target一直在更新，但u32Lightness保持不变，导致亮度来回变化；
     * 如果后续像hisi一样整到AE algo中生效节点可以控制，感觉在user Space二次开发，这块只能针对些场景workarond
     */
    if(g_u32Preu32Lightness == u32Lightness)
    {
        g_s32IfLightnessIsEqual = 1;
        return;
    }
    else
    {
        /*
         * 考虑客户人脸检测，提取区域不一样，导致即使下了一个相比上一次小的target，但是框的位置不一样，
         * 出来的u32Lightness却比上一次大的情况;这样会导致亮度的突变。
         *   case 1：小的target,出来比上一次大的人脸亮度
         *   case 2：大的target,出来比上一次小的人脸亮度
         */
        if((g_u32TargetPre > g_s32Target && g_u32Preu32Lightness < u32Lightness)
                || (g_u32TargetPre < g_s32Target && g_u32Preu32Lightness > u32Lightness))
        {
            /*
             * 需要考虑：类似连续多次u32Lightness=171 g_u32Preu32Lightness=171 过渡到下一次u32Lightness=206
             * 且 g_u32TargetPre>target的情况 需要调整的。
             */
            if(g_s32IfLightnessIsEqual == 1)
            {
                g_s32IfLightnessIsEqual = 0;
            }
            else
            {
                //有些场景，还是很容易踩到case 1和case 2，但是实际没有进入区间内（大于th0,小于th2）;
                if(g_s32IfInTh0Th1 == 0 && g_s32IfConutC1C2 > 15)
                {
                    g_u32Preu32Lightness = u32Lightness;
                    g_s32IfConutC1C2     = 0;
                }
                else
                {
                    g_s32IfConutC1C2++;
                    return;
                }
            }
        }
        else
        {
            g_u32TargetPre = g_s32Target;
        }
        g_u32Preu32Lightness = u32Lightness;
    }

    //提到前面是为了，防止稳定了还一直在call AE的function
    if(u32Lightness >= g_au32Thr[0] && u32Lightness <= g_au32Thr[1])
    {
        ISP_LOG("chn:%d ,Normal\n",u32ChnId);
        g_s32IfInTh0Th1=1;
        return;
    }

    //--------------GetAeStatus------------
    MI_ISP_AE_QueryExposureInfo(u32DevId, u32ChnId, &g_stExpoInfo);
    ISP_LOG("u32ChnId = %d"
            " g_stExpoInfo.stExpoValueShort.u32SensorGain:%d,"
            " g_stExpoInfo.stExpoValueShort.u32ISPGain:%d,"
            " g_stExpoInfo.stExpoValueShort.u32US=%d\n",
            u32ChnId,
            g_stExpoInfo.g_stExpoInfoValueShort.u32SensorGain,
            g_stExpoInfo.g_stExpoInfoValueShort.u32ISPGain,
            g_stExpoInfo.g_stExpoInfoValueShort.u32US);

    ISP_LOG("u32ChnId = %d"
            " g_stExpoInfo.stExpoValueLong.u32SensorGain:%d,"
            " g_stExpoInfo.stExpoValueLong.u32ISPGain:%d,"
            " g_stExpoInfo.g_stExpoInfoValueLong.u32US=%d\n",
            u32ChnId,
            g_stExpoInfo.g_stExpoInfoValueLong.u32SensorGain,
            g_stExpoInfo.g_stExpoInfoValueLong.u32ISPGain,
            g_stExpoInfo.g_stExpoInfoValueLong.u32US);

    //----------------IsStable-------------
    // 从无人脸进来，到有人脸检测的过渡,g_u32BaseTarget要切换到恢复默认时候的 target
    if(g_s32NoFaceCount == 1)
    {
        g_u32BaseTarget   = g_s32Target;
        g_s32NoFaceCount  = 0;//一旦检测到人脸就需要清0
    }
    else
    {
        g_u32BaseTarget = g_stExpoInfo.u32SceneTarget;
    }

    //-------------------adjust-----------------
    MI_ISP_AE_GetConverge(u32DevId, u32ChnId, &stAeConvCondition);
    if(u32Lightness < g_au32Thr[0]) // too dark
    {

        MI_S32 s32DiffDark = g_au32Thr[0] - u32Lightness;
        MI_S32 s32TargetSpeedDark = (s32DiffDark*g_u32TargetStep)/g_au32Thr[0];

        g_s32Target = g_u32BaseTarget + s32TargetSpeedDark;
        if(g_s32Target > 2000)
        {
            g_s32Target = 2000;
        }
        g_s32IfInTh0Th1=0;
    }
    else if(u32Lightness > g_au32Thr[1]) // too light
    {
        MI_S32 diff_light        =  u32Lightness - g_au32Thr[1];
        MI_S32 TargetSpeed_Light = (diff_light*g_u32TargetStep)/g_au32Thr[1];

        g_s32Target = g_u32BaseTarget - TargetSpeed_Light;
        if(g_s32Target < 50)
        {
            g_s32Target = 50;
        }
        g_s32IfInTh0Th1 = 0;
    }

    MI_ISP_AE_GetTarget(u32DevId, u32ChnId, &g_stLutTmpTarget);
    g_stLutTmpTarget.u16NumOfPoints = 1;
    g_stLutTmpTarget.u32Y[0]        = g_s32Target;
    MI_ISP_AE_SetTarget(u32DevId, u32ChnId, &g_stLutTmpTarget);
}

void ST_SaveOriAEData(MI_U32 u32DevId)
{
    MI_S32 s32Index ;
    MI_ISP_AE_ConvConditonType_t stAeConvCondition;
    // record the state without face
    MI_ISP_AE_GetTarget(u32DevId, RGB_AE_CHN, &g_stLutTarget);

    ISP_LOG("ST_ISP_AE_Flush rgb g_stLutTarget.u16NumOfPoints:%d\n",g_stLutTarget.u16NumOfPoints);
    for(s32Index = 0; s32Index < g_stLutTarget.u16NumOfPoints; s32Index++)
    {
        ISP_LOG("rgb Target[%d]:%d\n",i,g_stLutTarget.u32Y[s32Index]);
        printf("\n\nST_SaveOriAEData_target=%d\n\n",g_stLutTarget.u32Y[s32Index]);
    }

    MI_ISP_AE_GetConverge(u32DevId, RGB_AE_CHN, &g_stRgbAeConvCondition);

    stAeConvCondition.stConvSpeed.u32SpeedY[0] = g_au32AdjustmentAeSpeed[0];
    stAeConvCondition.stConvSpeed.u32SpeedY[1] = g_au32AdjustmentAeSpeed[1];
    stAeConvCondition.stConvSpeed.u32SpeedY[2] = g_au32AdjustmentAeSpeed[2];
    stAeConvCondition.stConvSpeed.u32SpeedY[3] = g_au32AdjustmentAeSpeed[3];
    stAeConvCondition.stConvSpeed.u32SpeedX[0] = 100;
    stAeConvCondition.stConvSpeed.u32SpeedX[1] = 470;
    stAeConvCondition.stConvSpeed.u32SpeedX[2] = 470;
    stAeConvCondition.stConvSpeed.u32SpeedX[3] = 2000;
    stAeConvCondition.stConvThrd.u32InThd      = g_au32AdjustmentAeCov[0];
    stAeConvCondition.stConvThrd.u32OutThd     = g_au32AdjustmentAeCov[1];
    MI_ISP_AE_SetConverge(u32DevId, RGB_AE_CHN, &stAeConvCondition);
}

//没有检查到人脸的时候，call一次即可，一段时间没有人脸的时候不能一直call,否则导致AE频繁trigger
void ST_RestoreAEData(MI_U32 u32DevId)
{
    MI_S32 s32Index = 0;

    MI_ISP_AE_ConvConditonType_t stAeConvCondition; // cover default parames smooth
    stAeConvCondition.stConvSpeed.u32SpeedY[0] = g_au32AdjustmentAeSpeed[0];
    stAeConvCondition.stConvSpeed.u32SpeedY[1] = g_au32AdjustmentAeSpeed[1];
    stAeConvCondition.stConvSpeed.u32SpeedY[2] = g_au32AdjustmentAeSpeed[2];
    stAeConvCondition.stConvSpeed.u32SpeedY[3] = g_au32AdjustmentAeSpeed[3];
    stAeConvCondition.stConvSpeed.u32SpeedX[0] = 100;
    stAeConvCondition.stConvSpeed.u32SpeedX[1] = 470;
    stAeConvCondition.stConvSpeed.u32SpeedX[2] = 470;
    stAeConvCondition.stConvSpeed.u32SpeedX[3] = 2000;
    stAeConvCondition.stConvThrd.u32InThd      = g_au32AdjustmentAeCov[0];
    stAeConvCondition.stConvThrd.u32OutThd     = g_au32AdjustmentAeCov[1];

    for(s32Index = 0; s32Index < g_stLutTarget.u16NumOfPoints; s32Index++)
    {
        ISP_LOG("ST_RestoreAEData rgb Target[%d]:%d\n",i,g_stLutTarget.u32Y[s32Index]);
    }
    ST_SetAEData(u32DevId, -1, -1);
    MI_ISP_AE_SetConverge(u32DevId, RGB_AE_CHN,&stAeConvCondition);
    MI_ISP_AE_SetTarget(u32DevId, RGB_AE_CHN, &g_stLutTarget);
    MI_ISP_AE_QueryExposureInfo(u32DevId, RGB_AE_CHN, &g_stExpoInfo);

    g_s32Target    = g_stExpoInfo.u32SceneTarget;
    g_u32TargetPre = g_stExpoInfo.u32SceneTarget;
}

// 在稳定没有人脸的状况下 恢复api bin中的AE speed
void ST_RestoreAESpeedData(MI_U32 u32DevId)
{
    MI_ISP_AE_SetConverge(u32DevId, RGB_AE_CHN, &g_stRgbAeConvCondition);
}
