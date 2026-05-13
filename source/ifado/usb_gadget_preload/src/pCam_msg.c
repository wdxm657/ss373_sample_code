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

//==============================================================================
//
//                              INCLUDE FILE
//
//==============================================================================

#include <string.h>
#include "pCam_task.h"
#include "uvc_video.h"

//==============================================================================
//                              MACRO
//==============================================================================

#define MSG_Q_NUMS                                  (3)

//==============================================================================
//                         STRUCTURES AND TYPEDEF
//==============================================================================


//==============================================================================
//                              PROTOTYPE
//==============================================================================

//==============================================================================
//
//                              GLOBAL VARIABLE
//
//==============================================================================

static u16       msg_timeout = 10 * 1000 ; /*10 seconds timeout*/

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

//Blocking call
u32 pCam_SendMsg(PcamTaskWorkCallback_fp fp_cb, void *msg_data, unsigned long msg_data_size)
{
    u32 err = PCAM_ERROR_NONE;
    PcamTaskWorkResult_e   MsErr;
    CamOsRet_e  CamErr;
    CamOsTsem_t sem;

    CamErr = CamOsTsemInit(&sem, 0);
    if (CamErr != CAM_OS_OK)
    {
        CamOsPrintf("[%s] <<msg create sem err %d>>\r\n", __FUNCTION__, CamErr);
        return PCAM_SYS_ERR ;
    }

    MsErr = PcamTaskWqAdd(fp_cb, msg_data, msg_data_size, &sem, &err);

    if (MsErr == E_PCAM_TASK_WORK_FAIL)
    {
        CamOsPrintf("[%s] <<msg send err x%X>>\n", __FUNCTION__, MsErr);
        err =  PCAM_SYS_ERR ;
        goto pCam_SendMsg_error_return;
    }

    CamErr = CamOsTsemTimedDown(&sem, msg_timeout);

    if (CamErr != CAM_OS_OK)
    {
        CamOsPrintf("[%s] <<acquire sem err %d,", __FUNCTION__, CamErr);
        err = (CamErr == CAM_OS_TIMEOUT) ? PCAM_SYS_TIMEOUT : PCAM_SYS_ERR;
        goto pCam_SendMsg_error_return;
    }

pCam_SendMsg_error_return:
    CamErr = CamOsTsemDeinit(&sem);
    if (CamErr != CAM_OS_OK)
    {
        CamOsPrintf("[%s] <<msg del sem err %d>>\r\n", __FUNCTION__, CamErr);
    }

    return err;
}

u32 pCam_PostMsg(PcamTaskWorkCallback_fp fp_cb, void *msg_data, unsigned long msg_data_size)
{
    u32 err = PCAM_ERROR_NONE;
    PcamTaskWorkResult_e   MsErr;

    MsErr = PcamTaskWqAdd(fp_cb, msg_data, msg_data_size, NULL/*Non blocking*/, NULL/*ret*/);

    if (MsErr == E_PCAM_TASK_WORK_FAIL)
    {
        CamOsPrintf("[%s] MsSend err %d,\n", __FUNCTION__, MsErr);
        err = PCAM_SYS_ERR;
    }
    else
    {
        err = PCAM_ERROR_NONE;
    }

    return err;
}
