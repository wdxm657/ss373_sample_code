/**
 * @file calm_strategy.c
 * @brief 安抚策略内存态与 calm_strategy.bin 持久化；成功后轮换顺序
 */
#define LOG_TAG "calm_strategy"
#include "log.h"

#include "calm_strategy.h"
#include "app_config.h"

#include <stdio.h>
#include <string.h>

#define DS_STRATEGY_MAGIC 0x444D4353u /* 'SCMD' little-endian */

typedef struct __attribute__((packed))
{
    uint32_t magic;
    uint8_t version;
    uint8_t mode;
    uint8_t enabled_mask;
    uint8_t measure_cnt;
    uint8_t measure_order[DS_STRATEGY_MEASURE_MAX];
    uint8_t us_cnt;
    uint8_t us_order[DS_STRATEGY_US_MAX];
} ds_strategy_file_t;

static calm_strategy_t g_st;

static int calm_strategy_measure_valid(uint8_t m)
{
    return m >= DS_MEASURE_MUSIC && m <= DS_MEASURE_ULTRASONIC;
}

static int calm_strategy_us_valid(uint8_t u)
{
    return u >= DS_US_25KHZ && u <= DS_US_DUAL;
}

static void calm_strategy_set_default(void)
{
    g_st.mode = DS_CALM_MODE_AUTO;
    g_st.enabled_mask = DS_ENABLED_MUSIC | DS_ENABLED_OWNER | DS_ENABLED_US;
    g_st.measure_cnt = 3;
    g_st.measure_order[0] = DS_MEASURE_MUSIC;
    g_st.measure_order[1] = DS_MEASURE_OWNER_VOICE;
    g_st.measure_order[2] = DS_MEASURE_ULTRASONIC;
    g_st.us_cnt = 3;
    g_st.us_order[0] = DS_US_25KHZ;
    g_st.us_order[1] = DS_US_30KHZ;
    g_st.us_order[2] = DS_US_DUAL;
}

static void calm_strategy_rotate_to_front(uint8_t *arr, uint8_t cnt, uint8_t idx)
{
    uint8_t v;
    uint8_t i;

    if (!arr || cnt == 0 || idx >= cnt)
    {
        return;
    }
    v = arr[idx];
    for (i = idx; i > 0; i--)
    {
        arr[i] = arr[i - 1];
    }
    arr[0] = v;
}

static uint8_t calm_strategy_find_measure_idx(uint8_t measure)
{
    uint8_t i;

    for (i = 0; i < g_st.measure_cnt; i++)
    {
        if (g_st.measure_order[i] == measure)
        {
            return i;
        }
    }
    return 0;
}

static uint8_t calm_strategy_find_us_idx(uint8_t us_profile)
{
    uint8_t i;

    for (i = 0; i < g_st.us_cnt; i++)
    {
        if (g_st.us_order[i] == us_profile)
        {
            return i;
        }
    }
    return 0;
}

const char *calm_strategy_get_path_by_mode(ds_calm_mode_t mode)
{
    return (mode == DS_CALM_MODE_MANUAL) ? DS_STRATEGY_MANUAL_PATH : DS_STRATEGY_AUTO_PATH;
}

int calm_strategy_init(void)
{
    FILE *fp;
    uint8_t saved_mode;

    calm_strategy_set_default();

    /* 读取之前保存的 mode 文件，按对应模式加载策略 */
    fp = fopen(DS_STRATEGY_MODE_PATH, "rb");
    if (fp)
    {
        if (fread(&saved_mode, 1, 1, fp) == 1)
        {
            LOG_INFO("strategy init: read mode=%u from %s\n", saved_mode, DS_STRATEGY_MODE_PATH);
        }
        fclose(fp);

        if (saved_mode <= DS_CALM_MODE_MANUAL &&
            calm_strategy_load((ds_calm_mode_t)saved_mode) == 0)
        {
            LOG_INFO("strategy init ok: restored mode=%d from file\n", saved_mode);
            return 0;
        }
        LOG_WARN("strategy init: mode file exists but load failed (mode=%u)\n", saved_mode);
    }
    else
    {
        LOG_INFO("strategy init: no mode file, use auto default\n");
    }

    /* 无 mode 文件或加载失败，按自动模式初始化 */
    if (calm_strategy_load(DS_CALM_MODE_AUTO) != 0)
    {
        calm_strategy_save();
    }
    return 0;
}

void calm_strategy_deinit(void)
{
}

const calm_strategy_t *calm_strategy_get(void)
{
    return &g_st;
}

int calm_strategy_load(ds_calm_mode_t mode)
{
    FILE *fp;
    ds_strategy_file_t f;
    const char *path;

    path = calm_strategy_get_path_by_mode(mode);
    fp = fopen(path, "rb");
    if (!fp)
    {
        return -1;
    }
    if (fread(&f, sizeof(f), 1, fp) != 1)
    {
        fclose(fp);
        return -2;
    }
    fclose(fp);

    if (f.magic != DS_STRATEGY_MAGIC || f.version != 1u)
    {
        return -3;
    }
    if (f.measure_cnt == 0 || f.measure_cnt > DS_STRATEGY_MEASURE_MAX)
    {
        return -4;
    }
    if (f.us_cnt == 0 || f.us_cnt > DS_STRATEGY_US_MAX)
    {
        return -5;
    }
    if (f.enabled_mask == 0)
    {
        return -6;
    }

    g_st.mode = mode;
    g_st.enabled_mask = f.enabled_mask;
    g_st.measure_cnt = f.measure_cnt;
    memcpy(g_st.measure_order, f.measure_order, sizeof(g_st.measure_order));
    g_st.us_cnt = f.us_cnt;
    memcpy(g_st.us_order, f.us_order, sizeof(g_st.us_order));
    LOG_INFO("strategy loaded from %s\n", path);
    LOG_INFO("measure_order: %d, %d, %d\n", g_st.measure_order[0], g_st.measure_order[1], g_st.measure_order[2]);
    LOG_INFO("us_order: %d, %d, %d\n", g_st.us_order[0], g_st.us_order[1], g_st.us_order[2]);
    return 0;
}

int calm_strategy_save(void)
{
    FILE *fp;
    ds_strategy_file_t f;

    memset(&f, 0, sizeof(f));
    f.magic = DS_STRATEGY_MAGIC;
    f.version = 1;
    f.mode = (uint8_t)g_st.mode;
    f.enabled_mask = g_st.enabled_mask;
    f.measure_cnt = g_st.measure_cnt;
    memcpy(f.measure_order, g_st.measure_order, sizeof(f.measure_order));
    f.us_cnt = g_st.us_cnt;
    memcpy(f.us_order, g_st.us_order, sizeof(f.us_order));

    fp = fopen(calm_strategy_get_path_by_mode(g_st.mode), "wb");
    if (!fp)
    {
        LOG_ERROR("strategy save open failed: %s\n", calm_strategy_get_path_by_mode(g_st.mode));
        return -1;
    }
    if (fwrite(&f, sizeof(f), 1, fp) != 1)
    {
        fclose(fp);
        return -2;
    }
    fclose(fp);
    LOG_INFO("strategy saved mode=%d enabledMask=0x%02x "
             "measureCnt=%d measureOrder=[%d,%d,%d] "
             "usCnt=%d usOrder=[%d,%d,%d] path=%s\n",
             g_st.mode, g_st.enabled_mask,
             g_st.measure_cnt,
             g_st.measure_order[0], g_st.measure_order[1], g_st.measure_order[2],
             g_st.us_cnt,
             g_st.us_order[0], g_st.us_order[1], g_st.us_order[2],
             calm_strategy_get_path_by_mode(g_st.mode));
    return 0;

}

void calm_strategy_factory_reset(void)
{
    calm_strategy_set_default();
    calm_strategy_save();
}

int calm_strategy_set_from_uart_payload(const uint8_t *payload, uint16_t len)
{
    uint16_t idx;
    uint8_t m_cnt;
    uint8_t u_cnt;
    uint8_t i;

    LOG_INFO("strategy set from uart: len=%u\n", len);

    if (!payload || len < 4)
    {
        LOG_ERROR("strategy set: invalid len %u (need >=4)\n", len);
        return -1;
    }
    LOG_DEBUG("  payload[0]=mode=%u [1]=enabledMask=0x%02x [2]=measureCnt=%u\n",
              payload[0], payload[1], payload[2]);
    if (payload[0] > DS_CALM_MODE_MANUAL || payload[1] == 0)
    {
        LOG_ERROR("strategy set: invalid mode=%u or enabledMask=0x%02x\n",
                  payload[0], payload[1]);
        return -2;
    }
    m_cnt = payload[2];
    if (m_cnt == 0 || m_cnt > DS_STRATEGY_MEASURE_MAX)
    {
        LOG_ERROR("strategy set: invalid measureCnt=%u\n", m_cnt);
        return -3;
    }
    idx = 3;
    if ((uint16_t)(idx + m_cnt + 1) > len)
    {
        LOG_ERROR("strategy set: payload too short for measureOrder (need %u, have %u)\n",
                  (uint16_t)(idx + m_cnt + 1), len);
        return -4;
    }
    LOG_DEBUG("  measureOrder:");
    for (i = 0; i < m_cnt; i++)
    {
        LOG_DEBUG(" %u", payload[idx + i]);
        if (!calm_strategy_measure_valid(payload[idx + i]))
        {
            LOG_ERROR(" -> invalid measure[%u]=%u\n", i, payload[idx + i]);
            return -5;
        }
    }
    u_cnt = payload[idx + m_cnt];
    LOG_DEBUG("  usCnt=%u", u_cnt);
    if (u_cnt == 0 || u_cnt > DS_STRATEGY_US_MAX)
    {
        LOG_ERROR("strategy set: invalid usCnt=%u\n", u_cnt);
        return -6;
    }
    if ((uint16_t)(idx + m_cnt + 1 + u_cnt) > len)
    {
        LOG_ERROR("strategy set: payload too short for usOrder (need %u, have %u)\n",
                  (uint16_t)(idx + m_cnt + 1 + u_cnt), len);
        return -7;
    }
    LOG_DEBUG("  usOrder:");
    for (i = 0; i < u_cnt; i++)
    {
        LOG_DEBUG(" %u", payload[idx + m_cnt + 1 + i]);
        if (!calm_strategy_us_valid(payload[idx + m_cnt + 1 + i]))
        {
            LOG_ERROR(" -> invalid us[%u]=%u\n", i, payload[idx + m_cnt + 1 + i]);
            return -8;
        }
    }
    LOG_DEBUG("\n");

    g_st.mode = (ds_calm_mode_t)payload[0];
    g_st.enabled_mask = payload[1];
    g_st.measure_cnt = m_cnt;
    memcpy(g_st.measure_order, payload + idx, m_cnt);
    idx = (uint16_t)(idx + m_cnt + 1);
    g_st.us_cnt = u_cnt;
    memcpy(g_st.us_order, payload + idx, u_cnt);
    LOG_INFO("strategy set ok: mode=%d enabledMask=0x%02x "
             "measureOrder=[%d,%d,%d] usOrder=[%d,%d,%d]\n",
             g_st.mode, g_st.enabled_mask,
             g_st.measure_order[0], g_st.measure_order[1], g_st.measure_order[2],
             g_st.us_order[0], g_st.us_order[1], g_st.us_order[2]);
    calm_strategy_save();
    return 0;

}

uint16_t calm_strategy_fill_get_rsp(ds_calm_mode_t mode, uint8_t *rsp, uint16_t rsp_cap)
{
    uint16_t n;
    uint8_t i;

    /* 先加载请求的模式对应的策略文件到内存 */
    LOG_INFO("strategy get: loading mode=%d\n", mode);
    calm_strategy_load(mode);

    n = (uint16_t)(4 + g_st.measure_cnt + g_st.us_cnt);
    if (!rsp || rsp_cap < n)
    {
        LOG_ERROR("strategy get: rsp_cap=%u < n=%u\n", rsp_cap, n);
        return 0;
    }
    rsp[0] = DS_UART_STATUS_OK;
    rsp[1] = (uint8_t)g_st.mode;
    rsp[2] = g_st.enabled_mask;
    rsp[3] = g_st.measure_cnt;
    for (i = 0; i < g_st.measure_cnt; i++)
    {
        rsp[4 + i] = g_st.measure_order[i];
    }
    rsp[4 + g_st.measure_cnt] = g_st.us_cnt;
    for (i = 0; i < g_st.us_cnt; i++)
    {
        rsp[5 + g_st.measure_cnt + i] = g_st.us_order[i];
    }
    LOG_INFO("strategy get rsp: mode=%d enabledMask=0x%02x "
             "measureOrder=[%d,%d,%d] usOrder=[%d,%d,%d] n=%u\n",
             g_st.mode, g_st.enabled_mask,
             g_st.measure_order[0], g_st.measure_order[1], g_st.measure_order[2],
             g_st.us_order[0], g_st.us_order[1], g_st.us_order[2], n);
    return n;

}

int calm_strategy_set_mode(ds_calm_mode_t mode)
{
    FILE *fp;
    uint8_t raw;

    LOG_INFO("strategy set_mode: mode=%d\n", mode);

    if (mode > DS_CALM_MODE_MANUAL)
    {
        LOG_ERROR("strategy set_mode: invalid mode=%d\n", mode);
        return -1;
    }

    g_st.mode = mode;

    /* 把 mode 写入本地文件，供下次 init 读取 */
    raw = (uint8_t)mode;
    fp = fopen(DS_STRATEGY_MODE_PATH, "wb");
    if (fp)
    {
        fwrite(&raw, 1, 1, fp);
        fclose(fp);
        LOG_INFO("strategy mode saved to %s: mode=%u\n", DS_STRATEGY_MODE_PATH, raw);
    }
    else
    {
        LOG_ERROR("strategy mode save failed: %s\n", DS_STRATEGY_MODE_PATH);
    }

    /* 将策略保存到对应模式的文件 */
    return calm_strategy_save();

}

void calm_strategy_apply_success(uint8_t ok_measure, uint8_t ok_us_sub)
{
    uint8_t mi;
    uint8_t ui;

    if (g_st.mode != DS_CALM_MODE_AUTO)
    {
        return;
    }
    if (!calm_strategy_measure_valid(ok_measure))
    {
        return;
    }

    mi = calm_strategy_find_measure_idx(ok_measure);
    calm_strategy_rotate_to_front(g_st.measure_order, g_st.measure_cnt, mi);

    if (ok_measure == DS_MEASURE_ULTRASONIC && calm_strategy_us_valid(ok_us_sub))
    {
        ui = calm_strategy_find_us_idx(ok_us_sub);
        calm_strategy_rotate_to_front(g_st.us_order, g_st.us_cnt, ui);
    }

    calm_strategy_save();
    LOG_INFO("strategy reordered ok_measure=%u ok_us=%u\n", ok_measure, ok_us_sub);
}
