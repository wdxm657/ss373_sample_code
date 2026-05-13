#ifndef __USER_INIT_H__
#define __USER_INIT_H__

#include "main.h"

#define DEBUG_EN 0

#if (DEBUG_EN == 1)
#include <stdio.h>
#endif

#define AOV_SYS_PWR_CTR_GPIO_GROUP      GPIOA
#define AOV_SYS_PWR_CTR_GPIO_PIN        FL_GPIO_PIN_5

#define AOV_SRAM_PWR_CTR_GPIO_GROUP     GPIOA
#define AOV_SRAM_PWR_CTR_GPIO_PIN       FL_GPIO_PIN_6

#define AOV_POWER_OFF_TIME_HIGH_MS      1000
#define AOV_POWER_OFF_TIME_LOW_MS       1000
#define AOV_POWER_OFF_TIME_WIFI_MS      5000

#define AOV_SUSPEND_GPIO_GROUP          GPIOA
#define AOV_SUSPEND_GPIO_PIN            FL_GPIO_PIN_15
#define AOV_SUSPEND_EXTI_LINE           FL_GPIO_EXTI_LINE_3
#define AOV_SUSPEND_EXTI_LINE_PIN       FL_GPIO_EXTI_LINE_3_PA15
#define AOV_SUSPEND_EXTI_TRIGGER        FL_GPIO_EXTI_TRIGGER_EDGE_FALLING

#define AOV_UART_RX_GPIO_GROUP          GPIOA
#define AOV_UART_RX_GPIO_PIN            FL_GPIO_PIN_14

#define AOV_UART_TX_GPIO_GROUP          GPIOA
#define AOV_UART_TX_GPIO_PIN            FL_GPIO_PIN_13

#define AOV_UART_NUMBER                 UART0
#define AOV_UART_INTERRUPT_NUMBER       UART0_IRQn

#define AOV_UART_BAUDRATE               9600
#define AOV_UART_DATA_BIT               FL_UART_DATA_WIDTH_8B
#define AOV_UART_PARITY                 FL_UART_PARITY_NONE
#define AOV_UART_STOP                   FL_UART_STOP_BIT_WIDTH_1B
#define AOV_UART_DIRECTION_TX_RX        FL_UART_DIRECTION_TX_RX

#define AOV_MAX_UART_CMD_LEN            (0x08)

#define AOV_UART_SET_STR_PASSWORD_CMD_LEN   (4)
#define AOV_UART_SET_STR_PASSWORD_CMD_HEAD  's'

#define AOV_UART_GET_STR_PASSWORD_CMD_LEN   (2)
#define AOV_UART_GET_STR_PASSWORD_CMD_HEAD  'g'

#define UART_PD_CMD_SEQ_HEAD            (0x7B)
#define UART_PD_CMD_SEQ_LEN             (0x04)
#define UART_PD_CMD_SEQ_CMD_TYPE        (0x80)
#define UART_PD_CMD_SEQ_DATA_CHECKSUM   (0x00)  //Data check sum is 0 because there is no data in command sequnce

#define AOV_POWER_LEVEL_MASK            (0x03)
#define AOV_POWER_LEVEL_BIT_IS_VALID    (0x01)
#define AOV_POWER_LEVEL_HIGH            (0x02)

#define AOV_STR_PASSWORD_HIGH           (0xCB)
#define AOV_STR_PASSWORD_LOW            (0xA0)
#define AOV_STR_PASSWORD_LOW_MASK       (0xF0)

void UserInit(void);
void FoutInit(void);
void DelayUs(uint32_t count);
void DelayMs(uint32_t count);
void AOV_GPIO_Init(void);
void AOV_RESET_SYS_SRAM_POWER(void);
void AOV_I2C_Init(void);
void AOV_UART_Init(void);

uint8_t AOV_I2C_P12To42_Enable(void);
void Uart0_4_RxTx(void);
void SOC_Monitor(void);

#endif
