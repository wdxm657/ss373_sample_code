#define LOG_TAG "bark_control"
#include "log.h"

#include "bark_control.h"
#include "bark_detect.h"
#include "comfort_store.h"
#include "uart_proto.h"
#include "uart_cmd.h"
#include "app_types.h"

static uint8_t g_power_on = 1;

int bark_control_init(void)
{
    return bark_detect_init();
}

void bark_control_deinit(void)
{
    bark_detect_deinit();
}

void bark_control_set_power(uint8_t on)
{
    g_power_on = on ? 1 : 0;
    comfort_store_set_power(g_power_on);
    if (!g_power_on)
    {
        comfort_store_set_work_state(DS_WORK_OFF);
        bark_control_post_work_state(0);
    }
    else
    {
        comfort_store_set_work_state(DS_WORK_MONITORING);
        bark_control_post_work_state(0);
    }
}

void bark_control_post_work_state(uint8_t reason)
{
    uint8_t payload[2];
    payload[0] = (uint8_t)comfort_store_get_work_state();
    payload[1] = reason;
    if (uart_proto_send_evt(DS_EVT_WORK_STATE, 0, payload, sizeof(payload)) == 0)
    {
        LOG_INFO("EVT WORK_STATE work=%u reason=%u\n", payload[0], payload[1]);
    }
    else
    {
        LOG_ERROR("EVT WORK_STATE send failed\n");
    }
}

void bark_control_tick(void)
{
    if (!g_power_on)
    {
        return;
    }
    bark_detect_tick();
}

uint16_t bark_control_set_mode(
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap)
{
    if (!rsp || rsp_cap < 2 || !payload || payload_len < 1)
    {
        return 0;
    }
    rsp[0] = DS_UART_STATUS_OK;
    rsp[1] = payload[0];
    return 2;
}

uint16_t bark_control_handle_strategy_cmd(
    uint8_t cmd_id,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap)
{
    (void)payload;
    (void)payload_len;
    if (!rsp || rsp_cap < 1)
    {
        return 0;
    }
    if (cmd_id == DS_CMD_CALM_STRATEGY_GET)
    {
        rsp[0] = DS_UART_STATUS_OK;
        rsp[1] = DS_CALM_MODE_AUTO;
        rsp[2] = DS_ENABLED_MUSIC | DS_ENABLED_US;
        rsp[3] = 2;
        rsp[4] = DS_MEASURE_MUSIC;
        rsp[5] = DS_MEASURE_ULTRASONIC;
        rsp[6] = 3;
        rsp[7] = DS_US_25KHZ;
        rsp[8] = DS_US_30KHZ;
        rsp[9] = DS_US_DUAL;
        return 10;
    }
    rsp[0] = DS_UART_STATUS_OK;
    return 1;
}
