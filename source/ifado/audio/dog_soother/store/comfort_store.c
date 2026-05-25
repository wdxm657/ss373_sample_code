/**
 * @file comfort_store.c
 * @brief 运行时状态与 STATUS_GET / 出厂重置 / 记录拉取（部分占位）
 */
#define LOG_TAG "comfort_store"
#include "log.h"

#include "comfort_store.h"
#include "calm_strategy.h"
#include "app_config.h"
#include "audio.h"

#include <string.h>

/* 内存态，对应 STATUS_GET payload[1..8] */
typedef struct
{
    uint8_t power_on;           /* out[1] */
    ds_work_state_t work_state; /* out[2] */
    uint8_t bt_linked;          /* out[3] */
    uint8_t owner_voice_exist;  /* out[4] */
    uint8_t owner_duration_sec; /* 缓存时长，文件存在时刷新 */
    uint8_t volume;             /* out[5]，与 audio 模块同步待完善 */
    ds_calm_mode_t calm_mode;   /* out[6] */
    uint8_t enabled_mask;       /* out[7] DS_ENABLED_* */
    uint8_t us_mask;            /* out[8] */
} ds_runtime_t;
ds_work_state_t last_work_state;

void comfort_store_apply_strategy(void)
{
    const calm_strategy_t *st = calm_strategy_get();
    uint8_t us_mask = 0;
    uint8_t i;

    if (!st)
    {
        return;
    }
    for (i = 0; i < st->us_cnt; i++)
    {
        uint8_t u = st->us_order[i];
        if (u >= 1 && u <= 3)
        {
            us_mask |= (uint8_t)(1u << (u - 1));
        }
    }
    comfort_store_set_calm_runtime(st->mode, st->enabled_mask, us_mask);
}

static ds_runtime_t g_rt = {
    .power_on = 0,
    .work_state = DS_WORK_OFF,
    .bt_linked = 0,
    .owner_voice_exist = 0,
    .owner_duration_sec = 0,
    .volume = 30,
    .calm_mode = DS_CALM_MODE_AUTO,
    .enabled_mask = DS_ENABLED_MUSIC | DS_ENABLED_US,
    .us_mask = 0x07,
};

int comfort_store_init(void) /* 从 owner.wav 刷新录音存在标志 */
{
    audio_refresh_owner_info(&g_rt.owner_voice_exist, &g_rt.owner_duration_sec);
    LOG_INFO("comfort_store init\n");
    return 0;
}

void comfort_store_deinit(void) /* 无持久化，空实现 */
{
}

void comfort_store_fill_status_payload(uint8_t *out, uint16_t out_cap, uint16_t *out_len)
{
    /* out[0]=status, [1]power [2]work [3]bt [4]owner_exist [5]vol [6]mode [7]enabled [8]us */
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

void comfort_store_set_power(uint8_t on) /* POWER_CTRL 写入 */
{
    g_rt.power_on = on ? 1 : 0;
}

uint8_t comfort_store_get_power(void)
{
    return g_rt.power_on;
}

void comfort_store_set_bt_linked(uint8_t linked) /* BT_LINK_NOTIFY */
{
    g_rt.bt_linked = linked ? 1 : 0;
}

ds_work_state_t comfort_store_get_work_state(void)
{
    if (g_rt.work_state != last_work_state)
    {
        LOG_DEBUG("work_state changed from %d to %d\n", last_work_state, g_rt.work_state);
        last_work_state = g_rt.work_state;
    }

    return g_rt.work_state;
}

void comfort_store_set_work_state(ds_work_state_t state) /* bark_control 状态机更新 */
{
    g_rt.work_state = state;
}

void comfort_store_set_calm_runtime(ds_calm_mode_t mode, uint8_t enabled_mask, uint8_t us_mask)
{
    g_rt.calm_mode = mode;
    g_rt.enabled_mask = enabled_mask;
    g_rt.us_mask = us_mask;
}

uint16_t comfort_store_pull_records(/* 0x41 占位：rsp 固定 OK+空记录 */
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

uint16_t comfort_store_factory_reset(/* 删主人录音并恢复默认策略字段 */
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
    calm_strategy_factory_reset();
    comfort_store_apply_strategy();
    g_rt.work_state = DS_WORK_MONITORING;
    if (!rsp || rsp_cap < 1)
    {
        return 0;
    }
    rsp[0] = DS_UART_STATUS_OK;
    return 1;
}
