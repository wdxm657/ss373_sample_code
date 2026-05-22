#ifndef DOG_SOOTHER_SYSTEM_TIME_H_
#define DOG_SOOTHER_SYSTEM_TIME_H_

#include <stdint.h>

uint16_t system_time_apply(
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

#endif /* DOG_SOOTHER_SYSTEM_TIME_H_ */
