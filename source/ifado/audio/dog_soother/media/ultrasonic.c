#define LOG_TAG "ultrasonic"
#include "log.h"

#include "ultrasonic.h"

int ultrasonic_init(void)
{
    return 0;
}

void ultrasonic_deinit(void)
{
}

int ultrasonic_emit(uint8_t profile, unsigned int duration_ms)
{
    LOG_INFO("ultrasonic emit profile=%u duration=%u ms (stub)\n", profile, duration_ms);
    return 0;
}
