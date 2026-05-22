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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile int g_running = 1;

static void on_signal(int sig)
{
    (void)sig;
    g_running = 0;
}

static void print_usage(const char *prog)
{
    printf("Usage: %s [options]\n", prog);
    printf("  -D, --device PATH   UART device (default %s, env DS_UART_DEVICE)\n", DS_UART_DEVICE);
    printf("  -h, --help          Show help\n");
    printf("Env: DS_UART_DEBUG=1  hex log for TX/RX frames\n");
}

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
    const char *uart_dev = pick_uart_device(argc, argv);

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    LOG_INFO("dog_soother start, uart=%s baud=%u\n", uart_dev, DS_UART_BAUDRATE);

    if (comfort_store_init() != 0 || audio_init() != 0 || ultrasonic_init() != 0 || bark_control_init() != 0)
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

    /* 上电通知 MCU：当前为监测态 */
    bark_control_post_work_state(0);

    while (g_running)
    {
        bark_control_tick();
        audio_tick();
        usleep(50000);
    }

    uart_dispatch_deinit();
    uart_proto_deinit();
    bark_control_deinit();
    ultrasonic_deinit();
    audio_deinit();
    comfort_store_deinit();

    LOG_INFO("dog_soother exit\n");
    return 0;
}
