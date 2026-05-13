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

#include "st_common_venc.h"
#include "st_common.h"
#include "platform.h"

MI_S32 ST_Common_GetVencDefaultDevAttr(MI_VENC_InitParam_t *pstInitParam)
{
    ST_CHECK_POINTER(pstInitParam);
    memset(pstInitParam, 0x00, sizeof(MI_VENC_InitParam_t));

    pstInitParam->u32MaxWidth  = 2560;
    pstInitParam->u32MaxHeight = 1440;

    return 0;
}

MI_S32 ST_Common_GetVencDefaultChnAttr(MI_VENC_ModType_e eType, MI_VENC_ChnAttr_t *pstVencChnAttr)
{
    ST_CHECK_POINTER(pstVencChnAttr);
    memset(pstVencChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));

    if (eType == E_MI_VENC_MODTYPE_H264E)
    {
        pstVencChnAttr->stVeAttr.stAttrH264e.u32PicWidth         = 1920;
        pstVencChnAttr->stVeAttr.stAttrH264e.u32PicHeight        = 1080;
        pstVencChnAttr->stVeAttr.stAttrH264e.u32MaxPicWidth      = 2560;
        pstVencChnAttr->stVeAttr.stAttrH264e.u32MaxPicHeight     = 1440;
        pstVencChnAttr->stVeAttr.stAttrH264e.u32BFrameNum        = 2;
        pstVencChnAttr->stVeAttr.stAttrH264e.bByFrame            = TRUE;
        pstVencChnAttr->stVeAttr.stAttrH264e.u32Profile          = 2;
        pstVencChnAttr->stVeAttr.stAttrH264e.u32BufSize          = 1024 * 1024;
        pstVencChnAttr->stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_H264CBR;
        pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32BitRate        = 2000000;
        pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
        pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32Gop            = 30;
        pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum  = 30;
        pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen  = 1;
        pstVencChnAttr->stRcAttr.stAttrH264Cbr.u32StatTime       = 0;
        pstVencChnAttr->stVeAttr.eType                           = E_MI_VENC_MODTYPE_H264E;
    }
    else if (eType == E_MI_VENC_MODTYPE_H265E)
    {
        pstVencChnAttr->stVeAttr.stAttrH265e.u32PicWidth         = 1920;
        pstVencChnAttr->stVeAttr.stAttrH265e.u32PicHeight        = 1080;
        pstVencChnAttr->stVeAttr.stAttrH265e.u32MaxPicWidth      = 2560;
        pstVencChnAttr->stVeAttr.stAttrH265e.u32MaxPicHeight     = 1440;
        pstVencChnAttr->stVeAttr.stAttrH265e.bByFrame            = TRUE;
        pstVencChnAttr->stVeAttr.stAttrH265e.u32BufSize          = 1024 * 1024;
        pstVencChnAttr->stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_H265CBR;
        pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32BitRate        = 2000000;
        pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum  = 30;
        pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen  = 1;
        pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32Gop            = 30;
        pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
        pstVencChnAttr->stRcAttr.stAttrH265Cbr.u32StatTime       = 0;
        pstVencChnAttr->stVeAttr.eType                           = E_MI_VENC_MODTYPE_H265E;
    }
    else if (eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        pstVencChnAttr->stVeAttr.stAttrJpeg.u32PicWidth              = 1920;
        pstVencChnAttr->stVeAttr.stAttrJpeg.u32PicHeight             = 1080;
        pstVencChnAttr->stVeAttr.stAttrJpeg.u32MaxPicWidth           = 2560;
        pstVencChnAttr->stVeAttr.stAttrJpeg.u32MaxPicHeight          = 1440;
        pstVencChnAttr->stVeAttr.stAttrJpeg.bByFrame                 = true;
        pstVencChnAttr->stVeAttr.stAttrJpeg.u32RestartMakerPerRowCnt = 0;
        pstVencChnAttr->stVeAttr.stAttrJpeg.u32BufSize               = 1024 * 1024;
        pstVencChnAttr->stVeAttr.stAttrJpeg.bSupportDCF              = FALSE;
        pstVencChnAttr->stRcAttr.eRcMode                             = E_MI_VENC_RC_MODE_MJPEGFIXQP;
        pstVencChnAttr->stRcAttr.stAttrMjpegFixQp.u32Qfactor         = 50;
        pstVencChnAttr->stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum   = 30;
        pstVencChnAttr->stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen   = 1;
        pstVencChnAttr->stVeAttr.eType                               = E_MI_VENC_MODTYPE_JPEGE;
    }
#if VENC_SUPPORT_AV1
    else if (eType == E_MI_VENC_MODTYPE_AV1)
    {
        pstVencChnAttr->stVeAttr.stAttrAv1.u32PicWidth          = 1920;
        pstVencChnAttr->stVeAttr.stAttrAv1.u32PicHeight         = 1080;
        pstVencChnAttr->stVeAttr.stAttrAv1.u32MaxPicWidth       = 2560;
        pstVencChnAttr->stVeAttr.stAttrAv1.u32MaxPicHeight      = 1440;
        pstVencChnAttr->stVeAttr.stAttrAv1.bByFrame             = TRUE;
        pstVencChnAttr->stVeAttr.stAttrAv1.u32RefNum            = 2;
        pstVencChnAttr->stVeAttr.stAttrAv1.u32Profile           = 2;
        pstVencChnAttr->stVeAttr.stAttrAv1.u32BufSize           = 1024 * 1024;
        pstVencChnAttr->stVeAttr.stAttrAv1.bEnableSwitchFrame   = 0;
        pstVencChnAttr->stRcAttr.eRcMode                        = E_MI_VENC_RC_MODE_AV1CBR;
        pstVencChnAttr->stRcAttr.stAttrAv1Cbr.u32BitRate        = 2000000;
        pstVencChnAttr->stRcAttr.stAttrAv1Cbr.u32FluctuateLevel = 0;
        pstVencChnAttr->stRcAttr.stAttrAv1Cbr.u32Gop            = 30;
        pstVencChnAttr->stRcAttr.stAttrAv1Cbr.u32SrcFrmRateNum  = 30;
        pstVencChnAttr->stRcAttr.stAttrAv1Cbr.u32SrcFrmRateDen  = 1;
        pstVencChnAttr->stRcAttr.stAttrAv1Cbr.u32StatTime       = 0;
        pstVencChnAttr->stVeAttr.eType                          = E_MI_VENC_MODTYPE_AV1;
    }
#endif
    return 0;
}

MI_S32 ST_Common_VencCreateDev(MI_VENC_DEV DevId, MI_VENC_InitParam_t *pstInitParam)
{
    MI_S32 s32Ret = MI_SUCCESS;
    ST_CHECK_POINTER(pstInitParam);
    s32Ret |= MI_VENC_CreateDev(DevId, pstInitParam);

    return s32Ret;
}

MI_S32 ST_Common_VencDestroyDev(MI_VENC_DEV DevId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_VENC_DestroyDev(DevId);

    return s32Ret;
}

MI_S32 ST_Common_VencStartChn(MI_VENC_DEV DevId, MI_VENC_CHN VencChn, MI_VENC_ChnAttr_t *pstVencChnAttr)
{
    MI_S32            s32Ret = MI_SUCCESS;
    MI_VENC_ChnAttr_t stChnAttr;
    ST_CHECK_POINTER(pstVencChnAttr);
    MI_VENC_ModType_e eType = pstVencChnAttr->stVeAttr.eType;
    memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));

    if (eType == E_MI_VENC_MODTYPE_H264E)
    {
        stChnAttr.stVeAttr.stAttrH264e.u32PicWidth     = ALIGN_UP(pstVencChnAttr->stVeAttr.stAttrH264e.u32PicWidth, 2);
        stChnAttr.stVeAttr.stAttrH264e.u32PicHeight    = ALIGN_UP(pstVencChnAttr->stVeAttr.stAttrH264e.u32PicHeight, 2);
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth  = pstVencChnAttr->stVeAttr.stAttrH264e.u32MaxPicWidth;
        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = pstVencChnAttr->stVeAttr.stAttrH264e.u32MaxPicHeight;
        stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum    = pstVencChnAttr->stVeAttr.stAttrH264e.u32BFrameNum;
        stChnAttr.stVeAttr.stAttrH264e.u32BufSize      = pstVencChnAttr->stVeAttr.stAttrH264e.u32BufSize;
        stChnAttr.stVeAttr.stAttrH264e.bByFrame        = pstVencChnAttr->stVeAttr.stAttrH264e.bByFrame;
        stChnAttr.stVeAttr.stAttrH264e.u32Profile      = pstVencChnAttr->stVeAttr.stAttrH264e.u32Profile;
        stChnAttr.stRcAttr.eRcMode                     = pstVencChnAttr->stRcAttr.eRcMode;
        switch (stChnAttr.stRcAttr.eRcMode)
        {
            case E_MI_VENC_RC_MODE_H264CBR:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrH264Cbr, &pstVencChnAttr->stRcAttr.stAttrH264Cbr,
                       sizeof(MI_VENC_AttrH264Cbr_t));
            }
            break;
            case E_MI_VENC_RC_MODE_H264FIXQP:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrH264FixQp, &pstVencChnAttr->stRcAttr.stAttrH264FixQp,
                       sizeof(MI_VENC_AttrH264FixQp_t));
            }
            break;
            case E_MI_VENC_RC_MODE_H264VBR:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrH264Vbr, &pstVencChnAttr->stRcAttr.stAttrH264Vbr,
                       sizeof(MI_VENC_AttrH264Vbr_t));
            }
            break;
            case E_MI_VENC_RC_MODE_H264AVBR:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrH264Avbr, &pstVencChnAttr->stRcAttr.stAttrH264Avbr,
                       sizeof(MI_VENC_AttrH264Avbr_t));
            }
            break;
            default:
            {
                printf("RC Mode %d not support", stChnAttr.stRcAttr.eRcMode);
                return -1;
            }
            break;
        }
    }
    else if (eType == E_MI_VENC_MODTYPE_H265E)
    {
        stChnAttr.stVeAttr.stAttrH265e.u32PicWidth     = ALIGN_UP(pstVencChnAttr->stVeAttr.stAttrH265e.u32PicWidth, 2);
        stChnAttr.stVeAttr.stAttrH265e.u32PicHeight    = ALIGN_UP(pstVencChnAttr->stVeAttr.stAttrH265e.u32PicHeight, 2);
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth  = pstVencChnAttr->stVeAttr.stAttrH265e.u32MaxPicWidth;
        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = pstVencChnAttr->stVeAttr.stAttrH265e.u32MaxPicHeight;
        stChnAttr.stVeAttr.stAttrH265e.u32BufSize      = pstVencChnAttr->stVeAttr.stAttrH265e.u32BufSize;
        stChnAttr.stVeAttr.stAttrH265e.bByFrame        = pstVencChnAttr->stVeAttr.stAttrH265e.bByFrame;
        stChnAttr.stRcAttr.eRcMode                     = pstVencChnAttr->stRcAttr.eRcMode;
        switch (stChnAttr.stRcAttr.eRcMode)
        {
            case E_MI_VENC_RC_MODE_H265CBR:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrH265Cbr, &pstVencChnAttr->stRcAttr.stAttrH265Cbr,
                       sizeof(MI_VENC_AttrH265Cbr_t));
            }
            break;
            case E_MI_VENC_RC_MODE_H265FIXQP:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrH265FixQp, &pstVencChnAttr->stRcAttr.stAttrH265FixQp,
                       sizeof(MI_VENC_AttrH265FixQp_t));
            }
            break;
            case E_MI_VENC_RC_MODE_H265VBR:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrH265Vbr, &pstVencChnAttr->stRcAttr.stAttrH265Vbr,
                       sizeof(MI_VENC_AttrH265Vbr_t));
            }
            break;
            case E_MI_VENC_RC_MODE_H265AVBR:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrH265Avbr, &pstVencChnAttr->stRcAttr.stAttrH265Avbr,
                       sizeof(MI_VENC_AttrH265Avbr_t));
            }
            break;
            default:
            {
                printf("RC Mode %d not support", stChnAttr.stRcAttr.eRcMode);
                return -1;
            }
            break;
        }
    }
    else if (eType == E_MI_VENC_MODTYPE_JPEGE)
    {
        stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth     = ALIGN_UP(pstVencChnAttr->stVeAttr.stAttrJpeg.u32PicWidth, 2);
        stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight    = ALIGN_UP(pstVencChnAttr->stVeAttr.stAttrJpeg.u32PicHeight, 2);
        stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth  = pstVencChnAttr->stVeAttr.stAttrJpeg.u32MaxPicWidth;
        stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = pstVencChnAttr->stVeAttr.stAttrJpeg.u32MaxPicHeight;
        stChnAttr.stVeAttr.stAttrJpeg.u32BufSize      = pstVencChnAttr->stVeAttr.stAttrJpeg.u32BufSize;
        stChnAttr.stVeAttr.stAttrJpeg.bByFrame        = pstVencChnAttr->stVeAttr.stAttrJpeg.bByFrame;
        stChnAttr.stVeAttr.stAttrJpeg.bSupportDCF     = pstVencChnAttr->stVeAttr.stAttrJpeg.bSupportDCF;
        stChnAttr.stVeAttr.stAttrJpeg.u32RestartMakerPerRowCnt =
            pstVencChnAttr->stVeAttr.stAttrJpeg.u32RestartMakerPerRowCnt;
        stChnAttr.stRcAttr.eRcMode = pstVencChnAttr->stRcAttr.eRcMode;
        switch (stChnAttr.stRcAttr.eRcMode)
        {
            case E_MI_VENC_RC_MODE_MJPEGFIXQP:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrMjpegFixQp, &pstVencChnAttr->stRcAttr.stAttrMjpegFixQp,
                       sizeof(MI_VENC_AttrMjpegFixQp_t));
            }
            break;
            case E_MI_VENC_RC_MODE_MJPEGCBR:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrMjpegCbr, &pstVencChnAttr->stRcAttr.stAttrMjpegCbr,
                       sizeof(MI_VENC_AttrMjpegCbr_t));
            }
            break;
            case E_MI_VENC_RC_MODE_MJPEGVBR:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrMjpegVbr, &pstVencChnAttr->stRcAttr.stAttrMjpegVbr,
                       sizeof(MI_VENC_AttrMjpegVbr_t));
            }
            break;
            default:
            {
                printf("RC Mode %d not support", stChnAttr.stRcAttr.eRcMode);
                return -1;
            }
            break;
        }
    }
#if VENC_SUPPORT_AV1
    else if (eType == E_MI_VENC_MODTYPE_AV1)
    {
        stChnAttr.stVeAttr.stAttrAv1.u32PicWidth        = ALIGN_UP(pstVencChnAttr->stVeAttr.stAttrAv1.u32PicWidth, 2);
        stChnAttr.stVeAttr.stAttrAv1.u32PicHeight       = ALIGN_UP(pstVencChnAttr->stVeAttr.stAttrAv1.u32PicHeight, 2);
        stChnAttr.stVeAttr.stAttrAv1.u32MaxPicWidth     = pstVencChnAttr->stVeAttr.stAttrAv1.u32MaxPicWidth;
        stChnAttr.stVeAttr.stAttrAv1.u32MaxPicHeight    = pstVencChnAttr->stVeAttr.stAttrAv1.u32MaxPicHeight;
        stChnAttr.stVeAttr.stAttrAv1.u32BufSize         = pstVencChnAttr->stVeAttr.stAttrAv1.u32BufSize;
        stChnAttr.stVeAttr.stAttrAv1.bByFrame           = pstVencChnAttr->stVeAttr.stAttrAv1.bByFrame;
        stChnAttr.stVeAttr.stAttrAv1.u32Profile         = pstVencChnAttr->stVeAttr.stAttrAv1.u32Profile;
        stChnAttr.stVeAttr.stAttrAv1.bEnableSwitchFrame = pstVencChnAttr->stVeAttr.stAttrAv1.bEnableSwitchFrame;
        stChnAttr.stRcAttr.eRcMode                      = pstVencChnAttr->stRcAttr.eRcMode;
        switch (stChnAttr.stRcAttr.eRcMode)
        {
            case E_MI_VENC_RC_MODE_AV1CBR:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrAv1Cbr, &pstVencChnAttr->stRcAttr.stAttrAv1Cbr,
                       sizeof(MI_VENC_AttrAv1Cbr_t));
            }
            break;
            case E_MI_VENC_RC_MODE_AV1FIXQP:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrAv1FixQp, &pstVencChnAttr->stRcAttr.stAttrAv1FixQp,
                       sizeof(MI_VENC_AttrAv1FixQp_t));
            }
            break;
            case E_MI_VENC_RC_MODE_AV1VBR:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrAv1Vbr, &pstVencChnAttr->stRcAttr.stAttrAv1Vbr,
                       sizeof(MI_VENC_AttrAv1Vbr_t));
            }
            break;
            case E_MI_VENC_RC_MODE_AV1AVBR:
            {
                memcpy(&stChnAttr.stRcAttr.stAttrAv1Avbr, &pstVencChnAttr->stRcAttr.stAttrAv1Avbr,
                       sizeof(MI_VENC_AttrAv1Avbr_t));
            }
            break;
            default:
            {
                printf("RC Mode %d not support", stChnAttr.stRcAttr.eRcMode);
                return -1;
            }
            break;
        }
    }
#endif

    stChnAttr.stVeAttr.eType = pstVencChnAttr->stVeAttr.eType;
    s32Ret |= MI_VENC_CreateChn(DevId, VencChn, &stChnAttr);

    s32Ret |= MI_VENC_StartRecvPic(DevId, VencChn);

    return s32Ret;
}

MI_S32 ST_Common_VencStopChn(MI_VENC_DEV DevId, MI_VENC_CHN VencChn)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_VENC_StopRecvPic(DevId, VencChn);

    s32Ret |= MI_VENC_DestroyChn(DevId, VencChn);

    return s32Ret;
}
