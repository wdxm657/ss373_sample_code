/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved. */

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "yamnet/yamnet_wrapper.h"
#define ENABLE_AI_INPUT

#ifdef ENABLE_AI_INPUT
#include "mi_sys.h"
#include "mi_ai.h"
#include "st_common_audio.h"
#include "st_common.h"
#endif

#define YAMNET_SAMPLE_RATE 16000
#define YAMNET_MIN_SECONDS 1
#define YAMNET_DEFAULT_THRESHOLD 0.25f
#define YAMNET_DEFAULT_AIN_SECONDS 10
#define YAMNET_DEFAULT_AIN_GAIN (-10)

/* 与 yamnet_wrapper 一致: 96 帧 mel × 10ms hop = 0.96s 窗长, 50% 重叠 → 0.48s 步进 */
#define YAMNET_WINDOW_SEC 0.96f
#define YAMNET_HOP_SEC 0.96f
#define YAMNET_WINDOW_SAMPLES ((size_t)(YAMNET_SAMPLE_RATE * YAMNET_WINDOW_SEC))
#define YAMNET_HOP_SAMPLES ((size_t)(YAMNET_SAMPLE_RATE * YAMNET_HOP_SEC))
/* 每次从文件读取的块大小 (20ms)，按块 sleep 对齐 1x 实时 */
#define READ_CHUNK_SAMPLES 320
#define WINDOW_QUEUE_CAP 2
/* 滑动窗仅需保留约一窗+一步的 PCM，避免 MIC 长时间采集撑满内存 */
#define STREAM_BUF_SAMPLES (YAMNET_WINDOW_SAMPLES + YAMNET_HOP_SAMPLES + 4096)

typedef enum
{
    INPUT_NONE = 0,
    INPUT_WAV,
    INPUT_PCM,
    INPUT_AIN,
} input_type_t;

typedef struct
{
    input_type_t type;
    const char *in_file;
    const char *model_path;
    float threshold;
    int ain_seconds;
    int ain_gain;
} app_config_t;

typedef struct
{
    uint32_t sample_rate;
    uint16_t channels;
    int16_t *pcm;
    size_t sample_count;
} audio_pcm_t;

typedef struct
{
    int      win_idx;
    double   t_start_sec;
    double   t_end_sec;
    int      num_samples;
    int16_t *pcm; /* 窗数据副本，由推理线程 free */
} window_task_t;

typedef struct
{
    FILE  *fp;
    size_t total_samples;
    size_t read_samples;
    int    channels;
    int    is_wav;
} pcm_stream_t;

typedef struct
{
    window_task_t *slots;
    int cap;
    int count;
    int head;
    int tail;
    int reader_done;
    int infer_error;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} window_queue_t;

typedef struct
{
    app_config_t cfg;
    audio_pcm_t audio;
    yamnet_context_t *ctx;
    window_queue_t queue;
    int total_windows;
    double sum_rtf;
    double sum_infer_ms;
    pthread_mutex_t stats_mutex;
} stream_context_t;

static double get_time_sec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

static void sleep_until(double deadline_sec)
{
    double now = get_time_sec();
    double remain;
    useconds_t us;

    if (deadline_sec <= now)
    {
        return;
    }
    remain = deadline_sec - now;
    if (remain > 1000.0)
    {
        return;
    }
    us = (useconds_t)(remain * 1000000.0);
    if (us > 0)
    {
        usleep(us);
    }
}

static void print_usage(const char *prog)
{
    printf("Usage: %s -m <model_prefix> -I <wav|pcm|ain> [options]\n\n", prog);
    printf("Required:\n");
    printf("  -m, --model <prefix>   YAMNet ncnn model path (no .param/.bin suffix)\n");
    printf("  -I, --input <type>     Input: wav | pcm | ain (audio in / microphone)\n\n");
    printf("Input options:\n");
    printf("  -i, --in-file <path>   Input file (required for wav / pcm)\n");
    printf("  -s, --seconds <n>      Capture duration for ain, default %d\n",
           YAMNET_DEFAULT_AIN_SECONDS);
    printf("  -g, --gain <db>        AI gain for ain, default %d\n", YAMNET_DEFAULT_AIN_GAIN);
    printf("\nCommon:\n");
    printf("  -t, --threshold <f>    Cat/dog threshold, default %.2f\n",
           YAMNET_DEFAULT_THRESHOLD);
    printf("  -h, --help             Show this help\n\n");
    printf("Sliding window:\n");
    printf("  window %.2fs, hop %.2fs; reader feeds at 1x real-time (no burst enqueue)\n",
           YAMNET_WINDOW_SEC, YAMNET_HOP_SEC);
    printf("  thread-1: read/stream audio; thread-2: infer + Top-K + RTF\n\n");
    printf("Format:\n");
    printf("  wav : 16kHz mono (or stereo down-mix) 16-bit PCM WAV\n");
    printf("  pcm : raw s16le mono %d Hz, no header\n", YAMNET_SAMPLE_RATE);
    printf("  ain : capture from MI AI ADC (board only");
#ifdef ENABLE_AI_INPUT
    printf(")\n");
#else
    printf(", build with ENABLE_AI_INPUT)\n");
#endif
    printf("\nExamples:\n");
    printf("  %s -m ./models/yamnet -I wav -i test.wav\n", prog);
    printf("  %s -m ./models/yamnet -I pcm -i test.pcm\n", prog);
    printf("  %s -m ./models/yamnet -I ain -s 10 -g -10\n", prog);
}

static int streq(const char *a, const char *b)
{
    return a && b && strcmp(a, b) == 0;
}

static int parse_args(int argc, char **argv, app_config_t *cfg)
{
    int i;

    memset(cfg, 0, sizeof(*cfg));
    cfg->threshold = YAMNET_DEFAULT_THRESHOLD;
    cfg->ain_seconds = YAMNET_DEFAULT_AIN_SECONDS;
    cfg->ain_gain = YAMNET_DEFAULT_AIN_GAIN;

    for (i = 1; i < argc; i++)
    {
        if (streq(argv[i], "-h") || streq(argv[i], "--help"))
        {
            return 1;
        }
        else if ((streq(argv[i], "-m") || streq(argv[i], "--model")) && i + 1 < argc)
        {
            cfg->model_path = argv[++i];
        }
        else if ((streq(argv[i], "-I") || streq(argv[i], "--input")) && i + 1 < argc)
        {
            const char *t = argv[++i];
            if (streq(t, "wav"))
            {
                cfg->type = INPUT_WAV;
            }
            else if (streq(t, "pcm"))
            {
                cfg->type = INPUT_PCM;
            }
            else if (streq(t, "ain") || streq(t, "audioin") || streq(t, "ai"))
            {
                cfg->type = INPUT_AIN;
            }
            else
            {
                fprintf(stderr, "Unknown input type: %s\n", t);
                return -1;
            }
        }
        else if ((streq(argv[i], "-i") || streq(argv[i], "--in-file")) && i + 1 < argc)
        {
            cfg->in_file = argv[++i];
        }
        else if ((streq(argv[i], "-t") || streq(argv[i], "--threshold")) && i + 1 < argc)
        {
            cfg->threshold = (float)atof(argv[++i]);
        }
        else if ((streq(argv[i], "-s") || streq(argv[i], "--seconds")) && i + 1 < argc)
        {
            cfg->ain_seconds = atoi(argv[++i]);
        }
        else if ((streq(argv[i], "-g") || streq(argv[i], "--gain")) && i + 1 < argc)
        {
            cfg->ain_gain = atoi(argv[++i]);
        }
        else
        {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return -1;
        }
    }

    if (!cfg->model_path || cfg->type == INPUT_NONE)
    {
        return -1;
    }
    if ((cfg->type == INPUT_WAV || cfg->type == INPUT_PCM) && !cfg->in_file)
    {
        fprintf(stderr, "-i/--in-file is required for wav/pcm input\n");
        return -1;
    }
    if (cfg->type == INPUT_AIN && cfg->ain_seconds < YAMNET_MIN_SECONDS)
    {
        fprintf(stderr, "Capture duration must be >= %d second(s)\n", YAMNET_MIN_SECONDS);
        return -1;
    }
    return 0;
}

static uint32_t read_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint16_t read_u16_le(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int load_wav(const char *path, audio_pcm_t *out)
{
    FILE *fp = NULL;
    uint8_t hdr[12];
    uint16_t audio_format = 0;
    uint32_t data_size = 0;
    long data_offset = 0;
    int found_fmt = 0;
    int found_data = 0;

    memset(out, 0, sizeof(*out));
    out->sample_rate = YAMNET_SAMPLE_RATE;

    fp = fopen(path, "rb");
    if (!fp)
    {
        fprintf(stderr, "Failed to open wav: %s (%s)\n", path, strerror(errno));
        return -1;
    }

    if (fread(hdr, 1, 12, fp) != 12 || memcmp(hdr, "RIFF", 4) != 0 || memcmp(hdr + 8, "WAVE", 4) != 0)
    {
        fprintf(stderr, "Invalid WAV file: %s\n", path);
        fclose(fp);
        return -1;
    }

    while (!feof(fp))
    {
        uint8_t chunk_hdr[8];
        char chunk_id[5] = {0};
        uint32_t chunk_size = 0;
        uint16_t bits_per_sample = 0;

        if (fread(chunk_hdr, 1, 8, fp) != 8)
        {
            break;
        }

        memcpy(chunk_id, chunk_hdr, 4);
        chunk_size = read_u32_le(chunk_hdr + 4);

        if (memcmp(chunk_id, "fmt ", 4) == 0)
        {
            uint8_t fmt_buf[32];
            if (chunk_size < 16 || fread(fmt_buf, 1, 16, fp) != 16)
            {
                fprintf(stderr, "Invalid fmt chunk\n");
                fclose(fp);
                return -1;
            }

            audio_format = read_u16_le(fmt_buf);
            out->channels = read_u16_le(fmt_buf + 2);
            out->sample_rate = read_u32_le(fmt_buf + 4);
            bits_per_sample = read_u16_le(fmt_buf + 14);

            if (chunk_size > 16)
            {
                fseek(fp, (long)chunk_size - 16, SEEK_CUR);
            }
            found_fmt = 1;

            if (audio_format != 1)
            {
                fprintf(stderr, "Only PCM WAV supported (format=%u)\n", audio_format);
                fclose(fp);
                return -1;
            }
            if (bits_per_sample != 16)
            {
                fprintf(stderr, "Only 16-bit PCM supported (bits=%u)\n", bits_per_sample);
                fclose(fp);
                return -1;
            }
        }
        else if (memcmp(chunk_id, "data", 4) == 0)
        {
            data_offset = ftell(fp);
            data_size = chunk_size;
            found_data = 1;
            break;
        }
        else
        {
            fseek(fp, (long)chunk_size, SEEK_CUR);
        }
    }

    if (!found_fmt || !found_data)
    {
        fprintf(stderr, "WAV missing fmt or data chunk: %s\n", path);
        fclose(fp);
        return -1;
    }

    if (out->sample_rate != YAMNET_SAMPLE_RATE)
    {
        fprintf(stderr, "Sample rate must be %d Hz (got %u)\n", YAMNET_SAMPLE_RATE, out->sample_rate);
        fclose(fp);
        return -1;
    }

    if (out->channels < 1 || out->channels > 2)
    {
        fprintf(stderr, "Unsupported channel count: %u\n", out->channels);
        fclose(fp);
        return -1;
    }

    out->sample_count = data_size / sizeof(int16_t) / out->channels;
    if (out->sample_count == 0)
    {
        fprintf(stderr, "Empty WAV data\n");
        fclose(fp);
        return -1;
    }

    out->pcm = (int16_t *)malloc(out->sample_count * sizeof(int16_t));
    if (!out->pcm)
    {
        fclose(fp);
        return -1;
    }

    fseek(fp, data_offset, SEEK_SET);
    if (out->channels == 1)
    {
        if (fread(out->pcm, sizeof(int16_t), out->sample_count, fp) != out->sample_count)
        {
            fprintf(stderr, "Failed to read WAV PCM data\n");
            free(out->pcm);
            out->pcm = NULL;
            fclose(fp);
            return -1;
        }
    }
    else
    {
        size_t j;
        int16_t *stereo = (int16_t *)malloc(data_size);
        if (!stereo)
        {
            free(out->pcm);
            out->pcm = NULL;
            fclose(fp);
            return -1;
        }
        if (fread(stereo, 1, data_size, fp) != data_size)
        {
            free(stereo);
            free(out->pcm);
            out->pcm = NULL;
            fclose(fp);
            return -1;
        }
        for (j = 0; j < out->sample_count; j++)
        {
            out->pcm[j] =
                (int16_t)(((int32_t)stereo[j * 2] + (int32_t)stereo[j * 2 + 1]) / 2);
        }
        free(stereo);
        out->channels = 1;
    }

    fclose(fp);
    return 0;
}

static int load_pcm_raw(const char *path, audio_pcm_t *out)
{
    FILE *fp = NULL;
    long fsize = 0;

    memset(out, 0, sizeof(*out));
    out->sample_rate = YAMNET_SAMPLE_RATE;
    out->channels = 1;

    fp = fopen(path, "rb");
    if (!fp)
    {
        fprintf(stderr, "Failed to open pcm: %s (%s)\n", path, strerror(errno));
        return -1;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return -1;
    }
    fsize = ftell(fp);
    if (fsize < 0 || (fsize % (long)sizeof(int16_t)) != 0)
    {
        fprintf(stderr, "Invalid PCM file size: %s\n", path);
        fclose(fp);
        return -1;
    }

    out->sample_count = (size_t)fsize / sizeof(int16_t);
    if (out->sample_count == 0)
    {
        fprintf(stderr, "Empty PCM file\n");
        fclose(fp);
        return -1;
    }

    out->pcm = (int16_t *)malloc(out->sample_count * sizeof(int16_t));
    if (!out->pcm)
    {
        fclose(fp);
        return -1;
    }

    rewind(fp);
    if (fread(out->pcm, sizeof(int16_t), out->sample_count, fp) != out->sample_count)
    {
        fprintf(stderr, "Failed to read PCM data\n");
        free(out->pcm);
        out->pcm = NULL;
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

#ifdef ENABLE_AI_INPUT
static int capture_ai_pcm(int seconds, int gain, audio_pcm_t *out)
{
    MI_AI_Attr_t stAiDevAttr;
    MI_AUDIO_DEV stAiDevId = 0;
    MI_U8 stChnGrpId = 0;
    MI_AI_If_e enAiIf[] = {E_MI_AI_IF_ADC_AB, E_MI_AI_IF_ECHO_A};
    MI_AI_Data_t stMicFrame;
    MI_AI_Data_t stEchoFrame;
    MI_SYS_ChnPort_t stChnOutputPort;
    MI_S16 s16Gain = (MI_S16)gain;
    struct timeval nowRead, baseRead;
    size_t cap_samples = (size_t)seconds * YAMNET_SAMPLE_RATE;
    size_t total_samples = 0;
    MI_S32 s32Ret;

    memset(out, 0, sizeof(*out));
    out->sample_rate = YAMNET_SAMPLE_RATE;
    out->channels = 1;

    out->pcm = (int16_t *)malloc(cap_samples * sizeof(int16_t));
    if (!out->pcm)
    {
        return -1;
    }

    STCHECKRESULT(MI_SYS_Init(0));

    memset(&stAiDevAttr, 0, sizeof(stAiDevAttr));
    ST_Common_GetAiDefaultDevAttr(&stAiDevAttr);
    stAiDevAttr.enSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;
    ST_Common_AiOpenDev(stAiDevId, &stAiDevAttr);
    ST_Common_AiAttachIf(stAiDevId, stChnGrpId, enAiIf, 2);
    MI_AI_SetGain(stAiDevId, stChnGrpId, &s16Gain, 1);
    ST_Common_AiEnableChnGroup(stAiDevId, stChnGrpId);

    memset(&stChnOutputPort, 0, sizeof(stChnOutputPort));
    stChnOutputPort.eModId = E_MI_MODULE_ID_AI;
    stChnOutputPort.u32DevId = stAiDevId;
    stChnOutputPort.u32ChnId = stChnGrpId;
    stChnOutputPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stChnOutputPort, 3, 5));

    gettimeofday(&baseRead, NULL);
    printf("Capturing audio in for %d s (gain=%d dB)...\n", seconds, gain);

    while (total_samples < cap_samples)
    {
        size_t frame_samples;
        int16_t *dst;

        gettimeofday(&nowRead, NULL);
        if ((nowRead.tv_sec - baseRead.tv_sec) >= seconds)
        {
            break;
        }

        memset(&stMicFrame, 0, sizeof(stMicFrame));
        memset(&stEchoFrame, 0, sizeof(stEchoFrame));
        s32Ret = MI_AI_Read(stAiDevId, stChnGrpId, &stMicFrame, &stEchoFrame, -1);
        if (s32Ret != MI_SUCCESS)
        {
            fprintf(stderr, "MI_AI_Read failed: 0x%x\n", s32Ret);
            break;
        }

        frame_samples = stMicFrame.u32Byte[0] / sizeof(int16_t);
        if (frame_samples == 0 || total_samples + frame_samples > cap_samples)
        {
            MI_AI_ReleaseData(stAiDevId, stChnGrpId, &stMicFrame, &stEchoFrame);
            break;
        }

        dst = out->pcm + total_samples;
        memcpy(dst, stMicFrame.apvBuffer[0], stMicFrame.u32Byte[0]);
        total_samples += frame_samples;

        MI_AI_ReleaseData(stAiDevId, stChnGrpId, &stMicFrame, &stEchoFrame);
    }

    ST_Common_AiDisableChnGroup(stAiDevId, stChnGrpId);
    ST_Common_AiCloseDev(stAiDevId);
    STCHECKRESULT(MI_SYS_Exit(0));

    out->sample_count = total_samples;
    if (out->sample_count < YAMNET_WINDOW_SAMPLES)
    {
        fprintf(stderr, "Captured audio too short: %zu samples (need %zu)\n", out->sample_count,
                YAMNET_WINDOW_SAMPLES);
        free(out->pcm);
        out->pcm = NULL;
        return -1;
    }

    return 0;
}
#else
static int capture_ai_pcm(int seconds, int gain, audio_pcm_t *out)
{
    (void)seconds;
    (void)gain;
    (void)out;
    fprintf(stderr, "ain input not supported: rebuild with ENABLE_AI_INPUT\n");
    return -1;
}
#endif

static int pcm_stream_open(const app_config_t *cfg, pcm_stream_t *st)
{
    memset(st, 0, sizeof(*st));
    if (cfg->type == INPUT_WAV)
    {
        uint8_t hdr[12];
        uint32_t data_size = 0;
        long data_offset = 0;

        st->fp = fopen(cfg->in_file, "rb");
        if (!st->fp)
        {
            return -1;
        }
        st->is_wav = 1;
        if (fread(hdr, 1, 12, st->fp) != 12 || memcmp(hdr, "RIFF", 4) != 0)
        {
            fclose(st->fp);
            st->fp = NULL;
            return -1;
        }
        while (!feof(st->fp))
        {
            uint8_t chunk_hdr[8];
            char chunk_id[5] = {0};
            uint32_t chunk_size = 0;
            uint16_t channels = 1;
            uint32_t sample_rate = 0;
            uint16_t bits_per_sample = 16;
            uint16_t audio_format = 0;

            if (fread(chunk_hdr, 1, 8, st->fp) != 8)
            {
                break;
            }
            memcpy(chunk_id, chunk_hdr, 4);
            chunk_size = read_u32_le(chunk_hdr + 4);
            if (memcmp(chunk_id, "fmt ", 4) == 0)
            {
                uint8_t fmt_buf[32];
                if (chunk_size < 16 || fread(fmt_buf, 1, 16, st->fp) != 16)
                {
                    fclose(st->fp);
                    st->fp = NULL;
                    return -1;
                }
                audio_format = read_u16_le(fmt_buf);
                channels = read_u16_le(fmt_buf + 2);
                sample_rate = read_u32_le(fmt_buf + 4);
                bits_per_sample = read_u16_le(fmt_buf + 14);
                if (audio_format != 1 || bits_per_sample != 16 ||
                    sample_rate != YAMNET_SAMPLE_RATE || channels < 1 || channels > 2)
                {
                    fclose(st->fp);
                    st->fp = NULL;
                    return -1;
                }
                st->channels = channels;
                if (chunk_size > 16)
                {
                    fseek(st->fp, (long)chunk_size - 16, SEEK_CUR);
                }
            }
            else if (memcmp(chunk_id, "data", 4) == 0)
            {
                data_offset = ftell(st->fp);
                data_size = chunk_size;
                break;
            }
            else
            {
                fseek(st->fp, (long)chunk_size, SEEK_CUR);
            }
        }
        if (data_offset == 0)
        {
            fclose(st->fp);
            st->fp = NULL;
            return -1;
        }
        fseek(st->fp, data_offset, SEEK_SET);
        st->total_samples = data_size / sizeof(int16_t) / st->channels;
        return 0;
    }
    if (cfg->type == INPUT_PCM)
    {
        long fsize = 0;
        st->fp = fopen(cfg->in_file, "rb");
        if (!st->fp)
        {
            return -1;
        }
        if (fseek(st->fp, 0, SEEK_END) != 0)
        {
            fclose(st->fp);
            st->fp = NULL;
            return -1;
        }
        fsize = ftell(st->fp);
        rewind(st->fp);
        if (fsize < 0 || (fsize % (long)sizeof(int16_t)) != 0)
        {
            fclose(st->fp);
            st->fp = NULL;
            return -1;
        }
        st->channels = 1;
        st->total_samples = (size_t)fsize / sizeof(int16_t);
        return 0;
    }
    return -1;
}

static void pcm_stream_close(pcm_stream_t *st)
{
    if (st && st->fp)
    {
        fclose(st->fp);
        st->fp = NULL;
    }
}

static int pcm_stream_read_mono(pcm_stream_t *st, int16_t *dst, size_t max_samples, size_t *out_got)
{
    size_t got = 0;

    *out_got = 0;
    if (!st->fp || st->read_samples >= st->total_samples)
    {
        return 0;
    }

    if (st->channels == 1)
    {
        size_t want = max_samples;
        if (st->read_samples + want > st->total_samples)
        {
            want = st->total_samples - st->read_samples;
        }
        got = fread(dst, sizeof(int16_t), want, st->fp);
        st->read_samples += got;
        *out_got = got;
        return 0;
    }

    /* stereo -> mono */
    {
        size_t want_frames = max_samples;
        size_t i;
        int16_t *stereo = NULL;
        if (st->read_samples + want_frames > st->total_samples)
        {
            want_frames = st->total_samples - st->read_samples;
        }
        stereo = (int16_t *)malloc(want_frames * 2 * sizeof(int16_t));
        if (!stereo)
        {
            return -1;
        }
        got = fread(stereo, sizeof(int16_t), want_frames * 2, st->fp) / 2;
        for (i = 0; i < got; i++)
        {
            dst[i] = (int16_t)(((int32_t)stereo[i * 2] + (int32_t)stereo[i * 2 + 1]) / 2);
        }
        free(stereo);
        st->read_samples += got;
        *out_got = got;
    }
    return 0;
}

static void free_audio(audio_pcm_t *audio)
{
    free(audio->pcm);
    audio->pcm = NULL;
    audio->sample_count = 0;
}

static const char *input_type_name(input_type_t type)
{
    switch (type)
    {
    case INPUT_WAV:
        return "wav";
    case INPUT_PCM:
        return "pcm";
    case INPUT_AIN:
        return "ain";
    default:
        return "unknown";
    }
}

static int window_queue_init(window_queue_t *q, int cap)
{
    memset(q, 0, sizeof(*q));
    q->cap = cap;
    q->slots = (window_task_t *)calloc((size_t)cap, sizeof(window_task_t));
    if (!q->slots)
    {
        return -1;
    }
    if (pthread_mutex_init(&q->mutex, NULL) != 0)
    {
        free(q->slots);
        return -1;
    }
    if (pthread_cond_init(&q->not_empty, NULL) != 0 ||
        pthread_cond_init(&q->not_full, NULL) != 0)
    {
        pthread_mutex_destroy(&q->mutex);
        free(q->slots);
        return -1;
    }
    return 0;
}

static void window_queue_free_slot_pcm(window_queue_t *q, int idx)
{
    if (q->slots[idx].pcm)
    {
        free(q->slots[idx].pcm);
        q->slots[idx].pcm = NULL;
    }
}

static void window_queue_destroy(window_queue_t *q)
{
    int i;

    if (!q)
    {
        return;
    }
    if (q->slots)
    {
        for (i = 0; i < q->cap; i++)
        {
            window_queue_free_slot_pcm(q, i);
        }
    }
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
    free(q->slots);
    memset(q, 0, sizeof(*q));
}

static int window_queue_push(window_queue_t *q, const window_task_t *task)
{
    window_task_t copy;

    if (!task->pcm || task->num_samples <= 0)
    {
        return -1;
    }

    copy = *task;
    copy.pcm = (int16_t *)malloc((size_t)task->num_samples * sizeof(int16_t));
    if (!copy.pcm)
    {
        return -1;
    }
    memcpy(copy.pcm, task->pcm, (size_t)task->num_samples * sizeof(int16_t));

    pthread_mutex_lock(&q->mutex);
    while (q->count >= q->cap && !q->infer_error)
    {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }
    if (q->infer_error)
    {
        pthread_mutex_unlock(&q->mutex);
        free(copy.pcm);
        return -1;
    }
    window_queue_free_slot_pcm(q, q->tail);
    q->slots[q->tail] = copy;
    q->tail = (q->tail + 1) % q->cap;
    q->count++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
    return 0;
}

static int window_queue_pop(window_queue_t *q, window_task_t *task, int *done)
{
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0 && !q->reader_done && !q->infer_error)
    {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }
    if (q->count > 0)
    {
        *task = q->slots[q->head];
        q->slots[q->head].pcm = NULL;
        q->head = (q->head + 1) % q->cap;
        q->count--;
        pthread_cond_signal(&q->not_full);
        *done = 0;
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    *done = q->reader_done ? 1 : 0;
    pthread_mutex_unlock(&q->mutex);
    return q->infer_error ? -1 : 1;
}

static void window_queue_set_reader_done(window_queue_t *q)
{
    pthread_mutex_lock(&q->mutex);
    q->reader_done = 1;
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

static void print_window_topk(int win_idx, double t0, double t1, double infer_ms, double rtf,
                              const yamnet_detect_result_list_t *results)
{
    int i;

    printf("\n--- window #%d [%.2f - %.2f] s | infer %.2f ms | RTF %.4f ---\n", win_idx, t0, t1,
           infer_ms, rtf);
    for (i = 0; i < results->count; i++)
    {
        printf("  %d. [%03d] %-40s  %.2f%%\n", i + 1, results->results[i].cls_id,
               yamnet_get_class_name(results->results[i].cls_id),
               results->results[i].confidence * 100.0f);
    }
    fflush(stdout);
}

static void reader_signal_error(stream_context_t *sc)
{
    pthread_mutex_lock(&sc->queue.mutex);
    sc->queue.infer_error = 1;
    pthread_cond_broadcast(&sc->queue.not_empty);
    pthread_cond_broadcast(&sc->queue.not_full);
    pthread_mutex_unlock(&sc->queue.mutex);
    window_queue_set_reader_done(&sc->queue);
}

/* 已入队的 PCM 从缓冲区前端丢弃，限制常驻内存 */
static void audio_buf_compact(int16_t *pcm, size_t *buf_len, size_t *next_win_offset,
                              size_t *stream_origin)
{
    if (*next_win_offset == 0)
    {
        return;
    }
    {
        size_t keep = *buf_len - *next_win_offset;
        if (keep > 0)
        {
            memmove(pcm, pcm + *next_win_offset, keep * sizeof(int16_t));
        }
        *stream_origin += *next_win_offset;
        *buf_len = keep;
        *next_win_offset = 0;
    }
}

static int try_emit_windows(stream_context_t *sc, int16_t *buf, size_t *buf_len,
                            size_t *next_win_offset, size_t *stream_origin, int *win_idx)
{
    while (*next_win_offset + YAMNET_WINDOW_SAMPLES <= *buf_len)
    {
        window_task_t task;

        memset(&task, 0, sizeof(task));
        task.win_idx = (*win_idx)++;
        task.num_samples = (int)YAMNET_WINDOW_SAMPLES;
        task.t_start_sec = (double)(*stream_origin + *next_win_offset) / YAMNET_SAMPLE_RATE;
        task.t_end_sec = task.t_start_sec + YAMNET_WINDOW_SEC;
        task.pcm = buf + (*next_win_offset);

        if (window_queue_push(&sc->queue, &task) != 0)
        {
            return -1;
        }
        *next_win_offset += YAMNET_HOP_SAMPLES;
        sc->total_windows++;
    }
    audio_buf_compact(buf, buf_len, next_win_offset, stream_origin);
    return 0;
}

/* wav/pcm: 按 1x 实时速率流式读入，凑满一窗才入队（队列满则阻塞） */
static int reader_stream_file(stream_context_t *sc)
{
    pcm_stream_t stream;
    int16_t chunk_buf[READ_CHUNK_SAMPLES];
    size_t buf_cap = STREAM_BUF_SAMPLES;
    size_t buf_len = 0;
    size_t next_win_offset = 0;
    size_t stream_origin = 0;
    int win_idx = 0;
    double stream_t0;
    int ret = 0;

    if (pcm_stream_open(&sc->cfg, &stream) != 0)
    {
        return -1;
    }
    if (stream.total_samples < YAMNET_WINDOW_SAMPLES)
    {
        fprintf(stderr, "Audio too short: %zu samples\n", stream.total_samples);
        pcm_stream_close(&stream);
        return -1;
    }

    sc->audio.pcm = (int16_t *)malloc(buf_cap * sizeof(int16_t));
    if (!sc->audio.pcm)
    {
        pcm_stream_close(&stream);
        return -1;
    }

    printf("[reader] stream %s: %.2f s, real-time 1x, win %.2fs hop %.2fs\n",
           sc->cfg.in_file, (double)stream.total_samples / YAMNET_SAMPLE_RATE, YAMNET_WINDOW_SEC,
           YAMNET_HOP_SEC);
    fflush(stdout);

    stream_t0 = get_time_sec();
    while (stream.read_samples < stream.total_samples)
    {
        size_t got = 0;

        if (pcm_stream_read_mono(&stream, chunk_buf, READ_CHUNK_SAMPLES, &got) != 0)
        {
            ret = -1;
            break;
        }
        if (got == 0)
        {
            break;
        }

        if (buf_len + got > buf_cap)
        {
            audio_buf_compact(sc->audio.pcm, &buf_len, &next_win_offset, &stream_origin);
        }
        if (buf_len + got > buf_cap)
        {
            fprintf(stderr, "[reader] stream buffer overflow\n");
            ret = -1;
            break;
        }
        memcpy(sc->audio.pcm + buf_len, chunk_buf, got * sizeof(int16_t));
        buf_len += got;
        sc->audio.sample_count = stream_origin + buf_len;

        if (try_emit_windows(sc, sc->audio.pcm, &buf_len, &next_win_offset, &stream_origin,
                             &win_idx) != 0)
        {
            ret = -1;
            break;
        }

        /* 对齐 1x 播放进度，避免一次性读完 */
        sleep_until(stream_t0 + (double)stream.read_samples / YAMNET_SAMPLE_RATE);
    }

    pcm_stream_close(&stream);
    return ret;
}

#ifdef ENABLE_AI_INPUT
static int reader_stream_ain(stream_context_t *sc)
{
    MI_AI_Attr_t stAiDevAttr;
    MI_AUDIO_DEV stAiDevId = 0;
    MI_U8 stChnGrpId = 0;
    MI_AI_If_e enAiIf[] = {E_MI_AI_IF_ADC_AB, E_MI_AI_IF_ECHO_A};
    MI_AI_Data_t stMicFrame;
    MI_AI_Data_t stEchoFrame;
    MI_SYS_ChnPort_t stChnOutputPort;
    MI_S16 s16Gain = (MI_S16)sc->cfg.ain_gain;
    struct timeval nowRead, baseRead;
    size_t buf_cap = STREAM_BUF_SAMPLES;
    size_t buf_len = 0;
    size_t next_win_offset = 0;
    size_t stream_origin = 0;
    int win_idx = 0;
    MI_S32 s32Ret;

    sc->audio.pcm = (int16_t *)malloc(buf_cap * sizeof(int16_t));
    if (!sc->audio.pcm)
    {
        return -1;
    }

    STCHECKRESULT(MI_SYS_Init(0));
    memset(&stAiDevAttr, 0, sizeof(stAiDevAttr));
    ST_Common_GetAiDefaultDevAttr(&stAiDevAttr);
    stAiDevAttr.enSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;
    ST_Common_AiOpenDev(stAiDevId, &stAiDevAttr);
    ST_Common_AiAttachIf(stAiDevId, stChnGrpId, enAiIf, 2);
    MI_AI_SetGain(stAiDevId, stChnGrpId, &s16Gain, 1);
    ST_Common_AiEnableChnGroup(stAiDevId, stChnGrpId);

    memset(&stChnOutputPort, 0, sizeof(stChnOutputPort));
    stChnOutputPort.eModId = E_MI_MODULE_ID_AI;
    stChnOutputPort.u32DevId = stAiDevId;
    stChnOutputPort.u32ChnId = stChnGrpId;
    stChnOutputPort.u32PortId = 0;
    STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stChnOutputPort, 3, 5));

    gettimeofday(&baseRead, NULL);
    printf("[reader] ain capture %d s (gain=%d), real-time\n", sc->cfg.ain_seconds,
           sc->cfg.ain_gain);
    fflush(stdout);

    while (1)
    {
        size_t frame_samples;
        int16_t *dst;

        gettimeofday(&nowRead, NULL);
        if ((nowRead.tv_sec - baseRead.tv_sec) >= sc->cfg.ain_seconds)
        {
            break;
        }

        memset(&stMicFrame, 0, sizeof(stMicFrame));
        memset(&stEchoFrame, 0, sizeof(stEchoFrame));
        s32Ret = MI_AI_Read(stAiDevId, stChnGrpId, &stMicFrame, &stEchoFrame, -1);
        if (s32Ret != MI_SUCCESS)
        {
            break;
        }

        frame_samples = stMicFrame.u32Byte[0] / sizeof(int16_t);
        if (frame_samples == 0)
        {
            MI_AI_ReleaseData(stAiDevId, stChnGrpId, &stMicFrame, &stEchoFrame);
            continue;
        }

        if (buf_len + frame_samples > buf_cap)
        {
            audio_buf_compact(sc->audio.pcm, &buf_len, &next_win_offset, &stream_origin);
        }
        if (buf_len + frame_samples > buf_cap)
        {
            fprintf(stderr, "[reader] ain buffer overflow\n");
            MI_AI_ReleaseData(stAiDevId, stChnGrpId, &stMicFrame, &stEchoFrame);
            ST_Common_AiDisableChnGroup(stAiDevId, stChnGrpId);
            ST_Common_AiCloseDev(stAiDevId);
            STCHECKRESULT(MI_SYS_Exit(0));
            return -1;
        }

        dst = sc->audio.pcm + buf_len;
        memcpy(dst, stMicFrame.apvBuffer[0], stMicFrame.u32Byte[0]);
        buf_len += frame_samples;
        sc->audio.sample_count = stream_origin + buf_len;

        if (try_emit_windows(sc, sc->audio.pcm, &buf_len, &next_win_offset, &stream_origin,
                             &win_idx) != 0)
        {
            MI_AI_ReleaseData(stAiDevId, stChnGrpId, &stMicFrame, &stEchoFrame);
            ST_Common_AiDisableChnGroup(stAiDevId, stChnGrpId);
            ST_Common_AiCloseDev(stAiDevId);
            STCHECKRESULT(MI_SYS_Exit(0));
            return -1;
        }

        MI_AI_ReleaseData(stAiDevId, stChnGrpId, &stMicFrame, &stEchoFrame);
    }

    ST_Common_AiDisableChnGroup(stAiDevId, stChnGrpId);
    ST_Common_AiCloseDev(stAiDevId);
    STCHECKRESULT(MI_SYS_Exit(0));

    if (buf_len < YAMNET_WINDOW_SAMPLES)
    {
        fprintf(stderr, "Captured audio too short: %zu samples\n", buf_len);
        return -1;
    }
    return 0;
}
#else
static int reader_stream_ain(stream_context_t *sc)
{
    (void)sc;
    fprintf(stderr, "ain not supported: rebuild with ENABLE_AI_INPUT\n");
    return -1;
}
#endif

static void *reader_thread(void *arg)
{
    stream_context_t *sc = (stream_context_t *)arg;
    int ret = 0;

    if (sc->cfg.type == INPUT_AIN)
    {
        ret = reader_stream_ain(sc);
    }
    else
    {
        ret = reader_stream_file(sc);
    }

    if (ret != 0)
    {
        reader_signal_error(sc);
        return NULL;
    }

    printf("[reader] done, %d windows enqueued\n", sc->total_windows);
    fflush(stdout);
    window_queue_set_reader_done(&sc->queue);
    return NULL;
}

static void *infer_thread(void *arg)
{
    stream_context_t *sc = (stream_context_t *)arg;
    float *window_f = NULL;
    int16_t *window_pcm = NULL;
    int processed = 0;

    window_pcm = (int16_t *)malloc(YAMNET_WINDOW_SAMPLES * sizeof(int16_t));
    window_f = (float *)malloc(YAMNET_WINDOW_SAMPLES * sizeof(float));
    if (!window_pcm || !window_f)
    {
        sc->queue.infer_error = 1;
        window_queue_set_reader_done(&sc->queue);
        free(window_pcm);
        free(window_f);
        return NULL;
    }

    for (;;)
    {
        window_task_t task;
        int done = 0;
        int pop_ret;
        double t0, t1;
        double audio_sec;
        double infer_sec;
        double infer_ms;
        double rtf;
        yamnet_detect_result_list_t results;

        pop_ret = window_queue_pop(&sc->queue, &task, &done);
        if (pop_ret < 0)
        {
            break;
        }
        if (pop_ret > 0 && done)
        {
            break;
        }
        if (pop_ret > 0)
        {
            continue;
        }

        memcpy(window_pcm, task.pcm, (size_t)task.num_samples * sizeof(int16_t));

        t0 = get_time_sec();
        if (yamnet_preprocess_audio(window_pcm, task.num_samples, window_f) != 0)
        {
            fprintf(stderr, "[infer] preprocess failed at window %d\n", task.win_idx);
            free(task.pcm);
            sc->queue.infer_error = 1;
            break;
        }

        memset(&results, 0, sizeof(results));
        if (yamnet_inference_topk(sc->ctx, window_f, task.num_samples, &results) != 0)
        {
            fprintf(stderr, "[infer] inference failed at window %d\n", task.win_idx);
            free(task.pcm);
            sc->queue.infer_error = 1;
            break;
        }
        t1 = get_time_sec();

        audio_sec = (double)task.num_samples / YAMNET_SAMPLE_RATE;
        infer_sec = t1 - t0;
        infer_ms = infer_sec * 1000.0;
        rtf = (audio_sec > 0.0) ? (infer_sec / audio_sec) : 0.0;

        print_window_topk(task.win_idx, task.t_start_sec, task.t_end_sec, infer_ms, rtf, &results);
        free(task.pcm);
        task.pcm = NULL;

        pthread_mutex_lock(&sc->stats_mutex);
        sc->sum_rtf += rtf;
        sc->sum_infer_ms += infer_ms;
        processed++;
        pthread_mutex_unlock(&sc->stats_mutex);
    }

    pthread_mutex_lock(&sc->stats_mutex);
    if (processed > 0)
    {
        printf("\n=== summary: %d windows, avg infer %.2f ms, avg RTF %.4f ===\n", processed,
               sc->sum_infer_ms / processed, sc->sum_rtf / processed);
    }
    pthread_mutex_unlock(&sc->stats_mutex);

    /* 排空队列中未处理的窗，避免销毁时泄漏 */
    for (;;)
    {
        window_task_t task;
        int done = 0;
        int pop_ret = window_queue_pop(&sc->queue, &task, &done);

        if (pop_ret != 0 || done)
        {
            break;
        }
        free(task.pcm);
    }

    free(window_pcm);
    free(window_f);
    return NULL;
}

int main(int argc, char **argv)
{
    app_config_t cfg;
    stream_context_t sc;
    pthread_t tid_reader;
    pthread_t tid_infer;
    int parse_ret;
    int ret = 0;

    memset(&sc, 0, sizeof(sc));

    parse_ret = parse_args(argc, argv, &cfg);
    if (parse_ret == 1)
    {
        print_usage(argv[0]);
        return 0;
    }
    if (parse_ret != 0)
    {
        print_usage(argv[0]);
        return 1;
    }

    sc.cfg = cfg;

    if (window_queue_init(&sc.queue, WINDOW_QUEUE_CAP) != 0)
    {
        fprintf(stderr, "Failed to init window queue\n");
        return 1;
    }
    if (pthread_mutex_init(&sc.stats_mutex, NULL) != 0)
    {
        window_queue_destroy(&sc.queue);
        return 1;
    }

    yamnet_init_post_process();
    sc.ctx = yamnet_create_context(cfg.threshold);
    if (!sc.ctx)
    {
        fprintf(stderr, "Failed to create YAMNet context\n");
        ret = 1;
        goto cleanup;
    }

    if (yamnet_init_model(sc.ctx, cfg.model_path) != 0)
    {
        fprintf(stderr, "Failed to load model: %s.param / %s.bin\n", cfg.model_path, cfg.model_path);
        ret = 1;
        goto cleanup;
    }

    printf("Model: %s\n", cfg.model_path);
    printf("Input type: %s", input_type_name(cfg.type));
    if (cfg.in_file)
    {
        printf(" (%s)", cfg.in_file);
    }
    else if (cfg.type == INPUT_AIN)
    {
        printf(" (capture %ds, gain=%d)", cfg.ain_seconds, cfg.ain_gain);
    }
    printf("\n");
    printf("Sliding: window %.2fs, hop %.2fs, reader 1x real-time\n\n", YAMNET_WINDOW_SEC,
           YAMNET_HOP_SEC);

    if (pthread_create(&tid_infer, NULL, infer_thread, &sc) != 0)
    {
        fprintf(stderr, "Failed to create infer thread\n");
        ret = 1;
        goto cleanup;
    }

    if (pthread_create(&tid_reader, NULL, reader_thread, &sc) != 0)
    {
        fprintf(stderr, "Failed to create reader thread\n");
        sc.queue.infer_error = 1;
        window_queue_set_reader_done(&sc.queue);
        pthread_join(tid_infer, NULL);
        ret = 1;
        goto cleanup;
    }

    pthread_join(tid_reader, NULL);
    pthread_join(tid_infer, NULL);

    if (sc.queue.infer_error)
    {
        ret = 1;
    }

cleanup:
    if (sc.ctx)
    {
        yamnet_destroy_context(sc.ctx);
    }
    yamnet_deinit_post_process();
    free_audio(&sc.audio);
    pthread_mutex_destroy(&sc.stats_mutex);
    window_queue_destroy(&sc.queue);
    return ret;
}
