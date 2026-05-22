#define LOG_TAG "comfort_store"
#include "log.h"

#include "comfort_store.h"
#include "app_config.h"
#include "audio.h"

#include <string.h>

typedef struct
{
    uint8_t power_on;
    ds_work_state_t work_state;
    uint8_t bt_linked;
    uint8_t owner_voice_exist;
    uint8_t owner_duration_sec;
    uint8_t volume;
    ds_calm_mode_t calm_mode;
    uint8_t enabled_mask;
    uint8_t us_mask;
} ds_runtime_t;

static ds_runtime_t g_rt = {
    .power_on = 1,
    .work_state = DS_WORK_MONITORING,
    .bt_linked = 0,
    .owner_voice_exist = 0,
    .owner_duration_sec = 0,
    .volume = 30,
    .calm_mode = DS_CALM_MODE_AUTO,
    .enabled_mask = DS_ENABLED_MUSIC | DS_ENABLED_US,
    .us_mask = 0x07,
};

int comfort_store_init(void)
{
    audio_refresh_owner_info(&g_rt.owner_voice_exist, &g_rt.owner_duration_sec);
    LOG_INFO("comfort_store init\n");
    return 0;
}

void comfort_store_deinit(void)
{
}

void comfort_store_fill_status_payload(uint8_t *out, uint16_t out_cap, uint16_t *out_len)
{
    if (!out || !out_len || out_cap < 9)
    {
        if (out_len)
        {
            *out_len = 0;
        }
        return;
    }

    audio_refresh_owner_info(&g_rt.owner_voice_exist, &g_rt.owner_duration_sec);

    out[0] = DS_UART_STATUS_OK;
    out[1] = g_rt.power_on;
    out[2] = (uint8_t)g_rt.work_state;
    out[3] = g_rt.bt_linked;
    out[4] = g_rt.owner_voice_exist;
    out[5] = g_rt.volume;
    out[6] = (uint8_t)g_rt.calm_mode;
    out[7] = g_rt.enabled_mask;
    out[8] = g_rt.us_mask;
    *out_len = 9;
}

void comfort_store_set_power(uint8_t on)
{
    g_rt.power_on = on ? 1 : 0;
}

uint8_t comfort_store_get_power(void)
{
    return g_rt.power_on;
}

void comfort_store_set_bt_linked(uint8_t linked)
{
    g_rt.bt_linked = linked ? 1 : 0;
}

ds_work_state_t comfort_store_get_work_state(void)
{
    return g_rt.work_state;
}

void comfort_store_set_work_state(ds_work_state_t state)
{
    g_rt.work_state = state;
}

uint16_t comfort_store_pull_records(
    const uint8_t *req,
    uint16_t req_len,
    uint8_t *rsp,
    uint16_t rsp_cap)
{
    (void)req;
    (void)req_len;
    if (!rsp || rsp_cap < 4)
    {
        return 0;
    }
    rsp[0] = DS_UART_STATUS_OK;
    rsp[1] = 0;
    rsp[2] = 0xFF;
    rsp[3] = 0;
    return 4;
}

uint16_t comfort_store_factory_reset(
    const uint8_t *req,
    uint16_t req_len,
    uint8_t *rsp,
    uint16_t rsp_cap)
{
    (void)req;
    (void)req_len;
    audio_delete_owner_rec();
    g_rt.owner_voice_exist = 0;
    g_rt.owner_duration_sec = 0;
    g_rt.calm_mode = DS_CALM_MODE_AUTO;
    g_rt.enabled_mask = DS_ENABLED_MUSIC | DS_ENABLED_US;
    g_rt.us_mask = 0x07;
    g_rt.work_state = DS_WORK_MONITORING;
    if (!rsp || rsp_cap < 1)
    {
        return 0;
    }
    rsp[0] = DS_UART_STATUS_OK;
    return 1;
}
