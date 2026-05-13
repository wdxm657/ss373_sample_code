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

#ifndef __USB_AC_FU_RANGE_H__
#define __USB_AC_FU_RANGE_H__


#define VOL_MIN_VAL (18)            //base to be subtracted
#define VOL_RATIO   (256)
#define VOL_MAX     (36*VOL_RATIO ) // soso@20140911
#define VOL_MIN     (18*VOL_RATIO)  // soso@20140924
#define VOL_DEF     (36*VOL_RATIO)  //soso@20140930
#define VOL_RES     (128)//(128*4)  // 2 db gap. ( ait analog gain limitation)

#define VOL_MUTE 0x8000
#define SAM_MAX 48000
#define SAM_MIN 16000
#define SAM_RES 8000
#define SAM_DEF 16000

#define MUTE_MAX 1
#define MUTE_MIN 0
#define MUTE_RES 1
#define MUTE_DEF 0

#endif //#ifndef __USB_AC_FU_RANGE_H__

