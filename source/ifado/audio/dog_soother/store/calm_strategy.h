#ifndef DOG_SOOTHER_CALM_STRATEGY_H_
#define DOG_SOOTHER_CALM_STRATEGY_H_

#include <stdint.h>

#include "app_types.h"

#define DS_STRATEGY_MEASURE_MAX 3
#define DS_STRATEGY_US_MAX 3

typedef struct
{
    ds_calm_mode_t mode;
    uint8_t enabled_mask;
    uint8_t measure_cnt;
    uint8_t measure_order[DS_STRATEGY_MEASURE_MAX];
    uint8_t us_cnt;
    uint8_t us_order[DS_STRATEGY_US_MAX];
} calm_strategy_t;

int calm_strategy_init(void);
void calm_strategy_deinit(void);

const calm_strategy_t *calm_strategy_get(void);

int calm_strategy_load(ds_calm_mode_t mode);
int calm_strategy_save(void);

void calm_strategy_factory_reset(void);

int calm_strategy_set_from_uart_payload(const uint8_t *payload, uint16_t len);

uint16_t calm_strategy_fill_get_rsp(ds_calm_mode_t mode, uint8_t *rsp, uint16_t rsp_cap);

const char *calm_strategy_get_path_by_mode(ds_calm_mode_t mode);

void calm_strategy_apply_success(uint8_t ok_measure, uint8_t ok_us_sub);

int calm_strategy_set_mode(ds_calm_mode_t mode);

#endif /* DOG_SOOTHER_CALM_STRATEGY_H_ */
