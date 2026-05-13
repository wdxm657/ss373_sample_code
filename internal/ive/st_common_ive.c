/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.ST_Common_GetIspDefaultPortAttr
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "st_common_ive.h"
#include "st_common.h"


MI_S32 ST_Common_IveCreateHandle(MI_IVE_HANDLE hHandle)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_IVE_Create(hHandle);

    return s32Ret;
}

MI_S32 ST_Common_IveDestroy(MI_IVE_HANDLE hHandle)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= MI_IVE_Destroy(hHandle);

    return s32Ret;
}

MI_S32 ST_Common_IveAllocateMemory(MI_U32 u32Size, MI_PHY *pu64PhyAddr, MI_U8 **ppu8VirAddr, MI_U8 bMemset)
{
    MI_S32 s32Ret = MI_SUCCESS;

    s32Ret |= ST_Common_AllocateMemory(u32Size, pu64PhyAddr, ppu8VirAddr);

    if(s32Ret != MI_SUCCESS)
    {
        return s32Ret;
    }

    if (*ppu8VirAddr == NULL)
    {
        return MI_IVE_ERR_NOMEM;
    }

    if (bMemset)
    {
        memset(*ppu8VirAddr, 0 , u32Size);
        MI_SYS_FlushInvCache(*ppu8VirAddr, u32Size);
    }
    return MI_SUCCESS;
}

MI_S32 ST_Common_IveFreeMemory(MI_PHY u64PhyAddr, MI_U8 *pu8VirAddr, MI_U32 u32Size)
{
    MI_S32 s32Ret = MI_SUCCESS;
    if (NULL != pu8VirAddr)
    {
       s32Ret = ST_Common_FreeMemory(u64PhyAddr, pu8VirAddr, u32Size);
    }
    return s32Ret;
}

MI_S32 ST_Common_IveAllocateImage(MI_IVE_Image_t *pstImage,
                            MI_IVE_ImageType_e  eImageType,
                            MI_U16              u16Stride,
                            MI_U16              u16Width,
                            MI_U16              u16Height)
{
    MI_S32 s32Ret = MI_SUCCESS;

    int    i                            = 0;
    MI_U32 u32Size[IVE_MAX_IMAGE_PANEL] = {0};

    memset(pstImage, 0, sizeof(MI_IVE_Image_t));

    pstImage->eType     = eImageType;
    pstImage->u16Width  = u16Width;
    pstImage->u16Height = u16Height;

    switch(eImageType) {
        // 64bit Gray
        case E_MI_IVE_IMAGE_TYPE_S64C1:
        case E_MI_IVE_IMAGE_TYPE_U64C1:
            u32Size[0] = u16Stride * u16Height * sizeof(MI_U64);

            pstImage->azu16Stride[0] = u16Stride;
            break;

        // 32bit Gray
        case E_MI_IVE_IMAGE_TYPE_S32C1:
        case E_MI_IVE_IMAGE_TYPE_U32C1:
            u32Size[0] = u16Stride * u16Height * sizeof(MI_U32);

            pstImage->azu16Stride[0] = u16Stride;
            break;

        // 16bit Gray
        case E_MI_IVE_IMAGE_TYPE_S16C1:
        case E_MI_IVE_IMAGE_TYPE_U16C1:
            u32Size[0] = u16Stride * u16Height * sizeof(MI_U16);

            pstImage->azu16Stride[0] = u16Stride;
            break;

        // 8bit Gray
        case E_MI_IVE_IMAGE_TYPE_S8C1:
        case E_MI_IVE_IMAGE_TYPE_U8C1:
            u32Size[0] = u16Stride * u16Height * sizeof(MI_U8);

            pstImage->azu16Stride[0] = u16Stride;
            break;

        // YUV 420 semi plane
        case E_MI_IVE_IMAGE_TYPE_YUV420P:
        case E_MI_IVE_IMAGE_TYPE_YUV420SP:
            u32Size[0] = u16Stride * u16Height * sizeof(MI_U8);
            u32Size[1] = u32Size[0] / 2;

            pstImage->azu16Stride[0] = u16Stride;
            pstImage->azu16Stride[1] = u16Stride;
            break;

        // YUV 422 semi plane
        case E_MI_IVE_IMAGE_TYPE_YUV422P:
        case E_MI_IVE_IMAGE_TYPE_YUV422SP:
            u32Size[0] = u16Stride * u16Height * sizeof(MI_U8);
            u32Size[1] = u32Size[0];

            pstImage->azu16Stride[0] = u16Stride;
            pstImage->azu16Stride[1] = u16Stride;
            break;

        // RGB packed
        case E_MI_IVE_IMAGE_TYPE_U8C3_PACKAGE:
            u32Size[0] = u16Stride * u16Height * sizeof(MI_U8) * 3;

            pstImage->azu16Stride[0] = u16Stride;
            break;

        // RGB plane
        case E_MI_IVE_IMAGE_TYPE_U8C3_PLANAR:
            u32Size[0] = u16Stride * u16Height * sizeof(MI_U8);
            u32Size[2] = u32Size[1] = u32Size[0];

            pstImage->azu16Stride[0] = u16Stride;
            pstImage->azu16Stride[1] = u16Stride;
            pstImage->azu16Stride[2] = u16Stride;
            break;

        case E_MI_IVE_IMAGE_TYPE_S8C2_PACKAGE:
            u32Size[0] = u16Stride * u16Height * sizeof(MI_U8) * 2;

            pstImage->azu16Stride[0] = u16Stride;
            break;

        case E_MI_IVE_IMAGE_TYPE_S8C2_PLANAR:
            u32Size[0] = u16Stride * u16Height * sizeof(MI_U8);
            u32Size[1] = u32Size[0];

            pstImage->azu16Stride[0] = u16Stride;
            pstImage->azu16Stride[1] = u16Stride;
            break;

        case E_MI_IVE_IMAGE_TYPE_YUV422_YUYV:
            u32Size[0] = u16Stride * u16Height * sizeof(MI_U8) * 2;

            pstImage->azu16Stride[0] = u16Stride;
            break;

        default:
            printf("[%s]:Format(%#x) is not support!!\n", __FUNCTION__, pstImage->eType);
            return MI_IVE_ERR_ILLEGAL_PARAM;
    }

    for (i = 0; i < IVE_MAX_IMAGE_PANEL; ++i)
    {
        if (0 != u32Size[i])
        {
            if (MI_SUCCESS != (s32Ret = ST_Common_IveAllocateMemory(u32Size[i], &pstImage->aphyPhyAddr[i], &pstImage->apu8VirAddr[i], 1)))
            {
                goto ERROR;
            }
        }
    }

    return s32Ret;

ERROR:
    for (i = 0; i < IVE_MAX_IMAGE_PANEL; ++i)
    {
        ST_Common_FreeMemory(pstImage->aphyPhyAddr[i], pstImage->apu8VirAddr[i], u32Size[i]);
    }

    memset(pstImage, 0, sizeof(MI_IVE_Image_t));
    return MI_IVE_ERR_NOMEM;
}

MI_S32 ST_Common_IveFreeImage(MI_IVE_Image_t *pstImage)
{
    MI_S32 s32Ret = MI_SUCCESS;

    int    i                            = 0;
    MI_U32 u32Size[IVE_MAX_IMAGE_PANEL] = {0};

    switch(pstImage->eType) {
        case E_MI_IVE_IMAGE_TYPE_S64C1:
        case E_MI_IVE_IMAGE_TYPE_U64C1:
            u32Size[0] = pstImage->azu16Stride[0]*pstImage->u16Height*sizeof(MI_U64);
            break;

        case E_MI_IVE_IMAGE_TYPE_S32C1:
        case E_MI_IVE_IMAGE_TYPE_U32C1:
            u32Size[0] = pstImage->azu16Stride[0]*pstImage->u16Height*sizeof(MI_U32);
            break;

        case E_MI_IVE_IMAGE_TYPE_S16C1:
        case E_MI_IVE_IMAGE_TYPE_U16C1:
            u32Size[0] = pstImage->azu16Stride[0]*pstImage->u16Height*sizeof(MI_U16);
            break;

        case E_MI_IVE_IMAGE_TYPE_S8C1:
        case E_MI_IVE_IMAGE_TYPE_U8C1:
            u32Size[0] = pstImage->azu16Stride[0]*pstImage->u16Height*sizeof(MI_U8);
            break;

        case E_MI_IVE_IMAGE_TYPE_U8C3_PACKAGE:
            u32Size[0] = pstImage->azu16Stride[0]*pstImage->u16Height*sizeof(MI_U8) * 3;
            break;

        case E_MI_IVE_IMAGE_TYPE_S8C2_PACKAGE:
            u32Size[0] = pstImage->azu16Stride[0]*pstImage->u16Height*sizeof(MI_U8) * 2;
            break;

        case E_MI_IVE_IMAGE_TYPE_YUV420P:
        case E_MI_IVE_IMAGE_TYPE_YUV420SP:
            u32Size[0] = pstImage->azu16Stride[0]*pstImage->u16Height*sizeof(MI_U8);
            u32Size[1] = u32Size[0] / 2;
            break;

        case E_MI_IVE_IMAGE_TYPE_YUV422P:
        case E_MI_IVE_IMAGE_TYPE_YUV422SP:
            u32Size[0] = pstImage->azu16Stride[0]*pstImage->u16Height*sizeof(MI_U8);
            u32Size[1] = u32Size[0];
            break;

        case E_MI_IVE_IMAGE_TYPE_S8C2_PLANAR:
            u32Size[0] = pstImage->azu16Stride[0]*pstImage->u16Height*sizeof(MI_U8);
            u32Size[1] = u32Size[0];
            break;

        case E_MI_IVE_IMAGE_TYPE_U8C3_PLANAR:
            u32Size[0] = pstImage->azu16Stride[0]*pstImage->u16Height*sizeof(MI_U8);
            u32Size[2] = u32Size[1] = u32Size[0];
            break;
        case E_MI_IVE_IMAGE_TYPE_YUV422_YUYV:
            u32Size[0] = pstImage->azu16Stride[0] * pstImage->u16Height*sizeof(MI_U8) * 2;
            break;

        default:
            printf("[%s]:Format(%#x) is not support!!\n", __FUNCTION__, pstImage->eType);
            s32Ret = MI_IVE_ERR_ILLEGAL_PARAM;
    }

    for (i = 0; i < IVE_MAX_IMAGE_PANEL; ++i)
    {
        if (0 != u32Size[i])
        {
            ST_Common_IveFreeMemory(pstImage->aphyPhyAddr[i], pstImage->apu8VirAddr[i], u32Size[i]);
        }
    }

    memset(pstImage, 0, sizeof(MI_IVE_Image_t));

    return s32Ret;
}


MI_S32 ST_Common_IveAllocateBuffer(MI_IVE_MemInfo_t *pstBuffer, MI_U32 u32Size)
{
    MI_S32 s32Set = MI_SUCCESS;

    memset(pstBuffer, 0, sizeof(MI_IVE_MemInfo_t));

    pstBuffer->u32Size = u32Size;

    s32Set |= ST_Common_IveAllocateMemory(u32Size, &pstBuffer->phyPhyAddr, &pstBuffer->pu8VirAddr, 1);

    if (pstBuffer->pu8VirAddr == NULL)
    {
        s32Set = MI_IVE_ERR_NOMEM;
    }

    return s32Set;
}

MI_S32 ST_Common_IveFreeBuffer(MI_IVE_MemInfo_t *pstBuffer)
{
    MI_S32 s32Set = MI_SUCCESS;

    s32Set |= ST_Common_FreeMemory(pstBuffer->phyPhyAddr, pstBuffer->pu8VirAddr, pstBuffer->u32Size);

    memset(pstBuffer, 0, sizeof(MI_IVE_MemInfo_t));

    return s32Set;
}

MI_S32 ST_Common_ReadFromFile(int s32FileHandle, char *Des, int u32Stride, int u32Width, MI_U32 u32Height, MI_U8 u8BytesPerPixel)
{
    MI_S32 s32Ret;
    int i            = 0;
    int s32Size      = (u32Stride != u32Width) ? u32Width * u8BytesPerPixel : u32Stride * u32Height * u8BytesPerPixel;
    int s32ReadTimes = (u32Stride != u32Width) ? u32Height : 1;

    for (i = 0; i < s32ReadTimes; ++i)
    {
        s32Ret = read(s32FileHandle, Des + i * u32Stride * u8BytesPerPixel, s32Size);
        if(s32Ret < 0)
        {
            return s32Ret;
        }
    }
    MI_SYS_FlushInvCache(Des, u32Stride * u32Height * u8BytesPerPixel);

    return MI_SUCCESS;
}

MI_S32 ST_Common_IveInitInputImageEx(MI_IVE_SrcImage_t *image, int file_handle)
{
    MI_S32 s32Ret = MI_SUCCESS;
    int    i                              = 0;
    MI_U8  u8BytesPerPixel                = 0;
    MI_U32 u32Height[IVE_MAX_IMAGE_PANEL] = {0};

    switch(image->eType)
    {
        // Gray
        case E_MI_IVE_IMAGE_TYPE_S8C1:
        case E_MI_IVE_IMAGE_TYPE_U8C1:
            u32Height[0]    = image->u16Height;
            u8BytesPerPixel = sizeof(MI_U8);
            break;

        case E_MI_IVE_IMAGE_TYPE_U16C1:
        case E_MI_IVE_IMAGE_TYPE_S16C1:
            u32Height[0]    = image->u16Height;
            u8BytesPerPixel = sizeof(MI_U16);
            break;

        case E_MI_IVE_IMAGE_TYPE_S32C1:
        case E_MI_IVE_IMAGE_TYPE_U32C1:
            u32Height[0]    = image->u16Height;
            u8BytesPerPixel = sizeof(MI_U32);
            break;

        case E_MI_IVE_IMAGE_TYPE_S64C1:
        case E_MI_IVE_IMAGE_TYPE_U64C1:
            u32Height[0]    = image->u16Height;
            u8BytesPerPixel = sizeof(MI_U64);
            break;

        // YUV 420 semi plane
        case E_MI_IVE_IMAGE_TYPE_YUV420SP:
            u32Height[0]     = image->u16Height;
            u32Height[1]     = u32Height[0] / 2;
            u8BytesPerPixel = sizeof(MI_U8);
            break;

        // YUV 422 semi plane
        case E_MI_IVE_IMAGE_TYPE_YUV422SP:
            u32Height[0]     = image->u16Height;
            u32Height[1]     = u32Height[0];
            u8BytesPerPixel = sizeof(MI_U8);
            break;

        // RGB packed
        case E_MI_IVE_IMAGE_TYPE_U8C3_PACKAGE:
            u32Height[0]     = image->u16Height;
            u8BytesPerPixel = sizeof(MI_U8) * 3;
            break;

        // RGB plane
        case E_MI_IVE_IMAGE_TYPE_U8C3_PLANAR:
            u32Height[0]     = image->u16Height;
            u32Height[2]     = u32Height[1] = u32Height[0];
            u8BytesPerPixel = sizeof(MI_U8);
            break;
        case E_MI_IVE_IMAGE_TYPE_YUV422_YUYV:
            u32Height[0]     = image->u16Height;
            u8BytesPerPixel = sizeof(MI_U8) * 2;
            break;
        default:
            printf("Unimplemented format %X\n", image->eType);
            return MI_IVE_ERR_ILLEGAL_PARAM;
    }

    for (i = 0; i < IVE_MAX_IMAGE_PANEL; ++i)
    {
        if (0 != u32Height[i])
        {
            s32Ret = ST_Common_ReadFromFile(file_handle, (char*)(image->apu8VirAddr[i]), image->azu16Stride[i], image->u16Width, u32Height[i], u8BytesPerPixel);
            if(s32Ret != MI_SUCCESS)
            {
                return s32Ret;
            }
        }
    }

    return MI_SUCCESS;
}

MI_S32 ST_Common_IveInitInputImage(MI_IVE_SrcImage_t *image, const char *file_name)
{
    int file_handle;
    MI_S32 s32Ret = MI_SUCCESS;

    file_handle = open(file_name, O_RDONLY);
    if (file_handle < 0)
    {
        printf("Can't open file_handle %s (%d: %s)\n", file_name, errno, strerror(errno));
        return MI_IVE_ERR_ILLEGAL_PARAM;
    }

    s32Ret = ST_Common_IveInitInputImageEx(image, file_handle);

    close(file_handle);

    return s32Ret;
}

MI_S32 ST_Common_IveSaveOutputImageEx(MI_IVE_DstImage_t *image, int file_handle)
{
    MI_S32 s32Ret = MI_SUCCESS;
    int    i                            = 0;
    MI_U32 u32Size[IVE_MAX_IMAGE_PANEL] = {0};

    // write to file_handle
    switch(image->eType)
    {
        case E_MI_IVE_IMAGE_TYPE_S64C1:
        case E_MI_IVE_IMAGE_TYPE_U64C1:
            u32Size[0] = image->azu16Stride[0]*image->u16Height*sizeof(MI_U64);
            break;

        case E_MI_IVE_IMAGE_TYPE_S32C1:
        case E_MI_IVE_IMAGE_TYPE_U32C1:
            u32Size[0] = image->azu16Stride[0]*image->u16Height*sizeof(MI_U32);
            break;

        case E_MI_IVE_IMAGE_TYPE_S16C1:
        case E_MI_IVE_IMAGE_TYPE_U16C1:
            u32Size[0] = image->azu16Stride[0]*image->u16Height*sizeof(MI_U16);
            break;

        case E_MI_IVE_IMAGE_TYPE_S8C1:
        case E_MI_IVE_IMAGE_TYPE_U8C1:
            u32Size[0] = image->azu16Stride[0]*image->u16Height*sizeof(MI_U8);
            break;

        case E_MI_IVE_IMAGE_TYPE_YUV420SP:
            u32Size[0] = image->azu16Stride[0]*image->u16Height*sizeof(MI_U8);
            u32Size[1] = u32Size[0] / 2;
            break;

        case E_MI_IVE_IMAGE_TYPE_YUV422SP:
            u32Size[0] = image->azu16Stride[0]*image->u16Height*sizeof(MI_U8);
            u32Size[1] = u32Size[0];
            break;

        case E_MI_IVE_IMAGE_TYPE_U8C3_PACKAGE:
            u32Size[0] = image->azu16Stride[0]*image->u16Height*sizeof(MI_U8) * 3;
            break;

        case E_MI_IVE_IMAGE_TYPE_U8C3_PLANAR:
            u32Size[0] = image->azu16Stride[0]*image->u16Height*sizeof(MI_U8);
            u32Size[2] = u32Size[1] = u32Size[0];
            break;

        case E_MI_IVE_IMAGE_TYPE_S8C2_PACKAGE:
            u32Size[0] = image->azu16Stride[0]*image->u16Height*sizeof(MI_U8) * 2;
            break;

        case E_MI_IVE_IMAGE_TYPE_S8C2_PLANAR:
            u32Size[0] = image->azu16Stride[0]*image->u16Height*sizeof(MI_U8);
            u32Size[1] = u32Size[0];
            break;

        case E_MI_IVE_IMAGE_TYPE_YUV422_YUYV:
            u32Size[0] = image->azu16Stride[0]*image->u16Height*sizeof(MI_U8) * 2;
            break;

        default:
            printf("[%s]:Format(%#x) is not support!!\n", __FUNCTION__, image->eType);
            return MI_IVE_ERR_ILLEGAL_PARAM;
    }

    for (i = 0; i < IVE_MAX_IMAGE_PANEL; ++i)
    {
        if (0 != u32Size[i])
        {
            MI_SYS_FlushInvCache(image->apu8VirAddr[i], u32Size[i]);
            s32Ret = write(file_handle, (char*)(image->apu8VirAddr[i]), u32Size[i]);
            if(s32Ret < 0)
            {
                return s32Ret;
            }
        }
    }
    return MI_SUCCESS;
}

MI_S32 ST_Common_IveSaveOutputImage(MI_IVE_DstImage_t *image, const char *file_name)
{
    MI_S32 s32Ret;
    int file_handle;

    file_handle = open(file_name, O_RDWR | O_CREAT, 0666);

    if (file_handle < 0)
    {
        printf("Can't open file_handle %s (%d: %s)\n", file_name, errno, strerror(errno));
        return MI_IVE_ERR_ILLEGAL_PARAM;
    }

    s32Ret = ST_Common_IveSaveOutputImageEx(image, file_handle);

    close(file_handle);

    return s32Ret;
}

MI_S32 ST_Common_IveInitInputBufferEx(MI_IVE_MemInfo_t *buffer, int file_handle, int size)
{
    MI_S32 s32Ret;
    s32Ret = read(file_handle, (char*)(buffer->pu8VirAddr), size);
    {
        if(s32Ret < 0)
        {
            return s32Ret;
        }
    }
    MI_SYS_FlushInvCache(buffer->pu8VirAddr, size);

    return MI_SUCCESS;
}

MI_S32 ST_Common_IveInitInputBuffer(MI_IVE_MemInfo_t *buffer, const char *file_name, int size)
{
    int file_handle;
    MI_S32 s32Ret = MI_SUCCESS;

    file_handle = open(file_name, O_RDONLY);
    if (file_handle < 0)
    {
        printf("Can't open file_handle %s (%d: %s)\n", file_name, errno, strerror(errno));
        return MI_IVE_ERR_ILLEGAL_PARAM;
    }

    s32Ret = read(file_handle, (char*)(buffer->pu8VirAddr), size);
    MI_SYS_FlushInvCache(buffer->pu8VirAddr, size);

    close(file_handle);

    return s32Ret;
}

MI_S32 ST_Common_IveSaveOutputBufferEx(MI_IVE_MemInfo_t *buffer, int file_handle, int size)
{
    MI_S32 s32Ret;
    MI_SYS_FlushInvCache(buffer->pu8VirAddr, size);
    s32Ret = write(file_handle, buffer->pu8VirAddr, size);
    if(s32Ret < 0)
    {
        return s32Ret;
    }
    return MI_SUCCESS;
}

MI_S32 ST_Common_IveSaveOutputBuffer(MI_IVE_MemInfo_t *buffer, const char *file_name, int size)
{
    MI_S32 s32Ret;
    int file_handle;

    file_handle = open(file_name, O_RDWR | O_CREAT, 0666);

    if (file_handle < 0)
    {
        printf("Can't open file_handle %s (%d: %s)\n", file_name, errno, strerror(errno));
        return MI_IVE_ERR_ILLEGAL_PARAM;
    }
    s32Ret = ST_Common_IveSaveOutputBufferEx(buffer, file_handle, size);

    close(file_handle);

    return s32Ret;
}

