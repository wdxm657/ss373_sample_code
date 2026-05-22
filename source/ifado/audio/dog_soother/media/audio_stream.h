#ifndef DOG_SOOTHER_AUDIO_STREAM_H_
#define DOG_SOOTHER_AUDIO_STREAM_H_

#include <stddef.h>
#include <stdint.h>

#define AUDIO_STREAM_FRAME_MAX_SAMPLES 4096

/* 一帧 AI PCM，供 detect/rec 队列传递 */
typedef struct
{
    size_t num_samples;
    int16_t pcm[AUDIO_STREAM_FRAME_MAX_SAMPLES];
} audio_stream_frame_t;

/* AI 就绪后启动采集线程：读 PCM → detect 队列（满则丢最旧）/ rec 队列（仅录制时） */
int audio_stream_start(void);
void audio_stream_stop(void);

/* audio 注册：返回非 0 时采集线程向 rec 队列投递 */
void audio_stream_set_rec_active_fn(int (*fn)(void));

/*
 * 识别/录制线程阻塞取帧。
 * 返回 0 成功；1 队列已关闭；负值 参数错误。
 */
int audio_stream_detect_pop(audio_stream_frame_t *out);
int audio_stream_rec_pop(audio_stream_frame_t *out);

#endif /* DOG_SOOTHER_AUDIO_STREAM_H_ */
