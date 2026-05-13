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

#include "mi_isp.h"
#include "mi_isp_cus3a_api.h"
#include "isp_cus3a_if.h"
#include "st_common_cus3a.h"

#define log_info 1

int ae_init(void* pdata, ISP_AE_INIT_PARAM *init_state)
{
    printf("****** ae_init ,shutter=%d,shutter_step=%d,sensor_gain_min=%d,sensor_gain_max=%d *******\n",
            init_state->shutter,
            init_state->shutter_step,
            init_state->sensor_gain,
            init_state->sensor_gain_max
          );
    return 0;
}

void ae_release(void* pdata)
{
    printf("************* ae_release *************\n");
}

void ae_run(void* pdata, const ISP_AE_INFO *info, ISP_AE_RESULT *result)
{
    unsigned int max = info->AvgBlkY*info->AvgBlkX;
    unsigned int avg=0;
    unsigned int n;

    result->Change              = 0;
    result->i4BVx16384          = 16384;
    result->HdrRatio            = 16384; //user define hdr exposure ratio
    result->HdrRatio1           = 1024;
    result->IspGain             = 1024;
    result->SensorGain          = 4096;
    result->Shutter             = 20000;
    result->IspGainHdrShort     = 1024;
    result->SensorGainHdrShort  = 1024;
    result->ShutterHdrShort     = 1000;
    //result->Size         = sizeof(CusAEResult_t);

    for(n=0;n<max;++n)
    {
        avg += info->avgs[n].y;
    }
    if (max > 0)
    {
        avg /= max;
    }

    result->AvgY         = avg;

    unsigned int y_lower = 0x48;
    unsigned int y_upper = 0x58;
    unsigned int change_ratio = 6; // percentage
    unsigned int Gain_Min = 1024*2;
    unsigned int Gain_Max = 1024*1000;
    unsigned int Shutter_Min = 150;
    unsigned int Shutter_Max = 33333;

    result->SensorGain = info->SensorGain;
    result->Shutter = info->Shutter;

    if(avg<y_lower){
        if (info->Shutter<Shutter_Max){
            result->Shutter = info->Shutter + (info->Shutter*change_ratio/100);
            if (result->Shutter > Shutter_Max) result->Shutter = Shutter_Max;
        }else{
            result->SensorGain = info->SensorGain + (info->SensorGain*change_ratio/100);
            if (result->SensorGain > Gain_Max) result->SensorGain = Gain_Max;
        }
        result->Change = 1;
    }else if(avg>y_upper){
        if (info->SensorGain>Gain_Min){
            result->SensorGain = info->SensorGain - (info->SensorGain*change_ratio/100);
            if (result->SensorGain < Gain_Min) result->SensorGain = Gain_Min;
        }else{
            result->Shutter = info->Shutter - (info->Shutter*change_ratio/100);
            if (result->Shutter < Shutter_Min) result->Shutter = Shutter_Min;
        }
        result->Change = 1;
    }
    #if log_info
        printf("Image avg = 0x%X \n", avg);
        printf("SensorGain: %d -> %d \n", info->SensorGain, result->SensorGain);
        printf("Shutter: %d -> %d \n", info->Shutter, result->Shutter);
    #endif
}

int awb_init(void *pdata)
{
    printf("************ awb_init **********\n");
    return 0;
}

void awb_run(void* pdata, const ISP_AWB_INFO *info, ISP_AWB_RESULT *result)
{
    static u32 count = 0;
    int avg_r = 0;
    int avg_g = 0;
    int avg_b = 0;
    int tar_rgain = 1024;
    int tar_bgain = 1024;
    int x = 0;
    int y = 0;

    result->R_gain = info->CurRGain;
    result->G_gain = info->CurGGain;
    result->B_gain = info->CurBGain;
    result->Change = 0;
    result->ColorTmp = 6000;

    if (++count % 4 == 0)
    {
        //center area YR/G/B avg
        for (y = 30; y<60; ++y)
        {
            for (x = 32; x<96; ++x)
            {
                avg_r += info->avgs[info->AvgBlkX*y + x].r;
                avg_g += info->avgs[info->AvgBlkX*y + x].g;
                avg_b += info->avgs[info->AvgBlkX*y + x].b;
            }
        }
        avg_r /= 30 * 64;
        avg_g /= 30 * 64;
        avg_b /= 30 * 64;

        if (avg_r <1)
            avg_r = 1;
        if (avg_g <1)
            avg_g = 1;
        if (avg_b <1)
            avg_b = 1;

#if log_info
        printf("AVG R / G / B = %d, %d, %d \n", avg_r, avg_g, avg_b);
#endif

        // calculate Rgain, Bgain
        tar_rgain = avg_g * 1024 / avg_r;
        tar_bgain = avg_g * 1024 / avg_b;

        if (tar_rgain > info->CurRGain) {
            if (tar_rgain - info->CurRGain < 384)
                result->R_gain = tar_rgain;
            else
                result->R_gain = info->CurRGain + (tar_rgain - info->CurRGain)/10;
        }else{
            if (info->CurRGain - tar_rgain < 384)
                result->R_gain = tar_rgain;
            else
                result->R_gain = info->CurRGain - (info->CurRGain - tar_rgain)/10;
        }

        if (tar_bgain > info->CurBGain) {
            if (tar_bgain - info->CurBGain < 384)
                result->B_gain = tar_bgain;
            else
                result->B_gain = info->CurBGain + (tar_bgain - info->CurBGain)/10;
        }else{
            if (info->CurBGain - tar_bgain < 384)
                result->B_gain = tar_bgain;
            else
                result->B_gain = info->CurBGain - (info->CurBGain - tar_bgain)/10;
        }

        result->Change = 1;
        result->G_gain = 1024;

#if log_info
        printf("[current] r=%d, g=%d, b=%d \n", info->CurRGain, info->CurGGain, info->CurBGain);
        printf("[result] r=%d, g=%d, b=%d \n", result->R_gain, result->G_gain, result->B_gain);
#endif
    }
}

void awb_release(void *pdata)
{
    printf("************ awb_release **********\n");
}

int af_init(void *pdata, ISP_AF_INIT_PARAM *param)
{
    MI_U32 u32ch = 0;

    printf("************ af_init **********\n");

#define USE_NORMAL_MODE 1

#ifdef USE_NORMAL_MODE
    //Init Normal mode setting
    static CusAFWin_t afwin =
    {
        AF_ROI_MODE_NORMAL,
        1,
        {{   0,    0,  255,  255},
        { 256,    0,  511,  255},
        { 512,    0,  767,  255},
        { 768,    0, 1023,  255},
        {   0,  256,  255,  511},
        { 256,  256,  511,  511},
        { 512,  256,  767,  511},
        { 768,  256, 1023,  511},
        {   0,  512,  255,  767},
        { 256,  512,  511,  767},
        { 512,  512,  767,  767},
        { 768,  512, 1023,  767},
        {   0,  768,  255, 1023},
        { 256,  768,  511, 1023},
        { 512,  768,  767, 1023},
        { 768,  768, 1023, 1023}}
    };

    MI_ISP_CUS3A_SetAFWindow(E_ISP_DEV_0, u32ch, &afwin);

#else
    //Init Matrix mode setting
    static CusAFWin_t afwin =
    {
        //full image, equal divide to 16x16
        //window setting need to multiple of two
        AF_ROI_MODE_MATRIX,
        16,
        {{0, 0, 62, 62},
        {64, 64, 126, 126},
        {128, 128, 190, 190},
        {192, 192, 254, 254},
        {256, 256, 318, 318},
        {320, 320, 382, 382},
        {384, 384, 446, 446},
        {448, 448, 510, 510},
        {512, 512, 574, 574},
        {576, 576, 638, 638},
        {640, 640, 702, 702},
        {704, 704, 766, 766},
        {768, 768, 830, 830},
        {832, 832, 894, 894},
        {896, 896, 958, 958},
        {960, 960, 1022, 1022}}

        /*
        //use two row only => 16x2 win
        //and set taf_roimode.u32_vertical_block_number = 2
        AF_ROI_MODE_MATRIX,
        2,
        {{0, 0, 62, 62},
        {64, 64, 126, 126},
        {128, 0, 190, 2},      //win2 v_str, v_end doesn't use, set to (0, 2)
        {192, 0, 254, 2},
        {256, 0, 318, 2},
        {320, 0, 382, 2},
        {384, 0, 446, 2},
        {448, 0, 510, 2},
        {512, 0, 574, 2},
        {576, 0, 638, 2},
        {640, 0, 702, 2},
        {704, 0, 766, 2},
        {768, 0, 830, 2},
        {832, 0, 894, 2},
        {896, 0, 958, 2},
        {960, 0, 1022, 2}}
        */
    };

        MI_ISP_CUS3A_SetAFWindow(E_ISP_DEV_0, u32ch, &afwin);

#endif

    //set AF Filter
    static CusAFFilter_t affilter =
    {
        //filter setting with sign value
        //{s9, s10, s9, s7, s7}

        //[0.3~0.6] : 20, 37, 20, -68, 53; 20, -39, 20, 34, 51; 26, 0, -26, -18, 38;
        //[0.2~0.5] : 20, 35, 20, -96, 55; 20, -39, 20, -1, 36; 26, 0, -26, -52, 38;
        //[0.1~0.6]  : 37, 0, -37, -107, 49; 37, 0, -37, 25, 28; 32, 0, -32, -41, 0;
        //[0.08~0.24]: 13, 11, 13, -87, 54; 13, -26, 13, -120, 60; 15, 0, -15, -103, 50;
        //[0.03~0.25]: 19, 0, -19, -122, 59; 19, 0, -19, -73, 36; 17, 0, -17, -91, 30;
        //[0.07~0.1]: 8, -13, 8, -120, 62; 8, -16, 8, -124, 63; 3, 0, -3, -121, 61;

        //here use [0.3~0.6] & [0.08~0.24]
        //convert to hw format (sign bit with msb)
        20, 37, 20, 68+128, 53, 0, 1023, 0, 1023,
        13, 11, 13, 87+128, 54, 0, 1023, 0, 1023,
        1, 20, 39+1024, 20, 34, 51, 1, 26, 0, 26+512, 18+128, 38,
        1, 13, 26+1024, 13,  120+128, 60, 1, 15, 0, 15+512,  103+128, 50,
    };
    MI_ISP_CUS3A_SetAFFilter(E_ISP_DEV_0, 0, &affilter);

    //set AF Sq
    CusAFFilterSq_t sq = {
        .bSobelYSatEn = 0,
        .u16SobelYThd = 1023,
        .bIIRSquareAccEn = 1,
        .bSobelSquareAccEn = 0,
        .u16IIR1Thd = 0,
        .u16IIR2Thd = 0,
        .u16SobelHThd = 0,
        .u16SobelVThd = 0,
        .u8AFTbl1X = {6,7,7,6,6,6,7,6,6,7,6,6,},
        .u16AFTbl1Y = {0,32,288,800,1152,1568,2048,3200,3872,4607,6271,7199,8191},
        .u8AFTbl2X = {6,7,7,6,6,6,7,6,6,7,6,6,},
        .u16AFTbl2Y = {0,32,288,800,1152,1568,2048,3200,3872,4607,6271,7199,8191},
    };


    MI_ISP_CUS3A_SetAFFilterSq(E_ISP_DEV_0, 0, &sq);
    printf("****[%s] af_init done ****\n", __FUNCTION__);

    return 0;
}

void af_release(void *pdata)
{
    printf("****[%s] af_release ****\n", __FUNCTION__);
}

/*
    MI_U8 iir_1[5*16];  //[5]: iir 37bit, use 5*u8 datatype,     [16]: 16wins
    MI_U8 iir_2[5*16];  //[5]: iir 37bit, use 5*u8 datatype,     [16]: 16wins
    MI_U8 luma[5*16];   //[5]: luma 34bit, use 5*u8 datatype, [16]: 16wins
    MI_U8 fir_v[5*16];  //[5]: fir 37bit, use 5*u8 datatype,     [16]: 16wins
    MI_U8 fir_h[5*16];  //[5]: fir 37bit, use 5*u8 datatype,     [16]: 16wins
    MI_U8 ysat[3*16];   //[3]: ysat 24bit, use 3*u8 datatype,  [16]: 16wins
*/

void af_run(void *pdata, const ISP_AF_INFO *af_info, ISP_AF_RESULT *result)
{
#if 1
    int i = 0, x = 0;

    //printf("\n\n");

    //print row0 16wins
    x = 0;
    for (i = 0; i < 16; i++)
    {
        printf("\n[AF]win%d-%d iir0:0x%02x%02x%02x%02x%02x, iir1:0x%02x%02x%02x%02x%02x, luma:0x%02x%02x%02x%02x%02x, sobelh:0x%02x%02x%02x%02x%02x, sobelv:0x%02x%02x%02x%02x%02x ysat:0x%02x%02x%02x",
               x, i,
               af_info->pStats->stParaAPI[x].high_iir[4 + i * 5], af_info->pStats->stParaAPI[x].high_iir[3 + i * 5], af_info->pStats->stParaAPI[x].high_iir[2 + i * 5], af_info->pStats->stParaAPI[x].high_iir[1 + i * 5], af_info->pStats->stParaAPI[x].high_iir[0 + i * 5],
               af_info->pStats->stParaAPI[x].low_iir[4 + i * 5], af_info->pStats->stParaAPI[x].low_iir[3 + i * 5], af_info->pStats->stParaAPI[x].low_iir[2 + i * 5], af_info->pStats->stParaAPI[x].low_iir[1 + i * 5], af_info->pStats->stParaAPI[x].low_iir[0 + i * 5],
               af_info->pStats->stParaAPI[x].luma[4 + i * 5], af_info->pStats->stParaAPI[x].luma[3 + i * 5], af_info->pStats->stParaAPI[x].luma[2 + i * 5], af_info->pStats->stParaAPI[x].luma[1 + i * 5], af_info->pStats->stParaAPI[x].luma[0 + i * 5],
               af_info->pStats->stParaAPI[x].sobel_h[4 + i * 5], af_info->pStats->stParaAPI[x].sobel_h[3 + i * 5], af_info->pStats->stParaAPI[x].sobel_h[2 + i * 5], af_info->pStats->stParaAPI[x].sobel_h[1 + i * 5], af_info->pStats->stParaAPI[x].sobel_h[0 + i * 5],
               af_info->pStats->stParaAPI[x].sobel_v[4 + i * 5], af_info->pStats->stParaAPI[x].sobel_v[3 + i * 5], af_info->pStats->stParaAPI[x].sobel_v[2 + i * 5], af_info->pStats->stParaAPI[x].sobel_v[1 + i * 5], af_info->pStats->stParaAPI[x].sobel_v[0 + i * 5],
               af_info->pStats->stParaAPI[x].ysat[2 + i * 3], af_info->pStats->stParaAPI[x].ysat[1 + i * 3], af_info->pStats->stParaAPI[x].ysat[0 + i * 3]
              );
    }
    printf("\n");
#endif
}

int af_ctrl(void *pdata, ISP_AF_CTRL_CMD cmd, void* param)
{
    return 0;
}

int g_Cus3AInited = 0;
void St_Common_EnableCUS3A(int IspdevId, int IspChnId, MI_BOOL bAEenable, MI_BOOL bAWBenable, MI_BOOL bAFenable)
{
    ISP_AE_INTERFACE tAeIf;
    ISP_AWB_INTERFACE tAwbIf;
    ISP_AF_INTERFACE tAfIf;
    ISP_AE_INTERFACE *pAeIf = NULL;
    ISP_AWB_INTERFACE *pAwbIf = NULL;
    ISP_AF_INTERFACE *pAfIf = NULL;
    static int IqModInfo[2];

    printf("== sample CUS3A ENABLE == AE[%d] AWB[%d] AF[%d]\n",bAEenable,bAWBenable,bAFenable);

    if(!g_Cus3AInited)
    {
        CUS3A_Init(IspdevId);
        g_Cus3AInited = 1;
    }

    if(bAEenable)
    {
        /*AE*/
        IqModInfo[0] = IspdevId;
        IqModInfo[1] = IspChnId;
        tAeIf.ctrl = NULL;
        tAeIf.pdata = IqModInfo;
        tAeIf.init = ae_init;
        tAeIf.release = ae_release;
        tAeIf.run = ae_run;
        pAeIf = &tAeIf;
        CUS3A_RegInterfaceEX((CUS3A_ISP_DEV_e)IspdevId, (CUS3A_ISP_CH_e)IspChnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AE, pAeIf);
        CUS3A_SetAlgoAdaptor((CUS3A_ISP_DEV_e)IspdevId, (CUS3A_ISP_CH_e)IspChnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AE);
    }
    if(bAWBenable)
    {
        /*AWB*/
        IqModInfo[0] = IspdevId;
        IqModInfo[1] = IspChnId;
        tAwbIf.ctrl = NULL;
        tAwbIf.pdata = IqModInfo;
        tAwbIf.init = awb_init;
        tAwbIf.release = awb_release;
        tAwbIf.run = awb_run;
        pAwbIf = &tAwbIf;
        CUS3A_RegInterfaceEX((CUS3A_ISP_DEV_e)IspdevId, (CUS3A_ISP_CH_e)IspChnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AWB, pAwbIf);
        CUS3A_SetAlgoAdaptor((CUS3A_ISP_DEV_e)IspdevId, (CUS3A_ISP_CH_e)IspChnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AWB);
    }
    if(bAFenable)
    {
        /*AF*/
        IqModInfo[0] = IspdevId;
        IqModInfo[1] = IspChnId;
        tAfIf.pdata = IqModInfo;
        tAfIf.init = af_init;
        tAfIf.release = af_release;
        tAfIf.run = af_run;
        tAfIf.ctrl = af_ctrl;
        pAfIf = &tAfIf;
        CUS3A_RegInterfaceEX((CUS3A_ISP_DEV_e)IspdevId, (CUS3A_ISP_CH_e)IspChnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AF, pAfIf);  //if no need ,set algo to NULL
        CUS3A_SetAlgoAdaptor((CUS3A_ISP_DEV_e)IspdevId, (CUS3A_ISP_CH_e)IspChnId, E_ALGO_ADAPTOR_1, E_ALGO_TYPE_AF);
    }

    return;
}

void St_Common_ReleaseCUS3A(int IspdevId)
{
    /*Release CUS3A*/
    if(g_Cus3AInited)
    {
        CUS3A_Release(IspdevId);
        g_Cus3AInited = 0;
    }
}
