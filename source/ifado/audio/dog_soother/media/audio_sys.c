/** @file audio_sys.c @brief MI_SYS_Init / Exit 封装 */
#define LOG_TAG "audio_sys"
#include "log.h"

#include "audio_sys.h"

#include "mi_sys.h"
#include "st_common.h"

static int g_sys_inited; /* 引用计数式：AI/AO 共用，由 audio_deinit 最后 Exit */

int audio_sys_init(void) /* MI_SYS_Init(0) */
{
    if (g_sys_inited)
    {
        return 0;
    }
    MI_S32 ret = MI_SYS_Init(0);
    if (ret != MI_SUCCESS)
    {
        LOG_ERROR("MI_SYS_Init failed 0x%x\n", ret);
        return -1;
    }
    g_sys_inited = 1;
    LOG_INFO("MI_SYS_Init ok\n");
    return 0;
}

void audio_sys_deinit(void) /* MI_SYS_Exit(0) */
{
    if (!g_sys_inited)
    {
        return;
    }
    MI_SYS_Exit(0);
    g_sys_inited = 0;
}
