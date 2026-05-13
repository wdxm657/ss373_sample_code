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

#include "linux/hrtimer.h"

#ifdef LIGHT_SUPPORT_MULTI_VIFDEVICE
typedef struct SSTAR_Light_Device_Multi_VifDevice_State_s
{
    int                         vifDevId;
    int                         deviceDoneState;
    struct CamOsListHead_t      vifdevice_list;
} SSTAR_Light_Device_Multi_VifDevice_State_t;
#endif

typedef struct SSTAR_Light_Device_State_s
{
    bool                   bDeviceHasInited;
    SSTAR_Light_Ctl_Attr_t stLight_Ctl_Attr;
    SSTAR_Switch_State_e   eIRcutState;
    SSTAR_Switch_State_e   eIRState;
    SSTAR_Switch_State_e   eLEDState;
    int                    reciveVifEventCnt;
    int                    doneFlagCnt;
    int                    openLightCnt;
    CamOsSpinlock_t        switchStateSpinlock;
    struct hrtimer         hrTimer;
    int                    LuxVaule;
    int                    bDebugLog;
#ifdef LIGHT_SUPPORT_MULTI_VIFDEVICE
    CamOsSpinlock_t        vifDeviceDoneSpinlock;
    struct CamOsListHead_t vifDeviceStateQueue;
#endif
} SSTAR_Light_Device_State_t;

typedef struct SSTAR_Light_Device_Resource_s
{
    u32 u32I2cNum;
    u32 u32IrGpioId;
    u32 u32LedGpioId;
    u32 u32IrCutGpioId[2];
} SSTAR_Light_Device_Resource_t;

#define Light_MISC_SPINLOCK(bInIrq, pstSpinlock) \
    do                                           \
    {                                            \
        if (bInIrq)                              \
        {                                        \
            CamOsSpinLock(pstSpinlock);          \
        }                                        \
        else                                     \
        {                                        \
            CamOsSpinLockIrqSave(pstSpinlock);   \
        }                                        \
    } while (0)

#define Light_MISC_SPINUNLOCK(bInIrq, pstSpinlock)  \
    do                                              \
    {                                               \
        if (bInIrq)                                 \
        {                                           \
            CamOsSpinUnlock(pstSpinlock);           \
        }                                           \
        else                                        \
        {                                           \
            CamOsSpinUnlockIrqRestore(pstSpinlock); \
        }                                           \
    } while (0)


int SSTAR_Light_Misc_Control_Hw_Resource_LightSensor_Init(void);
int SSTAR_Light_Misc_Control_Hw_Resource_LightSensor_DeInit(void);
int SSTAR_Light_Misc_Control_Hw_Resource_PowerOn(void);
int SSTAR_Light_Misc_Control_Hw_Resource_PowerOff(void);

int SSTAR_Light_Misc_Control_Hw_Set_Ircut_State(SSTAR_Switch_State_e state, SSTAR_Light_Device_State_t* pLightState);
int SSTAR_Light_Misc_Control_Hw_Get_LightSensor(SSTAR_Light_Device_State_t* pLightState);
SSTAR_Switch_State_e SSTAR_Light_Misc_Control_Hw_Get_LEDIR_Status(SSTAR_Light_Type_e          eLightType,
                                                                  SSTAR_Light_Device_State_t* pLight, bool bInIrq);
int SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(SSTAR_Switch_State_e openState, SSTAR_Light_Type_e eLightType,
                                                 unsigned int lightIntensity, SSTAR_Light_Device_State_t* pLight,
                                                 bool bInIrq);

int SSTAR_Light_Misc_Control_Hw_SetResource(u32 u32I2c, u32 u32Ir, u32 u32Led, u32 *pu32Ircut);
