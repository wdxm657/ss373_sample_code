#ifndef DOG_SOOTHER_UART_PROTO_H_
#define DOG_SOOTHER_UART_PROTO_H_

#include <stdint.h>

#define DS_UART_MAX_PAYLOAD 64
#define DS_UART_RX_BUF_SIZE 256

/* 完整帧解析成功后的回调（msg_type/cmd_id/seq/payload） */
typedef void (*ds_uart_frame_cb_t)(
    uint8_t msg_type,
    uint8_t cmd_id,
    uint8_t seq,
    const uint8_t *payload,
    uint16_t payload_len,
    void *user_data);

/* 打开串口并配置 8N1，不启动 RX 线程 */
int uart_proto_open(const char *device_path, unsigned int baudrate);

/* 启动 RX 线程；须先 uart_proto_set_rx_callback */
int uart_proto_start_rx(void);

void uart_proto_deinit(void);

/* 发送 RSP / EVT 帧，payload 最长 DS_UART_MAX_PAYLOAD */
int uart_proto_send_rsp(uint8_t cmd_id, uint8_t seq, const uint8_t *payload, uint16_t payload_len);
int uart_proto_send_evt(uint8_t cmd_id, uint8_t seq, const uint8_t *payload, uint16_t payload_len);

void uart_proto_set_rx_callback(ds_uart_frame_cb_t cb, void *user_data);

/* 便捷：open + set_rx_callback + start_rx（dispatch 未用时） */
int uart_proto_init(const char *device_path, unsigned int baudrate);

#endif /* DOG_SOOTHER_UART_PROTO_H_ */
