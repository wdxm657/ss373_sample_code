#ifndef DOG_SOOTHER_COMFORT_STORE_H_
#define DOG_SOOTHER_COMFORT_STORE_H_

#include <stdint.h>

#include "app_types.h"

int comfort_store_init(void);
void comfort_store_deinit(void);

/* 填充 STATUS_GET 响应体（9 字节，首字节为 status） */
void comfort_store_fill_status_payload(uint8_t *out, uint16_t out_cap, uint16_t *out_len);

void comfort_store_set_power(uint8_t on);
uint8_t comfort_store_get_power(void);

void comfort_store_set_bt_linked(uint8_t linked);

uint16_t comfort_store_pull_records(
    const uint8_t *req,
    uint16_t req_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

uint16_t comfort_store_factory_reset(
    const uint8_t *req,
    uint16_t req_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

ds_work_state_t comfort_store_get_work_state(void);
void comfort_store_set_work_state(ds_work_state_t state);

void comfort_store_set_calm_runtime(ds_calm_mode_t mode, uint8_t enabled_mask, uint8_t us_mask);

void comfort_store_apply_strategy(void);

#endif /* DOG_SOOTHER_COMFORT_STORE_H_ */
