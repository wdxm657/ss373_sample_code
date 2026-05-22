/**
 * @file uart_dispatch.c
 * @brief MCU REQ 分发到 comfort_store / bark_control / audio / system_time
 */
#define LOG_TAG "uart_dispatch"
#include "log.h"

#include "uart_dispatch.h"
#include "uart_proto.h"
#include "uart_cmd.h"
#include "app_types.h"

#include "comfort_store.h"
#include "bark_control.h"
#include "audio.h"
#include "system_time.h"

static void uart_reply_status(uint8_t seq) /* 0x11：9 字节状态 RSP */
{
    uint8_t payload[16];
    uint16_t len = 0;
    comfort_store_fill_status_payload(payload, sizeof(payload), &len);
    if (len == 0)
    {
        uint8_t err[1] = {DS_UART_STATUS_INTERNAL_ERROR};
        uart_proto_send_rsp(DS_CMD_STATUS_GET, seq, err, 1);
        return;
    }
    LOG_INFO("STATUS_GET rsp: work=%u vol=%u mode=%u\n", payload[2], payload[5], payload[6]);
    uart_proto_send_rsp(DS_CMD_STATUS_GET, seq, payload, len);
}

static void uart_on_req(uint8_t cmd_id, uint8_t seq, const uint8_t *payload, uint16_t payload_len) /* switch cmd */
{
    uint8_t rsp[DS_UART_MAX_PAYLOAD];
    uint16_t rsp_len = 0;

    LOG_INFO("REQ cmd=0x%02x seq=%u len=%u\n", cmd_id, seq, payload_len);

    switch (cmd_id)
    {
    case DS_CMD_STATUS_GET:
        uart_reply_status(seq);
        return;

    case DS_CMD_POWER_CTRL:
        if (payload_len >= 1)
        {
            bark_control_set_power(payload[0] ? 1 : 0);
            rsp[0] = DS_UART_STATUS_OK;
            rsp[1] = payload[0];
            rsp_len = 2;
            LOG_INFO("POWER_CTRL on=%u\n", payload[0]);
        }
        else
        {
            rsp[0] = DS_UART_STATUS_PARAM_ERROR;
            rsp_len = 1;
        }
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_VOLUME_SET:
        rsp_len = audio_set_volume(payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_OWNER_REC_START:
    case DS_CMD_OWNER_REC_STOP:
    case DS_CMD_OWNER_REC_PLAY:
    case DS_CMD_OWNER_REC_PLAY_STOP:
    case DS_CMD_OWNER_REC_DELETE:
    case DS_CMD_OWNER_REC_INFO_GET:
        rsp_len = audio_handle_uart_cmd(cmd_id, payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_CALM_MODE_SET:
        rsp_len = bark_control_set_mode(payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_CALM_STRATEGY_SET:
    case DS_CMD_CALM_STRATEGY_GET:
        rsp_len = bark_control_handle_strategy_cmd(cmd_id, payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_TIME_SET:
        rsp_len = system_time_apply(payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_CALM_RECORD_GET:
        rsp_len = comfort_store_pull_records(payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_FACTORY_RESET:
        rsp_len = comfort_store_factory_reset(payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_BT_LINK_NOTIFY:
        comfort_store_set_bt_linked(payload_len >= 1 ? payload[0] : 0);
        LOG_INFO("BT_LINK_NOTIFY linked=%u\n", payload_len >= 1 ? payload[0] : 0);
        rsp[0] = DS_UART_STATUS_OK;
        uart_proto_send_rsp(cmd_id, seq, rsp, 1);
        return;

    case DS_CMD_LOG_PULL:
        rsp[0] = DS_UART_STATUS_OK;
        uart_proto_send_rsp(cmd_id, seq, rsp, 1);
        return;

    default:
        LOG_DEBUG("unsupported cmd 0x%02x\n", cmd_id);
        rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
        uart_proto_send_rsp(cmd_id, seq, rsp, 1);
        return;
    }
}

static void uart_on_frame( /* uart_proto RX 回调：仅处理 REQ */
    uint8_t msg_type,
    uint8_t cmd_id,
    uint8_t seq,
    const uint8_t *payload,
    uint16_t payload_len,
    void *user_data)
{
    (void)user_data;
    if (msg_type == DS_UART_MSG_REQ)
    {
        uart_on_req(cmd_id, seq, payload, payload_len);
        return;
    }
    LOG_DEBUG("ignore msg_type=0x%02x cmd=0x%02x\n", msg_type, cmd_id);
}

int uart_dispatch_init(void) /* 注册 uart_on_frame */
{
    uart_proto_set_rx_callback(uart_on_frame, NULL);
    return 0;
}

void uart_dispatch_deinit(void) /* 注销回调 */
{
    uart_proto_set_rx_callback(NULL, NULL);
}

void uart_dispatch_on_frame( /* 测试注入，与 RX 路径相同 */
    uint8_t msg_type,
    uint8_t cmd_id,
    uint8_t seq,
    const uint8_t *payload,
    uint16_t payload_len)
{
    uart_on_frame(msg_type, cmd_id, seq, payload, payload_len, NULL);
}
