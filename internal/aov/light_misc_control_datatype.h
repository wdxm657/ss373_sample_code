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

#ifndef _LIGHT_MISC_CONTROL_DATATYPE_H_
#define _LIGHT_MISC_CONTROL_DATATYPE_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define IOCTL_LIGHT_MISC_CONTROL_INIT            _IOWR('k', 0, SSTAR_Light_Misc_Callback_Param_t)
#define IOCTL_LIGHT_MISC_CONTROL_DEINIT          _IO('k', 1)
#define IOCTL_LIGHT_MISC_CONTROL_SET_ATTR        _IOW('k', 2, SSTAR_Light_Ctl_Attr_t)
#define IOCTL_LIGHT_MISC_CONTROL_GET_ATTR        _IOR('k', 3, SSTAR_Light_Ctl_Attr_t)
#define IOCTL_LIGHT_MISC_CONTROL_SET_IRCUT       _IOW('k', 4, int)
#define IOCTL_LIGHT_MISC_CONTROL_GET_LIGHTSENSOR _IOR('k', 5, int)
#define IOCTL_LIGHT_MISC_CONTROL_GET_TIGMODE     _IOR('k', 6, int)

#define ERR_HW_GETLUX_FAILED   -0x0001
#define ERR_HW_GETLUX_NOUPDATE -0x0002

    typedef enum
    {
        E_SWITCH_STATE_OFF,
        E_SWITCH_STATE_ON,
        E_SWITCH_STATE_KEEP = 8,
        E_SWITCH_STATE_ERROR,
    } SSTAR_Switch_State_e;

    typedef enum
    {
        E_LIGHT_TYPE_IR,
        E_LIGHT_TYPE_LED,
        E_LIGHT_TYPE_MAX,
    } SSTAR_Light_Type_e;

    typedef enum
    {
        E_CONTROL_TYPE_LONG_TERM_OFF,
        E_CONTROL_TYPE_LONG_TERM_ON,
        E_CONTROL_TYPE_MULTI_FRAME,
        E_CONTROL_TYPE_MAX,
    } SSTAR_Control_Type_e;

    typedef struct SSTAR_Light_Ctl_Attr_s
    {
        SSTAR_Control_Type_e controlType;
        SSTAR_Light_Type_e   lightType;
        int                  delayOpenTimeMs;
        unsigned int         lightIntensity;
    } SSTAR_Light_Ctl_Attr_t;

    typedef struct SSTAR_Light_Misc_Callback_Param_s
    {
        int           vifDevId;
        unsigned long fun_ptr_addr;
    } SSTAR_Light_Misc_Callback_Param_t;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_LIGHT_MISC_CONTROL_DATATYPE_H_
