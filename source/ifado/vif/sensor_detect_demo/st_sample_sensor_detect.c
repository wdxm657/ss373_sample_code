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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define SYSFS_I2C_DIR "/dev/i2c-"

/*! @brief Sensor master clock select */
typedef enum {
    ST_CMU_CLK_27MHZ,
    ST_CMU_CLK_21P6MHZ,
    ST_CMU_CLK_12MHZ,
    ST_CMU_CLK_5P4MHZ,
    ST_CMU_CLK_36MHZ,
    ST_CMU_CLK_54MHZ,
    ST_CMU_CLK_43P2MHZ,
    ST_CMU_CLK_61P7MHZ,
    ST_CMU_CLK_72MHZ,
    ST_CMU_CLK_48MHZ,
    ST_CMU_CLK_24MHZ,
    ST_CMU_CLK_37P125MHZ,
    ST_CMU_CLK_LPLL_DIV1,
    ST_CMU_CLK_LPLL_DIV2,
    ST_CMU_CLK_LPLL_DIV4,
    ST_CMU_CLK_LPLL_DIV8,
} ST_MCLK_FREQ; //Depends on chip.

/*! @brief Internal use for I2C API*/
typedef enum {
    I2C_FMT_A8D8, /**< 8 bits Address, 8 bits Data */
    I2C_FMT_A16D8,/**< 16 bits Address 8 bits Data */
    I2C_FMT_A8D16,/**< 8 bits Address 16 bits Data */
    I2C_FMT_A16D16,/**< 16 bits Address 16 bits Data */
    I2C_FMT_END/**< Reserved */
} ST_I2C_FMT;

static void gpio_request(unsigned int gpio, char * direction)
{
    char cmd[64];

    memset(cmd, 0x0, sizeof(char)*64);
    sprintf(cmd, "echo %d > %s/export", gpio, SYSFS_GPIO_DIR);
    system(cmd);

    memset(cmd, 0x0, sizeof(char)*64);
    sprintf(cmd, "echo %s > %s/gpio%d/direction", direction, SYSFS_GPIO_DIR, gpio);
    system(cmd);
}

static void gpio_disable(unsigned int gpio)
{
    char cmd[64];

    memset(cmd, 0x0, sizeof(char)*64);
    sprintf(cmd, "echo %d > %s/unexport", gpio, SYSFS_GPIO_DIR);
    system(cmd);
}

void gpio_write(unsigned int gpio, unsigned int value)
{
    char cmd[64];

    memset(cmd, 0x0, sizeof(char)*64);
    sprintf(cmd, "echo %d > %s/gpio%d/value", value, SYSFS_GPIO_DIR, gpio);
    system(cmd);
}

void mclk_onoff(unsigned int id, unsigned int type, unsigned int lane, unsigned int onoff, ST_MCLK_FREQ mclk)
{
    char cmd[64];

    /* [mclk] - sensor mclk configuration.
       Usage: mclk SensorPadId BusType LaneNum MclkOnOff MclkRate
       -SensorPadId:      Select sensor ID 0~3
       -BusType:          Select sensor bus type 0:Parallel, 1:MIPI
       -LaneNum:          Lane number, 2 or 4
       -Mclk OnOff:       1:on, 0:off
       -Mclk rate(Hz):    0:27M, 1:21.6M, 2:12M, 3:5.4M, 4:36M, 5:54M, 6:43.2M, 7:61.7M, 8:72M, 9:48M, 10:24M */
    memset(cmd, 0x0, sizeof(char)*64);
    sprintf(cmd, "echo mclk %u %u %u %u %u > /dev/sensorif", id , type, lane, onoff, mclk);
    system(cmd);
}

// salveAddr : 8 bit slave address
int WriteRegisterPair(int bus, int slave, short reg, unsigned short value, ST_I2C_FMT fmt)
{
    unsigned char data[4];
    char path[64];
    int file;

    sprintf(path, "%s%d", SYSFS_I2C_DIR, bus);
    file = open(path, O_RDWR);
    if (file < 0)
    {
        printf("i2c-%d open failed\n", bus);
        return 1;
    }

    if (ioctl(file, I2C_SLAVE_FORCE, slave) < 0)
    {
        printf("i2c-%d SetAddress failed\n", bus);
        goto failed;
    }

    memset(data, 0, sizeof(data));
    switch (fmt)
    {
        default:
        case I2C_FMT_A8D8:
            data[0] = reg & 0xff;
            data[1] = value & 0xff;
            if (write(file, data, 2) != 2)
            {
                goto failed;
            }
            break;
        case I2C_FMT_A16D8:
            data[0] = (reg >> 8) & 0xff;
            data[1] = reg & 0xff;
            data[2] = value & 0xff;
            if (write(file, data, 3) != 3)
            {
                goto failed;
            }
            break;
        case I2C_FMT_A8D16:
            data[0] = reg & 0xff;
            data[1] = (value >> 8) & 0xff;
            data[2] = (value)&0xff;
            if (write(file, data, 3) != 3)
            {
                goto failed;
            }
            break;
        case I2C_FMT_A16D16:
            data[0] = (reg >> 8) & 0xff;
            data[1] = (reg)&0xff;
            data[2] = (value >> 8) & 0xff;
            data[3] = (value)&0xff;
            if (write(file, data, 4) != 4)
            {
                goto failed;
            }
            break;
    }

    close(file);
    return 0;
failed:
    printf("i2c-%d fmt:%d write register failed\n", bus, fmt);
    close(file);
    return 1;
}

int ReadRegisterPair(int bus, int slave, unsigned int reg, unsigned short *val, ST_I2C_FMT fmt)
{
    unsigned char reg_addr[2];
    char path[64];
    int file;

    sprintf(path, "%s%d", SYSFS_I2C_DIR, bus);
    file = open(path, O_RDWR);
    if (file < 0)
    {
        printf("i2c-%d open failed\n", bus);
        return 1;
    }

    if (ioctl(file, I2C_SLAVE_FORCE, slave) < 0)
    {
        printf("i2c-%d SetAddress failed\n", bus);
        close(file);
        return 1;
    }

    memset(reg_addr, 0, sizeof(unsigned char));
    switch (fmt)
    {
        case I2C_FMT_A8D8:
            reg_addr[0] = reg & 0xff;
            if (write(file, reg_addr, 1) != 1)
            {
                goto set_failed;
            }
            if (read(file, val, 1) != 1)
            {
                goto read_failed;
            }

            break;
        case I2C_FMT_A16D8:
            reg_addr[0] = (reg >> 8) & 0xff;
            reg_addr[1] = reg & 0xff;
            if (write(file, reg_addr, 2) != 2)
            {
                goto set_failed;
            }
            if (read(file, val, 1) != 1)
            {
                goto read_failed;
            }
            break;
        case I2C_FMT_A8D16:
            reg_addr[0] = reg & 0xff;
            if (write(file, reg_addr, 1) != 1)
            {
                goto set_failed;
            }
            if (read(file, val, 2) != 2)
            {
                goto read_failed;
            }

            break;
        case I2C_FMT_A16D16:
            reg_addr[0] = (reg >> 8) & 0xff;
            reg_addr[1] = reg & 0xff;
            if (write(file, reg_addr, 2) != 2)
            {
                goto set_failed;
            }
            if (read(file, val, 2) != 2)
            {
                goto read_failed;
            }
            break;
        default:
            break;
    }

    close(file);
    return 0;
set_failed:
    printf("i2c-%d fmt:%d set register failed\n", bus, fmt);
    close(file);
    return 1;
read_failed:
    printf("i2c-%d fmt:%d read value failed\n", bus, fmt);
    close(file);
    return 1;
}

int imx415_detect()
{
    unsigned short sensor_id_h = 0, sensor_id_l = 0;
    unsigned int   sensor_id = 0;
    int result = 0;

    gpio_request(74, "out");
    gpio_write(74, 0); // rst gpio bank 103e offset 51 bit0
    usleep(40000);
    gpio_write(74, 1); // rst gpio
    usleep(1);
    mclk_onoff(0, 1, 2, true, ST_CMU_CLK_27MHZ);
    usleep(20);

    WriteRegisterPair(2, 0x1A, 0x3000, 0, I2C_FMT_A16D8);   // standby cancle
    usleep(100000);
    ReadRegisterPair(2, 0x1A, 0x3F12, &sensor_id_h, I2C_FMT_A16D8);
    ReadRegisterPair(2, 0x1A, 0x3F13, &sensor_id_l, I2C_FMT_A16D8);
    gpio_disable(74);

    sensor_id = ((sensor_id_h & 0xF) << 8) + (sensor_id_l & 0xFF);
    if (sensor_id == 0x415)
    {
        printf("snrpad0 detect sensor imx415 successfully.\n");
    }
    else
    {
        result = 1;
        printf("snrpad0 detect sensor imx415 fail, sensor_id = %d\n", sensor_id);
    }
    printf("-------------------------------------------------------------\n");

    gpio_request(76, "out");
    gpio_write(76, 0); // rst gpio bank 103e offset 53 bit0
    usleep(40000);
    gpio_write(76, 1); // rst gpio
    usleep(1);
    mclk_onoff(2, 1, 2, true, ST_CMU_CLK_27MHZ);
    usleep(20);

    WriteRegisterPair(1, 0x1A, 0x3000, 0, I2C_FMT_A16D8);   // standby cancle
    usleep(100000);
    ReadRegisterPair(1, 0x1A, 0x3F12, &sensor_id_h, I2C_FMT_A16D8);
    ReadRegisterPair(1, 0x1A, 0x3F13, &sensor_id_l, I2C_FMT_A16D8);
    gpio_disable(76);

    sensor_id = ((sensor_id_h & 0xF) << 8) + (sensor_id_l & 0xFF);
    if (sensor_id == 0x415)
    {
        printf("snrpad2 detect sensor imx415successfully.\n");
    }
    else
    {
        result = 1;
        printf("snrpad2 detect sensor imx415 fail, sensor_id = %d\n", sensor_id);
    }
    printf("-------------------------------------------------------------\n");

    return result;
}

int main(int argc, char **argv)
{
    // example on ifado 032a 2+2lane chip, gpio num and i2c bus please refer to dts
    // example on imx415
    imx415_detect();

    return 0;
}
