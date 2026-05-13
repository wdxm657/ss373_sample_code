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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mi_sys.h"
#include "st_common.h"

#include "mi_isp_cus3a_api.h"
#include "mi_iqserver_datatype.h"

#define BUFFER_SIZE_OUT_16BITS(w) ((((w + 7) / 8) * 8) * 2) // unsigned char

typedef struct MI_IQSERVER_PictureInfo_s
{
    MI_U32 u32Type;
    MI_U32 u32Width;
    MI_U32 u32Height;
    MI_U32 u32SensorBit;
} MI_IQSERVER_PictureInfo_t;

typedef struct MI_IQSERVER_BuffArray_s
{
    MI_U64                    u64Handle;
    MI_U32                    u32MemType;
    MI_U32                    u32BuffSize;
    MI_U32                    u32ContentSize;
    MI_IQSERVER_PictureInfo_t stPictureInfo;

    void *pBufAddr;
    void *pVirAddr;
    void *next, *back;
} MI_IQSERVER_BuffArray_t;

typedef struct MI_IQSERVER_Buff_s
{
    MI_U32 u32Type;
    MI_U32 u32Size;

    MI_IQSERVER_BuffArray_t stBuffArray;
} MI_IQSERVER_Buff_t;

typedef enum
{
    E_MI_IQSERVER_MEM_SYS = 0,
    E_MI_IQSERVER_MEM_MMA,
} MI_IQSERVER_MemType_e;

MI_S32 MI_IQSERVER_FreeDmaBuffer(MI_PHY u64MiuAddr, void *pVirtAddr, MI_U32 u32FreeSize)
{
    MI_SYS_Munmap(pVirtAddr, u32FreeSize);
    MI_SYS_MMA_Free(0, u64MiuAddr);
    ST_INFO("size:%d, miu addr = 0x%llx, vir addr=%p\n", (int)u32FreeSize, u64MiuAddr, pVirtAddr);

    return 0;
}

void *MI_IQSERVER_pAllocDmaBuffer(MI_U32 nReqSize, MI_PHY *pPhysAddr, MI_PHY *pMiuAddr, MI_U8 bCache)
{
    MI_S32 s32Ret     = 0;
    MI_U64 u64PhyAddr = 0;
    void  *pVirtAddr  = NULL;

    s32Ret = MI_SYS_MMA_Alloc(0, NULL, nReqSize, &u64PhyAddr);
    if (s32Ret != MI_SUCCESS)
    {
        ST_ERR("MI_SYS_MMA_Alloc failed\n");
        return NULL;
    }

    *pMiuAddr  = u64PhyAddr;
    *pPhysAddr = *pMiuAddr;

    s32Ret = MI_SYS_Mmap(u64PhyAddr, nReqSize, &pVirtAddr, bCache);
    if (s32Ret != MI_SUCCESS)
    {
        MI_SYS_MMA_Free(0, u64PhyAddr);
        ST_ERR("MI_SYS_Mmap failed\n");
        return NULL;
    }

    ST_INFO("size:%d, cached:%d, miu addr = 0x%llX, vir addr=0x%p\n", (int)nReqSize, (int)bCache, *pMiuAddr,
               pVirtAddr);
    return pVirtAddr;
}

static MI_S32 MI_IQSERVER_GetSensorRawData_ByWDMA(MI_U32 u32IspDev, MI_U32 u32IspChn, MI_U32 u32ExposureType,
                                                  MI_IQSERVER_Buff_t *pGetBuf)
{
    MI_PHY miuAddr                = 0;
    MI_PHY physAddr               = 0;
    MI_U32 u32timeout             = 0;
    void  *pVirAddr               = NULL;
    MI_S32 ret                    = MI_SUCCESS;
    MI_U32 u32BufferSize          = 0;
    MI_U32 u32isp_out_image_count = 0;
    MI_U32 hdrMode                = 0; // E_MI_VIF_HDR_TYPE_OFF;

    CameraRawStoreNode_e eNode;
    CusISPOutImage_t     tisp_out_image;
    CusImageResolution_t timage_resolution;

    if (hdrMode > 0)
    {
        if (u32ExposureType == 0)
        {
            eNode = eRawStoreNode_P1HEAD;
        }
        else
        {
            eNode = eRawStoreNode_P0HEAD;
        }
    }
    else
    {
        eNode = eRawStoreNode_P0HEAD;
    }

    ST_INFO("hdr:%d\n", hdrMode);

    // Get width and height from isp api.
    timage_resolution.u32Node = (MI_U32)eNode;
    MI_ISP_CUS3A_GetImageResolution(u32IspDev, u32IspChn, &timage_resolution);
    timage_resolution.u32image_width = ALIGN_UP(timage_resolution.u32image_width, 4);

    ST_INFO("Image width=%d, height=%d, depth=%u\n", timage_resolution.u32image_width,
               timage_resolution.u32image_height, timage_resolution.u32PixelDepth);

    u32BufferSize = BUFFER_SIZE_OUT_16BITS(timage_resolution.u32image_width) * timage_resolution.u32image_height;
    ST_INFO("BUFFER SIZE %d \n", u32BufferSize);

    if (u32BufferSize == 0)
    {
        ST_ERR("Error! Buffer size is 0.\n");
        ret = E_MI_ERR_IQSERVER_NOBUF;
        return ret;
    }
    ST_INFO("%s %d\n", __FUNCTION__, __LINE__);

    pVirAddr = MI_IQSERVER_pAllocDmaBuffer(u32BufferSize, &physAddr, &miuAddr, FALSE);
    if (pVirAddr == NULL)
    {
        ST_ERR("Error! Allocate buffer Error!!!\n");
        ret = MI_ERR_IQSERVER_NOMEM;
        return ret;
    }
    {
        tisp_out_image.u32enable           = 1;
        tisp_out_image.u32image_width      = timage_resolution.u32image_width;
        tisp_out_image.u32image_height     = timage_resolution.u32image_height;
        tisp_out_image.u64physical_address = physAddr;
        tisp_out_image.u32Node             = (MI_U32)eNode;

        ST_INFO("%s %d u32IspDev:%u u32IspChn:%u\n", __FUNCTION__, __LINE__, u32IspDev, u32IspChn);
        MI_ISP_CUS3A_EnableISPOutImage(u32IspDev, u32IspChn, &tisp_out_image);

        u32timeout = 100;
        do
        {
            usleep(1000 * 10);
            MI_ISP_CUS3A_GetISPOutImageCount(u32IspDev, u32IspChn, &u32isp_out_image_count);
        } while ((u32isp_out_image_count < 1) && (--u32timeout > 0));

        ST_INFO("sensor raw image count:%d, time:%d ms.\n", u32isp_out_image_count, (100 - u32timeout) * 10);
        ST_INFO("%s %d\n", __FUNCTION__, __LINE__);

        tisp_out_image.u32enable = 0;
        MI_ISP_CUS3A_EnableISPOutImage(u32IspDev, u32IspChn, &tisp_out_image);
    }

    pGetBuf->u32Size = u32BufferSize;
    pGetBuf->stBuffArray.u32ContentSize = u32BufferSize;
    pGetBuf->stBuffArray.pVirAddr   = pVirAddr;
    pGetBuf->stBuffArray.u64Handle  = miuAddr;
    pGetBuf->stBuffArray.u32MemType = E_MI_IQSERVER_MEM_MMA;

    return 0;
}

void MI_IQSERVER_DestroyData(MI_IQSERVER_Buff_t *pBuffer)
{
    ST_INFO("%s %d\n", __FUNCTION__, __LINE__);
    MI_IQSERVER_BuffArray_t *p     = NULL;
    MI_IQSERVER_BuffArray_t *pHead = NULL;
    MI_IQSERVER_BuffArray_t *pNext = NULL;

    pHead = &(pBuffer->stBuffArray);
    pNext = pHead->next;
    while (pNext != NULL)
    {
        if (pNext->u32MemType == E_MI_IQSERVER_MEM_SYS)
        {
            if (pNext->u64Handle)
            {
                MI_SYS_ChnOutputPortPutBuf(pNext->u64Handle);
            }
        }
        else if (pNext->u32MemType == E_MI_IQSERVER_MEM_MMA)
        {
            if (pNext->u64Handle)
            {
                MI_IQSERVER_FreeDmaBuffer(pNext->u64Handle, pNext->pVirAddr, pNext->u32ContentSize);
            }
        }

        p           = pNext;
        pHead->next = pNext->next;
        pNext       = pNext->next;

        if (p->pBufAddr)
        {
            free(p->pBufAddr);
        }
        free(p);
    }
}


void ST_GetRawData_Usage(void)
{
    printf("Usage:./prog_vif_getrawdata_demo) dump realtime pipline vif raw data\n"
            "need telnet 1 run vif->isp realtime pipeline app, telnet 2 run prog_vif_getrawdata_demo\n");
}

int main(int argc, char **argv)
{
    FILE *fp = NULL;
    MI_U16 u16DumpNum = 1;
    char cFileName[128] = {0};

    if (argc > 2)
    {
        ST_GetRawData_Usage();
        return -1;
    }

    MI_SYS_Init(0);

    MI_IQSERVER_Buff_t stGetBuf;
    memset(&stGetBuf, 0, sizeof(MI_IQSERVER_Buff_t));

    MI_U32 u32IspDev       = 0;
    MI_U32 u32IspChn       = 0;
    MI_U32 u32ExposureType = 0; // default for hdr close.

    int ret = MI_IQSERVER_GetSensorRawData_ByWDMA(u32IspDev, u32IspChn, u32ExposureType, &stGetBuf);

    if (ret != 0)
    {
        printf("get sensor raw fail!\n");
        goto EXIT;
    }

    sprintf(cFileName, "./out/vif/getrawdata_demo_vif.raw");

    ret = ST_Common_WriteFile(cFileName, &fp, (MI_U8 *)stGetBuf.stBuffArray.pVirAddr, stGetBuf.u32Size, &u16DumpNum, TRUE);
    if (ret != 0)
    {
        printf("write sensor raw to file fail!\n");
        goto EXIT;
    }

    printf("get sensor raw success!\n");

EXIT:
    MI_IQSERVER_DestroyData(&stGetBuf);

    MI_SYS_Exit(0);

    return 0;
}
