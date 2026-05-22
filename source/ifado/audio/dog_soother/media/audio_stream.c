/**
 * @file audio_stream.c
 * @brief 采集线程：AI 读帧 → detect/rec 有界队列（detect 满丢最旧）
 */
#define LOG_TAG "audio_stream"
#include "log.h"

#include "audio_stream.h"
#include "audio_ai.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DETECT_Q_CAP 16 /* 识别队列深度，满则丢最旧 */
#define REC_Q_CAP 32    /* 录制队列深度，满则阻塞采集线程 */

/* 环形帧队列，供 detect/rec 线程阻塞 pop */
typedef struct
{
    pthread_mutex_t mu;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    int head;  /* 出队下标 */
    int tail;  /* 入队下标 */
    int count; /* 当前帧数 */
    int closed; /* stop 时置 1，唤醒等待线程 */
    int cap;
    audio_stream_frame_t *slots;
} frame_queue_t;

static volatile int g_run;
static pthread_t g_capture_thread;
static frame_queue_t g_detect_q;
static frame_queue_t g_rec_q;
static int (*g_rec_active_fn)(void); /* 非 NULL 且返回 1 时向 rec 队列 push */

static int frame_queue_init(frame_queue_t *q, int cap) /* 分配 slots 与 cond */
{
    memset(q, 0, sizeof(*q));
    q->cap = cap;
    q->slots = (audio_stream_frame_t *)calloc((size_t)cap, sizeof(audio_stream_frame_t));
    if (!q->slots)
    {
        return -1;
    }
    if (pthread_mutex_init(&q->mu, NULL) != 0)
    {
        free(q->slots);
        q->slots = NULL;
        return -1;
    }
    if (pthread_cond_init(&q->not_empty, NULL) != 0 ||
        pthread_cond_init(&q->not_full, NULL) != 0)
    {
        pthread_mutex_destroy(&q->mu);
        free(q->slots);
        q->slots = NULL;
        return -1;
    }
    return 0;
}

static void frame_queue_destroy(frame_queue_t *q) /* 释放队列资源 */
{
    if (!q)
    {
        return;
    }
    pthread_mutex_destroy(&q->mu);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
    free(q->slots);
    memset(q, 0, sizeof(*q));
}

static void frame_queue_close(frame_queue_t *q) /* 置 closed，唤醒 pop 线程 */
{
    pthread_mutex_lock(&q->mu);
    q->closed = 1;
    pthread_cond_broadcast(&q->not_empty);
    pthread_cond_broadcast(&q->not_full);
    pthread_mutex_unlock(&q->mu);
}

static int frame_queue_push(frame_queue_t *q, const audio_stream_frame_t *frame, int drop_oldest) /* 满时可选丢 head */
{
    pthread_mutex_lock(&q->mu);
    while (q->count >= q->cap && g_run && !q->closed)
    {
        if (!drop_oldest)
        {
            pthread_cond_wait(&q->not_full, &q->mu);
        }
        else
        {
            q->head = (q->head + 1) % q->cap;
            q->count--;
        }
    }
    if (!g_run || q->closed)
    {
        pthread_mutex_unlock(&q->mu);
        return -1;
    }
    q->slots[q->tail] = *frame;
    q->tail = (q->tail + 1) % q->cap;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mu);
    return 0;
}

static int frame_queue_pop(frame_queue_t *q, audio_stream_frame_t *out) /* 0=有帧；1=已关闭或停止 */
{
    pthread_mutex_lock(&q->mu);
    while (q->count == 0 && g_run && !q->closed)
    {
        pthread_cond_wait(&q->not_empty, &q->mu);
    }
    if (q->count > 0)
    {
        *out = q->slots[q->head];
        q->head = (q->head + 1) % q->cap;
        q->count--;
        pthread_cond_signal(&q->not_full);
        pthread_mutex_unlock(&q->mu);
        return 0;
    }
    pthread_mutex_unlock(&q->mu);
    return (q->closed || !g_run) ? 1 : 1;
}

static void *capture_thread(void *arg) /* audio_ai_read_pcm → push detect/rec */
{
    (void)arg;

    LOG_INFO("audio capture thread start\n");
    while (g_run)
    {
        int16_t pcm[AUDIO_STREAM_FRAME_MAX_SAMPLES];
        size_t samples = 0;
        audio_stream_frame_t frame;

        if (!audio_ai_is_ready())
        {
            usleep(10000);
            continue;
        }
        if (audio_ai_read_pcm(pcm, AUDIO_STREAM_FRAME_MAX_SAMPLES, &samples) != 0)
        {
            continue;
        }
        if (samples == 0)
        {
            continue;
        }

        memset(&frame, 0, sizeof(frame));
        frame.num_samples = samples;
        memcpy(frame.pcm, pcm, samples * sizeof(int16_t));

        if (frame_queue_push(&g_detect_q, &frame, 1) != 0)
        {
            break;
        }
        if (g_rec_active_fn && g_rec_active_fn())
        {
            if (frame_queue_push(&g_rec_q, &frame, 0) != 0)
            {
                break;
            }
        }
    }

    LOG_INFO("audio capture thread exit\n");
    return NULL;
}

void audio_stream_set_rec_active_fn(int (*fn)(void)) /* 由 audio_init 注册 audio_is_recording */
{
    g_rec_active_fn = fn;
}

int audio_stream_start(void) /* 初始化双队列并启动 capture_thread */
{
    if (g_run)
    {
        return 0;
    }
    if (!audio_ai_is_ready())
    {
        LOG_ERROR("audio_stream_start: AI not ready\n");
        return -1;
    }
    if (frame_queue_init(&g_detect_q, DETECT_Q_CAP) != 0 ||
        frame_queue_init(&g_rec_q, REC_Q_CAP) != 0)
    {
        frame_queue_destroy(&g_detect_q);
        frame_queue_destroy(&g_rec_q);
        return -1;
    }

    g_run = 1;
    if (pthread_create(&g_capture_thread, NULL, capture_thread, NULL) != 0)
    {
        g_run = 0;
        frame_queue_destroy(&g_detect_q);
        frame_queue_destroy(&g_rec_q);
        LOG_ERROR("pthread_create capture failed\n");
        return -1;
    }
    LOG_INFO("audio_stream started\n");
    return 0;
}

void audio_stream_stop(void) /* 关队列、打断 AI Read、join 采集线程 */
{
    if (!g_run)
    {
        return;
    }
    g_run = 0;
    frame_queue_close(&g_detect_q);
    frame_queue_close(&g_rec_q);
    audio_ai_stop_capture(); /* 否则 capture 可能永久阻塞在 MI_AI_Read */
    pthread_join(g_capture_thread, NULL);
    frame_queue_destroy(&g_detect_q);
    frame_queue_destroy(&g_rec_q);
    LOG_INFO("audio_stream stopped\n");
}

int audio_stream_detect_pop(audio_stream_frame_t *out) /* bark_detect 线程消费 */
{
    if (!out)
    {
        return -1;
    }
    return frame_queue_pop(&g_detect_q, out);
}

int audio_stream_rec_pop(audio_stream_frame_t *out) /* audio 录制线程消费 */
{
    if (!out)
    {
        return -1;
    }
    return frame_queue_pop(&g_rec_q, out);
}
