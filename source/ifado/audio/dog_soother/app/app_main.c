/**
 * @file app_main.c
 * @brief 模块初始化、UART 启动、主线程 pause 等待 SIGINT
 */
#define LOG_TAG "app_main"
#include "log.h"

#include "app_main.h"
#include "app_config.h"

#include "uart_proto.h"
#include "uart_dispatch.h"
#include "comfort_store.h"
#include "bark_control.h"
#include "audio.h"
#include "ultrasonic.h"
#include "led.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile int g_running = 1; /* SIGINT/SIGTERM 置 0 退出 */

/* 信号处理：请求主循环退出 */
static void on_signal(int sig)
{
    (void)sig;
    g_running = 0;
}

static void print_usage(const char *prog) /* 打印 -D/--device 等帮助 */
{
    printf("Usage: %s [options]\n", prog);
    printf("  -D, --device PATH   UART device (default %s, env DS_UART_DEVICE)\n", DS_UART_DEVICE);
    printf("  -h, --help          Show help\n");
    printf("Env: DS_UART_DEBUG=1  hex log for TX/RX frames\n");
}

/* 优先级：环境变量 DS_UART_DEVICE > 命令行 -D > app_config 默认 */
static const char *pick_uart_device(int argc, char **argv)
{
    const char *env = getenv("DS_UART_DEVICE");
    if (env && env[0])
    {
        return env;
    }
    for (int i = 1; i + 1 < argc; i++)
    {
        if (strcmp(argv[i], "-D") == 0 || strcmp(argv[i], "--device") == 0)
        {
            return argv[i + 1];
        }
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            print_usage(argv[0]);
            exit(0);
        }
    }
    return DS_UART_DEVICE;
}

int app_main_run(int argc, char **argv)
{
    /* 采集/录制/识别在各自线程运行，本线程仅阻塞等待退出 */
    const char *uart_dev = pick_uart_device(argc, argv);

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    LOG_INFO("dog_soother start, uart=%s baud=%u\n", uart_dev, DS_UART_BAUDRATE);

    /* 顺序：状态存储 → 音频(含采集/录) → 超声占位 → 识别线程；UART 在音频就绪后 */
    if (led_init() != 0 || comfort_store_init() != 0 || audio_init() != 0 || ultrasonic_init() != 0 || bark_control_init() != 0)
    {
        LOG_ERROR("module init failed\n");
        return 1;
    }

    if (uart_proto_open(uart_dev, DS_UART_BAUDRATE) != 0)
    {
        LOG_ERROR("uart open failed\n");
        return 1;
    }

    if (uart_dispatch_init() != 0)
    {
        LOG_ERROR("uart dispatch init failed\n");
        uart_proto_deinit();
        return 1;
    }

    if (uart_proto_start_rx() != 0)
    {
        LOG_ERROR("uart rx start failed\n");
        uart_proto_deinit();
        return 1;
    }

    /* 设置初始 LED 灯色 */
    led_indicate_state(comfort_store_get_power(), comfort_store_get_bt_linked());

    // bark_control_post_work_state(0);

    LOG_INFO("main idle (capture/rec/detect threads running), Ctrl+C to exit\n");
    {
        uint8_t last_power = 0xFF, last_bt = 0xFF;
        while (g_running)
        {
            bark_control_tick();
            uint8_t p = comfort_store_get_power();
            uint8_t b = comfort_store_get_bt_linked();
            if (p != last_power || b != last_bt)
            {
                last_power = p;
                last_bt    = b;
                led_indicate_state(p, b);
            }
            sleep(1);
        }
    }

    /* 先停 UART；再停音频流（关闭 detect/rec 队列）；最后停识别线程 */
    uart_dispatch_deinit();
    uart_proto_deinit();
    ultrasonic_deinit();
    led_deinit();
    audio_deinit();
    bark_control_deinit();
    comfort_store_deinit();

    LOG_INFO("dog_soother exit\n");
    return 0;
}
