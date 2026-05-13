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
#include <errno.h>
#include <sys/poll.h>
#include <fcntl.h>

#include "mi_sys.h"

#include "st_common.h"
#include "st_common_vif.h"
#include "st_common_isp.h"
#include "st_common_scl.h"
#include "st_common_venc.h"
#include "mi_iqserver.h"

#include "ss_uvc.h"
#include "ss_uvc_datatype.h"
#include "ss_uvc_xu.h"

#define H264_265DEV 0
#define JPEGDEV 8
#define USB_CAMERA0_INDEX 0
#define USB_CAMERA1_INDEX 1
#define USB_CAMERA2_INDEX 2
#define UVC_STREAM0 "uvc_stream0"
#define UVC_STREAM1 "uvc_stream1"
#define UVC_STREAM2 "uvc_stream2"
#define MAX_UVC_DEV_NUM 3
#define PATH_PREFIX "/mnt"
#define DEBUG_ES_FILE 0

#define ISP_DEV_ID 0
#define ISP_CHN_ID 0
#define ISP_PORT_ID 1
#define SCL_DEV_ID 1
#define SCL_CHANNLE_ID 0
#define SCL_PORT_ID 0

typedef struct
{
    MI_S32      dev_index;
    MI_U32      u32IspDev;
    MI_U32      u32IspChn;
    MI_U32      u32IspPort;
    MI_U32      u32SclDev;
    MI_U32      u32SclChn;
    MI_U32      u32SclPort;
    MI_U32      u32VencChn;
    const char *pszStreamName;
    bool        bForceIdr;
} ST_Stream_Attr_T;
typedef struct
{
    MI_U32           fcc;
    MI_U32           u32Width;
    MI_U32           u32Height;
    MI_U32           u32FrameRate;
    MI_SYS_ChnPort_t dstChnPort;
} ST_UvcSetting_Attr_T;

typedef struct VENC_STREAMS_s
{
    bool             used;
    MI_VENC_Stream_t stStream;
} VENC_STREAMS_t;

typedef struct
{
    VENC_STREAMS_t *pstuserptr_stream;
} ST_Uvc_Resource_t;

typedef struct
{
    char                 name[20];
    MI_S32               dev_index;
    SS_UVC_Handle_h      handle;
    ST_UvcSetting_Attr_T setting;
    ST_Uvc_Resource_t    res;
} ST_UvcDev_t;

typedef struct
{
    MI_S32      devnum;
    ST_UvcDev_t dev[];
} ST_UvcSrc_t;

typedef struct ST_UvcInputParam_s
{
    MI_BOOL bCheckResult;
    MI_U8   u8SensorIndex;
    MI_U8   u8HdrType;
    MI_U8   u8CmdIndex;
    MI_U16  u16Width;
    MI_U16  u16Height;
} ST_UvcInputParam_t;

static ST_Stream_Attr_T g_stStreamAttr[] = {[USB_CAMERA0_INDEX] =
                                                {
                                                    .dev_index     = 0,
                                                    .pszStreamName = UVC_STREAM0,
                                                    .u32IspDev     = ISP_DEV_ID,
                                                    .u32IspChn     = ISP_CHN_ID,
                                                    .u32IspPort    = ISP_PORT_ID,
                                                    .u32SclDev     = SCL_DEV_ID,
                                                    .u32SclChn     = SCL_CHANNLE_ID,
                                                    .u32SclPort    = SCL_PORT_ID,
                                                    .u32VencChn    = 0,
                                                    .bForceIdr     = FALSE,
                                                },
                                            [USB_CAMERA1_INDEX] =
                                                {
                                                    .dev_index     = 1,
                                                    .pszStreamName = UVC_STREAM1,
                                                    .u32IspDev     = ISP_DEV_ID,
                                                    .u32IspChn     = ISP_CHN_ID + 1,
                                                    .u32IspPort    = ISP_PORT_ID,
                                                    .u32SclDev     = SCL_DEV_ID,
                                                    .u32SclChn     = SCL_CHANNLE_ID + 1,
                                                    .u32SclPort    = SCL_PORT_ID,
                                                    .u32VencChn    = 1,
                                                    .bForceIdr     = FALSE,
                                                },
                                            [USB_CAMERA2_INDEX] =
                                                {
                                                    .dev_index     = 2,
                                                    .pszStreamName = UVC_STREAM2,
                                                    .u32IspDev     = ISP_DEV_ID,
                                                    .u32IspChn     = ISP_CHN_ID + 2,
                                                    .u32IspPort    = ISP_PORT_ID,
                                                    .u32SclDev     = SCL_DEV_ID,
                                                    .u32SclChn     = SCL_CHANNLE_ID + 2,
                                                    .u32SclPort    = SCL_PORT_ID,
                                                    .u32VencChn    = 2,
                                                    .bForceIdr     = FALSE,
                                                }};
static ST_UvcSrc_t *      g_UvcSrc;
static ST_UvcInputParam_t gstUvcInputParm = {0};

static MI_U8    g_bitrate[MAX_UVC_DEV_NUM] = {0, 0, 0};
static MI_U8    g_qfactor[MAX_UVC_DEV_NUM] = {0, 0, 0};
static MI_U8    g_maxbuf_cnt               = 3;
static MI_U8    g_device_num               = 1;
static char     IqBinPath[128];
static MI_U8    Transmission_Mode = 0;
static char     g_pipeline_active = 0;
static MI_U8    g_en_suspend      = 0;
pthread_mutex_t uvcdev_mutex;

static ST_UvcDev_t *Get_UVC_Device(void *uvc)
{
    SS_UVC_Device_t *pdev = (SS_UVC_Device_t *)uvc;

    for (MI_S32 i = 0; i < g_UvcSrc->devnum; i++)
    {
        ST_UvcDev_t *dev = &g_UvcSrc->dev[i];
        if (!strcmp(dev->name, pdev->name))
        {
            return dev;
        }
    }
    return NULL;
}

MI_S32 UVC_MM_FillBuffer(void *uvc, SS_UVC_BufInfo_t *bufInfo)
{
    MI_S32 s32Ret    = 0;
    MI_U32 u32Size   = 0;
    MI_U32 i         = 0;
    MI_U32 fcc       = 0;
    MI_U32 SclDevId  = 0;
    MI_U32 SclChnId  = 0;
    MI_U32 SclPortId = 0;
    MI_U32 VencDevId = 0;
    MI_U32 VencChnId = 0;

    MI_U8 * u8CopyData = (MI_U8 *)bufInfo->b.buf;
    MI_U32 *pu32length = (MI_U32 *)&bufInfo->length;

    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;
    MI_VENC_Stream_t  stStream;
    MI_VENC_Pack_t    stPack[4];
    MI_VENC_ChnStat_t stStat;

    MI_SYS_BufInfo_t  stBufInfo;
    MI_SYS_BUF_HANDLE stBufHandle;
    MI_SYS_ChnPort_t  stDstChnPort;

    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufHandle, 0, sizeof(MI_SYS_BUF_HANDLE));
    memset(&stDstChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&stPack, 0, sizeof(MI_VENC_Pack_t) * 4);
    memset(&stStream, 0, sizeof(MI_VENC_Stream_t));
    memset(&stStat, 0, sizeof(MI_VENC_ChnStat_t));

    ST_UvcDev_t *dev = Get_UVC_Device(uvc);
    if (!dev)
        return -1;
    fcc       = dev->setting.fcc;
    SclDevId  = pstStreamAttr[dev->dev_index].u32SclDev;
    SclChnId  = pstStreamAttr[dev->dev_index].u32SclChn;
    SclPortId = pstStreamAttr[dev->dev_index].u32SclPort;
    VencDevId = (fcc == V4L2_PIX_FMT_MJPEG) ? JPEGDEV : H264_265DEV;
    VencChnId = pstStreamAttr[dev->dev_index].u32VencChn;

    switch (fcc)
    {
        case V4L2_PIX_FMT_YUYV:
        {
            stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
            stDstChnPort.u32DevId  = SclDevId;
            stDstChnPort.u32ChnId  = SclChnId;
            stDstChnPort.u32PortId = SclPortId;

            s32Ret = MI_SYS_ChnOutputPortGetBuf(&stDstChnPort, &stBufInfo, &stBufHandle);
            if (MI_SUCCESS != s32Ret)
            {
                return -EINVAL;
            }

            *pu32length = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
            memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[0], *pu32length);

            s32Ret = MI_SYS_ChnOutputPortPutBuf(stBufHandle);
            if (MI_SUCCESS != s32Ret)
                printf("%s Release Frame Failed\n", __func__);
        }
        break;
        case V4L2_PIX_FMT_NV12:
        {
            stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
            stDstChnPort.u32DevId  = SclDevId;
            stDstChnPort.u32ChnId  = SclChnId;
            stDstChnPort.u32PortId = SclPortId;

            s32Ret = MI_SYS_ChnOutputPortGetBuf(&stDstChnPort, &stBufInfo, &stBufHandle);
            if (MI_SUCCESS != s32Ret)
                return -EINVAL;

            *pu32length = stBufInfo.stFrameData.u16Height
                          * (stBufInfo.stFrameData.u32Stride[0] + stBufInfo.stFrameData.u32Stride[1] / 2);
            memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[0],
                   stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0]);
            u8CopyData += stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
            memcpy(u8CopyData, stBufInfo.stFrameData.pVirAddr[1],
                   stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[1] / 2);

            s32Ret = MI_SYS_ChnOutputPortPutBuf(stBufHandle);
            if (MI_SUCCESS != s32Ret)
                printf("%s Release Frame Failed\n", __func__);
        }
        break;
        case V4L2_PIX_FMT_MJPEG:
        {
            stStream.pstPack = stPack;
            s32Ret           = MI_VENC_Query(VencDevId, VencChnId, &stStat);
            if (s32Ret != 0 || stStat.u32CurPacks == 0)
                return -EINVAL;

            stStream.u32PackCount = stStat.u32CurPacks;

            s32Ret = MI_VENC_GetStream(VencDevId, VencChnId, &stStream, 40);
            if (0 != s32Ret)
                return -EINVAL;
            bufInfo->is_keyframe = TRUE;

            for (i = 0; i < stStat.u32CurPacks; i++)
            {
                u32Size = stStream.pstPack[i].u32Len;
                memcpy(u8CopyData, stStream.pstPack[i].pu8Addr, u32Size);
                u8CopyData += u32Size;
            }
            *pu32length      = u8CopyData - (MI_U8 *)bufInfo->b.buf;
            bufInfo->is_tail = TRUE; // default is frameEnd

            s32Ret = MI_VENC_ReleaseStream(VencDevId, VencChnId, &stStream);
            if (0 != s32Ret)
                printf("%s Release Frame Failed\n", __func__);
        }
        break;
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
        {
            stStream.pstPack = stPack;
            s32Ret           = MI_VENC_Query(VencDevId, VencChnId, &stStat);
            if (s32Ret != 0 || stStat.u32CurPacks == 0)
                return -EINVAL;

            stStream.u32PackCount = stStat.u32CurPacks;

            s32Ret = MI_VENC_GetStream(VencDevId, VencChnId, &stStream, 40);
            if (0 != s32Ret)
                return -EINVAL;
            if (((fcc == V4L2_PIX_FMT_H264) && (stStream.stH264Info.eRefType == E_MI_VENC_BASE_IDR))
                || ((fcc == V4L2_PIX_FMT_H265) && (stStream.stH265Info.eRefType == E_MI_VENC_BASE_IDR)))
            {
                bufInfo->is_keyframe = TRUE;
            }
            else
            {
                bufInfo->is_keyframe = FALSE;
            }

            for (i = 0; i < stStat.u32CurPacks; i++)
            {
                u32Size = stStream.pstPack[i].u32Len;
                memcpy(u8CopyData, stStream.pstPack[i].pu8Addr, u32Size);
                u8CopyData += u32Size;
            }
            *pu32length      = u8CopyData - (MI_U8 *)bufInfo->b.buf;
            bufInfo->is_tail = TRUE; // default is frameEnd

            s32Ret = MI_VENC_ReleaseStream(VencDevId, VencChnId, &stStream);
            if (0 != s32Ret)
                printf("%s Release Frame Failed\n", __func__);

            if (pstStreamAttr[dev->dev_index].bForceIdr && dev->setting.fcc != V4L2_PIX_FMT_MJPEG)
            {
                pstStreamAttr[dev->dev_index].bForceIdr = FALSE;
                s32Ret                                  = MI_VENC_RequestIdr(VencDevId, VencChnId, TRUE);
                if (0 != s32Ret)
                    printf("MI_VENC_RequestIdr failed:0x%x\n", s32Ret);
            }
        }
        break;
        default:
            printf("unknown format %d\n", fcc);
            return -EINVAL;
    }

    return 0;
}

MI_S32 UVC_UP_FinishBuffer(void *uvc, SS_UVC_BufInfo_t *bufInfo)
{
    MI_S32            s32Ret    = 0;
    MI_U32            fcc       = 0;
    MI_U32            VencDevId = 0;
    MI_U32            VencChnId = 0;
    ST_UvcDev_t *     dev       = Get_UVC_Device(uvc);
    MI_SYS_BUF_HANDLE stBufHandle;
    ST_Stream_Attr_T *pstStreamAttr  = g_stStreamAttr;
    VENC_STREAMS_t *  pUserptrStream = NULL;

    fcc       = dev->setting.fcc;
    VencDevId = (fcc == V4L2_PIX_FMT_MJPEG) ? JPEGDEV : H264_265DEV;
    VencChnId = pstStreamAttr[dev->dev_index].u32VencChn;

    if (!dev)
        return -1;
    switch (fcc)
    {
        break;
        case V4L2_PIX_FMT_YUYV:
        {
            stBufHandle = bufInfo->b.handle;

            s32Ret = MI_SYS_ChnOutputPortPutBuf(stBufHandle);
            if (MI_SUCCESS != s32Ret)
            {
                printf("%s Release Frame Failed\n", __func__);
                return s32Ret;
            }
            break;
        }
        case V4L2_PIX_FMT_NV12:
        {
            stBufHandle = bufInfo->b.handle;

            s32Ret = MI_SYS_ChnOutputPortPutBuf(stBufHandle);
            if (MI_SUCCESS != s32Ret)
            {
                printf("%s Release Frame Failed\n", __func__);
                return s32Ret;
            }
            break;
        }
        case V4L2_PIX_FMT_MJPEG:
        {
            pUserptrStream = (VENC_STREAMS_t *)bufInfo->b.handle;

            s32Ret = MI_VENC_ReleaseStream(VencDevId, VencChnId, &pUserptrStream->stStream);
            if (0 != s32Ret)
            {
                printf("%s Release Frame Failed\n", __func__);
                return s32Ret;
            }
            pUserptrStream->used = FALSE;
        }
        break;
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
        {
            pUserptrStream = (VENC_STREAMS_t *)bufInfo->b.handle;

            s32Ret = MI_VENC_ReleaseStream(VencDevId, VencChnId, &pUserptrStream->stStream);
            if (0 != s32Ret)
            {
                printf("%s Release Frame Failed\n", __func__);
                return s32Ret;
            }
            pUserptrStream->used = FALSE;
        }
        break;
    }

    return 0;
}

MI_S32 UVC_UP_FillBuffer(void *uvc, SS_UVC_BufInfo_t *bufInfo)
{
    MI_S32 s32Ret    = 0;
    MI_U32 fcc       = 0;
    MI_U32 SclDevId  = 0;
    MI_U32 SclChnId  = 0;
    MI_U32 SclPortId = 0;
    MI_U32 VencDevId = 0;
    MI_U32 VencChnId = 0;

    ST_UvcDev_t *     dev            = Get_UVC_Device(uvc);
    ST_Stream_Attr_T *pstStreamAttr  = g_stStreamAttr;
    VENC_STREAMS_t *  pUserptrStream = NULL;
    MI_VENC_Stream_t *pstStream      = NULL;
    MI_VENC_ChnStat_t stStat;
    MI_SYS_BufInfo_t  stBufInfo;
    MI_SYS_BUF_HANDLE stBufHandle;
    MI_SYS_ChnPort_t  stDstChnPort;

    memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));
    memset(&stBufHandle, 0, sizeof(MI_SYS_BUF_HANDLE));
    memset(&stDstChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    memset(&stStat, 0x0, sizeof(MI_VENC_ChnStat_t));

    if (!dev)
        return -1;

    fcc       = dev->setting.fcc;
    SclDevId  = pstStreamAttr[dev->dev_index].u32SclDev;
    SclChnId  = pstStreamAttr[dev->dev_index].u32SclChn;
    SclPortId = pstStreamAttr[dev->dev_index].u32SclPort;
    VencDevId = (fcc == V4L2_PIX_FMT_MJPEG) ? JPEGDEV : H264_265DEV;
    VencChnId = pstStreamAttr[dev->dev_index].u32VencChn;

    switch (fcc)
    {
        case V4L2_PIX_FMT_YUYV:
        {
            stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
            stDstChnPort.u32DevId  = SclDevId;
            stDstChnPort.u32ChnId  = SclChnId;
            stDstChnPort.u32PortId = SclPortId;
            s32Ret                 = MI_SYS_ChnOutputPortGetBuf(&stDstChnPort, &stBufInfo, &stBufHandle);
            if (MI_SUCCESS != s32Ret)
                return -EINVAL;

            bufInfo->b.start  = (long unsigned int)stBufInfo.stFrameData.pVirAddr[0];
            bufInfo->length   = stBufInfo.stFrameData.u16Height * stBufInfo.stFrameData.u32Stride[0];
            bufInfo->b.handle = (long unsigned int)stBufHandle;
            break;
        }
        case V4L2_PIX_FMT_NV12:
        {
            stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
            stDstChnPort.u32DevId  = SclDevId;
            stDstChnPort.u32ChnId  = SclChnId;
            stDstChnPort.u32PortId = SclPortId;
            s32Ret                 = MI_SYS_ChnOutputPortGetBuf(&stDstChnPort, &stBufInfo, &stBufHandle);
            if (MI_SUCCESS != s32Ret)
                return -EINVAL;

            bufInfo->b.start = (long unsigned int)stBufInfo.stFrameData.pVirAddr[0];
            bufInfo->length  = stBufInfo.stFrameData.u16Height
                              * (stBufInfo.stFrameData.u32Stride[0] + stBufInfo.stFrameData.u32Stride[1] / 2);
            bufInfo->b.handle = (long unsigned int)stBufHandle;
            break;
        }
        case V4L2_PIX_FMT_MJPEG:
        {
            for (MI_S32 i = 0; i < g_maxbuf_cnt; i++)
            {
                if (!dev->res.pstuserptr_stream[i].used)
                {
                    pUserptrStream = &dev->res.pstuserptr_stream[i];
                    break;
                }
            }
            if (!pUserptrStream)
            {
                return -EINVAL;
            }
            pstStream = &pUserptrStream->stStream;
            memset(pstStream->pstPack, 0, sizeof(MI_VENC_Pack_t) * 4);
            s32Ret = MI_VENC_Query(VencDevId, VencChnId, &stStat);
            if (s32Ret != 0 || stStat.u32CurPacks == 0)
                return -EINVAL;

            pstStream->u32PackCount = 1; // only need 1 packet
            s32Ret                  = MI_VENC_GetStream(VencDevId, VencChnId, pstStream, 40);
            if (0 != s32Ret)
                return -EINVAL;

            bufInfo->is_keyframe = FALSE;
            bufInfo->b.start     = (long unsigned int)pstStream->pstPack[0].pu8Addr;
            bufInfo->length      = pstStream->pstPack[0].u32Len;
            bufInfo->b.handle    = (long unsigned int)pUserptrStream;
            pUserptrStream->used = TRUE;
        }
        break;
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
        {
            for (MI_S32 i = 0; i < g_maxbuf_cnt; i++)
            {
                if (!dev->res.pstuserptr_stream[i].used)
                {
                    pUserptrStream = &dev->res.pstuserptr_stream[i];
                    break;
                }
            }
            if (!pUserptrStream)
            {
                return -EINVAL;
            }
            pstStream = &pUserptrStream->stStream;
            memset(pstStream->pstPack, 0, sizeof(MI_VENC_Pack_t) * 4);
            s32Ret = MI_VENC_Query(VencDevId, VencChnId, &stStat);
            if (s32Ret != 0 || stStat.u32CurPacks == 0)
                return -EINVAL;

            pstStream->u32PackCount = 1; // only need 1 packet
            s32Ret                  = MI_VENC_GetStream(VencDevId, VencChnId, pstStream, 40);
            if (0 != s32Ret)
                return -EINVAL;
            if (((fcc == V4L2_PIX_FMT_H264) && (pstStream->stH264Info.eRefType == E_MI_VENC_BASE_IDR))
                || ((fcc == V4L2_PIX_FMT_H265) && (pstStream->stH265Info.eRefType == E_MI_VENC_BASE_IDR)))
            {
                bufInfo->is_keyframe = TRUE;
            }
            else
            {
                bufInfo->is_keyframe = FALSE;
            }

            bufInfo->b.start     = (long unsigned int)pstStream->pstPack[0].pu8Addr;
            bufInfo->length      = pstStream->pstPack[0].u32Len;
            bufInfo->b.handle    = (long unsigned int)pUserptrStream;
            pUserptrStream->used = TRUE;

            if (pstStreamAttr[dev->dev_index].bForceIdr && fcc != V4L2_PIX_FMT_MJPEG)
            {
                pstStreamAttr[dev->dev_index].bForceIdr = FALSE;
                s32Ret                                  = MI_VENC_RequestIdr(VencDevId, VencChnId, TRUE);
                if (0 != s32Ret)
                    printf("MI_VENC_RequestIdr failed:%d\n", s32Ret);
            }
        }
        break;
        default:
            return -EINVAL;
    }

    return 0;
}

MI_S32 UVC_StartCapture(void *uvc, Stream_Params_t format)
{
    MI_U32 fcc           = 0;
    MI_U32 u32Width      = 0;
    MI_U32 u32Height     = 0;
    MI_U32 u32FrameRate  = 0;
    MI_U32 u32VenBitRate = 0;
    MI_U32 u32VenQfactor = 80;
    bool   bByFrame      = TRUE;
    char   IqApiBinFilePath[128];

    MI_U32 SnrPadId      = 0;
    MI_U8  u8SensorRes   = 0;
    MI_U32 u32VifGroupId = 0;
    MI_U32 u32VifDevId   = 0;
    MI_U32 u32VifPortId  = 0;
    MI_U32 IspDevId      = 0;
    MI_U32 IspChnId      = 0;
    MI_U32 IspPortId     = 0;
    MI_U32 SclDevId      = 0;
    MI_U32 SclChnId      = 0;
    MI_U32 SclPortId     = 0;
    MI_U32 VencDevId     = 0;
    MI_U32 VencChnId     = 0;

    MI_U32 u32SrcFrmrate = 0;
    MI_U32 u32DstFrmrate = 0;
    MI_U32 u32BindParam  = 0;

    MI_SNR_PlaneInfo_t      stPlaneInfo;
    MI_VIF_GroupAttr_t      stVifGroupAttr;
    MI_VIF_DevAttr_t        stVifDevAttr;
    MI_VIF_OutputPortAttr_t stVifPortAttr;

    MI_ISP_DevAttr_t      stIspDevAttr;
    MI_ISP_ChannelAttr_t  stIspChnAttr;
    MI_ISP_ChnParam_t     stIspChnParam;
    MI_SYS_WindowRect_t   stIspInputCrop;
    MI_ISP_OutPortParam_t stIspOutPortParam;

    MI_SCL_DevAttr_t      stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t     stSclChnParam;
    MI_SYS_WindowRect_t   stSclInputCrop;
    MI_SCL_OutPortParam_t stSclOutPortParam;

    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;

    MI_SYS_ChnPort_t  stSrcChnPort;
    MI_SYS_ChnPort_t  stDstChnPort;
    MI_SYS_BindType_e eBindType;
    MI_SYS_ChnPort_t  stSclChnPort;

    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;

    memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));
    memset(&stVifGroupAttr, 0x00, sizeof(MI_VIF_GroupAttr_t));
    memset(&stVifDevAttr, 0x00, sizeof(MI_VIF_DevAttr_t));
    memset(&stVifDevAttr, 0x00, sizeof(MI_VIF_DevAttr_t));
    memset(&stVifPortAttr, 0x00, sizeof(MI_VIF_OutputPortAttr_t));

    memset(&stIspDevAttr, 0x00, sizeof(MI_ISP_DevAttr_t));
    memset(&stIspChnAttr, 0x00, sizeof(MI_ISP_ChannelAttr_t));
    memset(&stIspChnParam, 0x00, sizeof(MI_ISP_ChnParam_t));
    memset(&stIspInputCrop, 0x00, sizeof(MI_SYS_WindowRect_t));
    memset(&stIspOutPortParam, 0x00, sizeof(MI_ISP_OutPortParam_t));

    memset(&stSclDevAttr, 0x00, sizeof(MI_SCL_DevAttr_t));
    memset(&stSclChnAttr, 0x00, sizeof(MI_SCL_ChannelAttr_t));
    memset(&stSclChnParam, 0x00, sizeof(MI_SCL_ChnParam_t));
    memset(&stSclInputCrop, 0x00, sizeof(MI_SYS_WindowRect_t));
    memset(&stSclOutPortParam, 0x00, sizeof(MI_SCL_OutPortParam_t));

    memset(&stVencInitParam, 0x00, sizeof(MI_VENC_InitParam_t));
    memset(&stVencChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));

    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stSclChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));

    /************************************************
    Init General Param
    *************************************************/
    ST_UvcDev_t *dev = Get_UVC_Device(uvc);
    if (!dev)
        return -1;

    memset(&dev->setting, 0x00, sizeof(dev->setting));
    dev->setting.fcc          = format.fcc;
    dev->setting.u32Width     = format.width;
    dev->setting.u32Height    = format.height;
    dev->setting.u32FrameRate = format.frameRate;

    fcc          = dev->setting.fcc;
    u32Width     = dev->setting.u32Width;
    u32Height    = dev->setting.u32Height;
    u32FrameRate = dev->setting.u32FrameRate;

    IspDevId  = pstStreamAttr[dev->dev_index].u32IspDev;
    IspChnId  = pstStreamAttr[dev->dev_index].u32IspChn;
    IspPortId = pstStreamAttr[dev->dev_index].u32IspPort;
    SclDevId  = pstStreamAttr[dev->dev_index].u32SclDev;
    SclChnId  = pstStreamAttr[dev->dev_index].u32SclChn;
    SclPortId = pstStreamAttr[dev->dev_index].u32SclPort;
    VencDevId = (fcc == V4L2_PIX_FMT_MJPEG) ? JPEGDEV : H264_265DEV;
    VencChnId = pstStreamAttr[dev->dev_index].u32VencChn;

    /************************************************
    Init User Param
    *************************************************/
    if (u32Width * u32Height > 2560 * 1440)
    {
        u32VenBitRate = 1024 * 1024 * 8;
    }
    else if (u32Width * u32Height >= 1920 * 1080)
    {
        u32VenBitRate = 1024 * 1024 * 4;
    }
    else if (u32Width * u32Height < 640 * 480)
    {
        u32VenBitRate = 1024 * 500;
    }
    else
    {
        u32VenBitRate = 1024 * 1024 * 2;
    }

    if (g_bitrate[dev->dev_index])
    {
        u32VenBitRate = g_bitrate[dev->dev_index] * 1024 * 1024;
    }

    if (g_qfactor[dev->dev_index])
    {
        u32VenQfactor = g_qfactor[dev->dev_index];
    }
    else
    {
        u32VenQfactor = 80;
    }

    if (!dev->res.pstuserptr_stream)
    {
        dev->res.pstuserptr_stream = (VENC_STREAMS_t *)calloc(g_maxbuf_cnt, sizeof(VENC_STREAMS_t));
        for (MI_S32 i = 0; i < g_maxbuf_cnt; i++)
        {
            dev->res.pstuserptr_stream[i].stStream.pstPack = (MI_VENC_Pack_t *)calloc(4, sizeof(MI_VENC_Pack_t));
        }
    }
    /************************************************
    start pipeline
    *************************************************/
    pthread_mutex_lock(&uvcdev_mutex);
    if (g_pipeline_active < 1)
    {
        STCHECKRESULT(ST_Common_Sys_Init());
        u8SensorRes = gstUvcInputParm.u8SensorIndex;

        if (gstUvcInputParm.u8HdrType)
            STCHECKRESULT(ST_Common_SensorInit(SnrPadId, TRUE, u8SensorRes, 0xFF));
        else
            STCHECKRESULT(ST_Common_SensorInit(SnrPadId, FALSE, u8SensorRes, 0xFF));

        STCHECKRESULT(MI_SNR_GetPlaneInfo(SnrPadId, 0, &stPlaneInfo));
        gstUvcInputParm.u16Width  = stPlaneInfo.stCapRect.u16Width;
        gstUvcInputParm.u16Height = stPlaneInfo.stCapRect.u16Height;
        ST_Common_GetVifDefaultGrouptAttr(&stVifGroupAttr);

        if (gstUvcInputParm.u8HdrType)
            stVifGroupAttr.eHDRType = E_MI_VIF_HDR_TYPE_VC;
        else
            stVifGroupAttr.eHDRType = E_MI_VIF_HDR_TYPE_OFF;

        STCHECKRESULT(ST_Common_VifCreateDevGroup(u32VifGroupId, &stVifGroupAttr));

        ST_Common_GetVifDefaultDevAttr(&stVifDevAttr);
        STCHECKRESULT(ST_Common_VifEnableDev(u32VifDevId, &stVifDevAttr));

        ST_Common_GetVifDefaultPortAttr(&stVifPortAttr);
        STCHECKRESULT(ST_Common_VifEnablePort(u32VifDevId, u32VifPortId, &stVifPortAttr));

        ST_Common_GetIspDefaultDevAttr(&stIspDevAttr);
        STCHECKRESULT(ST_Common_IspCreateDevice(IspDevId, &stIspDevAttr));
    }
    g_pipeline_active = g_pipeline_active + 1;
    pthread_mutex_unlock(&uvcdev_mutex);
    /************************************************
    init isp
    *************************************************/

    ST_Common_GetIspDefaultChnAttr(&stIspChnAttr, &stIspInputCrop, &stIspChnParam);

    if (gstUvcInputParm.u8HdrType)
        stIspChnParam.eHDRType = E_MI_ISP_HDR_TYPE_VC;
    else
        stIspChnParam.eHDRType = E_MI_ISP_HDR_TYPE_OFF;

    STCHECKRESULT(ST_Common_IspStartChn(IspDevId, IspChnId, &stIspChnAttr, &stIspInputCrop, &stIspChnParam));

    ST_Common_GetIspDefaultPortAttr(&stIspOutPortParam);
    stIspOutPortParam.stCropRect.u16Width  = gstUvcInputParm.u16Width;
    stIspOutPortParam.stCropRect.u16Height = gstUvcInputParm.u16Height;
    STCHECKRESULT(ST_Common_IspEnablePort(IspDevId, IspChnId, IspPortId, &stIspOutPortParam));

    /************************************************
    init scl
    *************************************************/

    ST_Common_GetSclDefaultDevAttr(&stSclDevAttr);
    stSclDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL2;
    STCHECKRESULT(ST_Common_SclCreateDevice(SclDevId, &stSclDevAttr));

    ST_Common_GetSclDefaultChnAttr(&stSclChnAttr, &stSclInputCrop, &stSclChnParam);
    STCHECKRESULT(ST_Common_SclStartChn(SclDevId, SclChnId, &stSclChnAttr, &stSclInputCrop, &stSclChnParam));

    if (fcc != V4L2_PIX_FMT_YUYV && fcc != V4L2_PIX_FMT_NV12)
    {
        ST_Common_GetVencDefaultDevAttr(&stVencInitParam);
        STCHECKRESULT(ST_Common_VencCreateDev(VencDevId, &stVencInitParam));
    }

    switch (fcc)
    {
        case V4L2_PIX_FMT_YUYV:
        {
            ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);
            stSclOutPortParam.stSCLOutputSize.u16Width  = u32Width;
            stSclOutPortParam.stSCLOutputSize.u16Height = u32Height;
            stSclOutPortParam.ePixelFormat              = E_MI_SYS_PIXEL_FRAME_YUV422_YUYV;
            STCHECKRESULT(ST_Common_SclEnablePort(SclDevId, SclChnId, SclPortId, &stSclOutPortParam));

            stSclChnPort.eModId    = E_MI_MODULE_ID_SCL;
            stSclChnPort.u32DevId  = SclDevId;
            stSclChnPort.u32ChnId  = SclChnId;
            stSclChnPort.u32PortId = SclPortId;
            STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stSclChnPort, 3, 4));
            break;
        }
        case V4L2_PIX_FMT_NV12:
        {
            ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);
            stSclOutPortParam.stSCLOutputSize.u16Width  = u32Width;
            stSclOutPortParam.stSCLOutputSize.u16Height = u32Height;
            stSclOutPortParam.ePixelFormat              = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
            STCHECKRESULT(ST_Common_SclEnablePort(SclDevId, SclChnId, SclPortId, &stSclOutPortParam));

            stSclChnPort.eModId    = E_MI_MODULE_ID_SCL;
            stSclChnPort.u32DevId  = SclDevId;
            stSclChnPort.u32ChnId  = SclChnId;
            stSclChnPort.u32PortId = SclPortId;
            STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stSclChnPort, 3, 4));
            break;
        }
        case V4L2_PIX_FMT_MJPEG:
        {
            ST_Common_GetVencDefaultChnAttr(E_MI_VENC_MODTYPE_JPEGE, &stVencChnAttr);
            stVencChnAttr.stVeAttr.stAttrJpeg.u32PicWidth              = u32Width;
            stVencChnAttr.stVeAttr.stAttrJpeg.u32PicHeight             = u32Height;
            stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth           = u32Width;
            stVencChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight          = u32Height;
            stVencChnAttr.stVeAttr.eType                               = E_MI_VENC_MODTYPE_JPEGE;
            stVencChnAttr.stVeAttr.stAttrJpeg.bByFrame                 = true;
            stVencChnAttr.stVeAttr.stAttrJpeg.u32RestartMakerPerRowCnt = 0;
            stVencChnAttr.stVeAttr.stAttrJpeg.u32BufSize               = 2 * u32Height * u32Height;
            stVencChnAttr.stRcAttr.eRcMode                             = E_MI_VENC_RC_MODE_MJPEGFIXQP;
            stVencChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor         = u32VenQfactor;
            stVencChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum   = 30;
            stVencChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen   = 1;
            STCHECKRESULT(ST_Common_VencStartChn(VencDevId, VencChnId, &stVencChnAttr));
        }
        break;
        case V4L2_PIX_FMT_H264:
        {
            ST_Common_GetVencDefaultChnAttr(E_MI_VENC_MODTYPE_H264E, &stVencChnAttr);
            stVencChnAttr.stVeAttr.stAttrH264e.u32PicWidth         = u32Width;
            stVencChnAttr.stVeAttr.stAttrH264e.u32PicHeight        = u32Height;
            stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth      = u32Width;
            stVencChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight     = u32Height;
            stVencChnAttr.stVeAttr.stAttrH264e.bByFrame            = bByFrame;
            stVencChnAttr.stVeAttr.stAttrH264e.u32BFrameNum        = 2;
            stVencChnAttr.stVeAttr.stAttrH264e.u32Profile          = 1;
            stVencChnAttr.stVeAttr.stAttrH264e.u32BufSize          = 5 * u32Height * u32Height;
            stVencChnAttr.stVeAttr.eType                           = E_MI_VENC_MODTYPE_H264E;
            stVencChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_H264CBR;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate        = u32VenBitRate;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum  = u32FrameRate;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen  = 1;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop            = 30;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
            stVencChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime       = 0;
            STCHECKRESULT(ST_Common_VencStartChn(VencDevId, VencChnId, &stVencChnAttr));
            MI_VENC_ParamH264SliceSplit_t stSliceSplit = {TRUE, 0xF};
            STCHECKRESULT(MI_VENC_SetH264SliceSplit(VencDevId, VencChnId, &stSliceSplit));
        }
        break;
        case V4L2_PIX_FMT_H265:
        {
            ST_Common_GetVencDefaultChnAttr(E_MI_VENC_MODTYPE_H265E, &stVencChnAttr);
            stVencChnAttr.stVeAttr.stAttrH265e.u32PicWidth         = u32Width;
            stVencChnAttr.stVeAttr.stAttrH265e.u32PicHeight        = u32Height;
            stVencChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth      = u32Width;
            stVencChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight     = u32Height;
            stVencChnAttr.stVeAttr.stAttrH265e.bByFrame            = bByFrame;
            stVencChnAttr.stVeAttr.stAttrH265e.u32BufSize          = 5 * u32Height * u32Height;
            stVencChnAttr.stVeAttr.eType                           = E_MI_VENC_MODTYPE_H265E;
            stVencChnAttr.stRcAttr.eRcMode                         = E_MI_VENC_RC_MODE_H265CBR;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate        = u32VenBitRate;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum  = u32FrameRate;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen  = 1;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32Gop            = 30;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
            stVencChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime       = 0;
            STCHECKRESULT(ST_Common_VencStartChn(VencDevId, VencChnId, &stVencChnAttr));
            MI_VENC_ParamH265SliceSplit_t stSliceSplit = {TRUE, (u32Height + 31) / 32};
            STCHECKRESULT(MI_VENC_SetH265SliceSplit(VencDevId, VencChnId, &stSliceSplit));
        }
        break;
        default:
            ST_ERR("unkown format fcc:0x%x\n", fcc);
            return -EINVAL;
    }

    /************************************************
    bind vif->isp
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
    stSrcChnPort.u32DevId  = u32VifDevId;
    stSrcChnPort.u32ChnId  = 0;
    stSrcChnPort.u32PortId = u32VifPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stDstChnPort.u32DevId  = IspDevId;
    stDstChnPort.u32ChnId  = IspChnId;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));
    /************************************************
    bind isp->scl
    *************************************************/
    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = IspDevId;
    stSrcChnPort.u32ChnId  = IspChnId;
    stSrcChnPort.u32PortId = IspPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId  = SclDevId;
    stDstChnPort.u32ChnId  = SclChnId;
    stDstChnPort.u32PortId = 0;
    u32SrcFrmrate          = 30;
    u32DstFrmrate          = 30;
    eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    u32BindParam           = 0;
    STCHECKRESULT(
        MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType, u32BindParam));

    if (fcc != V4L2_PIX_FMT_YUYV && fcc != V4L2_PIX_FMT_NV12)
    {
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId  = SclDevId;
        stSrcChnPort.u32ChnId  = SclChnId;
        stSrcChnPort.u32PortId = SclPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = VencDevId;
        stDstChnPort.u32ChnId  = VencChnId;
        stDstChnPort.u32PortId = 0;
        u32SrcFrmrate          = 30;
        u32DstFrmrate          = 30;
        eBindType              = E_MI_SYS_BIND_TYPE_FRAME_BASE;
        u32BindParam           = 0;
        STCHECKRESULT(MI_SYS_BindChnPort2(0, &stSrcChnPort, &stDstChnPort, u32SrcFrmrate, u32DstFrmrate, eBindType,
                                          u32BindParam));
        ST_Common_GetSclDefaultPortAttr(&stSclOutPortParam);
        stSclOutPortParam.stSCLOutputSize.u16Width  = u32Width;
        stSclOutPortParam.stSCLOutputSize.u16Height = u32Height;
        STCHECKRESULT(ST_Common_SclEnablePort(SclDevId, SclChnId, SclPortId, &stSclOutPortParam));
    }
    /************************************************
    Load IQ Bin
    *************************************************/
    if (strlen(IqBinPath) == 0)
    {
        memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));
        MI_SNR_GetPlaneInfo(SnrPadId, 0, &stPlaneInfo);
        sprintf(IqApiBinFilePath, "/config/iqfile/%s_api.bin", stPlaneInfo.s8SensorName);
    }
    else
    {
        strcpy(IqApiBinFilePath, IqBinPath);
    }
    ST_Common_IspSetIqBin(0, 0, IqApiBinFilePath);

    printf("Capture u32Width: %d, u32height: %d, format: %s\n", u32Width, u32Height,
           fcc == V4L2_PIX_FMT_YUYV
               ? "YUYV"
               : (fcc == V4L2_PIX_FMT_NV12
                      ? "NV12"
                      : (fcc == V4L2_PIX_FMT_MJPEG ? "MJPEG" : (fcc == V4L2_PIX_FMT_H264 ? "H264" : "H265"))));

    return 0;
}

MI_S32 UVC_StopCapture(void *uvc)
{
    MI_U32 fcc = 0;

    MI_U32 SnrPadId      = 0;
    MI_U32 u32VifGroupId = 0;
    MI_U32 u32VifDevId   = 0;
    MI_U32 u32VifPortId  = 0;
    MI_U32 IspDevId      = 0;
    MI_U32 IspChnId      = 0;
    MI_U32 IspPortId     = 0;
    MI_U32 SclDevId      = 0;
    MI_U32 SclChnId      = 0;
    MI_U32 SclPortId     = 0;
    MI_U32 VencDevId     = 0;
    MI_U32 VencChnId     = 0;

    MI_SNR_PlaneInfo_t      stPlaneInfo;
    MI_VIF_GroupAttr_t      stVifGroupAttr;
    MI_VIF_DevAttr_t        stVifDevAttr;
    MI_VIF_OutputPortAttr_t stVifPortAttr;

    MI_ISP_DevAttr_t      stIspDevAttr;
    MI_ISP_ChannelAttr_t  stIspChnAttr;
    MI_ISP_ChnParam_t     stIspChnParam;
    MI_SYS_WindowRect_t   stIspInputCrop;
    MI_ISP_OutPortParam_t stIspOutPortParam;

    MI_SCL_DevAttr_t      stSclDevAttr;
    MI_SCL_ChannelAttr_t  stSclChnAttr;
    MI_SCL_ChnParam_t     stSclChnParam;
    MI_SYS_WindowRect_t   stSclInputCrop;
    MI_SCL_OutPortParam_t stSclOutPortParam;

    MI_VENC_InitParam_t         stVencInitParam;
    MI_VENC_ChnAttr_t           stVencChnAttr;

    MI_SYS_ChnPort_t stSrcChnPort;
    MI_SYS_ChnPort_t stDstChnPort;

    memset(&stPlaneInfo, 0x00, sizeof(MI_SNR_PlaneInfo_t));

    memset(&stVifGroupAttr, 0x00, sizeof(MI_VIF_GroupAttr_t));
    memset(&stVifDevAttr, 0x00, sizeof(MI_VIF_DevAttr_t));
    memset(&stVifDevAttr, 0x00, sizeof(MI_VIF_DevAttr_t));
    memset(&stVifPortAttr, 0x00, sizeof(MI_VIF_OutputPortAttr_t));

    memset(&stIspDevAttr, 0x00, sizeof(MI_ISP_DevAttr_t));
    memset(&stIspChnAttr, 0x00, sizeof(MI_ISP_ChannelAttr_t));
    memset(&stIspChnParam, 0x00, sizeof(MI_ISP_ChnParam_t));
    memset(&stIspInputCrop, 0x00, sizeof(MI_SYS_WindowRect_t));
    memset(&stIspOutPortParam, 0x00, sizeof(MI_ISP_OutPortParam_t));

    memset(&stSclDevAttr, 0x00, sizeof(MI_SCL_DevAttr_t));
    memset(&stSclChnAttr, 0x00, sizeof(MI_SCL_ChannelAttr_t));
    memset(&stSclChnParam, 0x00, sizeof(MI_SCL_ChnParam_t));
    memset(&stSclInputCrop, 0x00, sizeof(MI_SYS_WindowRect_t));
    memset(&stSclOutPortParam, 0x00, sizeof(MI_SCL_OutPortParam_t));

    memset(&stVencInitParam, 0x00, sizeof(MI_VENC_InitParam_t));
    memset(&stVencChnAttr, 0x00, sizeof(MI_VENC_ChnAttr_t));

    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));

    ST_UvcDev_t *dev = Get_UVC_Device(uvc);
    if (!dev)
        return -1;
    ST_Stream_Attr_T *pstStreamAttr = g_stStreamAttr;

    fcc       = dev->setting.fcc;
    IspDevId  = pstStreamAttr[dev->dev_index].u32IspDev;
    IspChnId  = pstStreamAttr[dev->dev_index].u32IspChn;
    IspPortId = pstStreamAttr[dev->dev_index].u32IspPort;
    SclDevId  = pstStreamAttr[dev->dev_index].u32SclDev;
    SclChnId  = pstStreamAttr[dev->dev_index].u32SclChn;
    SclPortId = pstStreamAttr[dev->dev_index].u32SclPort;
    VencDevId = (fcc == V4L2_PIX_FMT_MJPEG) ? JPEGDEV : H264_265DEV;
    VencChnId = pstStreamAttr[dev->dev_index].u32VencChn;

    if (dev->res.pstuserptr_stream)
    {
        for (MI_S32 i = 0; i < g_maxbuf_cnt; i++)
        {
            free(dev->res.pstuserptr_stream[i].stStream.pstPack);
        }
        free(dev->res.pstuserptr_stream);
        dev->res.pstuserptr_stream = NULL;
    }
    /************************************************
    Step1:  Stop Port And Unbind
    *************************************************/
    if (fcc != V4L2_PIX_FMT_YUYV && fcc != V4L2_PIX_FMT_NV12)
    {
        memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
        stSrcChnPort.eModId    = E_MI_MODULE_ID_SCL;
        stSrcChnPort.u32DevId  = SclDevId;
        stSrcChnPort.u32ChnId  = SclChnId;
        stSrcChnPort.u32PortId = SclPortId;
        stDstChnPort.eModId    = E_MI_MODULE_ID_VENC;
        stDstChnPort.u32DevId  = VencDevId;
        stDstChnPort.u32ChnId  = VencChnId;
        stDstChnPort.u32PortId = 0;
        STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

        STCHECKRESULT(ST_Common_SclDisablePort(SclDevId, SclChnId, SclPortId));
    }

    switch (fcc)
    {
        case V4L2_PIX_FMT_YUYV:
        {
            STCHECKRESULT(ST_Common_SclDisablePort(SclDevId, SclChnId, SclPortId));
        }
        break;
        case V4L2_PIX_FMT_NV12:
        {
            STCHECKRESULT(ST_Common_SclDisablePort(SclDevId, SclChnId, SclPortId));
        }
        break;
        case V4L2_PIX_FMT_MJPEG:
        {
            STCHECKRESULT(ST_Common_VencStopChn(VencDevId, VencChnId));
        }
        break;
        case V4L2_PIX_FMT_H264:
        {
            STCHECKRESULT(ST_Common_VencStopChn(VencDevId, VencChnId));
        }
        break;
        case V4L2_PIX_FMT_H265:
        {
            STCHECKRESULT(ST_Common_VencStopChn(VencDevId, VencChnId));
        }
        break;
        default:
            ST_ERR("unkown format fcc:0x%x\n", fcc);
            return -EINVAL;
    }
    if (fcc != V4L2_PIX_FMT_YUYV && fcc != V4L2_PIX_FMT_NV12)
    {
        STCHECKRESULT(ST_Common_VencDestroyDev(VencDevId));
    }

    memset(&stSrcChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    memset(&stDstChnPort, 0x00, sizeof(MI_SYS_ChnPort_t));
    stSrcChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stSrcChnPort.u32DevId  = IspDevId;
    stSrcChnPort.u32ChnId  = IspChnId;
    stSrcChnPort.u32PortId = IspPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_SCL;
    stDstChnPort.u32DevId  = SclDevId;
    stDstChnPort.u32ChnId  = SclChnId;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));
    /************************************************
    deinit scl
    *************************************************/
    STCHECKRESULT(ST_Common_SclStopChn(SclDevId, SclChnId));
    STCHECKRESULT(ST_Common_SclDestroyDevice(SclDevId));

    /************************************************
    unbind vif->isp
    *************************************************/
    stSrcChnPort.eModId    = E_MI_MODULE_ID_VIF;
    stSrcChnPort.u32DevId  = u32VifDevId;
    stSrcChnPort.u32ChnId  = 0;
    stSrcChnPort.u32PortId = u32VifPortId;
    stDstChnPort.eModId    = E_MI_MODULE_ID_ISP;
    stDstChnPort.u32DevId  = IspDevId;
    stDstChnPort.u32ChnId  = IspChnId;
    stDstChnPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_UnBindChnPort(0, &stSrcChnPort, &stDstChnPort));

    /************************************************
    deinit isp
    *************************************************/
    STCHECKRESULT(ST_Common_IspDisablePort(IspDevId, IspChnId, IspPortId));
    STCHECKRESULT(ST_Common_IspStopChn(IspDevId, IspChnId));

    pthread_mutex_lock(&uvcdev_mutex);
    if (g_pipeline_active < 2)
    {
        STCHECKRESULT(ST_Common_IspDestroyDevice(IspDevId));
        /************************************************
        deinit vif/sensor
        *************************************************/
        STCHECKRESULT(ST_Common_VifDisablePort(u32VifDevId, u32VifPortId));
        STCHECKRESULT(ST_Common_VifDisableDev(u32VifDevId));
        STCHECKRESULT(ST_Common_VifDestroyDevGroup(u32VifGroupId));
        STCHECKRESULT(ST_Common_SensorDeInit(SnrPadId));

        /************************************************
        sys exit
        *************************************************/
        STCHECKRESULT(ST_Common_Sys_Exit());
    }
    g_pipeline_active = g_pipeline_active - 1;
    pthread_mutex_unlock(&uvcdev_mutex);
    return 0;
}

void UVC_ForceIdr(void *uvc)
{
    ST_UvcDev_t *dev = Get_UVC_Device(uvc);
    if ((dev->setting.fcc == V4L2_PIX_FMT_H264) || (dev->setting.fcc == V4L2_PIX_FMT_H265))
    {
        g_stStreamAttr[dev->dev_index].bForceIdr = TRUE;
    }
}

static MI_S32 UVC_Init(void *uvc)
{
    return MI_SUCCESS;
}

static MI_S32 UVC_Deinit(void *uvc)
{
    return MI_SUCCESS;
}

MI_S32 ST_UvcInitDev(ST_UvcDev_t *dev, MI_U32 maxpacket, MI_U8 mult, MI_U8 burst, MI_U8 c_intf, MI_U8 s_intf,
                     MI_S32 mode, MI_S32 type)
{
    SS_UVC_Setting_t pstSet = {g_maxbuf_cnt,         maxpacket, mult, burst, c_intf, s_intf, (UVC_IO_MODE_e)mode,
                               (Transfer_Mode_e)type};
    SS_UVC_MMAP_BufOpts_t    m    = {UVC_MM_FillBuffer};
    SS_UVC_USERPTR_BufOpts_t u    = {UVC_UP_FillBuffer, UVC_UP_FinishBuffer};
    SS_UVC_OPS_t             fops = {UVC_Init, UVC_Deinit, {{}}, UVC_StartCapture, UVC_StopCapture, UVC_ForceIdr};
    if (mode == UVC_MEMORY_MMAP)
        fops.m = m;
    else
        fops.u = u;
    printf(ASCII_COLOR_YELLOW
           "ST_UvcInitDev: name:%s bufcnt:%d mult:%d burst:%d ci:%d si:%d, Mode:%s, Type:%s" ASCII_COLOR_END "\n",
           dev->name, g_maxbuf_cnt, mult, burst, c_intf, s_intf, mode == UVC_MEMORY_MMAP ? "mmap" : "userptr",
           type == USB_ISOC_MODE ? "isoc" : "bulk");

    SS_UVC_ChnAttr_t pstAttr = {pstSet, fops};
    STCHECKRESULT(SS_UVC_Init(dev->name, NULL, &dev->handle));
    STCHECKRESULT(SS_UVC_CreateDev(dev->handle, &pstAttr));
    STCHECKRESULT(SS_UVC_StartDev(dev->handle));
    return MI_SUCCESS;
}

MI_S32 ST_UvcDeinit()
{
    if (!g_UvcSrc)
        return -1;

    for (MI_S32 i = 0; i < g_UvcSrc->devnum; i++)
    {
        ST_UvcDev_t *dev = &g_UvcSrc->dev[i];
        STCHECKRESULT(SS_UVC_StopDev((dev->handle)));
        STCHECKRESULT(SS_UVC_DestroyDev(dev->handle));
        STCHECKRESULT(SS_UVC_Uninit(dev->handle));
    }
    return MI_SUCCESS;
}

MI_S32 ST_UvcInit(MI_S32 devnum, MI_U32 *u32maxpacket, MI_U8 *u8mult, MI_U8 *u8burst, MI_U8 *u8intf, MI_S32 mode,
                  MI_S32 type)
{
    char devnode[20] = "/dev/video0";

    if (devnum > MAX_UVC_DEV_NUM)
    {
        printf("%s Max Uvc Dev Num %d\n", __func__, MAX_UVC_DEV_NUM);
        devnum = MAX_UVC_DEV_NUM;
    }

    g_UvcSrc = (ST_UvcSrc_t *)malloc(sizeof(g_UvcSrc) + sizeof(ST_UvcDev_t) * devnum);
    memset(g_UvcSrc, 0x0, sizeof(g_UvcSrc) + sizeof(ST_UvcDev_t) * devnum);
    g_UvcSrc->devnum = devnum;

    for (MI_S32 i = 0; i < devnum; i++)
    {
        ST_UvcDev_t *dev = &g_UvcSrc->dev[i];
        sprintf(devnode, "/dev/video%d", i);
        dev->dev_index = i;
        memcpy(dev->name, devnode, sizeof(devnode));
        ST_UvcInitDev(dev, u32maxpacket[i], u8mult[i], u8burst[i], u8intf[2 * i], u8intf[2 * i + 1], mode, type);
    }
    return MI_SUCCESS;
}

void ST_Uvc_Mode_Usage(void)
{
    printf("Usage:./prog_Uvc_device_demo 0  out one uvc device (include H264,H265,Mjpeg)\n");
    printf("Usage:./prog_Uvc_device_demo 1  out two uvc device (include H264,H265,Mjpeg)\n");
    printf("Usage:./prog_Uvc_device_demo 2  out three uvc device (include H264,H265,Mjpeg)\n");
    printf("Usage:./prog_Uvc_device_demo 1 index xxx select sensor index\n");
    printf("Usage:./prog_Uvc_device_demo 1 mode xxx select Transmission_Mode,0:USB_ISOC_MODE,1:USB_BULK_MODE\n");
    printf("Usage:./prog_Uvc_device_demo 1 suspend \n");
    printf("Usage:./prog_Uvc_device_demo 1 index xxx suspend \n");
}

MI_S32 ST_Uvc_GetCmdlineParam(MI_S32 argc, char **argv)
{
    gstUvcInputParm.u8SensorIndex = 0x00; // default 0x00，user input set 0xFF
    gstUvcInputParm.u8HdrType     = 0x00; // default 0x00，linear mode
    gstUvcInputParm.u8CmdIndex    = atoi(argv[1]);
    memset(IqBinPath, 0, 128);
    if (0 == gstUvcInputParm.u8CmdIndex || 1 == gstUvcInputParm.u8CmdIndex || 2 == gstUvcInputParm.u8CmdIndex)
    {
        g_device_num = gstUvcInputParm.u8CmdIndex + 1;
    }
    else
    {
        printf("pararm not support! Only support max three device\r\n");
        return -1;
    }
    for (MI_S32 i = 0; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "index"))
        {
            gstUvcInputParm.u8SensorIndex = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "hdr"))
        {
            gstUvcInputParm.u8HdrType = atoi(argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "iqbin"))
        {
            strcpy(IqBinPath, argv[i + 1]);
        }
        else if (0 == strcmp(argv[i], "mode"))
        {
            Transmission_Mode = atoi(argv[i + 1]);
            if (0 == Transmission_Mode)
            {
                Transmission_Mode = USB_ISOC_MODE;
            }
            else if (1 == Transmission_Mode)
            {
                Transmission_Mode = USB_BULK_MODE;
            }
        }
        else if (0 == strcmp(argv[i], "suspend"))
        {
            g_en_suspend = 1;
        }
    }
    return MI_SUCCESS;
}

static MI_S32 ST_UvcModePipeline_Preview()
{
    MI_U32 u32maxpacket[MAX_UVC_DEV_NUM] = {1024, 1024, 1024};
    MI_U8  u8mult[MAX_UVC_DEV_NUM] = {2, 0, 0}, u8burst[MAX_UVC_DEV_NUM] = {13, 13, 13};
    MI_U8  u8intf[2 * MAX_UVC_DEV_NUM] = {0, 1, 2};
    MI_S32 mode = UVC_MEMORY_MMAP, type = Transmission_Mode;

    /************************************************
    Open IQServer
    *************************************************/
    MI_IQSERVER_Open();

    STCHECKRESULT(ST_UvcInit(g_device_num, u32maxpacket, u8mult, u8burst, u8intf, mode, type));

    ST_Common_Pause();

    STCHECKRESULT(ST_UvcDeinit());
    return MI_SUCCESS;
}

static MI_BOOL   g_bExit = FALSE;
static pthread_t udc_state_thread;

static void *ST_UdcStateMonitor_Task(void *arg)
{
    struct pollfd fds;
    MI_BOOL       str_enabled = false;
    unsigned char state[30];

    if (!access(DWC3_DIR "/state", 0))
    {
        fds.fd = open(DWC3_DIR "/state", O_RDONLY);
        do
        {
            str_enabled = false;
            // dummy reads before the poll() call
            read(fds.fd, state, 30);
            fds.events  = POLLPRI;
            fds.revents = 0;
            poll(&fds, 1, 200);
            lseek(fds.fd, SEEK_SET, 0);
            memset(state, 0, sizeof(state));
            read(fds.fd, state, 30);
            // printf("usb link state: %s", state);
            if (0 != strncmp((const char *)state, "not attached", strlen("not attached")))
            {
                if (strncmp((const char *)state, "suspend", strlen("suspend")) == 0)
                {
                    if (0 == g_pipeline_active)
                        str_enabled = true;
                }
            }
            if (str_enabled)
            {
                system("echo mem > /sys/power/state");
            }
        } while (!g_bExit);
        close(fds.fd);
    }

    return NULL;
}

MI_S32 ST_UvcSuspendInit(void)
{
    if (g_en_suspend)
    {
        pthread_create(&udc_state_thread, NULL, ST_UdcStateMonitor_Task, NULL);
        pthread_setname_np(udc_state_thread, "UDC_State_Monitor");
    }
    return MI_SUCCESS;
}

MI_S32 ST_UvcSuspendDeInit(void)
{
    if (g_en_suspend)
    {
        g_bExit = true;
        pthread_join(udc_state_thread, NULL);
    }
    return MI_SUCCESS;
}

MI_S32 main(MI_S32 argc, char **argv)
{
    if (argc < 2)
    {
        ST_Uvc_Mode_Usage();
        return -1;
    }
    STCHECKRESULT(ST_Uvc_GetCmdlineParam(argc, argv));
    STCHECKRESULT(ST_UvcSuspendInit());
    STCHECKRESULT(ST_UvcModePipeline_Preview());
    STCHECKRESULT(ST_UvcSuspendDeInit());
    memset(&gstUvcInputParm, 0, sizeof(ST_UvcInputParam_t));
    return 0;
}
