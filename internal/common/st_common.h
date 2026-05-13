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
#ifndef _ST_COMMON_H_
#define _ST_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "mi_sys.h"
#include "mi_venc.h"
#include <sys/time.h>
#include <sys/types.h>

#ifndef ExecFunc
#define ExecFunc(_func_, _ret_)                                                            \
    do                                                                                     \
    {                                                                                      \
        MI_S32 s32Ret = MI_SUCCESS;                                                        \
        s32Ret        = _func_;                                                            \
        if (s32Ret != _ret_)                                                               \
        {                                                                                  \
            printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, s32Ret); \
            return s32Ret;                                                                 \
        }                                                                                  \
        else                                                                               \
        {                                                                                  \
            printf("[%s %d]exec function pass\n", __func__, __LINE__);                     \
        }                                                                                  \
    } while (0)
#endif

#ifndef STCHECKRESULT
#define STCHECKRESULT(_func_)                                                              \
    do                                                                                     \
    {                                                                                      \
        MI_S32 s32Ret = MI_SUCCESS;                                                        \
        s32Ret        = _func_;                                                            \
        if (s32Ret != MI_SUCCESS)                                                          \
        {                                                                                  \
            printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, s32Ret); \
            return s32Ret;                                                                 \
        }                                                                                  \
        else                                                                               \
        {                                                                                  \
            printf("(%s %d)exec function pass\n", __FUNCTION__, __LINE__);                 \
        }                                                                                  \
    } while (0)
#endif

#define STDBG_ENTER()                                               \
    printf("\n");                                                   \
    printf("[IN] [%s:%s:%d] \n", __FILE__, __FUNCTION__, __LINE__); \
    printf("\n");

#define STDBG_LEAVE()                                                \
    printf("\n");                                                    \
    printf("[OUT] [%s:%s:%d] \n", __FILE__, __FUNCTION__, __LINE__); \
    printf("\n");

#define ST_RUN()                                                        \
    printf("\n");                                                       \
    printf("[RUN] ok [%s:%s:%d] \n", __FILE__, __FUNCTION__, __LINE__); \
    printf("\n");

#define COLOR_NONE "\033[0m"
#define COLOR_BLACK "\033[0;30m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_WHITE "\033[1;37m"

#define ST_NOP(fmt, args...)
#define ST_DBG(fmt, args...)                                                     \
    do                                                                           \
    {                                                                            \
        printf(COLOR_GREEN "[DBG]:%s[%d]: " COLOR_NONE, __FUNCTION__, __LINE__); \
        printf(fmt, ##args);                                                     \
    } while (0)

#define ST_WARN(fmt, args...)                                                      \
    do                                                                             \
    {                                                                              \
        printf(COLOR_YELLOW "[WARN]:%s[%d]: " COLOR_NONE, __FUNCTION__, __LINE__); \
        printf(fmt, ##args);                                                       \
    } while (0)

#define ST_INFO(fmt, args...)                                \
    do                                                       \
    {                                                        \
        printf("[INFO]:%s[%d]: \n", __FUNCTION__, __LINE__); \
        printf(fmt, ##args);                                 \
    } while (0)

#define ST_ERR(fmt, args...)                                                   \
    do                                                                         \
    {                                                                          \
        printf(COLOR_RED "[ERR]:%s[%d]: " COLOR_NONE, __FUNCTION__, __LINE__); \
        printf(fmt, ##args);                                                   \
    } while (0)

#define ST_CHECK_POINTER(pointer)        \
    {                                    \
        if (pointer == NULL)             \
        {                                \
            ST_ERR("NULL pointer!!!\n"); \
            return -1;                   \
        }                                \
    }

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#define MI_U32VALUE(pu8Data, index) \
    (pu8Data[index] << 24) | (pu8Data[index + 1] << 16) | (pu8Data[index + 2] << 8) | (pu8Data[index + 3])

#define MAKE_YUYV_VALUE(y, u, v) ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK MAKE_YUYV_VALUE(0, 128, 128)
#define YUYV_WHITE MAKE_YUYV_VALUE(255, 128, 128)
#define YUYV_RED MAKE_YUYV_VALUE(76, 84, 255)
#define YUYV_GREEN MAKE_YUYV_VALUE(149, 43, 21)
#define YUYV_BLUE MAKE_YUYV_VALUE(29, 225, 107)

#define ALIGN_UP(x, align) ((((x) + (align)-1) / (align)) * (align))
#define ALIGN_BACK(x, a) (((x) / (a)) * (a))
#define ALIGN_FRONT(x, a) ((((x) + (a) / 2) / (a)) * (a))

#define THREAD_SLEEP_TIME_US 10 * 1000

MI_S32 ST_Common_Sys_Init(void);
MI_S32 ST_Common_Sys_Exit(void);

typedef enum
{
    E_ST_LIGHT_BRIGHT,
    E_ST_LIGHT_DARK
} ST_EnvBrightnessType_e;

typedef struct ST_Common_OutputFile_Attr_s
{
    MI_U16           u16DumpBuffNum;
    char             FilePath[128];
    pthread_mutex_t  Outputmutex;
    pthread_t        pGetDataThread;
    MI_BOOL          bThreadExit;
    MI_SYS_ChnPort_t stModuleInfo;
    FILE *           fp;
    MI_BOOL          bAppendFile;
} ST_Common_OutputFile_Attr_t;

typedef struct ST_Common_InputFile_Attr_s
{
    MI_U32               u32Width;
    MI_U32               u32Height;
    MI_SYS_PixelFormat_e ePixelFormat;
    char                 InputFilePath[256];
    pthread_t            pPutDatathread;
    MI_BOOL              bThreadExit;
    MI_SYS_ChnPort_t     stModuleInfo;
    MI_U32               u32Fps;
    MI_BOOL              bInputOnce;
} ST_Common_InputFile_Attr_t;

void   ST_Common_Flush(void);
void   ST_Common_Pause(void);
MI_S32 ST_Common_OpenSourceFile(const char *pFileName, FILE **fp);
MI_S32 ST_Common_CloseSourceFile(FILE **fp);
MI_S32 ST_Common_GetSourceFileSize(FILE *fp, unsigned int *size);
MI_S32 ST_Common_GetOneFrame(FILE *fp, char *pData, int yuvSize);
MI_S64 ST_Common_CalcDiffTime_MS(struct timeval *pstBeforeStamp, struct timeval *pstAfterStamp);
MI_U32 ST_Common_CalculateFrameSize(MI_SYS_BufInfo_t *pstBufInfo);
MI_S32 ST_Common_CheckMkdirOutFile(char *pFilePath);
MI_S32 ST_Common_WriteFile(char *pFilePath, FILE **fp, MI_U8 *pData, MI_U32 u32BufSize, MI_U16 *pu16DumpNum,
                           MI_BOOL bRecover);

MI_S32 ST_Common_GetOutputBufInfo(MI_SYS_ChnPort_t stChnPort, MI_SYS_BufInfo_t *pstBufInfo, MI_SYS_BUF_HANDLE *pHandle,
                                  struct timeval *pstTimeoutVal);
MI_S32 ST_Common_PutOutputBufInfo(MI_SYS_BUF_HANDLE *pHandle);

void *ST_Common_PutInputDataThread(void *args);
void *ST_Common_GetOutputDataThread(void *args);
void *ST_Common_GetVencOutThread(void *args);

MI_S32 ST_Common_CheckResult(ST_Common_OutputFile_Attr_t *pstOutFileAttr);
MI_S32 ST_Common_WaitDumpFinsh(ST_Common_OutputFile_Attr_t *pstOutFileAttr);

MI_S32 ST_Common_AllocateMemory(MI_U32 size, MI_PHY *phys_addr, MI_U8 **virt_addr);
MI_S32 ST_Common_FreeMemory(MI_PHY phys_addr, MI_U8 *virt_addr, MI_U32 size);
MI_U32 ST_Common_GetMs(void);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_H_
