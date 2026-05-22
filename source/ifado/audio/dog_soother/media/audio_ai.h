#ifndef DOG_SOOTHER_AUDIO_AI_H_
#define DOG_SOOTHER_AUDIO_AI_H_

#include <stddef.h>
#include <stdint.h>

int audio_ai_init(int gain_db);
void audio_ai_deinit(void);
int audio_ai_is_ready(void);

int audio_ai_set_gain(int gain_db);

/* 退出前调用：Disable ChnGroup 使阻塞中的 MI_AI_Read 返回，采集线程可结束 */
void audio_ai_stop_capture(void);

/* 读一帧 PCM（16kHz mono s16le）；成功返回 0 */
int audio_ai_read_pcm(int16_t *dst, size_t max_samples, size_t *out_samples);

#endif /* DOG_SOOTHER_AUDIO_AI_H_ */
