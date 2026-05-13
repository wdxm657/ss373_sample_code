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

#ifndef _ST_COMMON_GRAPH_H_
#define _ST_COMMON_GRAPH_H_

#include <stddef.h>
#include <stdint.h>

#define SS_GRAPH_CANVAS_PLANE_MAX_NUM (3)

typedef unsigned int Color;
typedef unsigned int AbsCoord;

typedef struct CanvasDesc_s
{
    struct
    {
        unsigned char bpp;
        unsigned char h_sample;
        unsigned char v_sample;
        Color (*color_convert)(Color c);
        void *user_data;
    } plane[SS_GRAPH_CANVAS_PLANE_MAX_NUM];
} CanvasDesc_t;

typedef struct Canvas_s
{
    AbsCoord width;
    AbsCoord height;
    char *   data[SS_GRAPH_CANVAS_PLANE_MAX_NUM];
    AbsCoord stride[SS_GRAPH_CANVAS_PLANE_MAX_NUM];
} Canvas_t;

typedef void (*FillBpp)(void *, size_t, size_t, unsigned int);

typedef struct ST_GraphAttr_s
{
    CanvasDesc_t desc;
    Canvas_t     canvas;
    FillBpp      fillbpp[SS_GRAPH_CANVAS_PLANE_MAX_NUM];
    unsigned int planeNum;
} ST_GraphAttr_t;

Color ConvertRGB2Y(Color c);
Color ConvertRGB2UV(Color c);

int ST_Common_SetGraphAttr(ST_GraphAttr_t *pGraphAttr);

int ST_Common_DrawFrame(ST_GraphAttr_t *pGraphAttr, const Color c, const AbsCoord x, const AbsCoord y, const AbsCoord w,
                        const AbsCoord h, const AbsCoord thickness);

#endif //_ST_COMMON_GRAPH_H_
