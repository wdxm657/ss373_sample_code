#ifndef DOG_SOOTHER_COMFORT_STORE_H_
#define DOG_SOOTHER_COMFORT_STORE_H_

#include <stdint.h>

#include "app_types.h"

int comfort_store_init(void);
void comfort_store_deinit(void);

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

#endif /* DOG_SOOTHER_COMFORT_STORE_H_ */
