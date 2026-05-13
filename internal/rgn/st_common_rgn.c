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

#include "st_common_rgn.h"
#include "st_common.h"

static MI_U32 predefined_colors_palette[] = {
    // Colors 0 to 15: original ANSI colors
    0x000000, 0xcd0000, 0x00cd00, 0xcdcd00, 0x0000ee, 0xcd00cd, 0x00cdcd, 0xe5e5e5,
    0x7f7f7f, 0xff0000, 0x00ff00, 0xffff00, 0x5c5cff, 0xff00ff, 0x00ffff, 0xffffff,
    // Color cube colors
    0x000000, 0x00005f, 0x000087, 0x0000af, 0x0000d7, 0x0000ff, 0x005f00, 0x005f5f,
    0x005f87, 0x005faf, 0x005fd7, 0x005fff, 0x008700, 0x00875f, 0x008787, 0x0087af,
    0x0087d7, 0x0087ff, 0x00af00, 0x00af5f, 0x00af87, 0x00afaf, 0x00afd7, 0x00afff,
    0x00d700, 0x00d75f, 0x00d787, 0x00d7af, 0x00d7d7, 0x00d7ff, 0x00ff00, 0x00ff5f,
    0x00ff87, 0x00ffaf, 0x00ffd7, 0x00ffff, 0x5f0000, 0x5f005f, 0x5f0087, 0x5f00af,
    0x5f00d7, 0x5f00ff, 0x5f5f00, 0x5f5f5f, 0x5f5f87, 0x5f5faf, 0x5f5fd7, 0x5f5fff,
    0x5f8700, 0x5f875f, 0x5f8787, 0x5f87af, 0x5f87d7, 0x5f87ff, 0x5faf00, 0x5faf5f,
    0x5faf87, 0x5fafaf, 0x5fafd7, 0x5fafff, 0x5fd700, 0x5fd75f, 0x5fd787, 0x5fd7af,
    0x5fd7d7, 0x5fd7ff, 0x5fff00, 0x5fff5f, 0x5fff87, 0x5fffaf, 0x5fffd7, 0x5fffff,
    0x870000, 0x87005f, 0x870087, 0x8700af, 0x8700d7, 0x8700ff, 0x875f00, 0x875f5f,
    0x875f87, 0x875faf, 0x875fd7, 0x875fff, 0x878700, 0x87875f, 0x878787, 0x8787af,
    0x8787d7, 0x8787ff, 0x87af00, 0x87af5f, 0x87af87, 0x87afaf, 0x87afd7, 0x87afff,
    0x87d700, 0x87d75f, 0x87d787, 0x87d7af, 0x87d7d7, 0x87d7ff, 0x87ff00, 0x87ff5f,
    0x87ff87, 0x87ffaf, 0x87ffd7, 0x87ffff, 0xaf0000, 0xaf005f, 0xaf0087, 0xaf00af,
    0xaf00d7, 0xaf00ff, 0xaf5f00, 0xaf5f5f, 0xaf5f87, 0xaf5faf, 0xaf5fd7, 0xaf5fff,
    0xaf8700, 0xaf875f, 0xaf8787, 0xaf87af, 0xaf87d7, 0xaf87ff, 0xafaf00, 0xafaf5f,
    0xafaf87, 0xafafaf, 0xafafd7, 0xafafff, 0xafd700, 0xafd75f, 0xafd787, 0xafd7af,
    0xafd7d7, 0xafd7ff, 0xafff00, 0xafff5f, 0xafff87, 0xafffaf, 0xafffd7, 0xafffff,
    0xd70000, 0xd7005f, 0xd70087, 0xd700af, 0xd700d7, 0xd700ff, 0xd75f00, 0xd75f5f,
    0xd75f87, 0xd75faf, 0xd75fd7, 0xd75fff, 0xd78700, 0xd7875f, 0xd78787, 0xd787af,
    0xd787d7, 0xd787ff, 0xd7af00, 0xd7af5f, 0xd7af87, 0xd7afaf, 0xd7afd7, 0xd7afff,
    0xd7d700, 0xd7d75f, 0xd7d787, 0xd7d7af, 0xd7d7d7, 0xd7d7ff, 0xd7ff00, 0xd7ff5f,
    0xd7ff87, 0xd7ffaf, 0xd7ffd7, 0xd7ffff, 0xff0000, 0xff005f, 0xff0087, 0xff00af,
    0xff00d7, 0xff00ff, 0xff5f00, 0xff5f5f, 0xff5f87, 0xff5faf, 0xff5fd7, 0xff5fff,
    0xff8700, 0xff875f, 0xff8787, 0xff87af, 0xff87d7, 0xff87ff, 0xffaf00, 0xffaf5f,
    0xffaf87, 0xffafaf, 0xffafd7, 0xffafff, 0xffd700, 0xffd75f, 0xffd787, 0xffd7af,
    0xffd7d7, 0xffd7ff, 0xffff00, 0xffff5f, 0xffff87, 0xffffaf, 0xffffd7, 0xffffff,
    // >= 233: Grey ramp
    0x000000, 0x121212, 0x1c1c1c, 0x262626, 0x303030, 0x3a3a3a, 0x444444, 0x4e4e4e,
    0x585858, 0x626262, 0x6c6c6c, 0x767676, 0x808080, 0x8a8a8a, 0x949494, 0x9e9e9e,
    0xa8a8a8, 0xb2b2b2, 0xbcbcbc, 0xc6c6c6, 0xd0d0d0, 0xdadada,
};

static MI_U16 gImage_Width = 1920;
static MI_U16 gImage_Height = 1080;


MI_S32 ST_Common_GetRgnDefaultInitAttr(MI_RGN_PaletteTable_t *pstPaletteTable)
{
    ST_CHECK_POINTER(pstPaletteTable);
    memset(pstPaletteTable, 0x00, sizeof(MI_RGN_PaletteTable_t));
    MI_U32 u32Length = sizeof(predefined_colors_palette) / sizeof(predefined_colors_palette[0]);
    MI_U32 u32Index = 0;

    for (u32Index = 0; u32Index < u32Length; u32Index++) {
        pstPaletteTable->astElement[u32Index].u8Alpha = 0xff;
        pstPaletteTable->astElement[u32Index].u8Red   = ( predefined_colors_palette[u32Index] & 0xff0000) >> 16;
        pstPaletteTable->astElement[u32Index].u8Green = ( predefined_colors_palette[u32Index] & 0xff00) >> 8;
        pstPaletteTable->astElement[u32Index].u8Blue  = ( predefined_colors_palette[u32Index] & 0xff);
    }

    return 0;
}

MI_S32 ST_Common_GetRgnDefaultCreateAttr(MI_RGN_Attr_t *pstRegion)
{
    ST_CHECK_POINTER(pstRegion);

    memset(pstRegion, 0x00, sizeof(MI_RGN_Attr_t));
    pstRegion->eType = E_MI_RGN_TYPE_OSD; // or E_MI_RGN_TYPE_COVER
    if (pstRegion->eType == E_MI_RGN_TYPE_OSD) {
        // Just OSD need to set stOsdInitParam
        pstRegion->stOsdInitParam.ePixelFmt        = E_MI_RGN_PIXEL_FORMAT_I4; // or others
        pstRegion->stOsdInitParam.stSize.u32Width  = 100; // u32Width <= 3840
        pstRegion->stOsdInitParam.stSize.u32Height = 100; // u32Height <= 2160
    }

    return 0;
}

MI_S32 ST_Common_GetRgnDefaultOsdAttr(MI_RGN_ChnPortParam_t *pstChnAttr)
{
    ST_CHECK_POINTER(pstChnAttr);

    pstChnAttr->bShow = TRUE; // or FALSE
    pstChnAttr->u32Layer = 0;

    pstChnAttr->stOsdChnPort.stPoint.s32X = 0;
    pstChnAttr->stOsdChnPort.stPoint.s32Y = 0;
    pstChnAttr->stOsdChnPort.u8PaletteIdx = 0;
    pstChnAttr->stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
    if (E_MI_RGN_PIXEL_ALPHA == pstChnAttr->stOsdChnPort.stOsdAlphaAttr.eAlphaMode)
    {
        pstChnAttr->stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;   // 0 ~ 255
        pstChnAttr->stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 255; // 0 ~ 255
    }
    else
    {
        pstChnAttr->stOsdChnPort.stOsdAlphaAttr.stAlphaPara.u8ConstantAlpha = 128; // 0 ~ 255
    }


    return 0;
}

MI_S32 ST_Common_GetRgnDefaultCoverColorAttr(MI_RGN_AreaType_e eAreaType, MI_RGN_ChnPortParam_t *pstChnAttr)
{
    ST_CHECK_POINTER(pstChnAttr);

    pstChnAttr->bShow = TRUE; // or FALSE
    pstChnAttr->u32Layer = 0;

    pstChnAttr->stCoverChnPort.eMode = E_MI_RGN_COVER_MODE_COLOR;
    pstChnAttr->stCoverChnPort.stColorAttr.u32Color = 0x00ff00; // R G B
    pstChnAttr->stCoverChnPort.eAreaType = eAreaType;

    if(pstChnAttr->stCoverChnPort.eAreaType == E_MI_RGN_AREA_TYPE_RECT)
    {
        //image(1920,1080), relative to 0-8192 position
        //for example set absolute coordinates[1280,960,540,540]
        //actually set relative coordinates [5461,6068,1024,1820]
        pstChnAttr->stCoverChnPort.stRect.s32X      = 1280 * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stRect.s32Y      = 800* 8192 / gImage_Height;
        pstChnAttr->stCoverChnPort.stRect.u32Width  = 240 * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stRect.u32Height = 240 * 8192 / gImage_Height;
    }
    else if(pstChnAttr->stCoverChnPort.eAreaType == E_MI_RGN_AREA_TYPE_POLY)
    {
        //set a rhombus
        //cover actual width/screen width * 8192
        pstChnAttr->stCoverChnPort.stPoly.astCoord[0].s32X = 720  * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[0].s32Y = 540 * 8192 / gImage_Height;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[1].s32X = 960 * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[1].s32Y = 300 * 8192 / gImage_Height;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[2].s32X = 1200 * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[2].s32Y = 540 * 8192 / gImage_Height;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[3].s32X = 960 * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[3].s32Y = 780 * 8192 / gImage_Height;
        pstChnAttr->stCoverChnPort.stPoly.u8VertexNum = 4;
    }

    return 0;
}

MI_S32 ST_Common_GetRgnDefaultCoverMosicAttr(MI_RGN_AreaType_e eAreaType, MI_RGN_ChnPortParam_t *pstChnAttr)
{
    ST_CHECK_POINTER(pstChnAttr);

    pstChnAttr->bShow = TRUE; // or FALSE
    pstChnAttr->u32Layer = 0;

    pstChnAttr->stCoverChnPort.eMode = E_MI_RGN_COVER_MODE_MOSAIC;
    pstChnAttr->stCoverChnPort.stMosaicAttr.eBlkSize = E_MI_RGN_BLOCK_SIZE_32;
    pstChnAttr->stCoverChnPort.eAreaType = eAreaType;

    if(pstChnAttr->stCoverChnPort.eAreaType == E_MI_RGN_AREA_TYPE_RECT)
    {
        //cover actual width/screen width * 8192
        pstChnAttr->stCoverChnPort.stRect.s32X      = 1280 * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stRect.s32Y      = 800 * 8192 / gImage_Height;
        pstChnAttr->stCoverChnPort.stRect.u32Width  = 240  * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stRect.u32Height = 240 * 8192 / gImage_Height;
    }
    else if(pstChnAttr->stCoverChnPort.eAreaType == E_MI_RGN_AREA_TYPE_POLY)
    {
        //cover actual width/screen width * 8192
        pstChnAttr->stCoverChnPort.stPoly.astCoord[0].s32X =  720 * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[0].s32Y =  540 * 8192 / gImage_Height;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[1].s32X =  960 * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[1].s32Y =  300 * 8192 / gImage_Height;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[2].s32X =  1200 * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[2].s32Y =  540 * 8192 / gImage_Height;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[3].s32X =  960 * 8192 / gImage_Width;
        pstChnAttr->stCoverChnPort.stPoly.astCoord[3].s32Y =  780 * 8192 / gImage_Height;
        pstChnAttr->stCoverChnPort.stPoly.u8VertexNum = 4;
    }

    return 0;
}

MI_S32 ST_Common_GetRgnDefaultFrameAttr(MI_RGN_ChnPortParam_t *pstChnAttr)
{
    ST_CHECK_POINTER(pstChnAttr);

    pstChnAttr->bShow = TRUE; // or FALSE
    pstChnAttr->u32Layer = 0;

    pstChnAttr->stFrameChnPort.u32Color = 0x00ff00;
    pstChnAttr->stFrameChnPort.u8Thickness = 8;

    //actual width/screen width * 8192
    pstChnAttr->stFrameChnPort.stRect.s32X      = 240 * 8192 / gImage_Width;
    pstChnAttr->stFrameChnPort.stRect.s32Y      = 135 * 8192 / gImage_Height;
    pstChnAttr->stFrameChnPort.stRect.u32Width  = 960 * 8192 / gImage_Width;
    pstChnAttr->stFrameChnPort.stRect.u32Height = 540 * 8192 / gImage_Height;

    return 0;
}

MI_S32 ST_Common_GetRgnDefaultRotateAttr(MI_RGN_ChnPortParam_t *pstChnAttr)
{
    ST_CHECK_POINTER(pstChnAttr);

    pstChnAttr->bShow = TRUE; // or FALSE, TRUE is Frame, FALSE is Cover
    pstChnAttr->u32Layer = 0;

    pstChnAttr->stLineChnPort.u32Color = 0x00ff00;
    pstChnAttr->stLineChnPort.bHollow = TRUE; // TRUE is Cover,FALSE is Frame
    pstChnAttr->stLineChnPort.u8BorderThickness = 10;

    //actual width/screen width * 8192
    pstChnAttr->stLineChnPort.stPointFrom.s32X = (gImage_Width * 0.5) - 100;
    pstChnAttr->stLineChnPort.stPointFrom.s32Y = (gImage_Height * 0.5) - 100;
    pstChnAttr->stLineChnPort.stPointTo.s32X   = (gImage_Width * 0.5) + 100;
    pstChnAttr->stLineChnPort.stPointTo.s32Y   = (gImage_Height * 0.5) - 100;
    pstChnAttr->stLineChnPort.u32LineWidth     = 200;

    return 0;
}


void ST_Common_Rgn_SetImageSize(MI_U16 image_width, MI_U16 image_height)
{
    gImage_Width = image_width;
    gImage_Height = image_height;
}



MI_S32 ST_Common_RgnInit(MI_U16 u16SocId, MI_RGN_PaletteTable_t *pstPaletteTable)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_RGN_Init(u16SocId, pstPaletteTable);

    return s32Ret;
}

MI_S32 ST_Common_RgnDeInit(MI_U16 u16SocId)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_RGN_DeInit(u16SocId);

    return s32Ret;
}

MI_S32 ST_Common_RgnCreate(MI_U16 u16SocId, MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_RGN_Create(u16SocId, hHandle, pstRegion);

    return s32Ret;
}


MI_S32 ST_Common_RgnDestroy(MI_U16 u16SocId, MI_RGN_HANDLE hHandle)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_RGN_Destroy(u16SocId, hHandle);

    return s32Ret;
}

MI_S32 ST_Common_RgnAttachChn(MI_U16 u16SocId, MI_RGN_HANDLE hHandle,
                MI_RGN_ChnPort_t* pstChnPort, MI_RGN_ChnPortParam_t *pstChnAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_RGN_AttachToChn(u16SocId, hHandle, pstChnPort, pstChnAttr);
    //s32Ret |= MI_RGN_SetDisplayAttr(u16SocId, hHandle, pstChnPort, pstChnAttr);

    return s32Ret;
}

MI_S32 ST_Common_RgnDetachChn(MI_U16 u16SocId, MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_RGN_DetachFromChn(u16SocId, hHandle, pstChnPort);

    return s32Ret;
}

MI_S32 ST_Common_Rgn_GetStrideByPixel(MI_U32 u32Width, MI_RGN_PixelFormat_e eFileFormat, MI_U32 *pu32Stride)
{

    ST_CHECK_POINTER(pu32Stride);

    if (eFileFormat == E_MI_RGN_PIXEL_FORMAT_ARGB1555 || eFileFormat == E_MI_RGN_PIXEL_FORMAT_ARGB4444
        || eFileFormat == E_MI_RGN_PIXEL_FORMAT_RGB565)
    {
        *pu32Stride = u32Width * 2;
    }
    else if (eFileFormat == E_MI_RGN_PIXEL_FORMAT_ARGB8888)
    {
        *pu32Stride = u32Width * 4;
    }
    else if (eFileFormat == E_MI_RGN_PIXEL_FORMAT_I8)
    {
        *pu32Stride = u32Width;
    }
    else if (eFileFormat == E_MI_RGN_PIXEL_FORMAT_I4)
    {
        *pu32Stride = u32Width / 2;
    }
    else if (eFileFormat == E_MI_RGN_PIXEL_FORMAT_I2)
    {
        *pu32Stride = u32Width / 4;
    }
    else
    {
        ST_ERR("unsupport rgn format %d\n", eFileFormat);
        return -1;
    }

    return 0;
}

MI_S32 ST_Common_RgnUpdateCanvas(MI_U16 u16SocId, MI_RGN_HANDLE hHandle, ST_OsdFileAttr_t *pstOsdFileAttr)
{
    MI_S32 s32Ret = MI_SUCCESS;
    FILE *pFile = NULL;
    MI_RGN_CanvasInfo_t stRgnCanvasInfo;
    MI_U32 u32FileStride = 0;

    ST_CHECK_POINTER(pstOsdFileAttr);

    pFile = fopen((char *)pstOsdFileAttr->FilePath, "rb");
    if (pFile == NULL)
    {
        ST_ERR("open file %s failed\n", pstOsdFileAttr->FilePath);
        return -1;
    }

    memset(&stRgnCanvasInfo, 0, sizeof(MI_RGN_CanvasInfo_t));
    s32Ret = MI_RGN_GetCanvasInfo(u16SocId, hHandle, &stRgnCanvasInfo);
    if (s32Ret != MI_SUCCESS)
    {
        ST_ERR("MI_RGN_GetCanvasInfo failed\n");
        fclose(pFile);
        return s32Ret;
    }

    s32Ret = ST_Common_Rgn_GetStrideByPixel(pstOsdFileAttr->u32FileWidth, pstOsdFileAttr->eFileFormat, &u32FileStride);
    if (s32Ret != MI_SUCCESS)
    {
        fclose(pFile);
        return s32Ret;
    }

    ST_INFO("handle %d, CanvasInfo stride %d,  file width %d, height %d, stride %d \n ",
        hHandle, stRgnCanvasInfo.u32Stride, pstOsdFileAttr->u32FileWidth, pstOsdFileAttr->u32FileHeight, u32FileStride);

    for (MI_U32 i = 0; i < pstOsdFileAttr->u32FileHeight; i++)
    {
        fread((MI_U8 *)stRgnCanvasInfo.virtAddr + i * stRgnCanvasInfo.u32Stride, 1, u32FileStride, pFile);
    }

    s32Ret = MI_RGN_UpdateCanvas(u16SocId, hHandle);
    if (s32Ret != MI_SUCCESS)
    {
        ST_ERR("MI_RGN_UpdateCanvas failed\n");
    }

    fclose(pFile);

    return s32Ret;
}

