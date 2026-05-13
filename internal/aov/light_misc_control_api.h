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

#ifndef _LIGHT_MISC_CONTROL_H_
#define _LIGHT_MISC_CONTROL_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "light_misc_control_datatype.h"
#include "mi_vif_datatype.h"
#include "mi_vif.h"

    int Dev_Light_Misc_Device_GetFd(void);
    int Dev_Light_Misc_Device_CloseFd(int dev_fd);
    int Dev_Light_Misc_Device_Init(int dev_fd, int vifDevId);
    int Dev_Light_Misc_Device_DeInit(int dev_fd, int vifDevId);
    int Dev_Light_Misc_Device_Set_Attr(int dev_fd, SSTAR_Light_Ctl_Attr_t* pstAttr);
    int Dev_Light_Misc_Device_Get_Attr(int dev_fd, SSTAR_Light_Ctl_Attr_t* pstAttr);
    int Dev_Light_Misc_Device_Set_Ircut(int dev_fd, SSTAR_Switch_State_e eOpenState);
    int Dev_Light_Misc_Device_Get_LightSensor_Value(int dev_fd);
    int Dev_Light_Misc_Device_Get_TIGMode(int dev_fd);

    inline int Dev_Light_Misc_Device_GetFd(void)
    {
        int snr_aux_fd = open("/dev/light_misc", O_RDWR);
        if (snr_aux_fd < 0)
        {
            printf("open dev fail:%d\n", snr_aux_fd);
            return -1;
        }
        return snr_aux_fd;
    }

    inline int Dev_Light_Misc_Device_CloseFd(int dev_fd)
    {
        return close(dev_fd);
    }

    inline int Dev_Light_Misc_Device_Init(int dev_fd, int vifDevId)
    {
        SSTAR_Light_Misc_Callback_Param_t callback_param;

        if (0 > dev_fd)
        {
            printf("dev not open\n");
            return -1;
        }

        callback_param.vifDevId = vifDevId;
        if (ioctl(dev_fd, IOCTL_LIGHT_MISC_CONTROL_INIT, &callback_param) != 0)
        {
            printf("fail to execute ioctl init\n");
            return -1;
        }

        printf("callback addr:0x%lx\n", callback_param.fun_ptr_addr);

        if (callback_param.fun_ptr_addr == 0)
        {
            printf("empty call back\n");
            return -1;
        }

        MI_VIF_CustFunction(vifDevId, (MI_VIF_CustCmd_e)E_MI_VIF_CUSTCMD_FRAMEEND_NOTIFY, sizeof(unsigned long),
                            &callback_param.fun_ptr_addr);

        return 0;
    }

    inline int Dev_Light_Misc_Device_DeInit(int dev_fd, int vifDevId)
    {
        SSTAR_Light_Misc_Callback_Param_t callback_param = {0};

        if (0 > dev_fd)
        {
            printf("dev not open\n");
            return -1;
        }

        MI_VIF_CustFunction(vifDevId, (MI_VIF_CustCmd_e)E_MI_VIF_CUSTCMD_FRAMEEND_NOTIFY, sizeof(unsigned long),
                            &callback_param.fun_ptr_addr);

        if (ioctl(dev_fd, IOCTL_LIGHT_MISC_CONTROL_DEINIT) != 0)
        {
            printf("fail to execute ioctl deinit\n");
            return -1;
        }
        return 0;
    }

    inline int Dev_Light_Misc_Device_Set_Attr(int dev_fd, SSTAR_Light_Ctl_Attr_t* pstAttr)
    {
        if (0 > dev_fd)
        {
            printf("dev not open\n");
            return -1;
        }
        if (!pstAttr)
        {
            printf("pstAttr is NULL\n");
            return -1;
        }
        if (ioctl(dev_fd, IOCTL_LIGHT_MISC_CONTROL_SET_ATTR, pstAttr) != 0)
        {
            printf("fail to execute ioctl set attr\n");
            return -1;
        }

        return 0;
    }

    inline int Dev_Light_Misc_Device_Get_Attr(int dev_fd, SSTAR_Light_Ctl_Attr_t* pstAttr)
    {
        if (0 > dev_fd)
        {
            printf("dev not open\n");
            return -1;
        }
        if (!pstAttr)
        {
            printf("pstAttr is NULL\n");
            return -1;
        }
        if (ioctl(dev_fd, IOCTL_LIGHT_MISC_CONTROL_GET_ATTR, pstAttr) != 0)
        {
            printf("fail to execute ioctl get attr\n");
            return -1;
        }

        return 0;
    }

    inline int Dev_Light_Misc_Device_Set_Ircut(int dev_fd, SSTAR_Switch_State_e eOpenState)
    {
        int openState = eOpenState;
        if (0 > dev_fd)
        {
            printf("dev not open\n");
            return -1;
        }

        if (ioctl(dev_fd, IOCTL_LIGHT_MISC_CONTROL_SET_IRCUT, &openState) != 0)
        {
            printf("fail to execute ioctl set ircut\n");
            return -1;
        }
        return 0;
    }

    inline int Dev_Light_Misc_Device_Get_LightSensor_Value(int dev_fd)
    {
        int value = 0;
        if (0 > dev_fd)
        {
            printf("dev not open\n");
            return -1;
        }

        if (ioctl(dev_fd, IOCTL_LIGHT_MISC_CONTROL_GET_LIGHTSENSOR, &value) != 0)
        {
            printf("fail to execute ioctl get lightsensor\n");
        }
        return value;
    }

    inline int Dev_Light_Misc_Device_Get_TIGMode(int dev_fd)
    {
        int value = 0;
        if (0 > dev_fd)
        {
            printf("dev not open\n");
            return -1;
        }
        if (ioctl(dev_fd, IOCTL_LIGHT_MISC_CONTROL_GET_TIGMODE, &value) != 0)
        {
            printf("fail to execute ioctl get tigmode\n");
        }
        return value;
    }

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_LIGHT_MISC_CONTROL_H_
