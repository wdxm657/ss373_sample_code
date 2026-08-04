#ifndef DOG_SOOTHER_COMFORT_STORE_H_
#define DOG_SOOTHER_COMFORT_STORE_H_

#include <stdint.h>

#include "app_types.h"

/* ========== 安抚记录（按条目） ========== */

/* 记录条目类型 */
#define DS_RECORD_ENTRY_BARK        0x01  /* 狗叫识别 */
#define DS_RECORD_ENTRY_MUSIC       0x02  /* 安抚音乐 */
#define DS_RECORD_ENTRY_OWNER       0x03  /* 主人音频 */
#define DS_RECORD_ENTRY_US_25K      0x04  /* 超声波1（25KHz） */
#define DS_RECORD_ENTRY_US_30K      0x05  /* 超声波2（30KHz） */
#define DS_RECORD_ENTRY_US_DUAL     0x06  /* 超声波3（双频） */
#define DS_RECORD_ENTRY_SNACK       0x07  /* 零食投喂 */
#define DS_RECORD_ENTRY_SUCCESS     0x10  /* 安抚成功 */
#define DS_RECORD_ENTRY_FAIL        0x11  /* 安抚失败 */

/* 单条记录最多条目数（1 吠叫 + 最多 6 措施 + 1 结果） */
#define DS_RECORD_ENTRY_MAX         8

/* 内存保留的最大记录数（超过时覆盖最旧记录） */
#define DS_RECORD_MAX               128

/* 单次 UART RSP 分片最多携带的 entry 数（64B payload 最多装 12 条） */
#define DS_RECORD_CHUNK_MAX         10

#pragma pack(push, 1)
typedef struct
{
    uint8_t  type;   /* DS_RECORD_ENTRY_* */
    uint32_t ts;     /* 时间戳（epoch sec） */
} ds_record_entry_t;

typedef struct
{
    uint32_t session_id;
    uint32_t start_ts;                     /* 首次识别时间戳 */
    uint32_t end_ts;                       /* 结束时间戳 */
    uint8_t  entry_cnt;                    /* 有效条目数 */
    ds_record_entry_t entries[DS_RECORD_ENTRY_MAX];
} ds_session_record_t;
#pragma pack(pop)

/* ========== 运行时状态 API ========== */

int comfort_store_init(void);
void comfort_store_deinit(void);

/* 填充 STATUS_GET 响应体（9 字节，首字节为 status） */
void comfort_store_fill_status_payload(uint8_t *out, uint16_t out_cap, uint16_t *out_len);

void comfort_store_set_power(uint8_t on);
uint8_t comfort_store_get_power(void);
uint8_t comfort_store_get_bt_linked(void);

void comfort_store_set_bt_linked(uint8_t linked);

uint16_t comfort_store_pull_records(
    const uint8_t *req,
    uint16_t req_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

/**
 * @brief 按 session_id 删除一条安抚记录
 * @param req      请求体: [session_id(4LE)]
 * @param req_len  请求长度
 * @param rsp      响应缓冲区: [status]
 * @param rsp_cap  响应缓冲区容量
 * @return 响应体长度
 */
uint16_t comfort_store_delete_record_by_id(
    const uint8_t *req,
    uint16_t req_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

uint16_t comfort_store_factory_reset(
    const uint8_t *req,
    uint16_t req_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

/**
 * @brief 检查是否有安抚记录存储
 * @return 1=有记录，0=无记录
 */
uint8_t comfort_store_has_records(void);

ds_work_state_t comfort_store_get_work_state(void);
void comfort_store_set_work_state(ds_work_state_t state);

void comfort_store_set_calm_runtime(ds_calm_mode_t mode, uint8_t enabled_mask, uint8_t us_mask);

void comfort_store_apply_strategy(void);

/* ========== 记录生成 API（供 bark_control 调用） ========== */

/**
 * @brief 开始一条新安抚记录（session 启动时调用）
 * @param session_id   会话 ID
 * @param bark_ts      首次吠叫识别时间戳
 */
void comfort_store_record_begin(uint32_t session_id, uint32_t bark_ts);

/**
 * @brief 追加一条执行条目（每步措施开始时调用）
 * @param session_id  会话 ID
 * @param type        条目类型（DS_RECORD_ENTRY_MUSIC / OWNER / US_*）
 * @param ts          执行时间戳
 */
void comfort_store_record_append_measure(uint32_t session_id, uint8_t type, uint32_t ts);

/**
 * @brief 完成一条安抚记录（session 结束时调用）
 * @param session_id  会话 ID
 * @param success     1=成功 0=失败
 * @param end_ts      结束时间戳
 */
void comfort_store_record_finish(uint32_t session_id, uint8_t success, uint32_t end_ts);

/**
 * @brief 丢弃一条未完成的安抚记录（从内存缓存移除，不写入文件）
 * @param session_id 会话 ID
 */
void comfort_store_discard_record(uint32_t session_id);

/**
 * @brief 将措施类型映射为记录条目类型
 * @param measure     措施类型（DS_MEASURE_MUSIC / OWNER_VOICE / ULTRASONIC / SNACK_FEED）
 * @param us_profile  超声波档位，仅 measure=ULTRASONIC 时有效
 * @return DS_RECORD_ENTRY_* 值，0 表示无效映射
 */
uint8_t comfort_store_measure_to_entry_type(uint8_t measure, uint8_t us_profile);

#endif /* DOG_SOOTHER_COMFORT_STORE_H_ */
