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

#include "cam_os_condition.h"
#include "light_misc_control_datatype.h"


int SSTAR_Light_Misc_Control_Set_DebugLog(bool bTurn);

int SSTAR_Light_Misc_Control_IOCTL_Init(int vifDevId, void ** pVifFrameDoneCallBack);
int SSTAR_Light_Misc_Control_IOCTL_DeInit(bool bFromIoctl);
int SSTAR_Light_Misc_Control_IOCTL_Set_Attr(SSTAR_Light_Ctl_Attr_t * attr);
int SSTAR_Light_Misc_Control_IOCTL_Get_Attr(SSTAR_Light_Ctl_Attr_t * attr);
int SSTAR_Light_Misc_Control_IOCTL_Set_Ircut_State(SSTAR_Switch_State_e state);
int SSTAR_Light_Misc_Control_IOCTL_Get_LightSensor(void);
int SSTAR_Light_Misc_Control_Debug_Get_State(char * buf);

int SSTAR_Light_Misc_Control_Resume_Notify_Main(struct device *dev);

int SSTAR_Light_Misc_Control_Init(void);
int SSTAR_Light_Misc_Control_DeInit(void);

int SSTAR_Light_Misc_Control_Resource_Config(struct platform_device *pdev);
int SSTAR_Light_Misc_Control_Resume(void);
int SSTAR_Light_Misc_Control_Suspend(void);
