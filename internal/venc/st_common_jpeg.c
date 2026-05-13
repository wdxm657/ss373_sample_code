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

#include "st_common_venc.h"
#include "st_common_jpeg.h"
#include "st_common.h"
#include "platform.h"


MI_S32 find_tag_idx(const char*buf, int len , char tag0, char tag1)
{
    for (int i = 0; i < len - 1; i++)
    {
        if (buf[i] == tag0 && buf[i+1] == tag1)
        {
            return i;
        }
    }
    return -1;
}

MI_S32 set_header_wh(char* buf, int len, int width, int height)
{
    // find SOF0
    int idx = find_tag_idx(buf, len, 0xff, 0xc0);
    if (idx == -1)
    {
        ST_ERR("error: splice impl find 0x%x%x error \n", 0xff, 0xc0);
        return -1;
    }
    // ff c0 00 11 08 width, height
    buf[idx + 5] = (height & 0xFF00)  >> 8;
    buf[idx + 6] = height & 0xFF;
    buf[idx + 7] = (width & 0xFF00) >> 8;
    buf[idx + 8] = width & 0xFF;

    return MI_SUCCESS;
}
MI_S32 copy_tag_data(char *buf, int len, int tag0, int tag1, char **output, int* osize)
{
    int i = find_tag_idx(buf, len, tag0, tag1);
    if (i == -1 || i == 0)
    {
        ST_ERR("error: splice impl find tag error %x%x ", tag0, tag1);
        ST_ERR("May be jpeg not u32RestartMakerPerRowCnt = 1 or ByFrame = 1\n");
        return -1;
    }
    if(output==NULL)
        ST_ERR("error: output point is NULL\n");
    //memcpy(output, buf, i);
    *output = buf;
    *osize = i;

    return MI_SUCCESS;
}

MI_S32 ST_Common_VencJpegSplice_HORIZONTAL(ST_JpegSpliceParam_t *pJpegSpliceParam, char* output, int *OutputBuf_Offset)
{
    MI_U32 w = pJpegSpliceParam->stInputInfo.u32Width;
    MI_U32 h = pJpegSpliceParam->stInputInfo.u32Height;
    MI_U32 frameNumber = pJpegSpliceParam->stOutputInfo.u32SpliceBufNum;
    int output_offset = *OutputBuf_Offset;
    MI_U32 width = w * frameNumber;
    MI_U32 height = h;
    int osize = 0;
    int marker = 0xd0;
    // int line = h / (fmt == YUV422) ? 8 : 16;
    MI_U32 line = h / 16;        // 420 is 16
    char *temp = NULL;
    int offset[JPEG_SPLICE_MAX_NUM];

    for(MI_U32 i=0; i < frameNumber; i++)
    {
        offset[i] = JPEG_HEADER_LEN;
    }
    memset(output, 0, JPEG_SINGLE_MAX_BUFSIZE * frameNumber);
    // copy header
    memcpy(output + output_offset, pJpegSpliceParam->stOutputInfo.pbuf[0], JPEG_HEADER_LEN);
    set_header_wh(output, JPEG_HEADER_LEN, width, height);
    output_offset += JPEG_HEADER_LEN;

    for (MI_U32 i = 0; i < line; i++)
    {
        for (MI_U32 j =0 ; j < frameNumber; j++)
        {
            if (i == line - 1)
            {
                if(-1==copy_tag_data(pJpegSpliceParam->stOutputInfo.pbuf[j] + offset[j], pJpegSpliceParam->stOutputInfo.au32bufLen[j] - offset[j], 0xff, 0xd9, &temp, &osize))
                    continue;
                memcpy(output + output_offset, temp, osize);
                offset[j] += osize;
                output_offset += osize;
                output[output_offset] = 0xFF;
                output[output_offset + 1] = marker;
                output_offset += 2;
                offset[j] += 2;
                marker = (marker + 1 == 0xd8) ? 0xd0 : marker + 1;
            }
            else
            {
                if(-1==copy_tag_data(pJpegSpliceParam->stOutputInfo.pbuf[j] + offset[j], pJpegSpliceParam->stOutputInfo.au32bufLen[j] - offset[j], 0xff, 0xd0 + (i % 8), &temp, &osize))
                    continue;
                memcpy(output + output_offset, temp, osize);
                offset[j] += osize;
                output_offset += osize;
                output[output_offset] = 0xFF;
                output[output_offset + 1] = marker;
                output_offset += 2;
                offset[j] += 2;
                marker = (marker + 1 == 0xd8) ? 0xd0 : marker + 1;
            }
        }
    }
    output[output_offset] = 0xFF;
    output[output_offset + 1] = 0xD9;
    output_offset += 2;

    *OutputBuf_Offset = output_offset;

    return MI_SUCCESS;
}


MI_S32 ST_Common_VencJpegSplice_VERTICAL(ST_JpegSpliceParam_t *pJpegSpliceParam, char* output, int *OutputBuf_Offset)
{
    MI_U32 w = pJpegSpliceParam->stInputInfo.u32Width;
    MI_U32 h = pJpegSpliceParam->stInputInfo.u32Height;
    MI_U32 frameNumber = pJpegSpliceParam->stOutputInfo.u32SpliceBufNum;
    int output_offset = *OutputBuf_Offset;
    MI_U32 width = w;
    MI_U32 height = h * frameNumber;
    int osize = 0;
    int marker = 0xd0;
    // int line = h / (fmt == YUV422) ? 8 : 16;
    MI_U32 line = h / 16;
    char *temp = NULL;
    int offset[JPEG_SPLICE_MAX_NUM];

    for(MI_U32 i=0; i < frameNumber; i++)
    {
        offset[i] = JPEG_HEADER_LEN;
    }
    memset(output, 0, JPEG_SINGLE_MAX_BUFSIZE * frameNumber);

    // copy header
    memcpy(output + output_offset, pJpegSpliceParam->stOutputInfo.pbuf[0], JPEG_HEADER_LEN);
    set_header_wh(output, JPEG_HEADER_LEN, width, height);
    output_offset += JPEG_HEADER_LEN;

    for (MI_U32 j =0 ; j < frameNumber; j++)
    {
        for (MI_U32 i = 0; i < line; i++)
        {
            if (i == line - 1)
            {
                if(-1==copy_tag_data(pJpegSpliceParam->stOutputInfo.pbuf[j] + offset[j], pJpegSpliceParam->stOutputInfo.au32bufLen[j] - offset[j], 0xff, 0xd9, &temp, &osize))
                    continue;
                memcpy(output + output_offset, temp, osize);
                offset[j] += osize;
                output_offset += osize;
                output[output_offset] = 0xFF;
                output[output_offset + 1] = marker;
                output_offset += 2;
                offset[j] += 2;
                marker = (marker + 1 == 0xd8) ? 0xd0 : marker + 1;
            }
            else
            {
                if(-1==copy_tag_data(pJpegSpliceParam->stOutputInfo.pbuf[j] + offset[j], pJpegSpliceParam->stOutputInfo.au32bufLen[j] - offset[j], 0xff, 0xd0 + (i % 8), &temp, &osize))
                    continue;
                memcpy(output + output_offset, temp, osize);
                offset[j] += osize;
                output_offset += osize;
                output[output_offset] = 0xFF;
                output[output_offset + 1] = marker;
                output_offset += 2;
                offset[j] += 2;
                marker = (marker + 1 == 0xd8) ? 0xd0 : marker + 1;
            }
        }
    }
    output[output_offset] = 0xFF;
    output[output_offset + 1] = 0xD9;
    output_offset += 2;

    *OutputBuf_Offset = output_offset;

    return MI_SUCCESS;
}

MI_S32 ST_Common_VencJpegSplice_Z(ST_JpegSpliceParam_t *pJpegSpliceParam, char* output, int *OutputBuf_Offset)
{
    MI_U32 w = pJpegSpliceParam->stInputInfo.u32Width;
    MI_U32 h = pJpegSpliceParam->stInputInfo.u32Height;
    MI_U32 frameNumber = pJpegSpliceParam->stOutputInfo.u32SpliceBufNum;
    int output_offset = *OutputBuf_Offset;
    int width = w * frameNumber /2;
    int height = h * frameNumber /2;
    int osize = 0;
    int marker = 0xd0;
    // int line = h / (fmt == YUV422) ? 8 : 16;
    MI_U32 line = h / 16;        // 420 is 16
    char *temp =NULL;
    int offset[frameNumber];

    for(MI_U32 i=0; i < frameNumber; i++)
    {
        offset[i] = JPEG_HEADER_LEN;
    }
    if(frameNumber != 4)
    {
        ST_ERR("u32BufNum=%d MUST be 4\n",frameNumber);
        free(output);
        return -1;
    }
    memset(output, 0, JPEG_SINGLE_MAX_BUFSIZE * frameNumber);
    // copy header
    memcpy(output + output_offset, pJpegSpliceParam->stOutputInfo.pbuf[0], JPEG_HEADER_LEN);
    set_header_wh(output, JPEG_HEADER_LEN, width, height);
    output_offset += JPEG_HEADER_LEN;

    for (MI_U32 i = 0; i < line; i++)
    {
        for (MI_U32 j =0 ; j < frameNumber /2 ; j++)
        {
            if (i == line - 1)
            {
                if(-1==copy_tag_data(pJpegSpliceParam->stOutputInfo.pbuf[j] + offset[j], pJpegSpliceParam->stOutputInfo.au32bufLen[j] - offset[j], 0xff, 0xd9, &temp, &osize))
                    continue;
                memcpy(output + output_offset, temp, osize);
                offset[j] += osize;
                output_offset += osize;
                output[output_offset] = 0xFF;
                output[output_offset + 1] = marker;
                output_offset += 2;
                offset[j] += 2;
                marker = (marker + 1 == 0xd8) ? 0xd0 : marker + 1;
            }
            else
            {
                if(-1==copy_tag_data(pJpegSpliceParam->stOutputInfo.pbuf[j] + offset[j], pJpegSpliceParam->stOutputInfo.au32bufLen[j] - offset[j], 0xff, 0xd0 + (i % 8), &temp, &osize))
                    continue;
                memcpy(output + output_offset, temp, osize);
                offset[j] += osize;
                output_offset += osize;
                output[output_offset] = 0xFF;
                output[output_offset + 1] = marker;
                output_offset += 2;
                offset[j] += 2;
                marker = (marker + 1 == 0xd8) ? 0xd0 : marker + 1;
            }
        }

    }

    for (MI_U32 i = 0; i < line; i++)
    {
        for (MI_U32 j =frameNumber /2  ; j < frameNumber; j++)
        {
            if (i == line - 1)
            {
                if(-1==copy_tag_data(pJpegSpliceParam->stOutputInfo.pbuf[j] + offset[j], pJpegSpliceParam->stOutputInfo.au32bufLen[j] - offset[j], 0xff, 0xd9, &temp, &osize))
                    continue;
                memcpy(output + output_offset, temp, osize);
                offset[j] += osize;
                output_offset += osize;
                output[output_offset] = 0xFF;
                output[output_offset + 1] = marker;
                output_offset += 2;
                offset[j] += 2;
                marker = (marker + 1 == 0xd8) ? 0xd0 : marker + 1;
            }
            else
            {
                if(-1==copy_tag_data(pJpegSpliceParam->stOutputInfo.pbuf[j] + offset[j], pJpegSpliceParam->stOutputInfo.au32bufLen[j] - offset[j], 0xff, 0xd0 + (i % 8), &temp, &osize))
                    continue;
                memcpy(output + output_offset, temp, osize);
                offset[j] += osize;
                output_offset += osize;
                output[output_offset] = 0xFF;
                output[output_offset + 1] = marker;
                output_offset += 2;
                offset[j] += 2;
                marker = (marker + 1 == 0xd8) ? 0xd0 : marker + 1;
            }
        }
    }

    output[output_offset] = 0xFF;
    output[output_offset + 1] = 0xD9;
    output_offset += 2;

    *OutputBuf_Offset = output_offset;

    return MI_SUCCESS;
}

MI_S32 ST_Common_SetJpegSpliceChnAttr(MI_VENC_ModType_e eType, MI_VENC_ChnAttr_t *pstVencChnAttr)
{
    if(eType != (MI_VENC_ModType_e)E_MI_VENC_MODTYPE_JPEGE)
    {
        printf("Splice mode must be JPEGE!\n");
        return -1;
    }

    pstVencChnAttr->stVeAttr.stAttrJpeg.bByFrame = TRUE;              //hpepeg for splice must
    pstVencChnAttr->stVeAttr.stAttrJpeg.u32RestartMakerPerRowCnt = 1; //hpepeg for splice must
    pstVencChnAttr->stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;  //hpepeg for splice must

    return MI_SUCCESS;
}