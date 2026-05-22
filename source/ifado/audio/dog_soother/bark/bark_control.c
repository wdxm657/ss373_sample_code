/**
 * @file bark_control.c
 * @brief 监听(10s/3次) → 单步安抚 → 复听 → 休息；策略见 calm_strategy
 */
#define LOG_TAG "bark_control"
#include "log.h"

#include "bark_control.h"
#include "bark_detect.h"
#include "calm_strategy.h"
#include "comfort_store.h"
#include "uart_proto.h"
#include "uart_cmd.h"
#include "app_types.h"
#include "app_config.h"

#include "audio.h"
#include "audio_ao.h"
#include "ultrasonic.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DS_EVT_SESSION_START DS_EVT_BARK_DETECTED

#define SESS_ACTIONS_MAX 8

typedef enum
{
    BARK_LISTEN_IDLE = 0,
    BARK_LISTEN_WINDOW,
    BARK_LISTEN_POST_MEASURE,
} bark_listen_phase_t;

typedef struct
{
    uint8_t measure;
    uint8_t us_profile;
} sess_action_t;

typedef struct
{
    uint8_t in_session;
    uint32_t session_id;
    uint32_t session_bark_ts;

    bark_listen_phase_t listen_phase;
    int win_count;
    time_t win_deadline;
    time_t measure_end_ts;

    sess_action_t actions[SESS_ACTIONS_MAX];
    uint8_t action_cnt;
    uint8_t action_idx;
    uint8_t acting_busy;
    uint8_t last_ok_measure;
    uint8_t last_ok_us_sub;
    time_t us_emit_end;

    time_t rest_deadline;
} bark_fsm_t;

static uint8_t g_power_on = 1;
static bark_fsm_t g_fsm;
static pthread_mutex_t g_fsm_mu = PTHREAD_MUTEX_INITIALIZER;
static uint32_t g_next_session_id = 1;

static int bark_measure_enabled(uint8_t measure, uint8_t enabled_mask)
{
    if (measure == DS_MEASURE_MUSIC)
    {
        return (enabled_mask & DS_ENABLED_MUSIC) != 0;
    }
    if (measure == DS_MEASURE_OWNER_VOICE)
    {
        return (enabled_mask & DS_ENABLED_OWNER) != 0;
    }
    if (measure == DS_MEASURE_ULTRASONIC)
    {
        return (enabled_mask & DS_ENABLED_US) != 0;
    }
    return 0;
}

static int bark_owner_exists(void)
{
    uint8_t exist = 0;
    uint8_t dur = 0;

    audio_refresh_owner_info(&exist, &dur);
    return (exist && dur > 0) ? 1 : 0;
}

static void bark_sync_store_from_strategy(void)
{
    comfort_store_apply_strategy();
}

static void bark_listen_reset_locked(void)
{
    g_fsm.listen_phase = BARK_LISTEN_IDLE;
    g_fsm.win_count = 0;
    g_fsm.win_deadline = 0;
    g_fsm.measure_end_ts = 0;
}

static void bark_set_work_state(ds_work_state_t st, uint8_t reason)
{
    if (comfort_store_get_work_state() == st)
    {
        return;
    }
    comfort_store_set_work_state(st);
    bark_control_post_work_state(reason);
}

static void bark_post_session_start_evt(uint32_t session_id, uint32_t bark_ts)
{
    uint8_t payload[8];

    payload[0] = (uint8_t)(session_id & 0xFF);
    payload[1] = (uint8_t)((session_id >> 8) & 0xFF);
    payload[2] = (uint8_t)((session_id >> 16) & 0xFF);
    payload[3] = (uint8_t)((session_id >> 24) & 0xFF);
    payload[4] = (uint8_t)(bark_ts & 0xFF);
    payload[5] = (uint8_t)((bark_ts >> 8) & 0xFF);
    payload[6] = (uint8_t)((bark_ts >> 16) & 0xFF);
    payload[7] = (uint8_t)((bark_ts >> 24) & 0xFF);
    uart_proto_send_evt(DS_EVT_SESSION_START, 0, payload, sizeof(payload));
    LOG_INFO("EVT session start id=%u bark_ts=%u\n", session_id, bark_ts);
}

static void bark_post_measure_evt(uint32_t session_id, uint8_t step, uint8_t measure, uint8_t sub)
{
    uint8_t payload[11];
    uint32_t ts = (uint32_t)time(NULL);

    payload[0] = (uint8_t)(session_id & 0xFF);
    payload[1] = (uint8_t)((session_id >> 8) & 0xFF);
    payload[2] = (uint8_t)((session_id >> 16) & 0xFF);
    payload[3] = (uint8_t)((session_id >> 24) & 0xFF);
    payload[4] = step;
    payload[5] = measure;
    payload[6] = sub;
    payload[7] = (uint8_t)(ts & 0xFF);
    payload[8] = (uint8_t)((ts >> 8) & 0xFF);
    payload[9] = (uint8_t)((ts >> 16) & 0xFF);
    payload[10] = (uint8_t)((ts >> 24) & 0xFF);
    uart_proto_send_evt(DS_EVT_MEASURE_EXEC, 0, payload, sizeof(payload));
    LOG_INFO("EVT measure step=%u type=%u sub=%u\n", step, measure, sub);
}

static void bark_post_session_result_evt(uint32_t session_id, uint8_t result, uint8_t ok_measure, uint8_t ok_sub)
{
    uint8_t payload[11];
    uint32_t end_ts = (uint32_t)time(NULL);

    payload[0] = (uint8_t)(session_id & 0xFF);
    payload[1] = (uint8_t)((session_id >> 8) & 0xFF);
    payload[2] = (uint8_t)((session_id >> 16) & 0xFF);
    payload[3] = (uint8_t)((session_id >> 24) & 0xFF);
    payload[4] = result;
    payload[5] = (uint8_t)(end_ts & 0xFF);
    payload[6] = (uint8_t)((end_ts >> 8) & 0xFF);
    payload[7] = (uint8_t)((end_ts >> 16) & 0xFF);
    payload[8] = (uint8_t)((end_ts >> 24) & 0xFF);
    payload[9] = ok_measure;
    payload[10] = ok_sub;
    uart_proto_send_evt(DS_EVT_SESSION_RESULT, 0, payload, sizeof(payload));
    LOG_INFO("EVT session result id=%u result=%u\n", session_id, result);
}

static void bark_append_us_actions_locked(const calm_strategy_t *st, uint8_t *n)
{
    uint8_t j;

    for (j = 0; j < st->us_cnt && *n < SESS_ACTIONS_MAX; j++)
    {
        g_fsm.actions[*n].measure = DS_MEASURE_ULTRASONIC;
        g_fsm.actions[*n].us_profile = st->us_order[j];
        (*n)++;
    }
}

static void bark_build_session_actions_locked(void)
{
    const calm_strategy_t *st = calm_strategy_get();
    uint8_t n = 0;
    uint8_t i;
    uint8_t us_substituted = 0;
    int owner_ok;

    memset(g_fsm.actions, 0, sizeof(g_fsm.actions));
    g_fsm.action_cnt = 0;
    if (!st)
    {
        return;
    }

    owner_ok = bark_owner_exists();
    for (i = 0; i < st->measure_cnt && n < SESS_ACTIONS_MAX; i++)
    {
        uint8_t m = st->measure_order[i];

        if (!bark_measure_enabled(m, st->enabled_mask))
        {
            continue;
        }
        if (m == DS_MEASURE_MUSIC)
        {
            g_fsm.actions[n].measure = DS_MEASURE_MUSIC;
            g_fsm.actions[n].us_profile = 0;
            n++;
        }
        else if (m == DS_MEASURE_OWNER_VOICE)
        {
            if (owner_ok)
            {
                g_fsm.actions[n].measure = DS_MEASURE_OWNER_VOICE;
                g_fsm.actions[n].us_profile = 0;
                n++;
            }
            else
            {
                bark_append_us_actions_locked(st, &n);
                us_substituted = 1;
            }
        }
        else if (m == DS_MEASURE_ULTRASONIC)
        {
            if (!us_substituted)
            {
                bark_append_us_actions_locked(st, &n);
            }
        }
    }
    g_fsm.action_cnt = n;
    LOG_INFO("session actions built cnt=%u owner_ok=%d\n", n, owner_ok);
}

static void bark_enter_rest_locked(uint8_t success)
{
    uint8_t ok_m = g_fsm.last_ok_measure;
    uint8_t ok_s = g_fsm.last_ok_us_sub;

    if (success)
    {
        calm_strategy_apply_success(ok_m, ok_s);
        bark_sync_store_from_strategy();
    }

    bark_post_session_result_evt(g_fsm.session_id, success ? 1u : 0u, ok_m, ok_s);
    bark_set_work_state(DS_WORK_RESTING, success ? 3u : 4u);
    bark_detect_set_active(0);
    g_fsm.in_session = 0;
    g_fsm.acting_busy = 0;
    g_fsm.rest_deadline = time(NULL) + DS_CALM_REST_SEC;
    bark_listen_reset_locked();
    LOG_INFO("session %u end success=%u rest %ds\n", g_fsm.session_id, success, DS_CALM_REST_SEC);
}

static void bark_start_session_locked(uint32_t bark_ts)
{
    g_fsm.in_session = 1;
    g_fsm.session_id = g_next_session_id++;
    g_fsm.session_bark_ts = bark_ts;
    g_fsm.action_idx = 0;
    g_fsm.last_ok_measure = 0;
    g_fsm.last_ok_us_sub = 0;
    bark_build_session_actions_locked();
    bark_post_session_start_evt(g_fsm.session_id, bark_ts);
    LOG_INFO("calm session %u started\n", g_fsm.session_id);
}

static void bark_begin_action_locked(void)
{
    const sess_action_t *act;

    if (g_fsm.action_idx >= g_fsm.action_cnt)
    {
        bark_enter_rest_locked(0);
        return;
    }

    act = &g_fsm.actions[g_fsm.action_idx];
    g_fsm.last_ok_measure = act->measure;
    g_fsm.last_ok_us_sub = act->us_profile;
    g_fsm.acting_busy = 1;
    bark_set_work_state(DS_WORK_ACTING, 2);
    bark_detect_set_active(0);
    bark_post_measure_evt(g_fsm.session_id, g_fsm.action_idx, act->measure, act->us_profile);

    switch (act->measure)
    {
    case DS_MEASURE_MUSIC:
        if (audio_ao_play_wav_file(DS_CALM_MUSIC_PATH) != 0)
        {
            LOG_ERROR("calm music play failed: %s\n", DS_CALM_MUSIC_PATH);
            g_fsm.acting_busy = 0;
        }
        else
        {
            LOG_INFO("calm music play %s\n", DS_CALM_MUSIC_PATH);
        }
        break;
    case DS_MEASURE_OWNER_VOICE:
        if (audio_ao_play_wav_file(DS_OWNER_PCM_PATH) != 0)
        {
            LOG_ERROR("owner play failed\n");
            g_fsm.acting_busy = 0;
        }
        else
        {
            LOG_INFO("owner play start\n");
        }
        break;
    case DS_MEASURE_ULTRASONIC:
        g_fsm.us_emit_end = time(NULL) + DS_CALM_US_EMIT_SEC;
        ultrasonic_emit(act->us_profile, (unsigned int)(DS_CALM_US_EMIT_SEC * 1000));
        LOG_INFO("ultrasonic profile=%u %ds\n", act->us_profile, DS_CALM_US_EMIT_SEC);
        break;
    default:
        g_fsm.acting_busy = 0;
        break;
    }
}

static void bark_after_measure_done_locked(void)
{
    time_t now = time(NULL);

    g_fsm.acting_busy = 0;
    g_fsm.action_idx++;
    bark_set_work_state(DS_WORK_MONITORING, 6);
    bark_detect_set_active(1);
    g_fsm.listen_phase = BARK_LISTEN_POST_MEASURE;
    g_fsm.win_count = 0;
    g_fsm.win_deadline = 0;
    g_fsm.measure_end_ts = now;
    LOG_INFO("measure done, post-listen action_idx=%u/%u\n",
             g_fsm.action_idx,
             g_fsm.action_cnt);
}

static void bark_measure_poll_locked(void)
{
    const sess_action_t *act;
    time_t now = time(NULL);

    if (!g_fsm.acting_busy)
    {
        return;
    }

    act = &g_fsm.actions[g_fsm.action_idx];
    if (act->measure == DS_MEASURE_MUSIC || act->measure == DS_MEASURE_OWNER_VOICE)
    {
        if (audio_ao_is_playing())
        {
            return;
        }
        bark_after_measure_done_locked();
        return;
    }

    if (act->measure == DS_MEASURE_ULTRASONIC)
    {
        if (now < g_fsm.us_emit_end)
        {
            return;
        }
        bark_after_measure_done_locked();
    }
}

static void bark_on_window_hit_locked(uint32_t epoch_sec)
{
    time_t now = (time_t)epoch_sec;

    if (comfort_store_get_work_state() == DS_WORK_RESTING)
    {
        return;
    }
    if (comfort_store_get_work_state() == DS_WORK_ACTING)
    {
        return;
    }

    if (g_fsm.listen_phase == BARK_LISTEN_IDLE)
    {
        g_fsm.listen_phase = BARK_LISTEN_WINDOW;
        g_fsm.win_count = 1;
        g_fsm.win_deadline = now + DS_MONITOR_WINDOW_SEC;
        LOG_INFO("bark window start deadline=%ld\n", (long)g_fsm.win_deadline);
        return;
    }

    if (g_fsm.listen_phase != BARK_LISTEN_WINDOW &&
        g_fsm.listen_phase != BARK_LISTEN_POST_MEASURE)
    {
        return;
    }

    if (now > g_fsm.win_deadline)
    {
        g_fsm.listen_phase = BARK_LISTEN_WINDOW;
        g_fsm.win_count = 1;
        g_fsm.win_deadline = now + DS_MONITOR_WINDOW_SEC;
        LOG_INFO("bark window restarted\n");
        return;
    }

    g_fsm.win_count++;
    LOG_INFO("bark window count=%d\n", g_fsm.win_count);

    if (g_fsm.win_count < DS_MONITOR_BARK_TRIGGER)
    {
        return;
    }

    if (!g_fsm.in_session)
    {
        bark_start_session_locked(epoch_sec);
        bark_begin_action_locked();
        bark_listen_reset_locked();
        return;
    }

    if (g_fsm.listen_phase == BARK_LISTEN_POST_MEASURE)
    {
        if (g_fsm.action_idx < g_fsm.action_cnt)
        {
            bark_begin_action_locked();
        }
        else
        {
            bark_enter_rest_locked(0);
        }
        bark_listen_reset_locked();
    }
}

static void bark_tick_listen_locked(void)
{
    time_t now = time(NULL);

    if (g_fsm.listen_phase == BARK_LISTEN_POST_MEASURE)
    {
        if (g_fsm.win_count == 0 && g_fsm.measure_end_ts > 0 &&
            now >= g_fsm.measure_end_ts + DS_MONITOR_WINDOW_SEC)
        {
            LOG_INFO("post-measure quiet success\n");
            bark_enter_rest_locked(1);
            return;
        }
        if (g_fsm.win_count > 0 && g_fsm.win_deadline > 0 && now >= g_fsm.win_deadline &&
            g_fsm.win_count < DS_MONITOR_BARK_TRIGGER)
        {
            LOG_INFO("post-measure window end count=%d success\n", g_fsm.win_count);
            bark_enter_rest_locked(1);
            return;
        }
    }

    if (g_fsm.listen_phase == BARK_LISTEN_WINDOW && g_fsm.win_deadline > 0 &&
        now >= g_fsm.win_deadline)
    {
        if (!g_fsm.in_session && g_fsm.win_count < DS_MONITOR_BARK_TRIGGER)
        {
            LOG_INFO("monitor window expired count=%d\n", g_fsm.win_count);
            bark_listen_reset_locked();
        }
    }
}

static void bark_tick_acting_locked(void)
{
    bark_measure_poll_locked();
    /* 播放/超声启动失败时 acting_busy=0，补一次收尾 */
    if (!g_fsm.acting_busy && g_fsm.in_session &&
        comfort_store_get_work_state() == DS_WORK_ACTING)
    {
        bark_after_measure_done_locked();
    }
}

static void bark_tick_resting_locked(void)
{
    if (time(NULL) >= g_fsm.rest_deadline)
    {
        bark_set_work_state(DS_WORK_MONITORING, 5);
        bark_detect_set_active(1);
        bark_listen_reset_locked();
        LOG_INFO("rest done, back to monitoring\n");
    }
}

int bark_control_init(void)
{
    if (calm_strategy_init() != 0)
    {
        return -1;
    }
    bark_sync_store_from_strategy();
    memset(&g_fsm, 0, sizeof(g_fsm));
    bark_listen_reset_locked();
    return bark_detect_init();
}

void bark_control_deinit(void)
{
    bark_detect_deinit();
    calm_strategy_deinit();
}

void bark_control_set_power(uint8_t on)
{
    g_power_on = on ? 1 : 0;
    comfort_store_set_power(g_power_on);
    pthread_mutex_lock(&g_fsm_mu);
    if (!g_power_on)
    {
        bark_detect_set_active(0);
        bark_set_work_state(DS_WORK_OFF, 0);
        g_fsm.in_session = 0;
        g_fsm.acting_busy = 0;
    }
    else
    {
        bark_set_work_state(DS_WORK_MONITORING, 0);
        bark_detect_set_active(1);
        bark_listen_reset_locked();
        g_fsm.in_session = 0;
    }
    pthread_mutex_unlock(&g_fsm_mu);
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

void bark_control_on_window_bark(int hit, uint32_t epoch_sec)
{
    if (!hit || !g_power_on)
    {
        return;
    }

    pthread_mutex_lock(&g_fsm_mu);
    bark_on_window_hit_locked(epoch_sec);
    pthread_mutex_unlock(&g_fsm_mu);
}

void bark_control_tick(void)
{
    if (!g_power_on)
    {
        return;
    }

    pthread_mutex_lock(&g_fsm_mu);
    switch (comfort_store_get_work_state())
    {
    case DS_WORK_MONITORING:
        bark_tick_listen_locked();
        break;
    case DS_WORK_ACTING:
        bark_tick_acting_locked();
        break;
    case DS_WORK_RESTING:
        bark_tick_resting_locked();
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&g_fsm_mu);
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
    if (payload[0] > DS_CALM_MODE_MANUAL)
    {
        rsp[0] = DS_UART_STATUS_PARAM_ERROR;
        return 1;
    }
    if (calm_strategy_set_mode((ds_calm_mode_t)payload[0]) != 0)
    {
        rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
        return 1;
    }
    bark_sync_store_from_strategy();
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
    if (!rsp || rsp_cap < 1)
    {
        return 0;
    }
    if (cmd_id == DS_CMD_CALM_STRATEGY_GET)
    {
        return calm_strategy_fill_get_rsp(rsp, rsp_cap);
    }
    if (cmd_id == DS_CMD_CALM_STRATEGY_SET)
    {
        if (calm_strategy_set_from_uart_payload(payload, payload_len) != 0)
        {
            rsp[0] = DS_UART_STATUS_PARAM_ERROR;
            return 1;
        }
        bark_sync_store_from_strategy();
        rsp[0] = DS_UART_STATUS_OK;
        return 1;
    }
    rsp[0] = DS_UART_STATUS_PARAM_ERROR;
    return 1;
}
