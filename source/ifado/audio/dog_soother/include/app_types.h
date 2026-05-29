#ifndef DOG_SOOTHER_APP_TYPES_H_
#define DOG_SOOTHER_APP_TYPES_H_

#include <stdint.h>

/* 工作状态，与 BLE_UART接口设计_V1.md / MCU app_ctrl 一致 */
typedef enum
{
    DS_WORK_OFF = 0,           /* （关机）未开启音频识别 */
    DS_WORK_MONITORING = 1,    /* 监测中，等待吠叫 */
    DS_WORK_IDENTIFYING = 2,   /* 识别/判定中 */
    DS_WORK_ACTING = 3,        /* 执行安抚措施 */
    DS_WORK_RESTING = 4,       /* 安抚后冷却 */
} ds_work_state_t;

/* 安抚模式 */
typedef enum
{
    DS_CALM_MODE_AUTO = 0,   /* 自动策略 */
    DS_CALM_MODE_MANUAL = 1, /* 手动指定措施 */
} ds_calm_mode_t;

/* UART 响应 status 字节，与协议文档一致 */
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

/* 安抚措施类型（策略序列中的 measure） */
#define DS_MEASURE_MUSIC 1
#define DS_MEASURE_OWNER_VOICE 2
#define DS_MEASURE_ULTRASONIC 3

/* 超声波档位 */
#define DS_US_25KHZ 1
#define DS_US_30KHZ 2
#define DS_US_DUAL 3

/* enabled_mask 位：音乐 / 主人录音 / 超声波 */
#define DS_ENABLED_MUSIC (1u << 0)
#define DS_ENABLED_OWNER (1u << 1)
#define DS_ENABLED_US (1u << 2)

#endif /* DOG_SOOTHER_APP_TYPES_H_ */
