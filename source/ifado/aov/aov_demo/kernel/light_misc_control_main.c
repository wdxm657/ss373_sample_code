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

#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

/* #include <linux/sched.h> */
/* #include <uapi/linux/sched/types.h> */

#include "light_misc_control_main.h"
#include "light_misc_control_hw.h"
#include "mi_vif_datatype.h"

static SSTAR_Light_Device_State_t *pLightState = NULL;

#ifdef LIGHT_SUPPORT_MULTI_VIFDEVICE
#define MAX_VIF_DEVICE_NUM MI_VIF_MAX_GROUP_DEV_CNT
#endif

#ifndef LIGHT_USE_HRTIMER
static struct task_struct *phtask_main      = NULL;
static CamOsCondition_t    resume_waitqueue = {0};
static bool                bWaitResume      = true;
static bool                bModuleExit      = false;

static int SSTAR_Light_Misc_Control_Thread_Creat(void);
static int SSTAR_Light_Misc_Control_Thread_Destory(void);
#endif

#define DEBUG_INFO(fmt, ...) \
    do { \
        if (pLightState->bDebugLog) { \
            printk(KERN_INFO "[%s]:%d " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

int SSTAR_Light_Misc_Control_Set_DebugLog(bool bTurn)
{
    if (pLightState && pLightState->bDeviceHasInited)
    {
        pLightState->bDebugLog = bTurn;
    }
    return 0;
}

int SSTAR_Light_Misc_Control_IOCTL_Set_Ircut_State(SSTAR_Switch_State_e state)
{
    return SSTAR_Light_Misc_Control_Hw_Set_Ircut_State(state, pLightState);
}

int SSTAR_Light_Misc_Control_IOCTL_Get_LightSensor(void)
{
    return SSTAR_Light_Misc_Control_Hw_Get_LightSensor(pLightState);
}

#ifdef LIGHT_SUPPORT_MULTI_VIFDEVICE
typedef enum
{
    E_MULTI_VIFDEV_FUN_RELEASE,
    E_MULTI_VIFDEV_FUN_MARKDOING,
    E_MULTI_VIFDEV_FUN_COUNT_DOING_CNT,
    E_MULTI_VIFDEV_FUN_HAD_INIT_DEV_CNT,
    E_MULTI_VIFDEV_FUN_MAX,
} SSTAR_MULTI_VIFDEV_FUN;

/* vifDevId only for E_MULTI_VIFDEV_FUN_COUNT_DOING_CNT   E_MULTI_VIFDEV_FUN_HAD_INIT_DEV_CNT*/
static int SSTAR_Light_Misc_Control_VifDevIdList_Fun(SSTAR_MULTI_VIFDEV_FUN fun, int vifDevId, bool bInIrq)
{
    int ret = 0;
    if (pLightState && pLightState->bDeviceHasInited)
    {
        SSTAR_Light_Device_Multi_VifDevice_State_t *vifDevId_State;
        SSTAR_Light_Device_Multi_VifDevice_State_t *vifDevId_TmpState;
        int                                         doingCnt    = 0;
        int                                         vifDevIdCnt = 0;
        bool                                        bFoundDev   = false;

        Light_MISC_SPINLOCK(bInIrq, &pLightState->vifDeviceDoneSpinlock);
        CAM_OS_LIST_FOR_EACH_ENTRY_SAFE(vifDevId_State, vifDevId_TmpState, &pLightState->vifDeviceStateQueue, vifdevice_list)
        {
            if (fun == E_MULTI_VIFDEV_FUN_RELEASE)
            {
                CAM_OS_LIST_DEL(&vifDevId_State->vifdevice_list);
                CamOsMemRelease(vifDevId_State);
            }
            else if (fun == E_MULTI_VIFDEV_FUN_MARKDOING)
            {
                vifDevId_State->deviceDoneState = 0;
            }
            else if (fun == E_MULTI_VIFDEV_FUN_COUNT_DOING_CNT)
            {
                if (vifDevId_State->vifDevId == vifDevId)
                {
                    vifDevId_State->deviceDoneState = 1;
                    bFoundDev                       = true;
                }
                if (vifDevId_State->deviceDoneState == 0)
                {
                    ret = ++doingCnt;
                    DEBUG_INFO("vifDevId:%d is doing ret:%d\n", vifDevId_State->vifDevId, ret);
                }
            }
            else if (fun == E_MULTI_VIFDEV_FUN_HAD_INIT_DEV_CNT)
            {
                if (vifDevId_State->vifDevId == vifDevId)
                {
                    printk(KERN_ERR "already init for vifDevId:%d vifDevId_State:%px\n", vifDevId_State->vifDevId,
                           vifDevId_State);
                    ret = -1;
                    break;
                }
                ret = ++vifDevIdCnt;
            }
            else
            {
                printk(KERN_ERR "unknow fun:%d\n", fun);
            }
        }
        Light_MISC_SPINUNLOCK(bInIrq, &pLightState->vifDeviceDoneSpinlock);
        if (fun == E_MULTI_VIFDEV_FUN_COUNT_DOING_CNT && !bFoundDev)
        {
            printk(KERN_ERR "not found vifDevId:%d in list, please check if this vifDev has been init", vifDevId);
        }
    }
    return ret;
}
#endif

static int SSTAR_Light_Misc_Control_FrameDone_CallBack(MI_VIF_FrameEndInfo_t *pstVifFrameEndInfo)
{
/***********************************************
This callback function serves as an interrupt callback for the completion of processing one VIF frame.
Inside the interrupt, it is not permitted to perform time-consuming actions or wait for operations,
such as interacting with I2C devices.
***********************************************/

    if (pLightState && pLightState->bDeviceHasInited)
    {
        DEBUG_INFO("vifDevId: %d doneFlag:%d frameCnt:%d\n", pstVifFrameEndInfo->vifDevId,
        pstVifFrameEndInfo->bDoneFlag, pstVifFrameEndInfo->u32FrameCnt);
        if (pstVifFrameEndInfo && pstVifFrameEndInfo->bDoneFlag)
        {
            if (pLightState->stLight_Ctl_Attr.controlType == E_CONTROL_TYPE_MULTI_FRAME)
            {
#ifdef LIGHT_SUPPORT_MULTI_VIFDEVICE
                int doingCnt = 0;
                doingCnt = SSTAR_Light_Misc_Control_VifDevIdList_Fun(E_MULTI_VIFDEV_FUN_COUNT_DOING_CNT,
                                                              pstVifFrameEndInfo->vifDevId, true);
                if (doingCnt != 0)
                {
                    DEBUG_INFO("remin %d vifDev doing\n", doingCnt);
                    return 0;
                }
#endif
                {
                    pLightState->reciveVifEventCnt = 0;
                    SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(
                        E_SWITCH_STATE_OFF, pLightState->stLight_Ctl_Attr.lightType, 0, pLightState, true);
                    if (pLightState->stLight_Ctl_Attr.lightType == E_LIGHT_TYPE_IR
                        && SSTAR_Light_Misc_Control_Hw_Get_LEDIR_Status(E_LIGHT_TYPE_LED, pLightState, true)
                               == E_SWITCH_STATE_ON)
                    {
                        SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(E_SWITCH_STATE_OFF, E_LIGHT_TYPE_LED, 0,
                                                                     pLightState, true);
                    }
                    if (pLightState->stLight_Ctl_Attr.lightType == E_LIGHT_TYPE_LED
                        && SSTAR_Light_Misc_Control_Hw_Get_LEDIR_Status(E_LIGHT_TYPE_IR, pLightState, true)
                               == E_SWITCH_STATE_ON)
                    {
                        SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(E_SWITCH_STATE_OFF, E_LIGHT_TYPE_IR, 0,
                                                                     pLightState, true);
                    }
                }
                DEBUG_INFO("turn off lightType:%d\n", pLightState->stLight_Ctl_Attr.lightType);
                pLightState->doneFlagCnt++;
            }
            /* we don't care about long-term on/off state */
        }
        else if (pstVifFrameEndInfo && !pstVifFrameEndInfo->bDoneFlag)
        {
            pLightState->reciveVifEventCnt = pstVifFrameEndInfo->u32FrameCnt;
        }
    }
    return 0;
}

static int SSTAR_Light_Misc_Control_Get_Vif_EventPtr(void **pVifFrameDoneCallBack)
{
    void *fun_ptr_addr = SSTAR_Light_Misc_Control_FrameDone_CallBack;
    if (pVifFrameDoneCallBack)
    {
        *pVifFrameDoneCallBack = fun_ptr_addr;
    }
    printk(KERN_INFO "fun_ptr_addr:0x%px\n", fun_ptr_addr);

    return 0;
}

#ifdef LIGHT_USE_HRTIMER
static enum hrtimer_restart light_misc_control_resume_timer_fn(struct hrtimer *timer)
{
    SSTAR_Light_Device_State_t *pLight = container_of(timer, SSTAR_Light_Device_State_t, hrTimer);

    if (pLight->stLight_Ctl_Attr.controlType == E_CONTROL_TYPE_MULTI_FRAME)
    {
        SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(E_SWITCH_STATE_ON, pLight->stLight_Ctl_Attr.lightType,
                                                     pLight->stLight_Ctl_Attr.lightIntensity, pLight, true);

        DEBUG_INFO("turn on lightType:%d in hrtimer\n", pLight->stLight_Ctl_Attr.lightType);
        pLight->openLightCnt++;
    }
    return HRTIMER_NORESTART;
}
#endif

int SSTAR_Light_Misc_Control_IOCTL_Init(int vifDevId, void **pVifFrameDoneCallBack)
{
    if (pVifFrameDoneCallBack)
    {
        if (pLightState && pLightState->bDeviceHasInited == false)
        {
            pLightState->bDeviceHasInited = true;

#ifdef LIGHT_USE_HRTIMER
            hrtimer_init(&pLightState->hrTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_HARD);
            pLightState->hrTimer.function = light_misc_control_resume_timer_fn;
#else
            CamOsConditionInit(&resume_waitqueue);
            SSTAR_Light_Misc_Control_Thread_Creat();
#endif
        }
        else
        {
            if (!pLightState)
            {
                printk(KERN_INFO "Light_Misc_Control Init fail! pLightState:%px\n", pLightState);
                return -1;
            }
        }

#ifdef LIGHT_SUPPORT_MULTI_VIFDEVICE
        if (pLightState && pLightState->bDeviceHasInited && vifDevId >= 0)
        {

            SSTAR_Light_Device_Multi_VifDevice_State_t * new_AddVifDevId_State;

            int vifDevIdCnt =
                SSTAR_Light_Misc_Control_VifDevIdList_Fun(E_MULTI_VIFDEV_FUN_HAD_INIT_DEV_CNT, vifDevId, false);

            if (vifDevIdCnt == -1)
            {
                return -1;
            }
            else if (vifDevIdCnt >= MAX_VIF_DEVICE_NUM)
            {
                printk(KERN_ERR "only support %d vifdevice now, had init %d vifDev\n", MAX_VIF_DEVICE_NUM, vifDevIdCnt);
                return -1;
            }

            new_AddVifDevId_State = CamOsMemAlloc(sizeof(SSTAR_Light_Device_Multi_VifDevice_State_t));
            if (!new_AddVifDevId_State)
            {
                printk(KERN_ERR "alloc new_AddVifDevId_State failed\n");
                return -1;
            }
            memset(new_AddVifDevId_State, 0x0, sizeof(SSTAR_Light_Device_Multi_VifDevice_State_t));
            new_AddVifDevId_State->vifDevId = vifDevId;

            Light_MISC_SPINLOCK(false, &pLightState->vifDeviceDoneSpinlock);
            CAM_OS_LIST_ADD(&new_AddVifDevId_State->vifdevice_list, &pLightState->vifDeviceStateQueue);
            Light_MISC_SPINUNLOCK(false, &pLightState->vifDeviceDoneSpinlock);

        }
#endif

        return SSTAR_Light_Misc_Control_Get_Vif_EventPtr(pVifFrameDoneCallBack);
    }
    else
    {
        printk(KERN_WARNING "Init Failed! pVifFrameDoneCallBack can't be null\n");
    }
    return 0;
}

int SSTAR_Light_Misc_Control_IOCTL_DeInit(bool bFromIoctl)
{
    if (pLightState && pLightState->bDeviceHasInited)
    {
#ifdef LIGHT_USE_HRTIMER
        hrtimer_try_to_cancel(&pLightState->hrTimer);
#else
        SSTAR_Light_Misc_Control_Thread_Destory();
        CamOsConditionDeinit(&resume_waitqueue);
#endif

#ifdef LIGHT_SUPPORT_MULTI_VIFDEVICE
        SSTAR_Light_Misc_Control_VifDevIdList_Fun(E_MULTI_VIFDEV_FUN_RELEASE, -1, false);
#endif

        pLightState->bDeviceHasInited = false;
    }
    else if (!pLightState && bFromIoctl)
    {
        printk(KERN_INFO "Light_Misc_Control already had DeInited or not been Inited before\n");
    }
    return 0;
}

int SSTAR_Light_Misc_Control_IOCTL_Set_Attr(SSTAR_Light_Ctl_Attr_t *attr)
{
    if (pLightState && pLightState->bDeviceHasInited)
    {
        if (attr)
        {
            if (memcmp(attr, &pLightState->stLight_Ctl_Attr, sizeof(SSTAR_Light_Ctl_Attr_t)) == 0)
            {
                printk(KERN_INFO "nothing be updated\n");
                printk(KERN_INFO "attr[%d:%d:%d:%d]\n", attr->controlType, attr->delayOpenTimeMs, attr->lightIntensity,
                       attr->lightType);
                printk(KERN_INFO "pLightState->stLight_Ctl_Attr[%d:%d:%d:%d]\n",
                       pLightState->stLight_Ctl_Attr.controlType, pLightState->stLight_Ctl_Attr.delayOpenTimeMs,
                       pLightState->stLight_Ctl_Attr.lightIntensity, pLightState->stLight_Ctl_Attr.lightType);

                return 0;
            }
            if (attr->controlType == E_CONTROL_TYPE_LONG_TERM_ON || attr->controlType == E_CONTROL_TYPE_LONG_TERM_OFF)
            {
                if (pLightState->stLight_Ctl_Attr.lightType != attr->lightType)
                {
                    if (pLightState->stLight_Ctl_Attr.lightType == E_LIGHT_TYPE_IR
                        && SSTAR_Light_Misc_Control_Hw_Get_LEDIR_Status(E_LIGHT_TYPE_IR, pLightState, false)
                               == E_SWITCH_STATE_ON)
                    {
                        SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(
                            E_SWITCH_STATE_OFF, pLightState->stLight_Ctl_Attr.lightType, 0, pLightState, false);
                    }
                    else if (pLightState->stLight_Ctl_Attr.lightType == E_LIGHT_TYPE_LED
                             && SSTAR_Light_Misc_Control_Hw_Get_LEDIR_Status(E_LIGHT_TYPE_LED, pLightState, false)
                                    == E_SWITCH_STATE_ON)
                    {
                        SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(
                            E_SWITCH_STATE_OFF, pLightState->stLight_Ctl_Attr.lightType, 0, pLightState, false);
                    }

                    if (attr->controlType == E_CONTROL_TYPE_LONG_TERM_OFF)
                    {
                        SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(E_SWITCH_STATE_OFF, attr->lightType, 0,
                                                                     pLightState, false);
                    }
                    else if (attr->controlType == E_CONTROL_TYPE_LONG_TERM_ON)
                    {
                        SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(E_SWITCH_STATE_ON, attr->lightType, 0, pLightState,
                                                                     false);
                    }
                }
                else
                {
                    if (pLightState->stLight_Ctl_Attr.lightType == E_LIGHT_TYPE_IR)
                    {
                        if (SSTAR_Light_Misc_Control_Hw_Get_LEDIR_Status(E_LIGHT_TYPE_IR, pLightState, false)
                                == E_SWITCH_STATE_ON
                            && attr->controlType == E_CONTROL_TYPE_LONG_TERM_OFF)
                        {
                            SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(
                                E_SWITCH_STATE_OFF, pLightState->stLight_Ctl_Attr.lightType, 0, pLightState, false);
                        }
                        else if (SSTAR_Light_Misc_Control_Hw_Get_LEDIR_Status(E_LIGHT_TYPE_IR, pLightState, false)
                                     == E_SWITCH_STATE_OFF
                                 && attr->controlType == E_CONTROL_TYPE_LONG_TERM_ON)
                        {
                            SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(
                                E_SWITCH_STATE_ON, pLightState->stLight_Ctl_Attr.lightType, 0, pLightState, false);
                        }
                    }
                    else if (pLightState->stLight_Ctl_Attr.lightType == E_LIGHT_TYPE_LED)
                    {
                        if (SSTAR_Light_Misc_Control_Hw_Get_LEDIR_Status(E_LIGHT_TYPE_LED, pLightState, false)
                                == E_SWITCH_STATE_ON
                            && attr->controlType == E_CONTROL_TYPE_LONG_TERM_OFF)
                        {
                            SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(
                                E_SWITCH_STATE_OFF, pLightState->stLight_Ctl_Attr.lightType, 0, pLightState, false);
                        }
                        else if (SSTAR_Light_Misc_Control_Hw_Get_LEDIR_Status(E_LIGHT_TYPE_LED, pLightState, false)
                                     == E_SWITCH_STATE_OFF
                                 && attr->controlType == E_CONTROL_TYPE_LONG_TERM_ON)
                        {
                            SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(
                                E_SWITCH_STATE_ON, pLightState->stLight_Ctl_Attr.lightType, 0, pLightState, false);
                        }
                    }
                    else
                    {
                        printk(KERN_ERR "lightType:%d error\n", pLightState->stLight_Ctl_Attr.lightType);
                    }
                }
                memcpy(&pLightState->stLight_Ctl_Attr, attr, sizeof(SSTAR_Light_Ctl_Attr_t));
            }
            else
            {
                /* The setting cannot take effect immediately, in case the setting is turned off before framedone,
                 * causing a frame to be half exposed */
                memcpy(&pLightState->stLight_Ctl_Attr, attr, sizeof(SSTAR_Light_Ctl_Attr_t));
            }
        }
    }
    else
    {
        printk(KERN_INFO "please init device first\n");
    }
    return 0;
}

int SSTAR_Light_Misc_Control_IOCTL_Get_Attr(SSTAR_Light_Ctl_Attr_t *attr)
{
    if (pLightState && pLightState->bDeviceHasInited)
    {
        if (attr)
        {
            memcpy(attr, &pLightState->stLight_Ctl_Attr, sizeof(SSTAR_Light_Ctl_Attr_t));
        }
    }
    return 0;
}

int SSTAR_Light_Misc_Control_Debug_Get_State(char *debugBuf)
{
    if (pLightState && pLightState->bDeviceHasInited)
    {
        sprintf(debugBuf,
                "controlType:%d lightType:%d delayOpenTimeMs:%d lightIntensity:%d\n"
                "eIRcutState:0x%x eIRState:%d eLEDState:%d reciveVifEventCnt:%d doneFlagCnt:%d openLightCnt:%d\n",
                pLightState->stLight_Ctl_Attr.controlType, pLightState->stLight_Ctl_Attr.lightType,
                pLightState->stLight_Ctl_Attr.delayOpenTimeMs, pLightState->stLight_Ctl_Attr.lightIntensity,
                pLightState->eIRcutState, pLightState->eIRState, pLightState->eLEDState, pLightState->reciveVifEventCnt,
                pLightState->doneFlagCnt, pLightState->openLightCnt);
        if (pLightState->bDebugLog)
        {
            sprintf(debugBuf + strlen(debugBuf), "debuglog enable\n");
        }
    }
    else
    {
        sprintf(debugBuf, "pLightState not init\n");
    }

    return 0;
}

#ifndef LIGHT_USE_HRTIMER
static int main_work_thread(void *data)
{
    while (0 == kthread_should_stop() && !bModuleExit)
    {
        /* msleep(300); */
        do
        {
            CamOsConditionTimedWait(&(resume_waitqueue), bWaitResume == false, 2000);
        } while (bWaitResume == true);
        if (!bModuleExit && pLightState && pLightState->bDeviceHasInited)
        {
            /* printk("we need do something in there when resume every time\n"); */
            bWaitResume = true;
            if (pLightState->stLight_Ctl_Attr.controlType == E_CONTROL_TYPE_MULTI_FRAME)
            {
                /* delay to open the lightType */
                mdelay(pLightState->stLight_Ctl_Attr.delayOpenTimeMs);
                SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(E_SWITCH_STATE_ON, pLightState->stLight_Ctl_Attr.lightType,
                                                             pLightState->stLight_Ctl_Attr.lightIntensity, pLightState,
                                                             true);
            }
        }
    }
    printk("thread exit \n");
    return 0;
}

static int SSTAR_Light_Misc_Control_Thread_Creat(void)
{
    /* struct sched_param tSched = {0}; */

    /* tSched.sched_priority = 99; */
    phtask_main = kthread_run(main_work_thread, NULL, "Light_Misc_Control_thread");
    /* sched_setscheduler(phtask_main, SCHED_RR, &tSched); */
    printk("thread run %p\n", phtask_main);
    return 0;
}

static int SSTAR_Light_Misc_Control_Thread_Destory(void)
{
    if (phtask_main != NULL)
    {
        bModuleExit = true;
        bWaitResume = false;
        CamOsConditionWakeUpAll(&resume_waitqueue);

        kthread_stop(phtask_main);
        printk("thread stop %p\n", phtask_main);
        phtask_main = NULL;
    }
    return 0;
}
#endif

#define HR_TIMER_DELAY_MSEC(x) (ns_to_ktime(((u64)(x)) * 1000000U))
int SSTAR_Light_Misc_Control_Resume_Notify_Main(struct device *dev)
{
    if (pLightState && pLightState->bDeviceHasInited /*  && pLightState->reciveVifEventCnt == 0 */)
    {
        if (pLightState->stLight_Ctl_Attr.controlType == E_CONTROL_TYPE_MULTI_FRAME)
        {
#ifndef LIGHT_USE_HRTIMER
            bWaitResume = false;
            CamOsConditionWakeUpAll(&resume_waitqueue);
#else
            hrtimer_start(&pLightState->hrTimer, HR_TIMER_DELAY_MSEC(pLightState->stLight_Ctl_Attr.delayOpenTimeMs),
                          HRTIMER_MODE_REL_HARD);
#endif
        }
#ifdef LIGHT_SUPPORT_MULTI_VIFDEVICE
        SSTAR_Light_Misc_Control_VifDevIdList_Fun(E_MULTI_VIFDEV_FUN_MARKDOING, -1, true);
#endif

    }
    return 0;
}

int SSTAR_Light_Misc_Control_Init(void)
{
    if (!pLightState)
    {
        pLightState = CamOsMemAlloc(sizeof(SSTAR_Light_Device_State_t));
        if (!pLightState)
        {
            printk(KERN_WARNING "[%s] Init failed\n", __FUNCTION__);
            return -1;
        }
        memset(pLightState, 0x0, sizeof(SSTAR_Light_Device_State_t));

        CamOsSpinInit(&pLightState->switchStateSpinlock);
#ifdef LIGHT_SUPPORT_MULTI_VIFDEVICE
        CamOsSpinInit(&pLightState->vifDeviceDoneSpinlock);
        CAM_OS_INIT_LIST_HEAD(&pLightState->vifDeviceStateQueue);
#endif
    }
    printk(KERN_INFO "[%s] init success\n", __FUNCTION__);

    SSTAR_Light_Misc_Control_Hw_Resource_PowerOn();
    return 0;
}

int SSTAR_Light_Misc_Control_DeInit(void)
{
    SSTAR_Light_Misc_Control_IOCTL_DeInit(false);
    if (pLightState)
    {
        CamOsSpinDeinit(&pLightState->switchStateSpinlock);
#ifdef LIGHT_SUPPORT_MULTI_VIFDEVICE
        SSTAR_Light_Misc_Control_VifDevIdList_Fun(E_MULTI_VIFDEV_FUN_RELEASE, -1, false);
        CamOsSpinDeinit(&pLightState->vifDeviceDoneSpinlock);
#endif
        CamOsMemRelease(pLightState);
    }

    SSTAR_Light_Misc_Control_Hw_Resource_PowerOff();
    return 0;
}

int SSTAR_Light_Misc_Control_Resource_Config(struct platform_device *pdev)
{
    int ret;
    u32 nI2CMaster, irGpioId, irCutGpioId[2];
    if (!pdev)
    {
        return -EINVAL;
    }

    ret = of_property_read_u32(pdev->dev.of_node, "lightsensor-i2c", &nI2CMaster);
    if (ret)
    {
        return -EFAULT;
    }

    ret = of_property_read_u32(pdev->dev.of_node, "ir-pad", &irGpioId);
    if (ret)
    {
        return -EFAULT;
    }

    ret = of_property_read_u32_array(pdev->dev.of_node, "ircut-pad", irCutGpioId, 2);
    if (ret)
    {
        return -EFAULT;
    }
    printk(KERN_INFO "dts Resource i2c:%d ir:%d ircut[%d:%d]\n", nI2CMaster, irGpioId, irCutGpioId[0], irCutGpioId[1]);

    SSTAR_Light_Misc_Control_Hw_SetResource(nI2CMaster, irGpioId, -1, irCutGpioId);
    return 0;
}

int SSTAR_Light_Misc_Control_Resume(void)
{
#ifdef LIGHT_SUPPORT_LIGHTSENSOR_POWEROFF
    /* Although light sensor supporting the poweroff function will save power, each initialization will take tig_mode
     * time to read the valid value */
    SSTAR_Light_Misc_Control_Hw_Resource_LightSensor_Init();
#endif
    return 0;
}

int SSTAR_Light_Misc_Control_Suspend(void)
{
#ifdef LIGHT_SUPPORT_LIGHTSENSOR_POWEROFF
    SSTAR_Light_Misc_Control_Hw_Resource_LightSensor_DeInit();
#endif
    return 0;
}
