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
#ifndef __LIGHT_SENSOR_H__
#define __LIGHT_SENSOR_H__

#include <stdio.h>

#define LIGHT_SENSOR_I2C_GROUP      (0)
#define LIGHT_SENSOR_I2C_SLAVE_ADDR (0x20)

#define LIGHT_SENSOR_SUCCESS            (0)
#define LIGHT_SENSOR_I2C_FAILURE        (-1)
#define LIGHT_SENSOR_GETLUX_FAILED      (-2)
#define LIGHT_SENSOR_GETLUX_NOUPDATE    (-3)

//Max check 200*5 = 1000 ms
#define LIGHT_SENSOR_CKS_DELAY_MS       (5)
#define LIGHT_SENSOR_CKS_RETYR_TIMES    (200)

typedef struct LightTable_s
{
    unsigned int lux;
    unsigned int u32Shutter;
    unsigned int u32Sensorgain;
    unsigned int u32Ispgain;
} LightTable_t;

typedef enum {
    I2C_FMT_A8D8, /**< 8 bits Address, 8 bits Data */
    I2C_FMT_A16D8,/**< 16 bits Address 8 bits Data */
    I2C_FMT_A8D16,/**< 8 bits Address 16 bits Data */
    I2C_FMT_A16D16,/**< 16 bits Address 16 bits Data */
    I2C_FMT_END/**< Reserved */
} ISP_I2C_FMT;

int Preload_LightSensorPowerOnOff(unsigned char bOnOff);
int Preload_LightSensorGetLux(LightTable_t *pLightParam);

#endif //__LIGHT_SENSOR_H__
