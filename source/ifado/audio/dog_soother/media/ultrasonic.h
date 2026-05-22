#ifndef DOG_SOOTHER_ULTRASONIC_H_
#define DOG_SOOTHER_ULTRASONIC_H_

#include <stdint.h>

int ultrasonic_init(void);
void ultrasonic_deinit(void);

int ultrasonic_emit(uint8_t profile, unsigned int duration_ms);

#endif /* DOG_SOOTHER_ULTRASONIC_H_ */
