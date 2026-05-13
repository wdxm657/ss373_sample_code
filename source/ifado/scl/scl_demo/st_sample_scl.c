#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <string.h>

#include "st_common.h"
#include "mi_scl.h"

#define SCL_USER_STRETCHBUFF_DEVID (1)
#define SCL_USER_STRETCHBUFF_PORTID E_MI_SCL_HWSCL2

typedef struct ST_StretchBuf_InputAttr_s
{
    char  sFileInputPath[128];
    MI_SYS_PixelFormat_e eInputPixel;
    MI_SYS_WindowSize_t stInputWinSize;
    MI_U32 u32InputStride;
    MI_SYS_WindowRect_t stCropWin;
}ST_StretchBuf_InputAttr_t;

typedef struct ST_StretchBuf_OutputAttr_s
{
    char  sFileOutputPath[128];
    MI_SYS_WindowSize_t stOutputWinSize;
    MI_SYS_PixelFormat_e eOutputPixel;
}ST_StretchBuf_OutputAttr_t;


MI_S32 ST_GetOneFrameByStride(FILE *fp, char *pData, int width, int stride, int yuvsize)
{
    MI_U32 current, end;
    int bufferheight = yuvsize / stride;
    int i = 0;

    if (fp == NULL)
    {
        perror("fp NULL");
        return -1;
    }

    current = ftell(fp);
    fseek(fp, 0, SEEK_END);
    end = ftell(fp);
    rewind(fp);

    if ((end - current) == 0 || (end - current) < yuvsize)
    {
        fseek(fp, 0, SEEK_SET);
        current = ftell(fp);
    }

    fseek(fp, current, SEEK_SET);
    printf("width %d, stride %d, buffer height %d \n", width, stride, bufferheight);
    for (i = 0; i < bufferheight; i++)
    {
        if (fread(pData + i * stride, 1, stride, fp) < stride)
        {
            printf("linecnt %d read size err \n", i);
            return 0;
        }
    }

    if (i == bufferheight)
    {
        return 1;
    }

    return 0;
}

MI_S32 ST_SclStretchBuff(ST_StretchBuf_InputAttr_t *pstStretchBuf_InputAttr, ST_StretchBuf_OutputAttr_t *pstStretchBuf_OutputAttr)
{
    char sDestFilePath[256];
    FILE  *src_fp;
    MI_S32 s32Ret = 0;
    MI_U32 u32SrcSize = 0, u32DestSize = 0;
    MI_PHY inputphyaddr = 0, outputaddr = 0;
    void *pviraddr = NULL, *pviroutaddr = NULL;
    MI_U32 u32SrcAddrOffset[2] = {0};
    MI_U32 u32DstAddrOffset[2] = {0};
    MI_U32 u32DestStride[3] = {0};
    MI_U16 u16ValidStride[3] = {0};
    MI_U16 u16DumpNum = 1;
    FILE *dset_fp = NULL;

    MI_SYS_PixelFormat_e eInputPixel = pstStretchBuf_InputAttr->eInputPixel;
    MI_U32 u32InputStride = pstStretchBuf_InputAttr->u32InputStride;
    MI_SYS_PixelFormat_e eOutputPixel = pstStretchBuf_OutputAttr->eOutputPixel;
    MI_SYS_WindowSize_t stInputWinSize = {0};
    MI_SYS_WindowRect_t stCropWin = {0};
    MI_SYS_WindowSize_t stOutputWinSize = {0};

    memcpy(&stInputWinSize, &pstStretchBuf_InputAttr->stInputWinSize, sizeof(MI_SYS_WindowSize_t));
    memcpy(&stCropWin, &pstStretchBuf_InputAttr->stCropWin, sizeof(MI_SYS_WindowRect_t));
    memcpy(&stOutputWinSize, &pstStretchBuf_OutputAttr->stOutputWinSize, sizeof(MI_SYS_WindowSize_t));


    s32Ret = ST_Common_OpenSourceFile(pstStretchBuf_InputAttr->sFileInputPath, &src_fp);
    if (s32Ret < 0)
    {
        printf("open file fail!\n");
        return -1;
    }

    printf("input pixel %d, stride %d, rect(%d,%d), output pixel %d, crop(%d,%d,%d,%d) rect(%d,%d) \n", eInputPixel, u32InputStride, stInputWinSize.u16Width, stInputWinSize.u16Height,
           eOutputPixel, stCropWin.u16X, stCropWin.u16Y, stCropWin.u16Width, stCropWin.u16Height, stOutputWinSize.u16Width, stOutputWinSize.u16Height);

    if (eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV || eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_UYVY || eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_YVYU || eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY)
    {
        u32SrcSize = u32InputStride * stInputWinSize.u16Height * 2;
        u16ValidStride[0] = stInputWinSize.u16Width * 2;
        u32SrcAddrOffset[0] = 0;
    }
    else if (eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 || eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21)
    {
        u32SrcSize = u32InputStride * stInputWinSize.u16Height * 3 / 2;
        u16ValidStride[0] = stInputWinSize.u16Width;
        u16ValidStride[1] = stInputWinSize.u16Width;
        u32SrcAddrOffset[0] = u32InputStride * stInputWinSize.u16Height;
    }
    else if (eInputPixel == E_MI_SYS_PIXEL_FRAME_ARGB8888 || eInputPixel == E_MI_SYS_PIXEL_FRAME_ABGR8888 || eInputPixel == E_MI_SYS_PIXEL_FRAME_BGRA8888)
    {
        u32SrcSize = u32InputStride * stInputWinSize.u16Height * 4;
        u16ValidStride[0] = stInputWinSize.u16Width * 4;
        u32SrcAddrOffset[0] = 0;
    }
    else if (eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422)
    {
        u32SrcSize = u32InputStride * stInputWinSize.u16Height * 2;
        u16ValidStride[0] = stInputWinSize.u16Width;
        u16ValidStride[1] = stInputWinSize.u16Width;
        u32SrcAddrOffset[0] = u32InputStride * stInputWinSize.u16Height;
    }
    else if (eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR)
    {
        u32SrcSize = u32InputStride * stInputWinSize.u16Height * 3 / 2;
        u16ValidStride[0] = stInputWinSize.u16Width;
        u16ValidStride[1] = stInputWinSize.u16Width / 2;
        u16ValidStride[2] = stInputWinSize.u16Width / 2;
        u32SrcAddrOffset[0] = u32InputStride * stInputWinSize.u16Height;
        u32SrcAddrOffset[1] = u32SrcAddrOffset[0] + u32InputStride / 2 * stInputWinSize.u16Height / 2;
    }
    else if (eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR)
    {
        u32SrcSize = u32InputStride * stInputWinSize.u16Height * 2;
        u16ValidStride[0] = stInputWinSize.u16Width;
        u16ValidStride[1] = stInputWinSize.u16Width / 2;
        u16ValidStride[2] = stInputWinSize.u16Width / 2;
        u32SrcAddrOffset[0] = u32InputStride * stInputWinSize.u16Height;
        u32SrcAddrOffset[1] = u32SrcAddrOffset[0] + u32InputStride / 2 * stInputWinSize.u16Height;
    }

    stOutputWinSize.u16Width = ALIGN_UP(stOutputWinSize.u16Width, 8);
    stOutputWinSize.u16Height = ALIGN_UP(stOutputWinSize.u16Height, 2);
    if (eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV || eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_UYVY || eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_YVYU || eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY)
    {
        u32DestStride[0] = ALIGN_UP(stOutputWinSize.u16Width, 16) * 2;
        u32DestSize = u32DestStride[0] * stOutputWinSize.u16Height;
        u32DstAddrOffset[0] = 0;
    }
    else if (eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420 || eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21)
    {
        u32DestStride[0] = ALIGN_UP(stOutputWinSize.u16Width, 16);
        u32DestStride[1] = ALIGN_UP(stOutputWinSize.u16Width, 16);
        u32DestSize = u32DestStride[0] * stOutputWinSize.u16Height * 3 / 2;
        u32DstAddrOffset[0] = u32DestStride[0] * stOutputWinSize.u16Height;
    }
    else if (eOutputPixel == E_MI_SYS_PIXEL_FRAME_ARGB8888 || eOutputPixel == E_MI_SYS_PIXEL_FRAME_ABGR8888 || eOutputPixel == E_MI_SYS_PIXEL_FRAME_BGRA8888)
    {
        u32DestStride[0] = ALIGN_UP(stOutputWinSize.u16Width, 16) * 4;
        u32DestSize = u32DestStride[0] * stOutputWinSize.u16Height;
        u32DstAddrOffset[0] = 0;
    }
    else if (eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422)
    {
        u32DestStride[0] = ALIGN_UP(stOutputWinSize.u16Width, 32);
        u32DestSize = u32DestStride[0] * stOutputWinSize.u16Height * 3 / 2;
        u32DstAddrOffset[0] = u32DestStride[0] * stOutputWinSize.u16Height;
    }
    else if (eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR)
    {
        u32DestStride[0] = ALIGN_UP(stOutputWinSize.u16Width, 32);
        u32DestSize = u32DestStride[0] * stOutputWinSize.u16Height * 3 / 2;
        u32DestStride[1] = u32DestStride[0] / 2;
        u32DestStride[2] = u32DestStride[0] / 2;
        u32DstAddrOffset[0] = u32DestStride[0] * stOutputWinSize.u16Height;
        u32DstAddrOffset[1] = u32DstAddrOffset[0] + u32DestStride[1] * stOutputWinSize.u16Height / 2;
    }
    else if (eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR)
    {
        u32DestStride[0] = ALIGN_UP(stOutputWinSize.u16Width, 32);
        u32DestSize = u32DestStride[0] * stOutputWinSize.u16Height * 2;
        u32DestStride[1] = u32DestStride[0] / 2;
        u32DestStride[2] = u32DestStride[0] / 2;
        u32DstAddrOffset[0] = u32DestStride[0] * stOutputWinSize.u16Height;
        u32DstAddrOffset[1] = u32DstAddrOffset[0] + u32DestStride[1] * stOutputWinSize.u16Height;
    }

    ST_INFO("srcsize %d, destsizee %d\n", u32SrcSize, u32DestSize);

    MI_SYS_MMA_Alloc(0, (MI_U8 *)"mma_heap_name0", u32SrcSize, &inputphyaddr);
    MI_SYS_Mmap(inputphyaddr, u32SrcSize, &pviraddr, FALSE);
    MI_SYS_MMA_Alloc(0, (MI_U8 *)"mma_heap_name0", u32DestSize, &outputaddr);
    MI_SYS_Mmap(outputaddr, u32DestSize, &pviroutaddr, FALSE);

    if (1 != ST_GetOneFrameByStride(src_fp, (char *)pviraddr, u16ValidStride[0], u32InputStride, u32SrcSize))
    {
        printf("read %s size %d fail \n", pstStretchBuf_InputAttr->sFileInputPath, u32SrcSize);
        s32Ret = -1;
        goto EXIT;
    }

    MI_SCL_DevAttr_t stDevAttr;
    stDevAttr.u32NeedUseHWOutPortMask = SCL_USER_STRETCHBUFF_PORTID;

    if(MI_SUCCESS != MI_SCL_CreateDevice(SCL_USER_STRETCHBUFF_DEVID, &stDevAttr))
    {
        ST_ERR("MI_SCL_CreateDevice fail dev %d!\n", SCL_USER_STRETCHBUFF_DEVID);
        s32Ret = -1;
        goto EXIT;
    }

    MI_SCL_DirectBuf_t stSrcBuff;
    memset(&stSrcBuff, 0x0, sizeof(MI_SCL_DirectBuf_t));
    stSrcBuff.u32Width = stInputWinSize.u16Width;
    stSrcBuff.u32Height = stInputWinSize.u16Height;
    stSrcBuff.ePixelFormat = eInputPixel;
    stSrcBuff.phyAddr[0] = inputphyaddr;
    stSrcBuff.phyAddr[1] = inputphyaddr + u32SrcAddrOffset[0];
    // stSrcBuff.phyAddr[2] = inputphyaddr+u32SrcAddrOffset[1];
    stSrcBuff.u32BuffSize = u32SrcSize;
    stSrcBuff.u32Stride[0] = u16ValidStride[0];
    stSrcBuff.u32Stride[1] = u16ValidStride[1];
    // stSrcBuff.u32Stride[2] = u16ValidStride[2];

    printf("src width %d, stride %d, size %d \n", stInputWinSize.u16Width, stSrcBuff.u32Stride[0], stSrcBuff.u32BuffSize);

    MI_SCL_DirectBuf_t stDestBuff;
    memset(&stDestBuff, 0x0, sizeof(MI_SCL_DirectBuf_t));
    stDestBuff.u32Width = stOutputWinSize.u16Width;
    stDestBuff.u32Height = stOutputWinSize.u16Height;
    stDestBuff.ePixelFormat = eOutputPixel;
    stDestBuff.phyAddr[0] = outputaddr;
    stDestBuff.phyAddr[1] = outputaddr + u32DstAddrOffset[0];
    // stDestBuff.phyAddr[2] = outputaddr+u32DstAddrOffset[1];
    stDestBuff.u32BuffSize = u32DestSize;
    stDestBuff.u32Stride[0] = u32DestStride[0];
    stDestBuff.u32Stride[1] = u32DestStride[1];
    // stDestBuff.u32Stride[2]=u32DestStride[2];

    MI_SYS_WindowRect_t stoutputCrop;
    memset(&stoutputCrop, 0x0, sizeof(MI_SYS_WindowRect_t));
    stoutputCrop.u16X = stCropWin.u16X;
    stoutputCrop.u16Y = stCropWin.u16Y;
    stoutputCrop.u16Width = stCropWin.u16Width;
    stoutputCrop.u16Height = stCropWin.u16Height;

    printf("dest width %d, stride %d, size %d \n", stOutputWinSize.u16Width, u32DestStride[0], stDestBuff.u32BuffSize);


    if(MI_SUCCESS != MI_SCL_StretchBuf(&stSrcBuff, &stoutputCrop, &stDestBuff, E_MI_SCL_FILTER_TYPE_AUTO))
    {
        ST_ERR("MI_SCL_StretchBuf error!\n");
        s32Ret = -1;
        goto EXIT;
    }

    if(MI_SUCCESS != MI_SCL_DestroyDevice(SCL_USER_STRETCHBUFF_DEVID))
    {
        ST_ERR("MI_SCL_DestroyDevice error!\n");
        s32Ret = -1;
        goto EXIT;
    }

    sprintf(sDestFilePath, "%s/scl_demo_stretchbuf.yuv", pstStretchBuf_OutputAttr->sFileOutputPath);

    ST_Common_WriteFile(sDestFilePath, &dset_fp, (MI_U8 *)pviroutaddr, u32DestSize, &u16DumpNum, TRUE);

EXIT:
    if (src_fp)
    {
        fclose(src_fp);
    }
    MI_SYS_Munmap(pviraddr, u32SrcSize);
    MI_SYS_Munmap(pviroutaddr, u32DestSize);
    MI_SYS_MMA_Free(0, inputphyaddr);
    MI_SYS_MMA_Free(0, outputaddr);

    return s32Ret;
}
static MI_S32 ST_Scl_InitStretchBufParam(ST_StretchBuf_InputAttr_t *pstStretchBuf_InputAttr,
            ST_StretchBuf_OutputAttr_t *pstStretchBuf_OutputAttr)
{

    memset(pstStretchBuf_InputAttr, 0x0, sizeof(ST_StretchBuf_InputAttr_t));
    memset(pstStretchBuf_OutputAttr, 0x0, sizeof(ST_StretchBuf_OutputAttr_t));

    sprintf(pstStretchBuf_InputAttr->sFileInputPath, "./resource/input/1920_1080_nv12.yuv");
    pstStretchBuf_InputAttr->eInputPixel = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    pstStretchBuf_InputAttr->stInputWinSize.u16Width = 1920;
    pstStretchBuf_InputAttr->stInputWinSize.u16Height = 1080;
    pstStretchBuf_InputAttr->stCropWin.u16X = 0;
    pstStretchBuf_InputAttr->stCropWin.u16Y = 0;
    pstStretchBuf_InputAttr->stCropWin.u16Width = 1920;
    pstStretchBuf_InputAttr->stCropWin.u16Height = 1080;
    pstStretchBuf_InputAttr->u32InputStride = 1920;

    sprintf(pstStretchBuf_OutputAttr->sFileOutputPath, "./out/scl");
    pstStretchBuf_OutputAttr->eOutputPixel = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    pstStretchBuf_OutputAttr->stOutputWinSize.u16Width  = 720;
    pstStretchBuf_OutputAttr->stOutputWinSize.u16Height = 540;

    return 0;
}

void ST_Scl_Usage(void)
{
    printf("Usage:./prog_scl_scl_demo) file -> scl(stretchbuf) -> file\n");

}

MI_S32 main(int argc, char **argv)
{
    ST_StretchBuf_InputAttr_t stStretchBuf_InputAttr;
    ST_StretchBuf_OutputAttr_t stStretchBuf_OutputAttr;

    ST_Scl_Usage();

    STCHECKRESULT(ST_Common_Sys_Init());

    ST_Scl_InitStretchBufParam(&stStretchBuf_InputAttr, &stStretchBuf_OutputAttr);

    STCHECKRESULT(ST_SclStretchBuff(&stStretchBuf_InputAttr, &stStretchBuf_OutputAttr));

    STCHECKRESULT(ST_Common_Sys_Exit());

    return 0;
}



