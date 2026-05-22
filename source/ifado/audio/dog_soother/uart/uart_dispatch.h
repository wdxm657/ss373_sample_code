#ifndef DOG_SOOTHER_UART_DISPATCH_H_
#define DOG_SOOTHER_UART_DISPATCH_H_

#include <stdint.h>

int uart_dispatch_init(void);
void uart_dispatch_deinit(void);

void uart_dispatch_on_frame(
    uint8_t msg_type,
    uint8_t cmd_id,
    uint8_t seq,
    const uint8_t *payload,
    uint16_t payload_len);

#endif /* DOG_SOOTHER_UART_DISPATCH_H_ */
