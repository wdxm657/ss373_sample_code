/**
 * @file bark_control.c
 * @brief 监听/复听 → 识别(10s/3次) → 执行 → 休息；策略见 calm_strategy
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
    BARK_LISTEN_IDENTIFY,
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
static uint8_t g_reward_enabled = 0; /* 零食奖励功能开关（MCU 同步，安抚成功时判定是否额外投喂） */
static bark_fsm_t g_fsm;
static pthread_mutex_t g_fsm_mu = PTHREAD_MUTEX_INITIALIZER;
static uint8_t g_next_session_id = 1;

static void bark_start_session_locked(uint32_t bark_ts);
static void bark_begin_action_locked(void);

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
    if (measure == DS_MEASURE_SNACK_FEED)
    {
        return (enabled_mask & DS_ENABLED_SNACK) != 0;
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

static void bark_enter_identifying_locked(time_t now)
{
    int post = (g_fsm.listen_phase == BARK_LISTEN_POST_MEASURE) ? 1 : 0;

    bark_set_work_state(DS_WORK_IDENTIFYING, post ? 8u : 1u);
    if (!post)
    {
        g_fsm.listen_phase = BARK_LISTEN_IDENTIFY;
    }
    g_fsm.win_count = 1;
    g_fsm.win_deadline = now + DS_MONITOR_WINDOW_SEC;
    LOG_INFO("identifying start post=%d deadline=%ld\n", post, (long)g_fsm.win_deadline);
}

static void bark_identifying_back_to_monitor_locked(void)
{
    bark_set_work_state(DS_WORK_MONITORING, 7);
    bark_listen_reset_locked();
    LOG_INFO("identifying end, back to monitoring\n");
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

static void bark_post_session_result_evt(uint32_t session_id, uint8_t result, uint8_t ok_measure, uint8_t ok_sub, uint8_t reward)
{
    uint8_t payload[12];
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
    payload[11] = reward; /* 1=本次成功附带零食奖励投喂 */
    uart_proto_send_evt(DS_EVT_SESSION_RESULT, 0, payload, sizeof(payload));
    LOG_INFO("EVT session result id=%u result=%u reward=%u\n", session_id, result, reward);
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
        LOG_DEBUG("measure_order  %d\n", m);

        if (st->mode == DS_CALM_MODE_AUTO && m == DS_MEASURE_SNACK_FEED)
        {
            continue;
        }
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
            /* 主人录音不存在时直接跳过，不追加超声波 */
        }
        else if (m == DS_MEASURE_ULTRASONIC)
        {
            if (!us_substituted)
            {
                bark_append_us_actions_locked(st, &n);
                us_substituted = 1;
            }
        }
        else if (m == DS_MEASURE_SNACK_FEED)
        {
            g_fsm.actions[n].measure = DS_MEASURE_SNACK_FEED;
            g_fsm.actions[n].us_profile = 0;
            n++;
        }
    }
    g_fsm.action_cnt = n;
    LOG_INFO("session actions built cnt=%u owner_ok=%d\n", n, owner_ok);
}

static void bark_enter_rest_locked(uint8_t success)
{
    uint8_t ok_m = g_fsm.last_ok_measure;
    uint8_t ok_s = g_fsm.last_ok_us_sub;
    uint32_t now_ts;
    uint8_t reward = 0;

    if (success)
    {
        calm_strategy_apply_success(ok_m, ok_s);
        bark_sync_store_from_strategy();
    }

    /* 零食奖励：安抚成功 + 奖励功能开启 + 最后措施不是投喂零食 → 额外投喂一次。
     * 该投喂不属于安抚流程措施（不追加记录条目），仅记录 reward 标记。 */
    if (success && g_reward_enabled && ok_m != DS_MEASURE_SNACK_FEED)
    {
        reward = 1;
        LOG_INFO("snack reward: session %u success, last_measure=%u -> feed once\n",
                 g_fsm.session_id, ok_m);
        uart_proto_send_evt(DS_EVT_SNACK_FEED, 0, NULL, 0);
    }

    now_ts = (uint32_t)time(NULL);
    comfort_store_record_finish(g_fsm.session_id, success, now_ts, reward);

    bark_post_session_result_evt(g_fsm.session_id, success ? 1u : 0u, ok_m, ok_s, reward);
    bark_set_work_state(DS_WORK_RESTING, success ? 3u : 4u);
    bark_detect_set_active(0);
    g_fsm.in_session = 0;
    g_fsm.acting_busy = 0;
    g_fsm.rest_deadline = now_ts + DS_CALM_REST_SEC;
    bark_listen_reset_locked();
    LOG_INFO("session %u end success=%u rest %ds\n", g_fsm.session_id, success, DS_CALM_REST_SEC);
}

static void bark_start_session_locked(uint32_t bark_ts)
{
    g_fsm.in_session = 1;
    g_fsm.session_id = g_next_session_id;
    if (g_next_session_id >= 254) { g_next_session_id = 1; } else { g_next_session_id++; }
    g_fsm.session_bark_ts = bark_ts;
    g_fsm.action_idx = 0;
    g_fsm.last_ok_measure = 0;
    g_fsm.last_ok_us_sub = 0;
    bark_build_session_actions_locked();
    bark_post_session_start_evt(g_fsm.session_id, bark_ts);
    comfort_store_record_begin(g_fsm.session_id, bark_ts);
    LOG_INFO("calm session %u started\n", g_fsm.session_id);
}

static void bark_begin_action_locked(void)
{
    const sess_action_t *act;
    uint8_t entry_type;

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

    /* 将措施类型映射为记录条目类型 */
    entry_type = comfort_store_measure_to_entry_type(act->measure, act->us_profile);
    if (entry_type)
    {
        comfort_store_record_append_measure(g_fsm.session_id, entry_type, (uint32_t)time(NULL));
    }

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
    case DS_MEASURE_SNACK_FEED:
        uart_proto_send_evt(DS_EVT_SNACK_FEED, 0, NULL, 0);
        g_fsm.acting_busy = 0;
        break;
    default:
        g_fsm.acting_busy = 0;
        break;
    }
}

static void bark_identifying_start_session_locked(uint32_t bark_ts)
{
    bark_start_session_locked(bark_ts);
    bark_begin_action_locked();
    bark_listen_reset_locked();
}

static void bark_identifying_post_continue_locked(void)
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
        ultrasonic_stop();
        bark_after_measure_done_locked();
    }
}

static void bark_on_window_hit_locked(uint32_t epoch_sec)
{
    time_t now = (time_t)epoch_sec;
    ds_work_state_t st = comfort_store_get_work_state();

    if (st == DS_WORK_RESTING || st == DS_WORK_ACTING)
    {
        return;
    }

    /* 监听 / 措施后复听：第一次狗叫 → 进入识别（复听保留 POST_MEASURE 标记） */
    if (st == DS_WORK_MONITORING &&
        (g_fsm.listen_phase == BARK_LISTEN_IDLE ||
         (g_fsm.listen_phase == BARK_LISTEN_POST_MEASURE && g_fsm.in_session)))
    {
        bark_enter_identifying_locked(now);
        return;
    }

    /* 识别：继续累计，满 3 次 → 首次进会话执行 / 复听进下一措施或失败休息 */
    if (st == DS_WORK_IDENTIFYING)
    {
        if (g_fsm.listen_phase != BARK_LISTEN_IDENTIFY &&
            g_fsm.listen_phase != BARK_LISTEN_POST_MEASURE)
        {
            return;
        }
        if (now > g_fsm.win_deadline)
        {
            bark_enter_identifying_locked(now);
            return;
        }
        g_fsm.win_count++;
        LOG_INFO("identifying count=%d post=%d\n",
                 g_fsm.win_count,
                 g_fsm.listen_phase == BARK_LISTEN_POST_MEASURE);
        if (g_fsm.win_count < DS_MONITOR_BARK_TRIGGER)
        {
            return;
        }
        if (g_fsm.listen_phase == BARK_LISTEN_POST_MEASURE)
        {
            bark_identifying_post_continue_locked();
        }
        else
        {
            bark_identifying_start_session_locked(epoch_sec);
        }
        return;
    }
}

static void bark_tick_listen_locked(void)
{
    time_t now = time(NULL);

    /* 措施后复听：一直无狗叫，超时判定安抚成功 */
    if (g_fsm.in_session && g_fsm.listen_phase == BARK_LISTEN_POST_MEASURE &&
        g_fsm.win_count == 0 && g_fsm.measure_end_ts > 0 &&
        now >= g_fsm.measure_end_ts + DS_MONITOR_WINDOW_SEC)
    {
        LOG_INFO("post-measure quiet success\n");
        bark_enter_rest_locked(1);
    }
}

static void bark_tick_identifying_locked(void)
{
    time_t now = time(NULL);

    if (g_fsm.win_deadline == 0 || now < g_fsm.win_deadline ||
        g_fsm.win_count >= DS_MONITOR_BARK_TRIGGER)
    {
        return;
    }

    if (g_fsm.in_session && g_fsm.listen_phase == BARK_LISTEN_POST_MEASURE)
    {
        LOG_INFO("post-measure identifying expired count=%d -> success rest\n",
                 g_fsm.win_count);
        bark_enter_rest_locked(1);
        return;
    }

    LOG_INFO("identifying window expired count=%d -> monitoring\n", g_fsm.win_count);
    bark_identifying_back_to_monitor_locked();
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
        /* 立即停止正在执行的安抚措施 */
        audio_ao_stop();
        ultrasonic_stop();

        /* 若会话进行中，丢弃未完成的记录 */
        if (g_fsm.in_session)
        {
            comfort_store_discard_record(g_fsm.session_id);
        }

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

void bark_control_set_reward_enabled(uint8_t on)
{
    uint8_t old = g_reward_enabled;
    g_reward_enabled = on ? 1 : 0;
    if (old != g_reward_enabled)
    {
        LOG_INFO("reward_enabled %u -> %u\n", old, g_reward_enabled);
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
    // Heartbeat: send every 2 ticks (~2 seconds) regardless of power state
    {
        static int hb_cnt = 0;
        hb_cnt++;
        if (hb_cnt >= 2)
        {
            hb_cnt = 0;
            // LOG_DEBUG("herart beat");
            uart_proto_send_evt(DS_EVT_HEARTBEAT, 0, NULL, 0);
        }
    }

    /* 每 5 秒检查：有完整安抚记录时通知 MCU（由 MCU 侧判断 BLE 是否已连接*/
    {
        static uint8_t s_check_cnt = 0;
        s_check_cnt++;
        if (s_check_cnt >= 2)
        {
            s_check_cnt = 0;
            if (comfort_store_has_records())
            {
                LOG_DEBUG("has_records");
                uart_proto_send_evt(DS_EVT_NEW_CALM_RECORD, 0, NULL, 0);
            }
        }
    }

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
    case DS_WORK_IDENTIFYING:
        bark_tick_identifying_locked();
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

uint8_t bark_control_is_session_active(void)
{
    return g_fsm.in_session;
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
        ds_calm_mode_t get_mode = DS_CALM_MODE_AUTO;
        if (payload_len >= 1)
        {
            get_mode = (payload[0] <= DS_CALM_MODE_MANUAL)
                           ? (ds_calm_mode_t)payload[0]
                           : DS_CALM_MODE_AUTO;
        }
        LOG_INFO("bark_control: DS_CMD_CALM_STRATEGY_GET mode=%d\n", get_mode);
        return calm_strategy_fill_get_rsp(get_mode, rsp, rsp_cap);
    }
    if (cmd_id == DS_CMD_CALM_STRATEGY_SET)
    {
        LOG_INFO("bark_control: DS_CMD_CALM_STRATEGY_SET len=%u\n", payload_len);
        if (calm_strategy_set_from_uart_payload(payload, payload_len) != 0)
        {
            rsp[0] = DS_UART_STATUS_PARAM_ERROR;
            LOG_ERROR("bark_control: strategy set failed\n");
            return 1;
        }
        bark_sync_store_from_strategy();
        rsp[0] = DS_UART_STATUS_OK;
        LOG_INFO("bark_control: strategy set ok\n");
        return 1;
    }
    rsp[0] = DS_UART_STATUS_PARAM_ERROR;
    return 1;
}
