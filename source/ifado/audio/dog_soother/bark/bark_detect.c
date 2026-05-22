#define LOG_TAG "bark_detect"
#include "log.h"

#include "bark_detect.h"
#include "app_config.h"

/* TODO: 从 legacy/test_yamnet.c 抽取 MI AI + YAMNet 滑窗推理 */

int bark_detect_init(void)
{
    LOG_INFO("bark_detect init (stub), model=%s\n", DS_YAMNET_MODEL_PREFIX);
    return 0;
}

void bark_detect_deinit(void)
{
}

void bark_detect_tick(void)
{
}
