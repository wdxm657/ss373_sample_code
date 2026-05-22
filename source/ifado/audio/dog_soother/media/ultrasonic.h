#ifndef DOG_SOOTHER_ULTRASONIC_H_
#define DOG_SOOTHER_ULTRASONIC_H_

#include <stdint.h>

int ultrasonic_init(void);
void ultrasonic_deinit(void);

/* 按 profile 发射超声波，duration_ms 为持续时间（占位实现可仅打日志） */
int ultrasonic_emit(uint8_t profile, unsigned int duration_ms);

#endif /* DOG_SOOTHER_ULTRASONIC_H_ */
