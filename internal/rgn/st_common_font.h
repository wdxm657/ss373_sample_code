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
#ifndef _ST_COMMON_FONT_H
#define _ST_COMMON_FONT_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "mi_rgn.h"
#include "ss_font.h"

#define COLOR_OF_RED_ARGB4444 0xFF00
#define COLOR_OF_GREEN_ARGB4444 0xF0F0
#define COLOR_OF_BULE_ARGB4444 0xF00F

#define COLOR_OF_RED_ARGB8888 0xFFFF0000
#define COLOR_OF_GREEN_ARGB8888 0xFF00FF00
#define COLOR_OF_BULE_ARGB8888 0xFF0000FF

#define COLOR_OF_RED_ARGB1555 0xFC00
#define COLOR_OF_GREEN_ARGB1555 0x83E0
#define COLOR_OF_BULE_ARGB1555 0x801F
#define COLOR_OF_YELLOW_ARGB1555 0xFFE0
#define COLOR_OF_WHITE_ARGB1555 0xFFFC

#define COLOR_OF_RED_RGB565 0xF800
#define COLOR_OF_GREEN_RGB565 0x07E0
#define COLOR_OF_BULE_RGB565 0x001F

enum en_ss_font_type
{
    SS_FONT_8x16 = 0,
    SS_FONT_16x16,
    SS_FONT_16x28,
    SS_FONT_16x28_W,
    SS_FONT_24x24,
    SS_FONT_24x32,
    SS_FONT_32x32,
};

typedef struct ST_Common_OsdDrawText_Attr_s
{
    int                   color;
    MI_RGN_HANDLE         handle;
    MI_RGN_PixelFormat_e  ePixelFmt;
    enum en_font_rotation rot;
    enum en_ss_font_type  eFontType;
    pthread_mutex_t       Osdmutex;
    char                  text[128];
    MI_U32                u32X;
    MI_U32                u32Y;
} ST_Common_OsdDrawText_Attr_t;

MI_S32 OsdDrawTextCanvas(ST_Common_OsdDrawText_Attr_t *pstDrawTextAttr);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_FONT_H
