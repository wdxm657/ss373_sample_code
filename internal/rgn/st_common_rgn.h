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
#ifndef _ST_COMMON_RGN_H_
#define _ST_COMMON_RGN_H_

#ifdef __cplusplus
extern "C"{
#endif	// __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_rgn.h"
#include "mi_rgn_datatype.h"

typedef struct ST_OsdFileAttr_s
{
    char   FilePath[128];
    MI_U32 u32FileWidth;
    MI_U32 u32FileHeight;
    MI_RGN_PixelFormat_e eFileFormat;
}ST_OsdFileAttr_t;

MI_S32 ST_Common_GetRgnDefaultInitAttr(MI_RGN_PaletteTable_t *pstPaletteTable);
MI_S32 ST_Common_GetRgnDefaultCreateAttr(MI_RGN_Attr_t *pstRegion);

MI_S32 ST_Common_GetRgnDefaultOsdAttr(MI_RGN_ChnPortParam_t *pstChnAttr);
MI_S32 ST_Common_GetRgnDefaultCoverColorAttr(MI_RGN_AreaType_e eAreaType, MI_RGN_ChnPortParam_t *pstChnAttr);
MI_S32 ST_Common_GetRgnDefaultCoverMosicAttr(MI_RGN_AreaType_e eAreaType, MI_RGN_ChnPortParam_t *pstChnAttr);
MI_S32 ST_Common_GetRgnDefaultFrameAttr(MI_RGN_ChnPortParam_t *pstChnAttr);
MI_S32 ST_Common_GetRgnDefaultRotateAttr(MI_RGN_ChnPortParam_t *pstChnAttr);

MI_S32 ST_Common_RgnInit(MI_U16 u16SocId, MI_RGN_PaletteTable_t *pstPaletteTable);
MI_S32 ST_Common_RgnDeInit(MI_U16 u16SocId);

MI_S32 ST_Common_RgnCreate(MI_U16 u16SocId, MI_RGN_HANDLE hHandle, MI_RGN_Attr_t *pstRegion);
MI_S32 ST_Common_RgnDestroy(MI_U16 u16SocId, MI_RGN_HANDLE hHandle);

MI_S32 ST_Common_RgnAttachChn(MI_U16 u16SocId, MI_RGN_HANDLE hHandle,
                MI_RGN_ChnPort_t* pstChnPort, MI_RGN_ChnPortParam_t *pstChnAttr);
MI_S32 ST_Common_RgnDetachChn(MI_U16 u16SocId, MI_RGN_HANDLE hHandle, MI_RGN_ChnPort_t *pstChnPort);

MI_S32 ST_Common_Rgn_GetStrideByPixel(MI_U32 u32Width, MI_RGN_PixelFormat_e eFileFormat, MI_U32 *pu32Stride);
MI_S32 ST_Common_RgnUpdateCanvas(MI_U16 u16SocId, MI_RGN_HANDLE hHandle, ST_OsdFileAttr_t *pstOsdFileAttr);


void ST_Common_Rgn_SetImageSize(MI_U16 image_width, MI_U16 image_height);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_RGN_H_
