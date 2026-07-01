/**
 * @file ultrasonic.c
 * @brief 超声波驱动占位（联调前仅日志）
 */
#define LOG_TAG "ultrasonic"
#include "log.h"

#include "ultrasonic.h"

#include "uart_proto.h"
#include "uart_cmd.h"

int ultrasonic_init(void)
{
    return 0;
}

void ultrasonic_deinit(void)
{
}

int ultrasonic_emit(uint8_t profile, unsigned int duration_ms)
{
    uint8_t payload[2];
    uint8_t dur_sec;

    dur_sec = (uint8_t)((duration_ms + 500) / 1000);  /* 毫秒取整到秒 */
    if (dur_sec < 1) dur_sec = 1;

    payload[0] = profile;
    payload[1] = dur_sec;

    LOG_INFO("ultrasonic emit: profile=%u duration=%us -> MCU\n", profile, dur_sec);
    uart_proto_send_evt(DS_CMD_ULTRA_EMIT, 0, payload, sizeof(payload));
    return 0;
}

int ultrasonic_stop(void)
{
    uint8_t payload[2] = {0, 0};

    LOG_INFO("ultrasonic stop -> MCU\n");
    uart_proto_send_evt(DS_CMD_ULTRA_EMIT, 0, payload, sizeof(payload));
    return 0;
}
