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
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "st_common_graph.h"

static void fill_8bpp(void *first, size_t n, size_t offset, unsigned int val)
{
    uint8_t *dst = (uint8_t *)first;
    dst += offset;
    memset(dst, val, n);
}
static void fill_16bpp(void *first, size_t n, size_t offset, unsigned int val)
{
    uint16_t *dst = (uint16_t *)first;
    dst += offset;
    for (size_t i = 0; i < n; ++i)
    {
        dst[i] = val;
    }
}
static void fill_32bpp(void *first, size_t n, size_t offset, unsigned int val)
{
    uint32_t *dst = (uint32_t *)first;
    dst += offset;
    for (size_t i = 0; i < n; ++i)
    {
        dst[i] = val;
    }
}

Color ConvertRGB2Y(Color c)
{
    unsigned char r = (c & 0xff0000) >> 16;
    unsigned char g = (c & 0x00ff00) >> 8;
    unsigned char b = (c & 0x0000ff);
    unsigned char y = (0.299 * r + 0.587 * g + 0.114 * b);
    return y;
}

Color ConvertRGB2UV(Color c)
{
    unsigned char r = (c & 0xff0000) >> 16;
    unsigned char g = (c & 0x00ff00) >> 8;
    unsigned char b = (c & 0x0000ff);
    unsigned char u = (0.5 * r - 0.4187 * g - 0.0813 * b + 128);
    unsigned char v = (-0.1687 * r - 0.3313 * g + 0.5 * b + 128);
    return (u << 8) | v;
}

int ST_Common_SetGraphAttr(ST_GraphAttr_t *pGraphAttr)
{
    pGraphAttr->planeNum = 0;
    for (unsigned int i = 0; i < SS_GRAPH_CANVAS_PLANE_MAX_NUM; ++i)
    {
        if (!(pGraphAttr->canvas.data[i]))
        {
            pGraphAttr->planeNum = i;
            break;
        }
        switch (pGraphAttr->desc.plane[i].bpp)
        {
            case 8:
                pGraphAttr->fillbpp[i] = fill_8bpp;
                break;
            case 16:
                pGraphAttr->fillbpp[i] = fill_16bpp;
                break;
            case 32:
                pGraphAttr->fillbpp[i] = fill_32bpp;
                break;
            default:
                pGraphAttr->fillbpp[i] = NULL;
                break;
        }
    }
    return 0;
}

int ST_Common_DrawFrame_Plane_idx(const ST_GraphAttr_t *pGraphAttr, const unsigned int plane_idx, const Color c,
                                  const AbsCoord x, const AbsCoord y, const AbsCoord w, const AbsCoord h,
                                  const AbsCoord thickness)
{
    if (x >= pGraphAttr->canvas.width)
    {
        printf("over canvas width\n");
        return -1;
    }
    if (y >= pGraphAttr->canvas.height)
    {
        printf("over canvas height\n");
        return -1;
    }

    AbsCoord x0 = x;
    AbsCoord y0 = y;
    AbsCoord x1 = x + w;
    AbsCoord y1 = y + h;
    if (x1 >= pGraphAttr->canvas.width)
    {
        x1 = pGraphAttr->canvas.width - 1;
    }
    if (y1 >= pGraphAttr->canvas.height)
    {
        y1 = pGraphAttr->canvas.height - 1;
    }
    AbsCoord line_h = thickness > (w >> 1) ? (w >> 1) : thickness;
    AbsCoord line_w = thickness > (h >> 1) ? (h >> 1) : thickness;
    line_w /= pGraphAttr->desc.plane[plane_idx].h_sample;
    line_h /= pGraphAttr->desc.plane[plane_idx].v_sample;
    x0 /= pGraphAttr->desc.plane[plane_idx].h_sample;
    y0 /= pGraphAttr->desc.plane[plane_idx].v_sample;
    x1 /= pGraphAttr->desc.plane[plane_idx].h_sample;
    y1 /= pGraphAttr->desc.plane[plane_idx].v_sample;
    for (size_t i = 0; i < line_h; ++i)
    {
        pGraphAttr->fillbpp[plane_idx](
            pGraphAttr->canvas.data[plane_idx] + (y0 + i) * pGraphAttr->canvas.stride[plane_idx], x1 - x0, x0, c);
        pGraphAttr->fillbpp[plane_idx](
            pGraphAttr->canvas.data[plane_idx] + (y1 - i) * pGraphAttr->canvas.stride[plane_idx], x1 - x0, x0, c);
    }
    for (size_t i = y0 + line_h; i <= y1 - line_h; ++i)
    {
        pGraphAttr->fillbpp[plane_idx](pGraphAttr->canvas.data[plane_idx] + i * pGraphAttr->canvas.stride[plane_idx],
                                       line_w, x0, c);
        pGraphAttr->fillbpp[plane_idx](pGraphAttr->canvas.data[plane_idx] + i * pGraphAttr->canvas.stride[plane_idx],
                                       line_w, x1 - line_w, c);
    }
    return 0;
}

int ST_Common_DrawFrame(ST_GraphAttr_t *pGraphAttr, const Color c, const AbsCoord x, const AbsCoord y, const AbsCoord w,
                        const AbsCoord h, const AbsCoord thickness)
{
    for (unsigned int i = 0; i < pGraphAttr->planeNum; ++i)
    {
        if (pGraphAttr->fillbpp[i])
        {
            Color real_c = c;
            if (pGraphAttr->desc.plane[i].color_convert)
            {
                real_c = pGraphAttr->desc.plane[i].color_convert(c);
            }
            ST_Common_DrawFrame_Plane_idx(pGraphAttr, i, real_c, x, y, w, h, thickness);
        }
    }
    return 0;
}
