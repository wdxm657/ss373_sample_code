/**
 * @file led.c
 * @brief RGB LED 控制（sysfs GPIO，保持文件描述符常开避免闪屏延迟）
 *
 * GPIO 映射：R=GPIO10, G=GPIO9, B=GPIO11
 */
#define LOG_TAG "led"
#include "log.h"

#include "led.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* ========== GPIO 引脚定义 ========== */
#define LED_R_GPIO 10
#define LED_G_GPIO 9
#define LED_B_GPIO 11

#define GPIO_BASE "/sys/class/gpio"
#define GPIO_EXPORT GPIO_BASE "/export"
#define GPIO_UNEXPORT GPIO_BASE "/unexport"

/* ========== 内部状态 ========== */
static const int g_gpios[3] = {LED_R_GPIO, LED_G_GPIO, LED_B_GPIO};
static volatile uint8_t g_current_color = 0;

/* 文件描述符缓存（避免每次写都 fopen/fclose 引入延迟）*/
static FILE *g_fp_r = NULL;
static FILE *g_fp_g = NULL;
static FILE *g_fp_b = NULL;

/* 闪烁线程 */
static volatile uint8_t g_blink_running = 0;
static pthread_t g_blink_thread;
static uint32_t g_blink_period_ms = 0;
static pthread_mutex_t g_led_mu = PTHREAD_MUTEX_INITIALIZER;

/* ========== sysfs GPIO 底层操作 ========== */

static int gpio_export(int gpio)
{
    FILE *fp = fopen(GPIO_EXPORT, "w");
    if (!fp) return -1;
    fprintf(fp, "%d", gpio);
    fclose(fp);
    return 0;
}

static int gpio_unexport(int gpio)
{
    FILE *fp = fopen(GPIO_UNEXPORT, "w");
    if (!fp) return -1;
    fprintf(fp, "%d", gpio);
    fclose(fp);
    return 0;
}

static int gpio_set_dir(int gpio, const char *dir)
{
    char path[64];
    FILE *fp;
    snprintf(path, sizeof(path), GPIO_BASE "/gpio%d/direction", gpio);
    fp = fopen(path, "w");
    if (!fp) return -1;
    fprintf(fp, "%s", dir);
    fclose(fp);
    return 0;
}

/* 打开 GPIO value 文件并缓存 FILE*，后续直接 fprintf */
static FILE *gpio_open_value(int gpio)
{
    char path[64];
    snprintf(path, sizeof(path), GPIO_BASE "/gpio%d/value", gpio);
    return fopen(path, "w");
}

/* 写 value（使用缓存的 FILE*）*/
static void gpio_write_fp(FILE *fp, int val)
{
    if (fp)
    {
        fprintf(fp, "%d\n", val ? 1 : 0);
        fflush(fp);
    }
}

/* ========== 内部函数 ========== */

static void led_apply_color(uint8_t color)
{
    gpio_write_fp(g_fp_r, (color & LED_COLOR_RED) ? 1 : 0);
    gpio_write_fp(g_fp_g, (color & LED_COLOR_GREEN) ? 1 : 0);
    gpio_write_fp(g_fp_b, (color & LED_COLOR_BLUE) ? 1 : 0);
}

/* 闪烁线程：使用 nanosleep 提高精度 */
static void *led_blink_thread(void *arg)
{
    (void)arg;
    uint32_t half_ms;
    struct timespec ts;

    while (g_blink_running)
    {
        half_ms = g_blink_period_ms / 2;
        if (half_ms < 50) half_ms = 50;

        pthread_mutex_lock(&g_led_mu);
        led_apply_color(g_current_color);
        pthread_mutex_unlock(&g_led_mu);

        ts.tv_sec  = half_ms / 1000;
        ts.tv_nsec = (long)(half_ms % 1000) * 1000000L;
        nanosleep(&ts, NULL);

        if (!g_blink_running) break;

        pthread_mutex_lock(&g_led_mu);
        led_apply_color(LED_COLOR_OFF);
        pthread_mutex_unlock(&g_led_mu);

        ts.tv_sec  = half_ms / 1000;
        ts.tv_nsec = (long)(half_ms % 1000) * 1000000L;
        nanosleep(&ts, NULL);
    }
    return NULL;
}

/* ========== 对外 API ========== */

int led_init(void)
{
    int i;

    for (i = 0; i < 3; i++)
    {
        gpio_unexport(g_gpios[i]);
        usleep(10000);

        if (gpio_export(g_gpios[i]) != 0)
        {
            LOG_ERROR("gpio %d export failed\n", g_gpios[i]);
            return -1;
        }
        usleep(10000);

        if (gpio_set_dir(g_gpios[i], "out") != 0)
        {
            LOG_ERROR("gpio %d set direction failed\n", g_gpios[i]);
            return -1;
        }
    }

    /* 打开 value 文件并保持 */
    g_fp_r = gpio_open_value(LED_R_GPIO);
    g_fp_g = gpio_open_value(LED_G_GPIO);
    g_fp_b = gpio_open_value(LED_B_GPIO);
    if (!g_fp_r || !g_fp_g || !g_fp_b)
    {
        LOG_ERROR("led open value files failed\n");
        return -1;
    }

    led_apply_color(LED_COLOR_OFF);
    g_current_color = LED_COLOR_OFF;
    LOG_INFO("led init ok (R=GPIO%d G=GPIO%d B=GPIO%d)\n",
             LED_R_GPIO, LED_G_GPIO, LED_B_GPIO);
    return 0;
}

void led_set_color(uint8_t color)
{
    led_blink_stop();
    pthread_mutex_lock(&g_led_mu);
    g_current_color = color & 0x07;
    led_apply_color(g_current_color);
    pthread_mutex_unlock(&g_led_mu);
    LOG_INFO("led set color=0x%02x\n", g_current_color);
}

void led_off(void)
{
    led_blink_stop();
    pthread_mutex_lock(&g_led_mu);
    g_current_color = LED_COLOR_OFF;
    led_apply_color(LED_COLOR_OFF);
    pthread_mutex_unlock(&g_led_mu);
    LOG_INFO("led off\n");
}

int led_blink_start(uint32_t period_ms)
{
    if (period_ms < 100) period_ms = 100;

    led_blink_stop();

    g_blink_period_ms = period_ms;
    g_blink_running = 1;

    if (pthread_create(&g_blink_thread, NULL, led_blink_thread, NULL) != 0)
    {
        g_blink_running = 0;
        LOG_ERROR("led blink thread create failed\n");
        return -1;
    }
    pthread_detach(g_blink_thread);
    LOG_INFO("led blink start period=%ums\n", period_ms);
    return 0;
}

void led_blink_stop(void)
{
    if (g_blink_running)
    {
        g_blink_running = 0;
        usleep(g_blink_period_ms * 1000 / 2 + 10000);
        pthread_mutex_lock(&g_led_mu);
        led_apply_color(g_current_color);
        pthread_mutex_unlock(&g_led_mu);
    }
}

void led_indicate_state(uint8_t power_on, uint8_t bt_linked)
{
    if (power_on)
    {
        if (bt_linked)
        {
            led_blink_stop();
            led_set_color(LED_COLOR_GREEN);
        }
        else
        {
            led_set_color(LED_COLOR_GREEN);
            led_blink_start(500);
        }
    }
    else
    {
        if (bt_linked)
        {
            led_blink_stop();
            led_set_color(LED_COLOR_BLUE);
        }
        else
        {
            led_set_color(LED_COLOR_BLUE);
            led_blink_start(500);
        }
    }
}

void led_deinit(void)
{
    led_off();
    usleep(50000);
    if (g_fp_r) fclose(g_fp_r);
    if (g_fp_g) fclose(g_fp_g);
    if (g_fp_b) fclose(g_fp_b);
    gpio_unexport(LED_R_GPIO);
    gpio_unexport(LED_G_GPIO);
    gpio_unexport(LED_B_GPIO);
    LOG_INFO("led deinit\n");
}
