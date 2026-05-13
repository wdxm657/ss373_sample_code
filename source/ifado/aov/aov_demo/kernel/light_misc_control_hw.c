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

#include <linux/module.h>
#include <drv_gpio_io.h>
#include <gpio.h>

#include "cam_os_condition.h"
#include "light_misc_control_datatype.h"

#include "light_misc_control_hw.h"
#include "light_misc_control_i2c.h"

int tig_mode = 0;
module_param(tig_mode, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(tig_mode, "light sensor tig mode 0: 100ms(default); 1: 400ms");

int ir_cut_select = 0;
module_param(ir_cut_select, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(ir_cut_select, "set ir cut pin sequence 0:ctrl 01(default); 1:ctrl 10");

SSTAR_Light_Device_Resource_t gDeviceResource = {0};

static void SSTAR_Light_Misc_Control_Hw_LightPower(bool bOnOff)
{
    u32 reg = 0x3, value = 0;
    value = bOnOff ? 0x4 : 0x6;
    printk(KERN_INFO "[%s] %d \n", __FUNCTION__, bOnOff);
    if (bOnOff == 1)
    {
        /* SNR_PERF_TIME(LastGetLuxTime); */
    }
    LightSensorReg_Write(gDeviceResource.u32I2cNum, reg, value);
}

static void SSTAR_Light_Misc_Control_Hw_TigMode(u32 value)
{
    u32 reg  = 0x4;
    u32 data = 0;
    switch (value)
    {
        case 0: // 100ms
            data = 0x25;
            break;
        case 1: // 400ms
            data = 0x94;
            break;
        case 2: // 688.5ms
            data = 0xFF;
            break;
        default:
            break;
    }
    LightSensorReg_Write(gDeviceResource.u32I2cNum, reg, data);
}

static void SSTAR_Light_Misc_Control_Hw_GetLux(s32* getLux)
{
    u16 chn0_l = 0, chn0_h = 0, chn1_l = 0, chn1_h = 0, chn0 = 0, chn1 = 0;
    u16 gain = 0, gain_l = 0, gain_h = 0, update = 0;
    s32 lux = ERR_HW_GETLUX_NOUPDATE;
    // u16 retrycnt = (tig_mode==0)?100:400;
    // u16 retrycnt = 200;
    /* unsigned long long  curtime = 0, difftime = 0; */

    // while(retrycnt > 0)
    {
        LightSensorReg_Read(gDeviceResource.u32I2cNum, 0x20, &update);
        if ((update & 0x01) != 0)
        {
            LightSensorReg_Read(gDeviceResource.u32I2cNum, 0x21, &chn0_l);
            LightSensorReg_Read(gDeviceResource.u32I2cNum, 0x22, &chn0_h);
            LightSensorReg_Read(gDeviceResource.u32I2cNum, 0x23, &chn1_l);
            LightSensorReg_Read(gDeviceResource.u32I2cNum, 0x24, &chn1_h);
            LightSensorReg_Read(gDeviceResource.u32I2cNum, 0x05, &gain);

            gain_l = gain & 0xF;
            gain_h = (gain & 0xF0) >> 4;
            chn0   = chn0_h << 8 | chn0_l;
            chn1   = chn1_h << 8 | chn1_l;

            lux = chn0 < 65535 ? (chn0 * 15 * 25 / gain_l - chn1 * 15 * 25 / gain_h) / 1000
                               : (chn0 * 15 * 25 / gain_l / 1000);

            if (lux < 0)
            {
                lux = ERR_HW_GETLUX_FAILED;
            }
            // break;
        }

        // retrycnt--;
        // CamOsMsDelay(1);
    }

    *getLux = lux;
}

int SSTAR_Light_Misc_Control_Hw_Resource_LightSensor_Init(void)
{
    SSTAR_Light_Misc_Control_Hw_LightPower(true);
    SSTAR_Light_Misc_Control_Hw_TigMode(tig_mode);
    return 0;
}
int SSTAR_Light_Misc_Control_Hw_Resource_LightSensor_DeInit(void)
{
    SSTAR_Light_Misc_Control_Hw_LightPower(false);
    return 0;
}

int SSTAR_Light_Misc_Control_Hw_Resource_PowerOn(void)
{
    SSTAR_Light_Misc_Control_Hw_Resource_LightSensor_Init();
    camdriver_gpio_request(NULL, gDeviceResource.u32IrGpioId);       // ir
    camdriver_gpio_request(NULL, gDeviceResource.u32IrCutGpioId[0]); // ircut
    camdriver_gpio_request(NULL, gDeviceResource.u32IrCutGpioId[1]); // ircut
    return 0;
}

int SSTAR_Light_Misc_Control_Hw_Resource_PowerOff(void)
{
    SSTAR_Light_Misc_Control_Hw_Resource_LightSensor_DeInit();
    camdriver_gpio_free(NULL, gDeviceResource.u32IrGpioId);
    camdriver_gpio_free(NULL, gDeviceResource.u32IrCutGpioId[0]);
    camdriver_gpio_free(NULL, gDeviceResource.u32IrCutGpioId[1]);
    return 0;
}

int SSTAR_Light_Misc_Control_Hw_Set_Ircut_State(SSTAR_Switch_State_e state, SSTAR_Light_Device_State_t* pLightState)
{
    printk(KERN_INFO "IRCUT turn to State:%d ,last IRCUT state %d\n", state, pLightState->eIRcutState);

    if (pLightState && pLightState->bDeviceHasInited)
    {
        /* In order to prevent ircut from being in the on or off state now and setting it to the same on or off state
         * again, since the stepper motor has already rotated to the limit before, it will heat up and there is a risk
         * of burning if it rotates in the same direction again. */
        if ((pLightState->eIRcutState & (~E_SWITCH_STATE_KEEP)) == state)
        {
            printk(KERN_INFO "eIRcutState had been set already\n");
        }
        else
        {
            if (state == E_SWITCH_STATE_ON)
            {
                if (ir_cut_select == 0)
                {
                    camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrCutGpioId[0], 0); // ir cut on
                    camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrCutGpioId[1], 1);
                }
                else if (ir_cut_select == 1)
                {
                    camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrCutGpioId[0], 1); // ir cut on
                    camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrCutGpioId[1], 0);
                }
            }
            else if (state == E_SWITCH_STATE_OFF)
            {
                if (ir_cut_select == 0)
                {
                    camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrCutGpioId[0], 1); // ir cut off
                    camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrCutGpioId[1], 0);
                }
                else if (ir_cut_select == 1)
                {
                    camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrCutGpioId[0], 0); // ir cut off
                    camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrCutGpioId[1], 1);
                }
            }
            else if (state == E_SWITCH_STATE_KEEP)
            {
                camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrCutGpioId[0], 0); // keep ircut state
                camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrCutGpioId[1], 0);
                /*  We use the fourth bit to record the keep status */
                pLightState->eIRcutState = state | pLightState->eIRcutState;
                return 0;
            }
            else
            {
                printk(KERN_ERR "[%s]eIRcutState:%d error\n", __FUNCTION__, pLightState->eIRcutState);
            }
            /* set IRcut state */
            pLightState->eIRcutState = state;
        }
    }
    return 0;
}

int SSTAR_Light_Misc_Control_Hw_Get_LightSensor(SSTAR_Light_Device_State_t* pLightState)
{
    int luxValue = ERR_HW_GETLUX_NOUPDATE;
    if (pLightState && pLightState->bDeviceHasInited)
    {
        /* try to read */
        SSTAR_Light_Misc_Control_Hw_GetLux(&luxValue);
        pLightState->LuxVaule = luxValue;
    }
    return luxValue;
}

SSTAR_Switch_State_e SSTAR_Light_Misc_Control_Hw_Get_LEDIR_Status(SSTAR_Light_Type_e          eLightType,
                                                                  SSTAR_Light_Device_State_t* pLight, bool bInIrq)
{
    SSTAR_Switch_State_e returnState = E_SWITCH_STATE_ERROR;
    if (!pLight)
    {
        printk(KERN_ERR "pLight is NULL\n");
        goto EXIT;
    }
    if (eLightType == E_LIGHT_TYPE_IR)
    {
        Light_MISC_SPINLOCK(bInIrq, &pLight->switchStateSpinlock);
        returnState = pLight->eIRState;
        Light_MISC_SPINUNLOCK(bInIrq, &pLight->switchStateSpinlock);
    }
    else if (eLightType == E_LIGHT_TYPE_LED)
    {
        Light_MISC_SPINLOCK(bInIrq, &pLight->switchStateSpinlock);
        returnState = pLight->eLEDState;
        Light_MISC_SPINUNLOCK(bInIrq, &pLight->switchStateSpinlock);
    }
    else
    {
        printk(KERN_ERR "[%s]eLightType:%d error\n", __FUNCTION__, eLightType);
    }
    return returnState;
EXIT:
    return E_SWITCH_STATE_ERROR;
}

int SSTAR_Light_Misc_Control_Hw_Set_LEDIR_Status(SSTAR_Switch_State_e openState, SSTAR_Light_Type_e eLightType,
                                                 unsigned int lightIntensity, SSTAR_Light_Device_State_t* pLight,
                                                 bool bInIrq)
{
    if (!bInIrq)
    {
        printk(KERN_INFO "turn eLightType:%d to State:%d ,last IR state %d, last LED state:%d\n", eLightType, openState,
               pLight->eIRState, pLight->eLEDState);
    }

    if (openState == E_SWITCH_STATE_OFF)
    {
        if (eLightType == E_LIGHT_TYPE_IR)
        {
            Light_MISC_SPINLOCK(bInIrq, &pLight->switchStateSpinlock);
            /* close IR */
            /* we call gpio_direction_output early than gpio driver resume, some register state must be not
             * ready, so we need config gpio in/out configuration every time when resume_noirq */
            camdriver_gpio_request(NULL, gDeviceResource.u32IrGpioId);
            camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrGpioId, 0);
            if (pLight)
            {
                pLight->eIRState = E_SWITCH_STATE_OFF;
            }
            Light_MISC_SPINUNLOCK(bInIrq, &pLight->switchStateSpinlock);
        }
        else if (eLightType == E_LIGHT_TYPE_LED)
        {
            Light_MISC_SPINLOCK(bInIrq, &pLight->switchStateSpinlock);

            /* close LED */
            if (pLight)
            {
                pLight->eLEDState = E_SWITCH_STATE_OFF;
            }
            Light_MISC_SPINUNLOCK(bInIrq, &pLight->switchStateSpinlock);
        }
    }
    else if (openState == E_SWITCH_STATE_ON)
    {
        if (eLightType == E_LIGHT_TYPE_IR)
        {
            Light_MISC_SPINLOCK(bInIrq, &pLight->switchStateSpinlock);
            /* open IR with lightIntensity*/

            camdriver_gpio_request(NULL, gDeviceResource.u32IrGpioId);
            camdriver_gpio_direction_output(NULL, gDeviceResource.u32IrGpioId, 1);
            if (pLight)
            {
                pLight->eIRState = E_SWITCH_STATE_ON;
            }
            Light_MISC_SPINUNLOCK(bInIrq, &pLight->switchStateSpinlock);
        }
        else if (eLightType == E_LIGHT_TYPE_LED)
        {
            Light_MISC_SPINLOCK(bInIrq, &pLight->switchStateSpinlock);
            /* open LED lightIntensity*/
            if (pLight)
            {
                pLight->eLEDState = E_SWITCH_STATE_ON;
            }
            Light_MISC_SPINUNLOCK(bInIrq, &pLight->switchStateSpinlock);
        }
    }
    else
    {
        printk(KERN_ERR "openState:%d error\n", openState);
    }

    return 0;
}

int SSTAR_Light_Misc_Control_Hw_SetResource(u32 u32I2c, u32 u32Ir, u32 u32Led, u32* pu32Ircut)
{
    gDeviceResource.u32I2cNum         = u32I2c;
    gDeviceResource.u32IrGpioId       = u32Ir;
    gDeviceResource.u32LedGpioId      = u32Led;
    gDeviceResource.u32IrCutGpioId[0] = pu32Ircut[0];
    gDeviceResource.u32IrCutGpioId[1] = pu32Ircut[1];

    return 0;
}
