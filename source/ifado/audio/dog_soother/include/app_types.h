#ifndef DOG_SOOTHER_APP_TYPES_H_
#define DOG_SOOTHER_APP_TYPES_H_

#include <stdint.h>

/* 与 BLE_UART接口设计_V1.md / MCU app_ctrl 对齐 */
typedef enum
{
    DS_WORK_OFF = 0,
    DS_WORK_MONITORING = 1,
    DS_WORK_IDENTIFYING = 2,
    DS_WORK_ACTING = 3,
    DS_WORK_RESTING = 4,
} ds_work_state_t;

typedef enum
{
    DS_CALM_MODE_AUTO = 0,
    DS_CALM_MODE_MANUAL = 1,
} ds_calm_mode_t;

typedef enum
{
    DS_UART_STATUS_OK = 0x00,
    DS_UART_STATUS_PARAM_ERROR = 0x01,
    DS_UART_STATUS_BUSY = 0x02,
    DS_UART_STATUS_STATE_CONFLICT = 0x03,
    DS_UART_STATUS_TIMEOUT = 0x04,
    DS_UART_STATUS_NOT_FOUND = 0x05,
    DS_UART_STATUS_INTERNAL_ERROR = 0x06,
} ds_uart_status_t;

#define DS_MEASURE_MUSIC 1
#define DS_MEASURE_OWNER_VOICE 2
#define DS_MEASURE_ULTRASONIC 3

#define DS_US_25KHZ 1
#define DS_US_30KHZ 2
#define DS_US_DUAL 3

#define DS_ENABLED_MUSIC (1u << 0)
#define DS_ENABLED_OWNER (1u << 1)
#define DS_ENABLED_US (1u << 2)

#endif /* DOG_SOOTHER_APP_TYPES_H_ */
