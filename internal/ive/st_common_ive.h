/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized diSclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _ST_COMMON_IVE_H_
#define _ST_COMMON_IVE_H_

#ifdef __cplusplus
extern "C"{
#endif// __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <stdlib.h>
#include <sys/types.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "mi_sys.h"
#include "mi_ive.h"
#include "mi_ive_datatype.h"

#define IVE_MAX_STRING_LENGTH (256)
#define IVE_RADIX_DECIMAL     (10)
#define IVE_MAX_IMAGE_PANEL   (3)
#define IVE_MAX_SRC_NUM       (4)
#define IVE_MAX_DST_NUM       (3)

#define CHECK_AND_FREE_IMAGE(stInputImage) \
{                                          \
    if (stInputImage.aphyPhyAddr[0])       \
    {                                      \
        ST_Common_IveFreeImage(&stInputImage);   \
        stInputImage.aphyPhyAddr[0] = 0;   \
    }                                      \
}

#define CHECK_AND_FREE_BUFFER(stInputBuffer) \
{                                            \
    if (stInputBuffer.phyPhyAddr)            \
    {                                        \
        ST_Common_IveFreeBuffer(&stInputBuffer);;  \
        stInputBuffer.phyPhyAddr = 0;        \
    }                                        \
}

MI_S32 ST_Common_IveCreateHandle(MI_IVE_HANDLE hHandle);
MI_S32 ST_Common_IveDestroy(MI_IVE_HANDLE hHandle);
MI_S32 ST_Common_IveAllocateMemory(MI_U32 u32Size, MI_PHY *pu64PhyAddr, MI_U8 **ppu8VirAddr, MI_U8 bMemset);
MI_S32 ST_Common_IveFreeMemory(MI_PHY u64PhyAddr, MI_U8 *pu8VirAddr, MI_U32 u32Size);
MI_S32 ST_Common_ReadFromFile(int s32FileHandle, char *Des, int u32Stride, int u32Width, MI_U32 u32Height, MI_U8 u8BytesPerPixel);

MI_S32 ST_Common_IveAllocateImage(MI_IVE_Image_t *pstImage, MI_IVE_ImageType_e enImageType, MI_U16 u16Stride, MI_U16 u16Width, MI_U16 u16Height);
MI_S32 ST_Common_IveFreeImage(MI_IVE_Image_t *pstImage);
MI_S32 ST_Common_IveAllocateBuffer(MI_IVE_MemInfo_t *pstBuffer, MI_U32 u32Size);
MI_S32 ST_Common_IveFreeBuffer(MI_IVE_MemInfo_t *pstBuffer);

MI_S32 ST_Common_IveInitInputImage(MI_IVE_SrcImage_t *image, const char *file_name);
MI_S32 ST_Common_IveInitInputImageEx(MI_IVE_SrcImage_t *image, int file_handle);
MI_S32 ST_Common_IveSaveOutputImage(MI_IVE_DstImage_t *image, const char *file_name);
MI_S32 ST_Common_IveSaveOutputImageEx(MI_IVE_DstImage_t *image, int file_handle);

MI_S32 ST_Common_IveInitInputBuffer(MI_IVE_MemInfo_t *buffer, const char *file_name, int size);
MI_S32 ST_Common_IveInitInputBufferEx(MI_IVE_MemInfo_t *buffer, int file_handle, int size);
MI_S32 ST_Common_IveSaveOutputBuffer(MI_IVE_MemInfo_t *buffer, const char *file_name, int size);
MI_S32 ST_Common_IveSaveOutputBufferEx(MI_IVE_MemInfo_t *buffer, int file_handle, int size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_IVE_H_

