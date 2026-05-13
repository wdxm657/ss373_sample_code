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
#include "cam_os_wrapper.h"
#include "usb_app_dbg.h"

//==============================================================================
//                              MACRO
//==============================================================================


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

PcamTaskWqPool_t gtPcamTaskWorkQ;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================


//------------------------------------------------------------------------------
//  Function    : pCam_task_init
//  Parameter   : None
//  Return Value : None
//  Description :
//------------------------------------------------------------------------------
int pCam_task_Init(void)
{
    return 0;
}

/**
 * pCam task handler.
 */
void pCam_PrcsMsg(void *pRecvMsg)
{}

int PcamTaskWqAdd(PcamTaskWorkCallback_fp fpCB, void *msg_data, unsigned long msg_data_size, CamOsTsem_t *msg_sem, u32 *pret)
{
    PcamTaskWqPool_t *ptWorkPool = &gtPcamTaskWorkQ;

    if(fpCB == NULL) return E_PCAM_TASK_WORK_FAIL;

    if(CamOsAtomicRead(&ptWorkPool->tNumWorks) >= MAX_PCAM_TASK_WQ_SIZE)
    {
        CamOsPrintf("%s,%d fail!\r\n", __FUNCTION__, __LINE__);
        return E_PCAM_TASK_WORK_FAIL;
    }

    if(msg_data_size >= MAX_PCAM_TASK_MAX_DATA_SIZE)
    {
        CamOsPrintf("%s, data size err:%d!\r\n", __FUNCTION__, msg_data_size);
        return E_PCAM_TASK_WORK_FAIL;
    }

    ptWorkPool->tWorks[ptWorkPool->nWIdx].fpCB = fpCB;
    memcpy((void *)ptWorkPool->tWorks[ptWorkPool->nWIdx].msg_data, msg_data, msg_data_size);
    ptWorkPool->tWorks[ptWorkPool->nWIdx].msg_data_size = msg_data_size;
    ptWorkPool->tWorks[ptWorkPool->nWIdx].msg_sem = msg_sem;
    ptWorkPool->tWorks[ptWorkPool->nWIdx].pret = pret;
    ptWorkPool->tWorks[ptWorkPool->nWIdx].eStatus = E_PCAM_TASK_WORK_WAIT;

    if(++(ptWorkPool->nWIdx) >= MAX_PCAM_TASK_WQ_SIZE) ptWorkPool->nWIdx = 0;

    CamOsAtomicIncReturn(&ptWorkPool->tNumWorks);
    CamOsTsemUp(&ptWorkPool->tSem);

    return CamOsAtomicRead(&ptWorkPool->tNumWorks);
}

int PcamTaskWqGet(PcamTaskWqPool_t *ptWorkPool, PcamTaskWork_t *ptWork)
{
    int nThreadNumWorks = 0;

    if(ptWorkPool == NULL || ptWork == NULL) return E_PCAM_TASK_WORK_FAIL;

    CamOsMutexLock(&ptWorkPool->tLock);

    if((CamOsAtomicRead(&ptWorkPool->tNumWorks) == 0) || (ptWorkPool->tWorks[ptWorkPool->nRIdx].eStatus == E_PCAM_TASK_WORK_INVALID))
    {
        CamOsMutexUnlock(&ptWorkPool->tLock);
        return E_PCAM_TASK_WORK_FAIL;
    }

    memcpy(ptWork, &ptWorkPool->tWorks[ptWorkPool->nRIdx], sizeof(PcamTaskWork_t));
    ptWorkPool->tWorks[ptWorkPool->nRIdx].eStatus = E_PCAM_TASK_WORK_INVALID;

    if(++(ptWorkPool->nRIdx) >= MAX_PCAM_TASK_WQ_SIZE) ptWorkPool->nRIdx = 0;

    CamOsAtomicDecReturn(&ptWorkPool->tNumWorks);
    nThreadNumWorks = CamOsAtomicRead(&ptWorkPool->tNumWorks);

    CamOsMutexUnlock(&ptWorkPool->tLock);

    return (nThreadNumWorks);
}

int PcamTaskWqClean(PcamTaskWqPool_t *ptWorkPool)
{
    int bRet = 0;
    int i = 0, s32WorkNum = 0;
    PcamTaskWork_t tWork;

    if(ptWorkPool == NULL) return E_PCAM_TASK_WORK_FAIL;

    s32WorkNum = CamOsAtomicRead(&ptWorkPool->tNumWorks);
    if(s32WorkNum <= 0) return E_PCAM_TASK_WORK_FAIL;

    for (i = 0 ; i < s32WorkNum ; i++)
    {
        if(PcamTaskWqGet(ptWorkPool, &tWork) >= 0)
        {
            continue;  // skip
        }
        else
        {
            break;
        }
    }

    return bRet;
}

static inline unsigned int PcamTaskWqProcShouldStop(int stop_thread_flag)
{
    return ((CAM_OS_OK == CamOsThreadShouldStop()) || (stop_thread_flag != 0));
}

void* PcamTaskWqProc(void* pArg)
{
    PcamTaskWqPool_t *ptWorkPool = (PcamTaskWqPool_t*)pArg;

    while(!PcamTaskWqProcShouldStop(ptWorkPool->nStopThread))
    {
        PcamTaskWork_t tWork;
        u32 ret = 0;
        CamOsRet_e eRet;

        eRet = CamOsTsemTimedDown(&ptWorkPool->tSem, 100);

        if(eRet == CAM_OS_OK)
        {
            while(!PcamTaskWqProcShouldStop(ptWorkPool->nStopThread))
            {
                if(PcamTaskWqGet(ptWorkPool, &tWork) >= 0)
                {
                    if(tWork.fpCB)
                    {
                        ret = tWork.fpCB(&(tWork.msg_data), tWork.msg_data_size);
                        if(tWork.pret)
                        {
                            *(tWork.pret) = ret;
                        }

                        if (tWork.msg_sem)
                        {
                            /*Release semaphore*/
                            CamOsTsemUp(tWork.msg_sem);
                        }
                    }
                }
                else
                    break;
            }
        }
        else if(eRet == CAM_OS_TIMEOUT)
        {
            //NOP
        }
    }

    ptWorkPool->thread_end = 1;

    return NULL;
}

void PcamTaskWqInit(PcamTaskWqPool_t* pWorkPool)
{
    if(pWorkPool == NULL) return;

    if(CamOsMutexInit(&pWorkPool->tLock) != CAM_OS_OK) return;

    CamOsAtomicSet(&pWorkPool->tNumWorks, 0);

    /* Initialize condition wait object*/
    CamOsTsemInit(&pWorkPool->tSem, 0);

    memset(pWorkPool->tWorks, 0, sizeof(pWorkPool->tWorks));

    pWorkPool->tThread = NULL;
    pWorkPool->nStopThread = 0;
    pWorkPool->thread_end = 0;
}

void PcamTaskWqDeinit(PcamTaskWqPool_t* pWorkPool)
{
    if(pWorkPool == NULL) return;

    pWorkPool->tThread = NULL;
    PcamTaskWqFlush(pWorkPool);
    CamOsTsemDeinit(&pWorkPool->tSem);
    CamOsMutexDestroy(&pWorkPool->tLock);
}

void PcamTaskWqFlush(PcamTaskWqPool_t* pWorkPool)
{
    if(pWorkPool == NULL) return;

    PcamTaskWqClean(pWorkPool);
    CamOsMutexLock(&pWorkPool->tLock);
    CamOsAtomicSet(&pWorkPool->tNumWorks, 0);
    pWorkPool->nRIdx = 0;
    pWorkPool->nWIdx = 0;
    memset(pWorkPool->tWorks, 0, sizeof(pWorkPool->tWorks));
    CamOsMutexUnlock(&pWorkPool->tLock);
}

void* pCam_Task(void* p)
{
    PcamTaskWqPool_t *pPcamTaskWqPool;

    CamOsPrintf("pCam_Task \n\r");

    pCam_task_Init();
    pPcamTaskWqPool = &gtPcamTaskWorkQ;
    PcamTaskWqInit(pPcamTaskWqPool);
    PcamTaskWqProc(pPcamTaskWqPool);
    return 0;
}

void pCam_Task_Stop(void)
{
    PcamTaskWqPool_t *pPcamTaskWqPool;
    u32 timeout = 1000;

    pPcamTaskWqPool = &gtPcamTaskWorkQ;
    pPcamTaskWqPool->nStopThread = 1;
    CamOsTsemUp(&pPcamTaskWqPool->tSem);
    while((pPcamTaskWqPool->thread_end == 0) &&(--timeout > 0))
    {
        CamOsMsSleep(1);
    }
    RET_ON(timeout == 0);
    PcamTaskWqDeinit(pPcamTaskWqPool);
}
