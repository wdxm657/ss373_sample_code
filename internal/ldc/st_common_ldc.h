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
#ifndef _ST_COMMON_LDC_H_
#define _ST_COMMON_LDC_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "mi_sys.h"
#include "mi_ldc.h"

#define CHECK_LDC_RESULT(_func_, _ret_, _exit_label_)                                     \
    do                                                                                    \
    {                                                                                     \
        _ret_ = _func_;                                                                   \
        if (_ret_ != MI_SUCCESS)                                                          \
        {                                                                                 \
            printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, _ret_); \
            goto _exit_label_;                                                            \
        }                                                                                 \
    } while (0)

    MI_S32 ST_Common_GetLdcDefaultDevAttr(MI_LDC_DevAttr_t *pstLdcDevAttr);
    MI_S32 ST_Common_GetLdcDefaultChnAttr(MI_LDC_ChnAttr_t *pstLdcChnAttr);
    MI_S32 ST_Common_GetLdcDefaultLdcModeChnAttr(MI_LDC_ChnLDCAttr_t *pstLdcModeChnAttr);
    MI_S32 ST_Common_GetLdcDefaultDisModeChnAttr(MI_LDC_ChnDISAttr_t *pstDisModeChnAttr);
    MI_S32 ST_Common_GetLdcDefaultNirModeChnAttr(MI_LDC_ChnNIRAttr_t *pstNirModeChnAttr);
    MI_S32 ST_Common_GetLdcDefaultStitchModeChnAttr(MI_LDC_ChnStitchAttr_t *pstStitchModeChnAttr);
    MI_S32 ST_Common_GetLdcDefaultPmfModeChnAttr(MI_LDC_ChnPMFAttr_t *pstPMFModeChnAttr);
    MI_S32 ST_Common_GetLdcDefaultLdcInputPortAttr(MI_LDC_InputPortAttr_t *pstInputPortAttr);
    MI_S32 ST_Common_GetLdcDefaultLdcOutputPortAttr(MI_LDC_OutputPortAttr_t *pstOutputPortAttr);

    MI_S32 ST_Common_LdcCreateDevice(MI_U32 devId, MI_LDC_DevAttr_t *pstDevAttr);
    MI_S32 ST_Common_LdcDestroyDevice(MI_U32 devId);
    MI_S32 ST_Common_LdcStartLdcModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                        MI_LDC_ChnLDCAttr_t     *pstLdcModeChnAttr,
                                        MI_LDC_InputPortAttr_t  *pstInputPortAttr,
                                        MI_LDC_OutputPortAttr_t *pstOutputPortAttr);
    MI_S32 ST_Common_LdcStartDisModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                        MI_LDC_ChnDISAttr_t     *pstDisModeChnAttr,
                                        MI_LDC_InputPortAttr_t  *pstInputPortAttr,
                                        MI_LDC_OutputPortAttr_t *pstOutputPortAttr);
    MI_S32 ST_Common_LdcStartStitchModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                           MI_LDC_ChnStitchAttr_t  *pstStitchModeChnAttr,
                                           MI_LDC_InputPortAttr_t  *pstInputPortAttr,
                                           MI_LDC_OutputPortAttr_t *pstOutputPortAttr);
    MI_S32 ST_Common_LdcStartNirModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                        MI_LDC_ChnNIRAttr_t     *pstNirModeChnAttr,
                                        MI_LDC_InputPortAttr_t  *pstInputPortAttr,
                                        MI_LDC_OutputPortAttr_t *pstOutputPortAttr);
    MI_S32 ST_Common_LdcStartDpuModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                        MI_LDC_ChnDPUAttr_t     *pstDpuModeChnAttr,
                                        MI_LDC_InputPortAttr_t  *pstInputPortAttr,
                                        MI_LDC_OutputPortAttr_t *pstOutputPortAttr);
    MI_S32 ST_Common_LdcStartPmfModeChn(MI_U32 devId, MI_U32 chnId, MI_LDC_ChnAttr_t *pstLdcChnAttr,
                                        MI_LDC_ChnPMFAttr_t     *pstPMFModeChnAttr,
                                        MI_LDC_InputPortAttr_t  *pstInputPortAttr,
                                        MI_LDC_OutputPortAttr_t *pstOutputPortAttr);
    MI_S32 ST_Common_LdcStopChn(MI_U32 devId, MI_U32 chnId);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_ST_COMMON_LDC_H_
