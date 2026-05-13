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

#include "mi_sys.h"
#include "mi_sys_datatype.h"
#include "cam_os_wrapper.h"
#include "cam_fs_wrapper.h"
#include "sys_sys_isw_cli.h"
#include "mhal_earlyinit_para.h"
#include "cdc_acm.h"

#include "st_common.h"
#include "st_cdc_acm.h"

static char cdcHelpTxt[] = "cdc send test";
static char cdcUsageTxt[] = "cdc send test";

void cdc_acm_rev_thread(void *rbuf, u32 size)
{
    MI_U32 i = 0;
    MI_U8 *pRevBuf = rbuf;
    if(pRevBuf == NULL || size == 0)
    {
        //CamOsPrintf("Cdc acm receive data Length 0!\n");
        return;
    }

    CamOsPrintf("Cdc acm receive data Length: %d\n", size);
    for(i=0; i<size; i++)
    {
         CamOsPrintf("%02x ", pRevBuf[i]);
         if((i+1)%8 == 0) CamOsPrintf("\n");
    }
}

void cdc_acm_send()
{
    MI_U32 i = 0;
    MI_U8 ch = 'a';
    MI_U8 str[64] = {0};
    MI_U8 loop = 10;

    CamOsPrintf("\nStart to Output!\n");

    loop = 1;
    while(loop--)
    {
        memset(str, 0, sizeof(str));

        i = 0;
        ch = 'a';
        while(ch <= 'z')
        {
            str[i++] = ch++;
        }
        str[i++] = '\n';
        str[i++] = '\r';
        ST_Cdc_Acm_Send_Packet(str, i);

        i = 0;
        ch = 'A';
        while(ch <= 'Z')
        {
            str[i++] = ch++;
        }
        str[i++] = '\n';
        str[i++] = '\r';
        ST_Cdc_Acm_Send_Packet(str, i);

        CamOsMsSleep(1000);
    }


    CamOsPrintf("\nEnd of Output!\n");
}

void cdc_app_init(void)
{
    CamOsPrintf("cdc app start \n");
}

void cdc_app_deinit(void)
{
    CamOsPrintf("cdc app stop \n");
}

struct cdc_acm_user_ops cdc_acm_ops =
{
    .cdc_acm_data_out = cdc_acm_rev_thread,
};

static int cdc_send_cli(CLI_t *pCli, char *p)
{
    cdc_acm_send();
    return 0;
}
SS_RTOS_CLI_CMD(cdc_send_test, cdcHelpTxt, cdcUsageTxt, cdc_send_cli);

#endif  //#if defined(CONFIG_USB_GADGET_CDC_SUPPORT)
