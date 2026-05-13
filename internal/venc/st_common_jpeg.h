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
#ifndef _ST_COMMON_JPEG_H_
#define _ST_COMMON_JPEG_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

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
#include <ctype.h>
#include <dirent.h>
#include "st_common.h"
#include "mi_sys.h"
#include "mi_venc.h"
#define JPEG_SPLICE_MAX_NUM (16)
#define JPEG_SINGLE_MAX_BUFSIZE (1024*1024)
#define JPEG_HEADER_LEN (0x280)

typedef enum
{
    E_JPEG_SPLICE_MODE_HORIZONTAL = 0,
    E_JPEG_SPLICE_MODE_VERTICAL,
    E_JPEG_SPLICE_MODE_Z,
    E_JPEG_SPLICE_MODE_MAX
} E_JPEG_SPLICE_MODE;

typedef struct ST_JPEG_Output_Attr_s
{
    char *pbuf[JPEG_SPLICE_MAX_NUM];
    MI_U32 au32bufLen[JPEG_SPLICE_MAX_NUM];
    MI_U32 u32SpliceBufNum;
    pthread_mutex_t  Outputmutex;
    pthread_t        pOutputPortGetDataThread;
    MI_BOOL          bThreadExit;
    MI_SYS_ChnPort_t stModuleInfo;
} ST_JPEG_Output_Attr_t;

typedef struct ST_JpegSpliceParam_s
{
    ST_Common_InputFile_Attr_t  stInputInfo;
    ST_JPEG_Output_Attr_t stOutputInfo;
    E_JPEG_SPLICE_MODE eSpliceMode;
    char dumpSpliceOutPath[128];
    char dumpJpegOutPath[128];
    FILE *dumpSpliceOutfp;
    FILE *dumpJpegOutfp;
    MI_U16 u16DumpBuffNum;
}ST_JpegSpliceParam_t;

MI_S32 ST_Common_VencJpegSplice_HORIZONTAL(ST_JpegSpliceParam_t *pJpegSpliceParam, char* output, int *OutputBuf_Offset);
MI_S32 ST_Common_VencJpegSplice_VERTICAL(ST_JpegSpliceParam_t *pJpegSpliceParam, char* output, int *OutputBuf_Offset);
MI_S32 ST_Common_VencJpegSplice_Z(ST_JpegSpliceParam_t *pJpegSpliceParam, char* output, int *OutputBuf_Offset);
MI_S32 ST_Common_SetJpegSpliceChnAttr(MI_VENC_ModType_e eType, MI_VENC_ChnAttr_t *pstVencChnAttr);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_VENC_H_
