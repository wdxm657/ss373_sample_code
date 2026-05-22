#ifndef DOG_SOOTHER_BARK_CONTROL_H_
#define DOG_SOOTHER_BARK_CONTROL_H_

#include <stdint.h>

int bark_control_init(void);
void bark_control_deinit(void);
void bark_control_tick(void);

void bark_control_set_power(uint8_t on);
void bark_control_post_work_state(uint8_t reason);

uint16_t bark_control_set_mode(
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

uint16_t bark_control_handle_strategy_cmd(
    uint8_t cmd_id,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

#endif /* DOG_SOOTHER_BARK_CONTROL_H_ */
