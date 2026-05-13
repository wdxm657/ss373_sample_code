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

#define LightSensorReg_Read(nr,_reg,_data)     (I2C_HACKING_I2CReadRegisterPair(nr, 0x20, _reg, _data, I2C_FMT_A8D8))
#define LightSensorReg_Write(nr,_reg,_data)    (I2C_HACKING_I2CWriteRegisterPair(nr, 0x20, _reg, _data, I2C_FMT_A8D8))

typedef enum {
    I2C_FMT_A8D8, /**< 8 bits Address, 8 bits Data */
    I2C_FMT_A16D8,/**< 16 bits Address 8 bits Data */
    I2C_FMT_A8D16,/**< 8 bits Address 16 bits Data */
    I2C_FMT_A16D16,/**< 16 bits Address 16 bits Data */
    I2C_FMT_END/**< Reserved */
} ISP_I2C_FMT;

int I2C_HACKING_I2CWriteRegisterPair(u16 nr, u16 slaveAddr, u16 reg, u16 value, ISP_I2C_FMT fmt);
int I2C_HACKING_I2CReadRegisterPair(u16 nr, u16 slaveAddr, u16 reg, u16 *val, ISP_I2C_FMT fmt);
