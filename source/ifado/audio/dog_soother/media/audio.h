#ifndef DOG_SOOTHER_AUDIO_H_
#define DOG_SOOTHER_AUDIO_H_

#include <stdint.h>

int audio_init(void);
void audio_deinit(void);
void audio_tick(void);

void audio_refresh_owner_info(uint8_t *exist, uint8_t *duration_sec);
void audio_delete_owner_rec(void);

uint16_t audio_set_volume(
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

uint16_t audio_handle_uart_cmd(
    uint8_t cmd_id,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

#endif /* DOG_SOOTHER_AUDIO_H_ */
