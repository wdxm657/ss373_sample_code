#ifndef DOG_SOOTHER_AUDIO_H_
#define DOG_SOOTHER_AUDIO_H_

#include <stdint.h>

/* 初始化 MI 音频、采集/录制线程 */
int audio_init(void);
void audio_deinit(void);

/* 从 owner.wav 刷新是否存在及时长（秒） */
void audio_refresh_owner_info(uint8_t *exist, uint8_t *duration_sec);
void audio_delete_owner_rec(void);

/* 从硬件读取当前音量（0~30） */
uint8_t audio_get_volume(void);

/* VOLUME_SET：解析 payload，写 rsp，返回 rsp 长度 */
uint16_t audio_set_volume(
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

/* 主人录音相关 UART 命令统一入口 */
uint16_t audio_handle_uart_cmd(
    uint8_t cmd_id,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap);

#endif /* DOG_SOOTHER_AUDIO_H_ */
