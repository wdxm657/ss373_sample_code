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
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mi_rgn.h"
#include "font_8x16.h"
#include "font_16x28.h"
#include "font_32x32.h"
#include "font_16x28_weight.h"
#include "font_24x24.h"
#include "font_24x32.h"
#include "font_16x16.h"
#include "ss_font.h"
#include "st_common_font.h"
#include "st_common.h"

#define LOG_ENABLE 0

static int _get_font_info(struct ss_font_info *info, enum en_ss_font_type type)
{
    switch (type)
    {
        case SS_FONT_8x16:
            memcpy(info, &font_8x16_info, sizeof(struct ss_font_info));
            break;
        case SS_FONT_16x28:
            memcpy(info, &font_16x28_info, sizeof(struct ss_font_info));
            break;
        case SS_FONT_16x16:
            memcpy(info, &font_16x16_info, sizeof(struct ss_font_info));
            break;
        case SS_FONT_16x28_W:
            memcpy(info, &font_16x28_weight_info, sizeof(struct ss_font_info));
            break;
        case SS_FONT_24x24:
            memcpy(info, &font_24x24_info, sizeof(struct ss_font_info));
            break;
        case SS_FONT_24x32:
            memcpy(info, &font_24x32_info, sizeof(struct ss_font_info));
            break;
        case SS_FONT_32x32:
            memcpy(info, &font_32x32_info, sizeof(struct ss_font_info));
            break;
        default:
            return -1;
    }

    return 0;
}

#define LINE_CHAR_COUNT 16
#define CHECK_FONT_INFO(__info, __type, __pix_w, __pix_h)                \
        do                                                               \
        {                                                                \
            int ret = _get_font_info(&__info, __type);                   \
            if (ret == -1)                                               \
            {                                                            \
                printf("FONT NOT FOUND!\n");                             \
                return -1;                                               \
            }                                                            \
            __pix_w = (LINE_CHAR_COUNT) * (__info).font_w;               \
            __pix_h = ((128 % (LINE_CHAR_COUNT)) ?                       \
                       ((128 / (LINE_CHAR_COUNT)) + 1) :                 \
                       ((128 / (LINE_CHAR_COUNT)))) * ((__info).font_h); \
        } while (0)



MI_S32 ST_GetBytePerPixel(MI_RGN_PixelFormat_e eFileFormat, MI_U32 *pu32BytePerPixel)
{

    ST_CHECK_POINTER(pu32BytePerPixel);

    if (eFileFormat == E_MI_RGN_PIXEL_FORMAT_ARGB1555 || eFileFormat == E_MI_RGN_PIXEL_FORMAT_ARGB4444
        || eFileFormat == E_MI_RGN_PIXEL_FORMAT_RGB565)
    {
        *pu32BytePerPixel = 2;
    }
    else if (eFileFormat == E_MI_RGN_PIXEL_FORMAT_ARGB8888)
    {
        *pu32BytePerPixel = 4;
    }
    else
    {
        ST_ERR("unsupport rgn format %d\n", eFileFormat);
        return -1;
    }

    return 0;
}


MI_S32 OsdDrawTextCanvas(ST_Common_OsdDrawText_Attr_t *pstDrawTextAttr)
{
    MI_U32              s32Ret          = MI_SUCCESS;
    char *              fb              = NULL;
    char *              u8SrcBuf        = NULL;
    MI_U8 *             u8DstBuf        = NULL;
    int                 pixel_w         = 0;
    int                 pixel_h         = 0;
    MI_U32              stride          = 0;
    MI_U32              pointX          = 0;
    MI_U32              pointY          = 0;
    MI_U32              u32BytePerPixel = 0;
    struct ss_font_info info            = {0};
    char *              draw_text       = NULL;
    int                 draw_text_len   = 0;
    int                 CharPerLine     = 0;
    int                 i               = 0;

    MI_RGN_CanvasInfo_t stCanvasInfo;
    MI_RGN_CanvasInfo_t *pstCanvasInfo = NULL;

    ST_CHECK_POINTER(pstDrawTextAttr);

    pthread_mutex_lock(&pstDrawTextAttr->Osdmutex);
    s32Ret = MI_RGN_GetCanvasInfo(0, pstDrawTextAttr->handle, &stCanvasInfo);
    if (s32Ret != MI_SUCCESS)
    {
        printf("MI_RGN_GetCanvasInfo error s32Ret=%d\n", s32Ret);
        pthread_mutex_unlock(&pstDrawTextAttr->Osdmutex);
        return s32Ret;
    }
    pstCanvasInfo = &stCanvasInfo;

    //get default font info
    CHECK_FONT_INFO(info, pstDrawTextAttr->eFontType, pixel_w, pixel_h);

    pointX = ALIGN_BACK(pstDrawTextAttr->u32X, 2);
    pointY = ALIGN_BACK(pstDrawTextAttr->u32Y, 2);
    if ((MI_U32)(pointX + pixel_w) > pstCanvasInfo->stSize.u32Width)
        pixel_w = pstCanvasInfo->stSize.u32Width - pointX;

    if ((MI_U32)(pointY + pixel_h) > pstCanvasInfo->stSize.u32Height)
        pixel_h = pstCanvasInfo->stSize.u32Height - pointY;

    if (pixel_w <= 0 || pixel_h <= 0)
    {
        printf("pixel_w = %d, pixel_h = %d error\n", pixel_w, pixel_h);
        pthread_mutex_unlock(&pstDrawTextAttr->Osdmutex);
        return -1;
    }

    if(pstDrawTextAttr->rot == FONT_ROT_90
        || pstDrawTextAttr->rot == FONT_ROT_270)
    {
        /*Exchange the value of w/h*/
        pixel_w ^= pixel_h;
        pixel_h ^= pixel_w;
        pixel_w ^= pixel_h;
    }

    if(0 != ST_GetBytePerPixel(pstDrawTextAttr->ePixelFmt, &u32BytePerPixel))
    {
        pthread_mutex_unlock(&pstDrawTextAttr->Osdmutex);
        return -1;
    }

    stride = pixel_w * u32BytePerPixel;
    CharPerLine = pixel_w / info.font_w;

#if LOG_ENABLE
    printf("PIX %dx%d, CharPerLine %d, stride %d, fontW %d, fontH %d\n", pixel_w, pixel_h, CharPerLine, stride, info.font_w, info.font_h);
#endif
    fb = (char *)malloc(pixel_w * pixel_h * u32BytePerPixel);
    if (!fb)
    {
        printf("malloc fb buffer error!\n");
        pthread_mutex_unlock(&pstDrawTextAttr->Osdmutex);
        return -1;
    }
    memset(fb, 0x0, sizeof(char)*pixel_w * pixel_h * u32BytePerPixel);

    struct text_draw_offset
    {
        int  x_line_start;
        int  y_line_start;
        int  x_line_off;
        int  y_line_off;
        int  fb_line_off;
    } text_off = {0, 0, 0, 0, 0}, text_loop = {0, 0, 0, 0, 0};\

    if(pstDrawTextAttr ->rot == FONT_ROT_NONE)
    {
        /*  --->Start from Here
         *  | --->
         *  |________
         * |*         |
         * |          |
         * |          |
         * |_________ |
         */
        text_off.x_line_start = 0;
        text_off.y_line_start = 0;
        text_off.x_line_off   = 0;
        text_off.y_line_off   = info.font_h;
    }
    else if(pstDrawTextAttr->rot == FONT_ROT_90)
    {
        /*    ________
         *   |        |
         *   |        |
         *   |        |
         * ^ |        |
         * | |*_______|
         *   --->Start from Here
         */
        text_off.x_line_start = 0;
        text_off.y_line_start = pixel_h - 1;
        text_off.x_line_off   = info.font_h;
        text_off.y_line_off   = 0;
    }
    else if (pstDrawTextAttr->rot == FONT_ROT_180)
    {
        /*  __________
         * |          |
         * |          |^
         * |          ||
         * |_________*|--->Start from Here
         */
        text_off.x_line_start = pixel_w - 1;
        text_off.y_line_start = pixel_h - 1;
        text_off.x_line_off   = 0;
        text_off.y_line_off   = -info.font_h;
    }
    else if(pstDrawTextAttr->rot == FONT_ROT_270)
    {
        /*  ________
         * |       *|--->Start from Here
         * |        ||
         * |        |V
         * |        |
         * |________|
         */
        text_off.x_line_start = pixel_w - 1;
        text_off.y_line_start = 0;
        text_off.x_line_off   = -info.font_h;
        text_off.y_line_off   = 0;
    }
    else
    {
        printf("Error setting of rotation.\n");
        pthread_mutex_unlock(&pstDrawTextAttr->Osdmutex);
        free(fb);
        return -1;
    }

    text_loop = text_off;
    draw_text = pstDrawTextAttr->text;
    for (i = 0; i < (pixel_h / info.font_h) ; i++)
    {
        int text_PerLineLen = 0;
        char *text;

        draw_text_len = strlen(draw_text);
        if(strlen(draw_text) == 0)
        {
            break;
        }

        text = strchr(draw_text, '\n');
        if(text == NULL)
        {
            text_PerLineLen = draw_text_len;
        }
        else
        {
            text_PerLineLen = text - draw_text;
        }

        if(text_PerLineLen > CharPerLine)
        {
            ST_WARN("draw test OverRange oneLine len, please add ch '\\n'\n");
            break;
        }

#if LOG_ENABLE
        printf("X %d Y %d, Text_PerLineLen %d\n", text_loop.x_line_start, text_loop.y_line_start, text_PerLineLen);
#endif
        display_text(fb, text_loop.x_line_start,  text_loop.y_line_start,
         stride, pstDrawTextAttr->color, u32BytePerPixel, draw_text, text_PerLineLen,
         pstDrawTextAttr->rot, info.font_w, info.font_h, info.font);
        if (MI_SUCCESS != s32Ret)
        {
            printf("font drawtext error(0x%X)\n", s32Ret);
            pthread_mutex_unlock(&pstDrawTextAttr->Osdmutex);\
            free(fb);
            return s32Ret;
        }

        text_loop.x_line_start += text_loop.x_line_off;
        text_loop.y_line_start += text_loop.y_line_off;
        draw_text += text_PerLineLen + 1;
    }

    if(draw_text_len > 0)
    {
        ST_WARN("draw test maybe OverRange\n");
    }

    u8SrcBuf = fb;
    u8DstBuf = (MI_U8 *)((pstCanvasInfo->virtAddr) + pstCanvasInfo->u32Stride * pointY + pointX * u32BytePerPixel);
    for (i = 0; i < pixel_h; i++)
    {
        memcpy(u8DstBuf, u8SrcBuf, stride);
        u8SrcBuf += stride;
        u8DstBuf += pstCanvasInfo->u32Stride;
    }

    s32Ret = MI_RGN_UpdateCanvas(0, pstDrawTextAttr->handle);
    if (s32Ret != MI_SUCCESS)
    {
        printf("MI_RGN_UpdateCanvas fail\n");
        pthread_mutex_unlock(&pstDrawTextAttr->Osdmutex);
        free(fb);
        return s32Ret;
    }

    pthread_mutex_unlock(&pstDrawTextAttr->Osdmutex);
    free(fb);

    return s32Ret;
}


