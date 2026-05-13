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
#include <unistd.h>
#include <string.h>

#include "st_common_fdae.h"
#include "st_common.h"
#include "platform.h"


static ST_FDAE_PARA_t g_fdae_para[] =
{
 [0] =
    {
     .u32face_luma_target = 130,
     .u8face_tolerance    = 20,
     .u8face_luma_step    = 20,
     .stAEtarget_Storage  = {0},
     .bAE_INIT  = FALSE,
     .bResetAE  = FALSE,
     .afwin               = {0},
     .af_info             = {{{{0}}}},
    },
 [1] =
   {
    .u32face_luma_target = 130,
    .u8face_tolerance    = 20,
    .u8face_luma_step    = 20,
    .stAEtarget_Storage  = {0},
    .bAE_INIT  = FALSE,
    .bResetAE  = FALSE,
    .afwin               = {0},
    .af_info             = {{{{0}}}},
   }
};


void ST_FDAE_Init3A(ST_FDAE_PARA_t** ppstFdae_para, MI_U32 u32Index)
{
       *ppstFdae_para =  &g_fdae_para[u32Index];
}

void ST_FDAE_Adjust3A(MI_U32 DevId, MI_U32 Channel, ST_FDAE_PARA_t* pstFdae_para, ST_FDAE_DtBox_t stBox, MI_U32 u32SensorFrameCount){

        MI_U8 u32face_luma_target = pstFdae_para->u32face_luma_target;
        MI_U8 u8face_tolerance    = pstFdae_para->u8face_tolerance;
        MI_U8 u8face_luma_step    = pstFdae_para->u8face_luma_step;
        MI_ISP_AE_IntpLutType_t*  pstAEtarget_Storage = &pstFdae_para->stAEtarget_Storage;
        MI_BOOL* pbResetAE        = &pstFdae_para->bResetAE;
        MI_BOOL* pbAE_INIT        = &pstFdae_para->bAE_INIT;
        CusAFWin_t*   afwin       = &pstFdae_para->afwin;
        CusAFStats_t* af_info     = &pstFdae_para->af_info;

        MI_ISP_AE_IntpLutType_t AEtarget;
        MI_S32 s32ae_step = 0;
        MI_S32 s32ae_boundary = 0;
        MI_U8 u8FrameCount = 0;

        // if AE not init, save first AE state
        if(! (*pbAE_INIT) ) {
          *pbAE_INIT = TRUE;
           MI_ISP_AE_GetTarget(DevId, Channel, pstAEtarget_Storage);
        }

        u8FrameCount = (MI_U8)(u32SensorFrameCount % 2);

        if(u8FrameCount == 0)
        {
            if(stBox.fScore != -1)
            {
            // Let Ae reset if no face detection next time.
            *pbResetAE = TRUE;
            /////////////////////////////////////
            // AF window
            /////////////////////////////////////
            afwin->stParaAPI[0].u32StartX = (MI_U32)stBox.u16Xmin;
            afwin->stParaAPI[0].u32StartY = (MI_U32)stBox.u16Ymin;
            afwin->stParaAPI[0].u32EndX = (MI_U32)stBox.u16Xmax;
            afwin->stParaAPI[0].u32EndY = (MI_U32)stBox.u16Ymax;
            afwin->mode = AF_ROI_MODE_NORMAL;
            afwin->u32_vertical_block_number = 1;
            MI_ISP_CUS3A_SetAFWindow(DevId, Channel, afwin);
            MI_ISP_CUS3A_GetAFStats(DevId, Channel, af_info);

            /////////////////////////////////////
            // AE
            /////////////////////////////////////
            // get luma value and average by area
            MI_U32 *u32luma = (MI_U32*)(&(af_info->stParaAPI[0].luma[0]));
            *u32luma = *u32luma / ((MI_U32)(stBox.u32Area));
            //printf(" *u32luma =  %d target %d tol %d\n",*u32luma,u32face_luma_target,u8face_tolerance);

            if(*u32luma > (u32face_luma_target + u8face_tolerance))
            {
                s32ae_step = -(((*u32luma - u32face_luma_target + 5)/5)*5);
                if(s32ae_step < -u8face_luma_step)
                    s32ae_step = -u8face_luma_step;
            }
            else if (*u32luma < (u32face_luma_target - u8face_tolerance))
            {
                s32ae_step = ((u32face_luma_target - *u32luma + 5)/5)*5;
                if(s32ae_step > u8face_luma_step)
                    s32ae_step = u8face_luma_step;
            }
            else{
                s32ae_step = 0;
            }
            MI_ISP_AE_GetTarget(DevId, Channel, &AEtarget);

            //printf(" Before AETarget %d\n",AEtarget.u32Y[0]);
            if(s32ae_step!=0)
            {
                 for(MI_S32 s32i = 0;  s32i< AEtarget.u16NumOfPoints; s32i++)
                 {
                        AEtarget.u32Y[s32i]= AEtarget.u32Y[s32i] + s32ae_step;
                        if ( (MI_S32)AEtarget.u32Y[s32i] < 225 )
                        {
                            if((AEtarget.u32Y[s32i] - s32ae_step)== 225)
                            {
                                s32ae_boundary = 1;
                            }
                            AEtarget.u32Y[s32i] = 225;
                        }
                        if ( (MI_S32)AEtarget.u32Y[s32i] > (2200))
                        {
                            if((AEtarget.u32Y[s32i] - s32ae_step)== (2200))
                            {
                               s32ae_boundary = 1;
                            }
                            AEtarget.u32Y[s32i] = (470 * 2);
                        }
                    }

                    if(s32ae_boundary == 0)
                    {
                        MI_ISP_AE_SetTarget(DevId, Channel, &AEtarget);
                    }
                }
                //printf(" After AETarget %d\n",AEtarget.u32Y[0]);
                }
                else
                {
                        // Have yet to detect face and storage first ae
                        if(*pbResetAE)
                        {
                            MI_ISP_AE_SetTarget(DevId, Channel, pstAEtarget_Storage);
                            *pbResetAE = FALSE;
                        }
                        //printf(" NoFace AETarget %d\n",AEtarget.u32Y[0]);
                }
    }

}


