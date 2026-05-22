#ifndef DOG_SOOTHER_APP_CONFIG_H_
#define DOG_SOOTHER_APP_CONFIG_H_

/* UART：与 BLE MCU 对接，115200 8N1；设备节点以板级 DTS 为准 */
#ifndef DS_UART_DEVICE
#define DS_UART_DEVICE "/dev/ttyS1"
#endif

#define DS_UART_BAUDRATE 115200

/* 用户数据目录 */
#ifndef DS_USERDATA_DIR
#define DS_USERDATA_DIR "/customer/sample_code/bin/userdata"
#endif

#define DS_OWNER_PCM_PATH   DS_USERDATA_DIR "/music/owner.wav"
#define DS_STRATEGY_PATH    DS_USERDATA_DIR "/params/calm_strategy.bin"
#define DS_COMFORT_DB_PATH  DS_USERDATA_DIR "/params/comfort_records.bin"

/* YAMNet 模型路径前缀（不含 .param/.bin 后缀） */
#ifndef DS_YAMNET_MODEL_PREFIX
#define DS_YAMNET_MODEL_PREFIX "/yamnet_model/yamnet.ncnn"
#endif

#define DS_OWNER_REC_MIN_SEC 3
#define DS_OWNER_REC_MAX_SEC 10
#define DS_CALM_REST_SEC     60
#define DS_MONITOR_PERIOD_MIN_SEC 10
#define DS_MONITOR_PERIOD_MAX_SEC 15

#endif /* DOG_SOOTHER_APP_CONFIG_H_ */
