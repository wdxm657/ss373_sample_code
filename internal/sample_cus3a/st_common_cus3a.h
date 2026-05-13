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

#ifndef _ST_COMMON_CUS3A_H_
#define _ST_COMMON_CUS3A_H_

#ifdef __cplusplus
extern "C" {
#endif

void St_Common_EnableCUS3A(int IspdevId, int IspChnId, MI_BOOL bAEenable, MI_BOOL bAWBenable, MI_BOOL bAFenable);
void St_Common_ReleaseCUS3A(int IspdevId);

#ifdef __cplusplus
}
#endif

#endif

