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

#include "linux/printk.h"
#include <cam_drv_i2c.h>

#include "light_misc_control_i2c.h"

int I2C_HACKING_I2CWriteRegisterPair(u16 nr, u16 slaveAddr, u16 reg, u16 value, ISP_I2C_FMT fmt)
{
    tI2cHandle tHandle;
    u8         data[4] = {0};

    CamI2cOpen(&tHandle, nr);

    switch (fmt)
    {
        case I2C_FMT_A8D8:
        {
            tI2cMsg msg;

            data[0] = reg & 0xff;
            data[1] = value & 0xff;

            msg.addr  = slaveAddr >> 1;
            msg.flags = 0;
            msg.buf   = data;
            msg.len   = 2;

            if (CamI2cTransfer(&tHandle, &msg, 1) < 0)
            {
                printk(KERN_INFO "[%s]%d i2c-%d timeout\n", __FUNCTION__, __LINE__, nr);
                CamI2cClose(&tHandle);
                return -1;
            }
        }
        break;
        default:
            break;
    }
    CamI2cClose(&tHandle);
    return 0;
}

int I2C_HACKING_I2CReadRegisterPair(u16 nr, u16 slaveAddr, u16 reg, u16 *val, ISP_I2C_FMT fmt)
{
    tI2cHandle tHandle;
    u8         data[4] = {0};

    CamI2cOpen(&tHandle, nr);

    switch (fmt)
    {
        case I2C_FMT_A8D8:
        {
            tI2cMsg msg[2];

            data[0] = reg & 0xff;

            msg[0].addr  = slaveAddr >> 1;
            msg[0].flags = 0;
            msg[0].buf   = data;
            msg[0].len   = 1;

            msg[1].addr  = slaveAddr >> 1;
            msg[1].flags = 1;
            msg[1].buf   = data;
            msg[1].len   = 1;

            if (CamI2cTransfer(&tHandle, msg, 2) < 0)
            {
                printk(KERN_INFO "[%s]%d i2c-%d timeout\n", __FUNCTION__, __LINE__, nr);
                CamI2cClose(&tHandle);
                return -1;
            }
            *val = data[0];
        }
        break;
        default:
            break;
    }

    CamI2cClose(&tHandle);

    return 0;
}
