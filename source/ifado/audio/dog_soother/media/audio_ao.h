#ifndef DOG_SOOTHER_AUDIO_AO_H_
#define DOG_SOOTHER_AUDIO_AO_H_

#include <stdint.h>

int audio_ao_init(void);
void audio_ao_deinit(void);
int audio_ao_is_ready(void);

/* APP 音量档位 0~30 映射到驱动增益 */
int audio_ao_set_volume_level(uint8_t level_0_30);
int audio_ao_get_volume_level(uint8_t *level_0_30);
int audio_ao_set_gain_db(int8_t db);  /* 直接设置增益 -60..30 dB */

/* 同步播放 WAV（16bit PCM）；可被 audio_ao_stop 打断 */
int audio_ao_play_wav_file(const char *path);

void audio_ao_stop(void);
int audio_ao_is_playing(void);

#endif /* DOG_SOOTHER_AUDIO_AO_H_ */
