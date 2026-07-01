#ifndef DOG_SOOTHER_ULTRASONIC_H_
#define DOG_SOOTHER_ULTRASONIC_H_

#include <stdint.h>

int ultrasonic_init(void);
void ultrasonic_deinit(void);

/* 通过 UART 通知 MCU 启动超声波发射 */
int ultrasonic_emit(uint8_t profile, unsigned int duration_ms);

/* 通过 UART 通知 MCU 停止超声波发射 */
int ultrasonic_stop(void);

#endif /* DOG_SOOTHER_ULTRASONIC_H_ */
