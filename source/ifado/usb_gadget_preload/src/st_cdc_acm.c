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
/** @file
 */

#if defined(CONFIG_USB_GADGET_CDC_SUPPORT)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mi_common_datatype.h"
#include "cam_os_wrapper.h"

#include "st_cdc_acm.h"

extern void usbd_cdc_acm_init();
extern int sstar_cdc_acm_sent_packet(void *addr, u32 size);

MI_BOOL ST_Cdc_Acm_Send_Packet(MI_U8 *buf, MI_U32 size)
{
    int ret = 0;

    if(buf == NULL)
    {
        CamOsPrintf("[WARN] buf is NULL!\n");
        return FALSE;
    }

    ret = sstar_cdc_acm_sent_packet(buf, size);
    if (ret) {
        CamOsPrintf("[WARN] cdc acm sent failed <%d>\n", ret);
        return  FALSE;
    }

    return TRUE;
}

MI_BOOL ST_Cdc_Acm_Init(struct cdc_acm_user_ops *ops)
{
    usbd_cdc_acm_init(ops);  //Init cdc device
    return TRUE;
}

MI_BOOL ST_Cdc_Acm_DeInit(void)
{

    return TRUE;
}

#endif //#if defined(CONFIG_USB_GADGET_CDC_SUPPORT)

