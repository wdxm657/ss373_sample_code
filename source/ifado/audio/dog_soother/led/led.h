#ifndef DOG_SOOTHER_LED_H_
#define DOG_SOOTHER_LED_H_

#include <stdint.h>

/* 预定义颜色 */
#define LED_COLOR_OFF   0x00
#define LED_COLOR_RED   0x01
#define LED_COLOR_GREEN 0x02
#define LED_COLOR_BLUE  0x04
#define LED_COLOR_YELLOW (LED_COLOR_RED | LED_COLOR_GREEN)
#define LED_COLOR_CYAN   (LED_COLOR_GREEN | LED_COLOR_BLUE)
#define LED_COLOR_PURPLE (LED_COLOR_RED | LED_COLOR_BLUE)
#define LED_COLOR_WHITE  (LED_COLOR_RED | LED_COLOR_GREEN | LED_COLOR_BLUE)

/**
 * @brief     初始化 LED GPIO（export + direction out）
 * @return    0=成功
 */
int led_init(void);

/**
 * @brief     设置 RGB 颜色（立即生效，停止闪烁）
 * @param color LED_COLOR_xxx 组合
 */
void led_set_color(uint8_t color);

/**
 * @brief     关闭所有 LED（清色 + 停止闪烁）
 */
void led_off(void);

/**
 * @brief     开始闪烁（使用当前颜色，需先调用 led_set_color）
 * @param period_ms  闪烁周期（毫秒），如 500=500ms 亮 500ms 灭
 * @return    0=成功
 */
int led_blink_start(uint32_t period_ms);

/**
 * @brief     停止闪烁，保持当前颜色常亮
 */
void led_blink_stop(void);

/**
 * @brief     根据电源和蓝牙状态指示 LED
 * @param power_on  0=关机, 1=开机
 * @param bt_linked 0=蓝牙未连, 1=蓝牙已连
 *
 * 状态→显示规则：
 *   关机+已连 → 蓝灯常亮
 *   关机+未连 → 蓝灯闪烁
 *   开机+已连 → 绿灯常亮
 *   开机+未连 → 绿灯闪烁
 */
void led_indicate_state(uint8_t power_on, uint8_t bt_linked);

/**
 * @brief     释放 LED GPIO（unexport）
 */
void led_deinit(void);

#endif /* DOG_SOOTHER_LED_H_ */
