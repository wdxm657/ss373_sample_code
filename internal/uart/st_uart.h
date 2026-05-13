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

#ifndef _LIBUART_H_
#define _LIBUART_H_

typedef struct
{
    MI_U32 baudrate;
    MI_U8  data;
    char   parity;
    MI_U8  stop;
    MI_U8  ctsrts;
    char *tty_device;
}stUartParm;



int ST_UART_INIT(stUartParm* param);
MI_S32 ST_Uart_WritePowerState(eUartPowerState ePowerState);
void ST_Uart_MemsetRecvBuf(void);
MI_S32 ST_Uart_WriteCMD(eUartCMD eUartCmd);
ST_WakeupType_e ST_Uart_GetWakeSource();
MI_S32 ST_Uart_SetWakeupSource(eUartWakeSrcType eWakeupSrc);

#endif
