/**
 * @file bark_detect.c
 * @brief YAMNet 滑窗推理，命中狗叫类则通知 bark_control
 */
#define LOG_TAG "bark_detect"
#include "log.h"

#include "bark_detect.h"
#include "bark_control.h"
#include "audio_stream.h"
#include "app_config.h"

#include "yamnet/yamnet_wrapper.h"

#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/* YAMNet AudioSet 狗叫相关类别（与 yamnet_wrapper.cpp 索引一致） */
static const int g_dog_bark_cls_ids[] = {
    69,  /* Dog */
    70,  /* Bark */
    71,  /* Yip */
    72,  /* Howl */
    73,  /* Bow-wow */
    74,  /* Growl */
    75,  /* Whimper */
    117, /* Canidae */
};

typedef struct
{
    yamnet_context_t *ctx;
    int16_t stream_pcm[DS_YAMNET_STREAM_BUF_SAMPLES];
    float window_f[DS_YAMNET_WINDOW_SAMPLES];
    size_t buf_len;
    size_t next_win_offset;
    size_t stream_origin;
    int win_idx;
    int ready;
} bark_yamnet_state_t;

static bark_yamnet_state_t g_yn;
static volatile int g_detect_thread_run;
static volatile int g_detect_active = 1;
static pthread_t g_detect_thread;

static uint32_t bark_now_epoch(void)
{
    return (uint32_t)time(NULL);
}

static int bark_results_has_dog_bark(const yamnet_detect_result_list_t *results)
{
    size_t c;
    int i;

    if (!results)
    {
        return 0;
    }
    for (i = 0; i < results->count; i++)
    {
        int cls = results->results[i].cls_id;
        float conf = results->results[i].confidence;

        if (conf < DS_BARK_CONFIDENCE_MIN)
        {
            continue;
        }
        for (c = 0; c < sizeof(g_dog_bark_cls_ids) / sizeof(g_dog_bark_cls_ids[0]); c++)
        {
            if (cls == g_dog_bark_cls_ids[c])
            {
                LOG_INFO("bark hit cls=%d %s conf=%.1f%%\n",
                         cls,
                         yamnet_get_class_name(cls),
                         conf * 100.0f);
                return 1;
            }
        }
    }
    return 0;
}

static void bark_stream_compact(void)
{
    if (g_yn.next_win_offset == 0)
    {
        return;
    }
    size_t keep = g_yn.buf_len - g_yn.next_win_offset;
    if (keep > 0)
    {
        memmove(g_yn.stream_pcm,
                g_yn.stream_pcm + g_yn.next_win_offset,
                keep * sizeof(int16_t));
    }
    g_yn.stream_origin += g_yn.next_win_offset;
    g_yn.buf_len = keep;
    g_yn.next_win_offset = 0;
}

/* 返回 1=本窗判定有狗叫，0=无 */
static int bark_run_window_inference(size_t win_offset_in_buf)
{
    yamnet_detect_result_list_t results;
    int has_bark;

    if (!g_yn.ready || !g_yn.ctx)
    {
        return 0;
    }
    if (win_offset_in_buf + DS_YAMNET_WINDOW_SAMPLES > g_yn.buf_len)
    {
        return 0;
    }

    g_yn.win_idx++;

    if (yamnet_preprocess_audio(g_yn.stream_pcm + win_offset_in_buf,
                                (int)DS_YAMNET_WINDOW_SAMPLES,
                                g_yn.window_f) != 0)
    {
        LOG_ERROR("yamnet_preprocess failed win=%d\n", g_yn.win_idx);
        return 0;
    }

    memset(&results, 0, sizeof(results));
    if (yamnet_inference_topk(g_yn.ctx, g_yn.window_f, (int)DS_YAMNET_WINDOW_SAMPLES, &results) != 0)
    {
        LOG_ERROR("yamnet_inference_topk failed win=%d\n", g_yn.win_idx);
        return 0;
    }

    has_bark = bark_results_has_dog_bark(&results);
    if (has_bark)
    {
        bark_control_on_window_bark(1, bark_now_epoch());
    }
    return has_bark;
}

static void bark_append_and_infer(const audio_stream_frame_t *frame)
{
    size_t got = frame->num_samples;

    if (got == 0)
    {
        return;
    }
    if (g_yn.buf_len + got > DS_YAMNET_STREAM_BUF_SAMPLES)
    {
        bark_stream_compact();
    }
    if (g_yn.buf_len + got > DS_YAMNET_STREAM_BUF_SAMPLES)
    {
        LOG_ERROR("yamnet stream buffer overflow, drop frame\n");
        return;
    }

    memcpy(g_yn.stream_pcm + g_yn.buf_len, frame->pcm, got * sizeof(int16_t));
    g_yn.buf_len += got;

    while (g_yn.next_win_offset + DS_YAMNET_WINDOW_SAMPLES <= g_yn.buf_len)
    {
        if (g_detect_active)
        {
            bark_run_window_inference(g_yn.next_win_offset);
        }
        g_yn.next_win_offset += DS_YAMNET_HOP_SAMPLES;
        bark_stream_compact();
    }
}

void bark_detect_set_active(int active)
{
    g_detect_active = active ? 1 : 0;
}

static int bark_yamnet_init(void)
{
    memset(&g_yn, 0, sizeof(g_yn));

    if (yamnet_init_post_process() != 0)
    {
        LOG_ERROR("yamnet_init_post_process failed\n");
        return -1;
    }

    g_yn.ctx = yamnet_create_context(DS_YAMNET_THRESHOLD);
    if (!g_yn.ctx)
    {
        LOG_ERROR("yamnet_create_context failed\n");
        yamnet_deinit_post_process();
        return -2;
    }

    if (yamnet_init_model(g_yn.ctx, DS_YAMNET_MODEL_PREFIX) != 0)
    {
        LOG_ERROR("yamnet_init_model failed, path=%s\n", DS_YAMNET_MODEL_PREFIX);
        yamnet_destroy_context(g_yn.ctx);
        g_yn.ctx = NULL;
        yamnet_deinit_post_process();
        return -3;
    }

    g_yn.ready = 1;
    LOG_INFO("yamnet ready model=%s bark_conf>=%.0f%%\n",
             DS_YAMNET_MODEL_PREFIX,
             DS_BARK_CONFIDENCE_MIN * 100.0f);
    return 0;
}

static void bark_yamnet_deinit(void)
{
    if (g_yn.ctx)
    {
        yamnet_destroy_context(g_yn.ctx);
        g_yn.ctx = NULL;
    }
    yamnet_deinit_post_process();
    g_yn.ready = 0;
    g_yn.buf_len = 0;
    g_yn.next_win_offset = 0;
    g_yn.stream_origin = 0;
    g_yn.win_idx = 0;
}

static void *bark_detect_thread(void *arg)
{
    (void)arg;

    LOG_INFO("bark_detect thread start\n");
    while (g_detect_thread_run)
    {
        audio_stream_frame_t frame;
        int ret = audio_stream_detect_pop(&frame);

        if (ret == 0)
        {
            bark_append_and_infer(&frame);
            continue;
        }
        if (ret == 1)
        {
            break;
        }
        if (!g_detect_thread_run)
        {
            break;
        }
        usleep(10000);
    }
    LOG_INFO("bark_detect thread exit\n");
    return NULL;
}

static int bark_detect_thread_start(void)
{
    g_detect_thread_run = 1;
    if (pthread_create(&g_detect_thread, NULL, bark_detect_thread, NULL) != 0)
    {
        g_detect_thread_run = 0;
        LOG_ERROR("pthread_create bark_detect failed\n");
        return -1;
    }
    return 0;
}

static void bark_detect_thread_stop(void)
{
    if (!g_detect_thread_run)
    {
        return;
    }
    g_detect_thread_run = 0;
    pthread_join(g_detect_thread, NULL);
}

int bark_detect_init(void)
{
    if (bark_yamnet_init() != 0)
    {
        return -1;
    }
    if (bark_detect_thread_start() != 0)
    {
        bark_yamnet_deinit();
        return -2;
    }
    return 0;
}

void bark_detect_deinit(void)
{
    bark_detect_thread_stop();
    bark_yamnet_deinit();
}
