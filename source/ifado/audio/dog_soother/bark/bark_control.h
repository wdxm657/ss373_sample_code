#ifndef DOG_SOOTHER_BARK_CONTROL_H_
#define DOG_SOOTHER_BARK_CONTROL_H_

#include <stdint.h>

int bark_control_init(void);
void bark_control_deinit(void);

void bark_control_set_power(uint8_t on);

/* MCU 同步零食奖励功能开关（0=关 1=开），奖励投喂在安抚成功时由状态机判定 */
void bark_control_set_reward_enabled(uint8_t on);

void bark_control_post_work_state(uint8_t reason);

/* 主循环每秒调用：监听窗、措施轮询、休息倒计时 */
void bark_control_tick(void);

/* bark_detect 每窗推理后调用；Top-K 已判定为狗叫时 hit=1 */
void bark_control_on_window_bark(int hit, uint32_t epoch_sec);

uint16_t bark_control_set_mode(
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

/* 安抚会话是否进行中（从第一次狗叫识别到休息开始）*/
uint8_t bark_control_is_session_active(void);

uint16_t bark_control_handle_strategy_cmd(
    uint8_t cmd_id,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

#endif /* DOG_SOOTHER_BARK_CONTROL_H_ */
