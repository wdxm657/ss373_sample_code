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

#ifndef _PCAM_TASK_H
#define _PCAM_TASK_H

#include "pCam_msg.h"

//==============================================================================
// MACRO DEFINE
//==============================================================================

/*
   1: enable pCam task process command, includes 4 types: PCAM_BLOCKING, PCAM_NONBLOCKING, PCAM_API, PCAM_OVERWR,
   0: disable pCam task process command,
 */
#define EN_PCAM_CMD_PRCS                            (1)
#define MAX_PCAM_TASK_WQ_SIZE (20)

#define MAX_PCAM_TASK_MAX_DATA_SIZE (64)

//==============================================================================
// VARIABLES
//==============================================================================



//==============================================================================
// STRUCTURES AND TYPEDEF
//==============================================================================

typedef enum
{
    E_PCAM_TASK_WORK_OK = 0,
    E_PCAM_TASK_WORK_FAIL = -1,
}PcamTaskWorkResult_e;


typedef enum
{
    E_PCAM_TASK_WORK_INVALID = 0,
    E_PCAM_TASK_WORK_WAIT,
}PcamTaskWorkStatus_e;

typedef struct
{
    PcamTaskWorkCallback_fp fpCB;
    unsigned char msg_data[MAX_PCAM_TASK_MAX_DATA_SIZE];
    unsigned long msg_data_size;
    CamOsTsem_t *msg_sem;
    u32 *pret;
    PcamTaskWorkStatus_e eStatus;
}PcamTaskWork_t;

typedef struct task_struct* PcamTaskWorkHandle;
typedef struct
{
    CamOsMutex_t tLock;
    CamOsAtomic_t tNumWorks;
    int nWIdx;
    int nRIdx;
    CamOsTsem_t tSem;
    PcamTaskWorkHandle tThread;
    PcamTaskWork_t tWorks[MAX_PCAM_TASK_WQ_SIZE];
    int nStopThread;
    int thread_end;
}PcamTaskWqPool_t;

//==============================================================================
// FUNCTION PROTOTYPES
//==============================================================================

int pCam_task_Init(void);

//CamOS
int PcamTaskWqAdd(PcamTaskWorkCallback_fp fpCB, void *msg_data, unsigned long msg_data_size, CamOsTsem_t *msg_sem, u32 *pret);
int PcamTaskWqGet(PcamTaskWqPool_t *ptWorkPool, PcamTaskWork_t *ptWork);
int PcamTaskWqClean(PcamTaskWqPool_t *ptWorkPool);
void* PcamTaskWqProc(void* pArg);
void PcamTaskWqInit(PcamTaskWqPool_t* pWorkPool);
void PcamTaskWqDeinit(PcamTaskWqPool_t* pWorkPool);
void PcamTaskWqFlush(PcamTaskWqPool_t* pWorkPool);

void* pCam_Task(void* p);
void pCam_Task_Stop(void);

#endif //#ifndef _PCAM_TASK_H

