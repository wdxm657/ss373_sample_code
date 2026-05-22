/**
 * @file system_time.c
 * @brief TIME_SET：epoch + 时区 quarter-hour 写 clock 与 TZ
 */
#define LOG_TAG "system_time"
#include "log.h"

#include "system_time.h"
#include "app_types.h"

#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

uint16_t system_time_apply(
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap)
{
    if (!rsp || rsp_cap < 1)
    {
        return 0;
    }
    if (!payload || payload_len < 5)
    {
        rsp[0] = DS_UART_STATUS_PARAM_ERROR;
        return 1;
    }

    /* payload: [0..3] epoch LE32, [4] tz quarter-hour (×15min) */
    uint32_t epoch = (uint32_t)payload[0]
        | ((uint32_t)payload[1] << 8)
        | ((uint32_t)payload[2] << 16)
        | ((uint32_t)payload[3] << 24);
    int8_t tz_q15 = (int8_t)payload[4];

    struct timespec ts;
    ts.tv_sec = (time_t)epoch;
    ts.tv_nsec = 0;
    if (clock_settime(CLOCK_REALTIME, &ts) != 0)
    {
        LOG_ERROR("clock_settime failed\n");
        rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
        return 1;
    }

    int tz_min = (int)tz_q15 * 15;
    int tz_h = tz_min / 60;
    int tz_m = tz_min % 60;
    if (tz_m < 0)
    {
        tz_m = -tz_m;
    }
    char tz_buf[32];
    snprintf(tz_buf, sizeof(tz_buf), "UTC%+d:%02d", -tz_h, tz_m);
    setenv("TZ", tz_buf, 1);
    tzset();

    LOG_INFO("time set epoch=%u tz=%s\n", epoch, tz_buf);
    rsp[0] = DS_UART_STATUS_OK;
    return 1;
}
