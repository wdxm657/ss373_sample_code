/**
 * @file comfort_store.c
 * @brief 运行时状态与 STATUS_GET / 出厂重置 / 安抚记录生成与获取
 */
#define LOG_TAG "comfort_store"
#include "log.h"

#include "comfort_store.h"
#include "calm_strategy.h"
#include "app_config.h"
#include "audio.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

/* ========== 安抚记录持久化文件格式 ========== */
#define DS_RECORD_FILE_MAGIC 0x44524344u /* 'DCRD' little-endian */
#define DS_RECORD_FILE_VER 1

#pragma pack(push, 1)
typedef struct
{
    uint32_t magic;  /* DS_RECORD_FILE_MAGIC */
    uint8_t version; /* DS_RECORD_FILE_VER */
    uint8_t count;   /* 有效记录数 */
} ds_record_file_hdr_t;

/* 文件中每条记录的固定部分 + entries 紧随其后 */
typedef struct
{
    uint32_t session_id;
    uint32_t start_ts;
    uint32_t end_ts;
    uint8_t entry_cnt;
    /* ds_record_entry_t entries[entry_cnt] 紧随其后 */
} ds_record_file_entry_hdr_t;
#pragma pack(pop)

/* ========== 内存态记录环形缓存 ========== */
static ds_session_record_t g_records[DS_RECORD_MAX];
static uint8_t g_record_cnt;  /* 有效记录数 (0..DS_RECORD_MAX) */
static uint8_t g_record_head; /* 最旧记录的索引（环形） */

/* ========== 运行时状态 ========== */

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
    .volume = 10,   /* +10 dB（与 audio_ao_init 默认增益一致）*/
    .calm_mode = DS_CALM_MODE_AUTO,
    .enabled_mask = DS_ENABLED_MUSIC | DS_ENABLED_US,
    .us_mask = 0x07,
};

/* ========== 记录文件读写 ========== */

static int comfort_store_load_records(void)
{
    FILE *fp;
    ds_record_file_hdr_t hdr;
    uint8_t i;

    fp = fopen(DS_COMFORT_DB_PATH, "rb");
    if (!fp)
    {
        return -1;
    }
    if (fread(&hdr, sizeof(hdr), 1, fp) != 1)
    {
        fclose(fp);
        return -2;
    }
    if (hdr.magic != DS_RECORD_FILE_MAGIC || hdr.version != DS_RECORD_FILE_VER)
    {
        fclose(fp);
        return -3;
    }

    g_record_cnt = 0;
    g_record_head = 0;
    for (i = 0; i < hdr.count && i < DS_RECORD_MAX; i++)
    {
        ds_record_file_entry_hdr_t fhdr;
        uint8_t j;

        if (fread(&fhdr, sizeof(fhdr), 1, fp) != 1)
        {
            break;
        }
        if (fhdr.entry_cnt > DS_RECORD_ENTRY_MAX)
        {
            break;
        }

        ds_session_record_t *rec = &g_records[g_record_cnt];
        rec->session_id = fhdr.session_id;
        rec->start_ts = fhdr.start_ts;
        rec->end_ts = fhdr.end_ts;
        rec->entry_cnt = fhdr.entry_cnt;
        for (j = 0; j < fhdr.entry_cnt; j++)
        {
            if (fread(&rec->entries[j], sizeof(ds_record_entry_t), 1, fp) != 1)
            {
                break;
            }
        }
        if (j < fhdr.entry_cnt)
        {
            break;
        }
        g_record_cnt++;
    }

    fclose(fp);
    LOG_INFO("loaded %u records from %s\n", g_record_cnt, DS_COMFORT_DB_PATH);
    return 0;
}

static int comfort_store_save_records(void)
{
    FILE *fp;
    ds_record_file_hdr_t hdr;
    uint8_t i;

    fp = fopen(DS_COMFORT_DB_PATH, "wb");
    if (!fp)
    {
        LOG_ERROR("save records open failed: %s\n", DS_COMFORT_DB_PATH);
        return -1;
    }

    hdr.magic = DS_RECORD_FILE_MAGIC;
    hdr.version = DS_RECORD_FILE_VER;
    hdr.count = g_record_cnt;
    if (fwrite(&hdr, sizeof(hdr), 1, fp) != 1)
    {
        fclose(fp);
        return -2;
    }

    for (i = 0; i < g_record_cnt; i++)
    {
        uint8_t idx = (g_record_head + i) % DS_RECORD_MAX;
        const ds_session_record_t *rec = &g_records[idx];
        ds_record_file_entry_hdr_t fhdr;
        uint8_t j;

        fhdr.session_id = rec->session_id;
        fhdr.start_ts = rec->start_ts;
        fhdr.end_ts = rec->end_ts;
        fhdr.entry_cnt = rec->entry_cnt;
        if (fwrite(&fhdr, sizeof(fhdr), 1, fp) != 1)
        {
            fclose(fp);
            return -3;
        }
        for (j = 0; j < rec->entry_cnt; j++)
        {
            if (fwrite(&rec->entries[j], sizeof(ds_record_entry_t), 1, fp) != 1)
            {
                fclose(fp);
                return -4;
            }
        }
    }

    fclose(fp);
    LOG_INFO("saved %u records to %s\n", g_record_cnt, DS_COMFORT_DB_PATH);
    return 0;
}

/* ========== 记录生成 API ========== */

static ds_session_record_t *comfort_store_find_record(uint32_t session_id)
{
    uint8_t i;

    for (i = 0; i < g_record_cnt; i++)
    {
        uint8_t idx = (g_record_head + i) % DS_RECORD_MAX;
        if (g_records[idx].session_id == session_id)
        {
            return &g_records[idx];
        }
    }
    return NULL;
}

uint8_t comfort_store_measure_to_entry_type(uint8_t measure, uint8_t us_profile)
{
    if (measure == DS_MEASURE_MUSIC)
    {
        return DS_RECORD_ENTRY_MUSIC;
    }
    if (measure == DS_MEASURE_OWNER_VOICE)
    {
        return DS_RECORD_ENTRY_OWNER;
    }
    if (measure == DS_MEASURE_ULTRASONIC)
    {
        if (us_profile == DS_US_25KHZ)
            return DS_RECORD_ENTRY_US_25K;
        if (us_profile == DS_US_30KHZ)
            return DS_RECORD_ENTRY_US_30K;
        if (us_profile == DS_US_DUAL)
            return DS_RECORD_ENTRY_US_DUAL;
    }
    return 0;
}

void comfort_store_record_begin(uint32_t session_id, uint32_t bark_ts)
{
    ds_session_record_t *rec;

    /* 缓存已满，淘汰最旧记录 */
    if (g_record_cnt >= DS_RECORD_MAX)
    {
        g_record_head = (g_record_head + 1) % DS_RECORD_MAX;
        g_record_cnt = DS_RECORD_MAX - 1;
    }

    /* 新记录写入 tail */
    uint8_t tail = (g_record_head + g_record_cnt) % DS_RECORD_MAX;
    rec = &g_records[tail];
    memset(rec, 0, sizeof(*rec));
    rec->session_id = session_id;
    rec->start_ts = bark_ts;
    rec->end_ts = 0;
    rec->entry_cnt = 0;

    /* 追加吠叫识别条目 */
    rec->entries[rec->entry_cnt].type = DS_RECORD_ENTRY_BARK;
    rec->entries[rec->entry_cnt].ts = bark_ts;
    rec->entry_cnt++;

    g_record_cnt++;
    LOG_INFO("record begin session=%u bark_ts=%u\n", session_id, bark_ts);
}

void comfort_store_record_append_measure(uint32_t session_id, uint8_t type, uint32_t ts)
{
    ds_session_record_t *rec = comfort_store_find_record(session_id);

    if (!rec)
    {
        LOG_INFO("record append: session %u not found\n", session_id);
        return;
    }
    if (rec->entry_cnt >= DS_RECORD_ENTRY_MAX)
    {
        LOG_INFO("record append: entry full session=%u\n", session_id);
        return;
    }

    rec->entries[rec->entry_cnt].type = type;
    rec->entries[rec->entry_cnt].ts = ts;
    rec->entry_cnt++;
    LOG_DEBUG("record append session=%u type=0x%02x ts=%u\n", session_id, type, ts);
}

void comfort_store_record_finish(uint32_t session_id, uint8_t success, uint32_t end_ts)
{
    ds_session_record_t *rec = comfort_store_find_record(session_id);

    if (!rec)
    {
        LOG_INFO("record finish: session %u not found\n", session_id);
        return;
    }

    rec->end_ts = end_ts;

    /* 追加结果条目 */
    if (rec->entry_cnt < DS_RECORD_ENTRY_MAX)
    {
        rec->entries[rec->entry_cnt].type = success ? DS_RECORD_ENTRY_SUCCESS : DS_RECORD_ENTRY_FAIL;
        rec->entries[rec->entry_cnt].ts = end_ts;
        rec->entry_cnt++;
    }

    /* 持久化到文件 */
    comfort_store_save_records();
    LOG_INFO("record finish session=%u success=%u entries=%u\n",
             session_id, success, rec->entry_cnt);
}

/* ========== 运行时状态 API ========== */

int comfort_store_init(void) /* 从 owner.wav 刷新录音存在标志，加载安抚记录 */
{
    audio_refresh_owner_info(&g_rt.owner_voice_exist, &g_rt.owner_duration_sec);
    comfort_store_load_records();
    LOG_INFO("comfort_store init\n");
    return 0;
}

void comfort_store_deinit(void) /* 持久化当前记录 */
{
    comfort_store_save_records();
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
    out[5] = audio_get_volume();
    out[6] = (uint8_t)g_rt.calm_mode;
    out[7] = g_rt.enabled_mask;
    out[8] = g_rt.us_mask;
    *out_len = 9;
}

void comfort_store_set_power(uint8_t on) /* POWER_CTRL 写入 */
{
    uint8_t old = g_rt.power_on;
    g_rt.power_on = on ? 1 : 0;
    if (old != g_rt.power_on)
    {
        LOG_INFO("state_change: power_on %u -> %u\n", old, g_rt.power_on);
    }
}

uint8_t comfort_store_get_power(void)
{
    return g_rt.power_on;
}

void comfort_store_set_bt_linked(uint8_t linked) /* BT_LINK_NOTIFY */
{
    uint8_t old = g_rt.bt_linked;
    g_rt.bt_linked = linked ? 1 : 0;
    if (old != g_rt.bt_linked)
    {
        LOG_INFO("state_change: bt_linked %u -> %u\n", old, g_rt.bt_linked);
    }
}

ds_work_state_t comfort_store_get_work_state(void)
{
    return g_rt.work_state;
}

void comfort_store_set_work_state(ds_work_state_t state) /* bark_control 状态机更新 */
{
    ds_work_state_t old = g_rt.work_state;
    g_rt.work_state = state;
    if (old != g_rt.work_state)
    {
        LOG_INFO("state_change: work_state %u -> %u\n", (uint8_t)old, (uint8_t)g_rt.work_state);
    }
}

void comfort_store_set_calm_runtime(ds_calm_mode_t mode, uint8_t enabled_mask, uint8_t us_mask)
{
    uint8_t changed = 0;

    if (g_rt.calm_mode != mode)
    {
        LOG_INFO("state_change: calm_mode %u -> %u\n", (uint8_t)g_rt.calm_mode, (uint8_t)mode);
        g_rt.calm_mode = mode;
        changed = 1;
    }
    if (g_rt.enabled_mask != enabled_mask)
    {
        LOG_INFO("state_change: enabled_mask 0x%02x -> 0x%02x\n", g_rt.enabled_mask, enabled_mask);
        g_rt.enabled_mask = enabled_mask;
        changed = 1;
    }
    if (g_rt.us_mask != us_mask)
    {
        LOG_INFO("state_change: us_mask 0x%02x -> 0x%02x\n", g_rt.us_mask, us_mask);
        g_rt.us_mask = us_mask;
        changed = 1;
    }
    if (!changed)
    {
        LOG_DEBUG("set_calm_runtime: no change (mode=%u enabled=0x%02x us=0x%02x)\n",
                  (uint8_t)mode, enabled_mask, us_mask);
    }
}

/* ========== 记录获取与删除（0x41 / 0x42 串口命令） ========== */

uint16_t comfort_store_pull_records(
    const uint8_t *req,
    uint16_t req_len,
    uint8_t *rsp,
    uint16_t rsp_cap)
{
    /*
    UART 协议：

    REQ payload: []（可选 payload 被忽略，始终返回最旧记录）

    RSP payload: [status(1), remainingCount(1), entryCount(1), entries...]
      status          - DS_UART_STATUS_OK 或 DS_UART_STATUS_NOT_FOUND
      remainingCount  - 取出本条后剩余记录数
      entryCount      - 本条记录的 entry 数
      entries         - entryCount 条 entry，每条 [type(1B), ts(4B)] = 5B

    无记录时返回 [DS_UART_STATUS_OK, 0, 0]。

    注意：直接从文件读取，不依赖内存缓存。
    */
    FILE *fp;
    ds_record_file_hdr_t hdr;
    ds_record_file_entry_hdr_t fhdr;
    uint8_t entry_cnt;
    uint8_t remaining;
    uint16_t pos;
    uint8_t i;

    (void)req;
    (void)req_len;

    if (!rsp || rsp_cap < 3)
    {
        return 0;
    }

    /* 从文件读取第一条记录 */
    fp = fopen(DS_COMFORT_DB_PATH, "rb");
    if (!fp)
    {
        /* 文件不存在或无记录 */
        rsp[0] = DS_UART_STATUS_OK;
        rsp[1] = 0;
        rsp[2] = 0;
        return 3;
    }

    if (fread(&hdr, sizeof(hdr), 1, fp) != 1 ||
        hdr.magic != DS_RECORD_FILE_MAGIC ||
        hdr.version != DS_RECORD_FILE_VER ||
        hdr.count == 0)
    {
        fclose(fp);
        rsp[0] = DS_UART_STATUS_OK;
        rsp[1] = 0;
        rsp[2] = 0;
        return 3;
    }

    /* 读取第一条记录头 */
    if (fread(&fhdr, sizeof(fhdr), 1, fp) != 1)
    {
        fclose(fp);
        rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
        return 1;
    }

    entry_cnt = (fhdr.entry_cnt <= DS_RECORD_ENTRY_MAX) ? fhdr.entry_cnt : 0;
    {
        uint16_t need = (uint16_t)(3 + entry_cnt * 5);
        if (need > rsp_cap || entry_cnt == 0)
        {
            fclose(fp);
            rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
            return 1;
        }

        remaining = (hdr.count > 1) ? (uint8_t)(hdr.count - 1) : 0;

        rsp[0] = DS_UART_STATUS_OK;
        rsp[1] = remaining;
        rsp[2] = entry_cnt;

        pos = 3;
        for (i = 0; i < entry_cnt; i++)
        {
            ds_record_entry_t entry;
            if (fread(&entry, sizeof(entry), 1, fp) != 1)
            {
                break;
            }
            rsp[pos] = entry.type;
            rsp[pos + 1] = (uint8_t)(entry.ts & 0xFF);
            rsp[pos + 2] = (uint8_t)((entry.ts >> 8) & 0xFF);
            rsp[pos + 3] = (uint8_t)((entry.ts >> 16) & 0xFF);
            rsp[pos + 4] = (uint8_t)((entry.ts >> 24) & 0xFF);
            pos += 5;
        }
    }

    fclose(fp);
    LOG_INFO("pull_records from file: entries=%u remaining=%u\n", entry_cnt, remaining);
    return pos;
}

uint16_t comfort_store_delete_oldest_record(
    const uint8_t *req,
    uint16_t req_len,
    uint8_t *rsp,
    uint16_t rsp_cap)
{
    /*
    UART 协议：

    REQ payload: []（可选 payload 被忽略）
    RSP payload: [status(1), remainingCount(1)]

    直接从文件读取所有记录，跳过第一条，写回文件。
    */
    FILE    *fp;
    uint8_t  record_buf[4096]; /* 足够容纳 DS_RECORD_MAX 条记录 */
    uint16_t file_len;
    uint16_t remain_len;

    (void)req;
    (void)req_len;

    if (!rsp || rsp_cap < 2)
    {
        return 0;
    }

    /* 读取整个文件 */
    fp = fopen(DS_COMFORT_DB_PATH, "rb");
    if (!fp)
    {
        rsp[0] = DS_UART_STATUS_OK;
        rsp[1] = 0;
        return 2;
    }
    file_len = (uint16_t)fread(record_buf, 1, sizeof(record_buf), fp);
    fclose(fp);

    if (file_len < sizeof(ds_record_file_hdr_t))
    {
        rsp[0] = DS_UART_STATUS_OK;
        rsp[1] = 0;
        return 2;
    }

    /* 解析文件头 */
    {
        ds_record_file_hdr_t *hdr = (ds_record_file_hdr_t *)record_buf;
        if (hdr->magic != DS_RECORD_FILE_MAGIC || hdr->version != DS_RECORD_FILE_VER || hdr->count == 0)
        {
            rsp[0] = DS_UART_STATUS_OK;
            rsp[1] = 0;
            return 2;
        }

        /* 跳过第一条记录：从 hdr 之后扫描 entries */
        uint16_t off = sizeof(ds_record_file_hdr_t);
        uint8_t  i;

        if (off + sizeof(ds_record_file_entry_hdr_t) > file_len)
        {
            rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
            return 1;
        }

        {
            ds_record_file_entry_hdr_t *e = (ds_record_file_entry_hdr_t *)&record_buf[off];
            uint8_t entry_cnt = (e->entry_cnt <= DS_RECORD_ENTRY_MAX) ? e->entry_cnt : 0;
            uint16_t rec_size = (uint16_t)(sizeof(ds_record_file_entry_hdr_t) + entry_cnt * sizeof(ds_record_entry_t));
            off += rec_size;
        }

        /* 剩余数据量 = 后续记录内容 */
        remain_len = (off < file_len) ? (uint16_t)(file_len - off) : 0;
        hdr->count--;

        /* 写回文件：剩余记录 + 更新后的文件头 */
        fp = fopen(DS_COMFORT_DB_PATH, "wb");
        if (!fp)
        {
            rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
            return 1;
        }
        fwrite(hdr, sizeof(ds_record_file_hdr_t), 1, fp);
        if (remain_len > 0)
        {
            fwrite(&record_buf[off], 1, remain_len, fp);
        }
        fclose(fp);

        /* 同步更新内存缓存中的记录计数 */
        if (g_record_cnt > 0)
        {
            g_record_cnt--;
            if (g_record_cnt == 0)
            {
                g_record_head = 0;
                memset(g_records, 0, sizeof(g_records));
            }
        }

        rsp[0] = DS_UART_STATUS_OK;
        rsp[1] = hdr->count;
        LOG_INFO("delete_oldest from file: remaining=%u\n", hdr->count);
        return 2;
    }
}

uint16_t comfort_store_factory_reset(/* 删主人录音、清记录、恢复默认策略 */
                                     const uint8_t *req,
                                     uint16_t req_len,
                                     uint8_t *rsp,
                                     uint16_t rsp_cap)
{
    (void)req;
    (void)req_len;

    audio_delete_owner_rec();
    LOG_INFO("state_change: owner_voice_exist %u -> 0 (factory_reset)\n", g_rt.owner_voice_exist);
    g_rt.owner_voice_exist = 0;
    g_rt.owner_duration_sec = 0;

    /* 清空安抚记录 */
    g_record_cnt = 0;
    g_record_head = 0;
    memset(g_records, 0, sizeof(g_records));
    comfort_store_save_records();

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
