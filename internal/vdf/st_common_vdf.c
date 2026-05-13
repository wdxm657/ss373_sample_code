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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
#include "mi_md.h"
#include "mi_od.h"
#include "mi_vg.h"
#include "mi_vdf.h"
#include "mi_sys.h"
#include "st_common.h"
#include "st_common_vdf.h"
#include "mi_sys_datatype.h"
#include "mi_vdf_datatype.h"


MI_S32 ST_Common_VdfCheckAlign(MI_U16 mb_size, MI_U16 stride)
{
    MI_U16 u16BufHAlignment;
    MDMB_MODE_e mbSize = (MDMB_MODE_e)mb_size;
    switch (mbSize)
    {
        case MDMB_MODE_MB_8x8:
            u16BufHAlignment = 16;   /* width */
            break;

        case MDMB_MODE_MB_16x16:
            u16BufHAlignment = 16;   /* width */
            break;

        case MDMB_MODE_MB_4x4:
        default:
            u16BufHAlignment = 16;   /* width */
            break;
    }

    if (stride % u16BufHAlignment)
    {
        ST_ERR("stride(%u) not align up (%u), please check input param\n", stride, u16BufHAlignment);
        return -1;
    }
    return 0;
}


MI_S32 ST_Common_GetVdfMdDefaultChnAttr(MI_VDF_ChnAttr_t *pstAttr, MI_U16 Width, MI_U16 Height)
{
    ST_CHECK_POINTER(pstAttr);

    pstAttr->enWorkMode = E_MI_VDF_WORK_MODE_MD;
    pstAttr->stMdAttr.u8Enable      = 1;
    pstAttr->stMdAttr.u8MdBufCnt  = 4;
    pstAttr->stMdAttr.u8VDFIntvl  = 0;
    pstAttr->stMdAttr.ccl_ctrl.u16InitAreaThr = 8;
    pstAttr->stMdAttr.ccl_ctrl.u16Step = 2;
    pstAttr->stMdAttr.stMdDynamicParamsIn.sensitivity = 80;
    pstAttr->stMdAttr.stMdDynamicParamsIn.learn_rate = 128;
    pstAttr->stMdAttr.stMdDynamicParamsIn.md_thr = 50;
    pstAttr->stMdAttr.stMdDynamicParamsIn.obj_num_max = 0;
    pstAttr->stMdAttr.stMdStaticParamsIn.width     = ALIGN_UP(Width, 16);//720;//g_width;
    pstAttr->stMdAttr.stMdStaticParamsIn.height  = ALIGN_UP(Height, 2); //576;//g_height;
    pstAttr->stMdAttr.stMdStaticParamsIn.stride  = ALIGN_UP(Width, 16);//must < divp width & 16 align
    pstAttr->stMdAttr.stMdStaticParamsIn.color     = 1;
    pstAttr->stMdAttr.stMdStaticParamsIn.mb_size = MDMB_MODE_MB_8x8;

    pstAttr->stMdAttr.stMdStaticParamsIn.md_alg_mode  = MDALG_MODE_SAD;
    pstAttr->stMdAttr.stMdStaticParamsIn.sad_out_ctrl = MDSAD_OUT_CTRL_8BIT_SAD;
    pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.num      = 4;
    pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].x = 0;
    pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].y = 0;
    pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].x = Width - 1;
    pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].y = 0;
    pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].x = Width - 1;
    pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].y = Height - 1;
    pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].x = 0;
    pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].y = Height - 1;

    printf("MD width:%d,height:%d, 0(%dx%d), 1(%dx%d), 2(%dx%d), 3(%dx%d)\n",
              pstAttr->stMdAttr.stMdStaticParamsIn.width,
              pstAttr->stMdAttr.stMdStaticParamsIn.height,
              pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].x,
              pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[0].y,
              pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].x,
              pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[1].y,
              pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].x,
              pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[2].y,
              pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].x,
              pstAttr->stMdAttr.stMdStaticParamsIn.roi_md.pnt[3].y);

    return 0;
}

MI_S32 ST_Common_GetVdfOdDefaultChnAttr(MI_VDF_ChnAttr_t *pstAttr, MI_U16 Width, MI_U16 Height)
{
    ST_CHECK_POINTER(pstAttr);

    pstAttr->enWorkMode = E_MI_VDF_WORK_MODE_OD;
    pstAttr->stOdAttr.u8OdBufCnt  = 4;
    pstAttr->stOdAttr.u8VDFIntvl  = 0;

    pstAttr->stOdAttr.stOdDynamicParamsIn.thd_tamper     = 3;
    pstAttr->stOdAttr.stOdDynamicParamsIn.tamper_blk_thd = 1;
    pstAttr->stOdAttr.stOdDynamicParamsIn.min_duration   = 15;

    pstAttr->stOdAttr.stOdStaticParamsIn.inImgW = ALIGN_UP(Width, 16);
    pstAttr->stOdAttr.stOdStaticParamsIn.inImgH = ALIGN_UP(Height, 2);
    pstAttr->stOdAttr.stOdStaticParamsIn.inImgStride = ALIGN_UP(Width, 16);
    pstAttr->stOdAttr.stOdStaticParamsIn.nClrType = OD_Y;
    pstAttr->stOdAttr.stOdStaticParamsIn.div = OD_WINDOW_3X3;
    pstAttr->stOdAttr.stOdStaticParamsIn.alpha = 2;
    pstAttr->stOdAttr.stOdStaticParamsIn.M = 120;
    pstAttr->stOdAttr.stOdStaticParamsIn.MotionSensitivity = 0;
    pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.num = 4;
    pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[0].x = 0;
    pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[0].y = 0;
    pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[1].x = Width - 1;
    pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[1].y = 0;
    pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[2].x = Width - 1;
    pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[2].y = Height - 1;
    pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[3].x = 0;
    pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[3].y = Height - 1;
    printf("OD width:%d,height:%d, 0(%dx%d), 1(%dx%d), 2(%dx%d), 3(%dx%d)\n",
           pstAttr->stOdAttr.stOdStaticParamsIn.inImgW,
           pstAttr->stOdAttr.stOdStaticParamsIn.inImgH,
           pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[0].x,
           pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[0].y,
           pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[1].x,
           pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[1].y,
           pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[2].x,
           pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[2].y,
           pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[3].x,
           pstAttr->stOdAttr.stOdStaticParamsIn.roi_od.pnt[3].y);

    return 0;
}

MI_S32 ST_Common_GetVdfVgDefaultChnAttr(MI_VDF_ChnAttr_t *pstAttr, MI_U16 Width, MI_U16 Height)
{
    ST_CHECK_POINTER(pstAttr);

    pstAttr->enWorkMode = E_MI_VDF_WORK_MODE_VG;
    pstAttr->stVgAttr.u8VgBufCnt  = 4;
    pstAttr->stVgAttr.u8VDFIntvl  = 0;

    pstAttr->stVgAttr.height = ALIGN_UP(Height, 2);
    pstAttr->stVgAttr.width = ALIGN_UP(Width, 16);
    pstAttr->stVgAttr.stride = ALIGN_UP(Width, 16);

    pstAttr->stVgAttr.object_size_thd = VG_SENSITIVELY_HIGH;
    pstAttr->stVgAttr.indoor = 1;
    pstAttr->stVgAttr.function_state = VG_VIRTUAL_GATE;
    pstAttr->stVgAttr.line_number = 2;

    pstAttr->stVgAttr.line[0].px.x = 160;
    pstAttr->stVgAttr.line[0].px.y = 210;
    pstAttr->stVgAttr.line[0].py.x = 160;
    pstAttr->stVgAttr.line[0].py.y = 340;
    pstAttr->stVgAttr.line[0].pdx.x = 80;
    pstAttr->stVgAttr.line[0].pdx.y = 250;
    pstAttr->stVgAttr.line[0].pdy.x = 240;
    pstAttr->stVgAttr.line[0].pdy.y = 250;

    pstAttr->stVgAttr.line[1].px.x = 50;
    pstAttr->stVgAttr.line[1].px.y = 100;
    pstAttr->stVgAttr.line[1].py.x = 50;
    pstAttr->stVgAttr.line[1].py.y = 170;
    pstAttr->stVgAttr.line[1].pdx.x = 80;
    pstAttr->stVgAttr.line[1].pdx.y = 150;
    pstAttr->stVgAttr.line[1].pdy.x = 20;
    pstAttr->stVgAttr.line[1].pdy.y = 150;

    printf("VG line_number=%d, function_state=%d\n", pstAttr->stVgAttr.line_number, pstAttr->stVgAttr.function_state);
    return 0;
}