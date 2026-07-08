#ifndef DOG_SOOTHER_APP_CONFIG_H_
#define DOG_SOOTHER_APP_CONFIG_H_

/* --- UART（MCU 对接，115200 8N1）--- */
#ifndef DS_UART_DEVICE
#define DS_UART_DEVICE "/dev/ttyS1"
#endif
#define DS_UART_BAUDRATE 115200

/* --- 用户数据目录与文件路径 --- */
#ifndef DS_USERDATA_DIR
#define DS_USERDATA_DIR "/customer/sample_code/bin/userdata"
#endif
#define DS_OWNER_PCM_PATH   DS_USERDATA_DIR "/music/owner.wav"
#define DS_OWNER_REC_TMP_PATH   DS_USERDATA_DIR "/music/tmp/owner.wav"   // 临时录制路径
#define DS_OWNER_REC_MOVE_TIMEOUT_SEC 600   // tmp 文件 10 分钟内未保存则自动删除
#define DS_CALM_MUSIC_PATH  DS_USERDATA_DIR "/music/calm.wav"
#define DS_STRATEGY_PATH            DS_USERDATA_DIR "/params/calm_strategy.bin"
#define DS_STRATEGY_AUTO_PATH       DS_USERDATA_DIR "/params/calm_strategy_auto.bin"
#define DS_STRATEGY_MANUAL_PATH     DS_USERDATA_DIR "/params/calm_strategy_manual.bin"
#define DS_STRATEGY_MODE_PATH       DS_USERDATA_DIR "/params/calm_strategy_mode.bin"
#define DS_COMFORT_DB_PATH  DS_USERDATA_DIR "/params/comfort_records.bin"
#define DS_VOLUME_PATH      DS_USERDATA_DIR "/params/volume.bin"

/* YAMNet 模型路径前缀（不含 .param/.bin） */
#ifndef DS_YAMNET_MODEL_PREFIX
#define DS_YAMNET_MODEL_PREFIX DS_USERDATA_DIR "/yamnet_model/yamnet.ncnn"
#endif

/* YAMNet 滑窗参数（与 legacy/test_yamnet 一致） */
#define DS_YAMNET_THRESHOLD    0.25f
#define DS_YAMNET_WINDOW_SEC   0.96f
#define DS_YAMNET_HOP_SEC      0.96f /* 步进秒数；重叠由 yamnet 内部处理 */
#define DS_YAMNET_WINDOW_SAMPLES \
    ((size_t)(DS_AUDIO_SAMPLE_RATE * DS_YAMNET_WINDOW_SEC))
#define DS_YAMNET_HOP_SAMPLES \
    ((size_t)(DS_AUDIO_SAMPLE_RATE * DS_YAMNET_HOP_SEC))
#define DS_YAMNET_STREAM_BUF_SAMPLES \
    (DS_YAMNET_WINDOW_SAMPLES + DS_YAMNET_HOP_SAMPLES + 4096)

/* --- MI AI / AO 设备号（对齐 ai_demo / ao_demo）--- */
#define DS_AI_DEV_ID     0
#define DS_AI_CHN_GRP_ID 0
#define DS_AO_DEV_ID     0
/* 与 test_yamnet -g 30 对齐；过低（如 -10）易导致 YAMNet 全判 Silence */
#ifndef DS_AI_GAIN_DB
#define DS_AI_GAIN_DB    15
#endif
/* APP 音量 0~30：gain = level + DS_AO_VOLUME_BASE_GAIN */
#define DS_AO_VOLUME_BASE_GAIN (0)

#define DS_AUDIO_SAMPLE_RATE 16000

/* 主人录音：最短/最长秒数；安抚后休息秒数；监测周期范围 */
#define DS_OWNER_REC_MIN_SEC 3
#define DS_OWNER_REC_MAX_SEC 11
#define DS_CALM_REST_SEC     10

/* 监听：首次狗叫起 10s 内累计 3 次进入安抚执行 */
#define DS_MONITOR_WINDOW_SEC   10
#define DS_MONITOR_BARK_TRIGGER 3

/* 滑窗 Top-K：狗叫相关类置信度 ≥ 此值计为一次吠叫命中 */
#ifndef DS_BARK_CONFIDENCE_MIN
#define DS_BARK_CONFIDENCE_MIN 0.50f
#endif

/* 超声波单次发射时长（秒） */
#define DS_CALM_US_EMIT_SEC     5

#endif /* DOG_SOOTHER_APP_CONFIG_H_ */
