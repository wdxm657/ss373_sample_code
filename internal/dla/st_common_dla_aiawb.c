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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "mi_common_datatype.h"
#include "st_common_dla_aiawb.h"
#include "st_common.h"
#include "mi_isp_cus3a_api.h"
#include "mi_isp_awb.h"

#define ULTRA_AWB_NO_READY 0

#if ULTRA_AWB_NO_READY
MI_S32 ST_Common_AIAWB_Init(MI_SYS_ChnPort_t* pstIspChnPort, MI_SYS_ChnPort_t* pstIpuChnPort, MI_U8* pu8AWBModelPath)
{
    ST_CHECK_POINTER(pstIspChnPort);
    ST_CHECK_POINTER(pstIpuChnPort);
    ST_CHECK_POINTER(pu8AWBModelPath);

    CusUltraAwbEnable_t             u8AWBData;
    MI_IPU_OfflineModelStaticInfo_t s32OfflineModelInfo;
    MI_IPU_DevAttr_t                stDevAttr;
    MI_IPUChnAttr_t                 stChnAttr;
    MI_IPU_SubNet_InputOutputDesc_t stDesc;

    u8AWBData.bUltraAwb = TRUE;

    STCHECKRESULT(MI_ISP_CUS3A_EnableUltraAwb(pstIspChnPort->u32DevId, pstIspChnPort->u32ChnId, &u8AWBData));

    STCHECKRESULT(MI_IPU_GetOfflineModeStaticInfo(NULL, (char*)pu8AWBModelPath, &s32OfflineModelInfo));

    memset(&stDevAttr, 0, sizeof(stDevAttr));
    stDevAttr.u32MaxVariableBufSize = s32OfflineModelInfo.u32VariableBufferSize;
    STCHECKRESULT(MI_IPU_CreateDevice(&stDevAttr, NULL, NULL, 0));

    memset(&stChnAttr, 0, sizeof(stChnAttr));
    stChnAttr.u32InputBufDepth  = 1;
    stChnAttr.u32OutputBufDepth = 1;
    STCHECKRESULT(MI_IPU_CreateCHN(&(pstIpuChnPort->u32ChnId), &stChnAttr, NULL, (char*)pu8AWBModelPath));

    STCHECKRESULT(MI_IPU_GetInOutTensorDesc(pstIpuChnPort->u32ChnId, &stDesc));

    ST_INFO("UltraAWB model width:%d,height:%d\n", stDesc.astMI_InputTensorDescs[0].u32TensorShape[1],
            stDesc.astMI_InputTensorDescs[0].u32TensorShape[2]);
    ST_INFO("UltraAWB model outwidth:%d,outheight:%d\n", stDesc.astMI_OutputTensorDescs[0].u32TensorShape[1],
            stDesc.astMI_OutputTensorDescs[0].u32TensorShape[2]);

    return MI_SUCCESS;
}

MI_S32 ST_Common_AIAWB_DeInit(MI_SYS_ChnPort_t* pstIpuChnPort)
{
    ST_CHECK_POINTER(pstIpuChnPort);

    STCHECKRESULT(MI_IPU_DestroyCHN(pstIpuChnPort->u32ChnId));
    STCHECKRESULT(MI_IPU_DestroyDevice());

    return MI_SUCCESS;
}

MI_S32 ST_Common_AIAWB_Run(MI_SYS_ChnPort_t* pstIspChnPort, MI_SYS_ChnPort_t* pstIpuChnPort, MI_FLOAT fUltraAWbFps)
{
    ST_CHECK_POINTER(pstIspChnPort);
    ST_CHECK_POINTER(pstIpuChnPort);

    MI_S32                     s32Ret = -1;
    struct timeval             stStampstart, stStampend;
    MI_U64                     u64AiTime, u64ExpectTime;
    MI_ISP_AWB_HW_STATISTICS_t stAWBAvgData;
    MI_ISP_AWB_CtCaliType_t    stAWBCalidata;
    MI_ISP_AWB_CurCtCaliType_t stAWBCurCalidata;
    CusUltraAwbInfo_t          stUltraAWBData;
    MI_U16*                    pBase = NULL;
    MI_U16                     u16SourceIndex;
    MI_U16                     u16DestIndex;

    MI_FLOAT fCaliRoi_x;
    MI_FLOAT fCaliRoi_y;
    MI_FLOAT fCaliRoi_w;
    MI_FLOAT fCaliRoi_h;
    MI_FLOAT afCaliRoiCenter[2] = {0, 0};

    MI_IPU_TensorVector_t* pstInputTensorVector  = NULL;
    MI_IPU_TensorVector_t* pstOutputTensorVector = NULL;

    gettimeofday(&stStampstart, NULL);

    if (fUltraAWbFps <= 0)
    {
        fUltraAWbFps = 0.1;
    }

    s32Ret = MI_ISP_AWB_GetAwbHwAvgStats(pstIspChnPort->u32DevId, pstIspChnPort->u32ChnId, &stAWBAvgData);
    if (s32Ret != 0)
    {
        printf("MI_ISP_AWB_GetAwbHwAvgStats Failed\n");
        return 0;
    }

    s32Ret = MI_ISP_AWB_GetCtCaliAttr(pstIspChnPort->u32DevId, pstIspChnPort->u32ChnId, &stAWBCalidata);
    if (s32Ret != 0)
    {
        printf("MI_ISP_AWB_GetCtCaliAttr Failed\n");
        return 0;
    }

    s32Ret = MI_ISP_AWB_GetCurCtCaliAttr(pstIspChnPort->u32DevId, pstIspChnPort->u32ChnId, &stAWBCurCalidata);
    if (s32Ret != 0)
    {
        printf("MI_ISP_AWB_GetCurCtCaliAttr Failed\n");
        return 0;
    }

    pstInputTensorVector = (MI_IPU_TensorVector_t*)malloc(sizeof(MI_IPU_TensorVector_t));
    if (pstInputTensorVector == NULL)
    {
        printf("malloc pstInputTensorVector Failed\n");
        return -1;
    }

    pstOutputTensorVector = (MI_IPU_TensorVector_t*)malloc(sizeof(MI_IPU_TensorVector_t));
    if (pstOutputTensorVector == NULL)
    {
        free(pstInputTensorVector);
        printf("malloc pstOutputTensorVector Failed\n");
        return -1;
    }

    CHECK_DLA_RESULT(MI_IPU_GetInputTensors(pstIpuChnPort->u32ChnId, pstInputTensorVector), s32Ret, EXIT);

    CHECK_DLA_RESULT(MI_IPU_GetOutputTensors(pstIpuChnPort->u32ChnId, pstOutputTensorVector), s32Ret, EXIT);

    // Tensors0(AWBAvgData)
    memcpy(pstInputTensorVector->astArrayTensors[0].ptTensorData[0], stAWBAvgData.nAvg,
           sizeof(MI_ISP_AWB_AVGS) * AWB_HW_STAT_BLOCK);

    // Tensors1(AWBCurCalidata)
    u16SourceIndex = stAWBCalidata.u16EndIdx;
    for (u16DestIndex = 9; (u16DestIndex >= 0) && (u16DestIndex <= 9); u16DestIndex--)
    {
        fCaliRoi_x = (MI_FLOAT) * (stAWBCurCalidata.u16CtParams + (u16SourceIndex * 4) + 1);
        fCaliRoi_y = (MI_FLOAT) * (stAWBCurCalidata.u16CtParams + (u16SourceIndex * 4) + 2);
        fCaliRoi_w = (MI_FLOAT) * (stAWBCurCalidata.u16CtParams + (u16SourceIndex * 4) + 3);
        fCaliRoi_h = (MI_FLOAT) * (stAWBCurCalidata.u16CtParams + (u16SourceIndex * 4) + 4);

        // get center point
        afCaliRoiCenter[0] = fCaliRoi_x + (fCaliRoi_w / 2);
        afCaliRoiCenter[1] = fCaliRoi_y + (fCaliRoi_h / 2);

        memcpy((MI_FLOAT*)pstInputTensorVector->astArrayTensors[1].ptTensorData[0] + (u16DestIndex * 2),
               afCaliRoiCenter, sizeof(MI_FLOAT) * 2);

        if (u16SourceIndex == stAWBCalidata.u16StartIdx)
        {
            continue;
        }

        u16SourceIndex--;
    }

    MI_SYS_FlushInvCache(pstInputTensorVector->astArrayTensors[0].ptTensorData[0],
                         sizeof(MI_ISP_AWB_AVGS) * AWB_HW_STAT_BLOCK);
    MI_SYS_FlushInvCache(pstInputTensorVector->astArrayTensors[1].ptTensorData[0],
                         sizeof(MI_U16) * (MI_ISP_AWB_CT_CALI_CNT / 2));

    CHECK_DLA_RESULT(MI_IPU_Invoke(pstIpuChnPort->u32ChnId, pstInputTensorVector, pstOutputTensorVector), s32Ret, EXIT);

    stUltraAWBData.uNumberOfUltraAwb = 1;

    // Set RGB gain
    pBase = (MI_U16*)pstOutputTensorVector->astArrayTensors[0].ptTensorData[0];
    for (int i = 0; i < stUltraAWBData.uNumberOfUltraAwb; i++)
    {
        stUltraAWBData.atUltraAwbParam[i].Weight = *(pBase + (4 * i) + 0);
        stUltraAWBData.atUltraAwbParam[i].Rgain  = *(pBase + (4 * i) + 1);
        stUltraAWBData.atUltraAwbParam[i].Ggain  = *(pBase + (4 * i) + 2);
        stUltraAWBData.atUltraAwbParam[i].Bgain  = *(pBase + (4 * i) + 3);
    }
    CHECK_DLA_RESULT(MI_ISP_CUS3A_SetUltraAwbParam(pstIpuChnPort->u32DevId, pstIpuChnPort->u32ChnId, &stUltraAWBData),
                     s32Ret, EXIT);

    CHECK_DLA_RESULT(MI_IPU_PutInputTensors(pstIpuChnPort->u32ChnId, pstInputTensorVector), s32Ret, EXIT);

    CHECK_DLA_RESULT(MI_IPU_PutOutputTensors(pstIpuChnPort->u32ChnId, pstOutputTensorVector), s32Ret, EXIT);

    gettimeofday(&stStampend, NULL);

    u64AiTime =
        (stStampend.tv_sec * 1000000 + stStampend.tv_usec) - (stStampstart.tv_sec * 1000000 + stStampstart.tv_usec);
    u64ExpectTime = 1000000 / fUltraAWbFps;

    if (u64AiTime < u64ExpectTime)
    {
        usleep(u64ExpectTime - u64AiTime);
    }

EXIT:
    free(pstInputTensorVector);
    free(pstOutputTensorVector);
    pstInputTensorVector  = NULL;
    pstOutputTensorVector = NULL;

    return s32Ret;
}
#endif
