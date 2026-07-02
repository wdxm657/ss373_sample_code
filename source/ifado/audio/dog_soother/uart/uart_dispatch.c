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
#include "audio_ao.h"
#include "app_config.h"
#include "system_time.h"
#include <stdio.h>

/* 安抚记录数据库路径（同 app_config.h 中的 DS_COMFORT_DB_PATH）*/
#ifndef DS_COMFORT_DB_PATH
#define DS_COMFORT_DB_PATH  DS_USERDATA_DIR "/params/comfort_records.bin"
#endif
#define DS_COMFORT_DB_BK_PATH  DS_USERDATA_DIR "/params/comfort_records.bin.bk"

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

    switch (cmd_id)
    {
    case DS_CMD_STATUS_GET:
        LOG_INFO("STATUS_GET req\n");
        uart_reply_status(seq);
        return;

    case DS_CMD_POWER_CTRL:
        if (payload_len >= 1)
        {
            if (payload[0])
            {
                /* 开机时尝试从备份文件恢复安抚记录 */
                FILE *fp_src = fopen(DS_COMFORT_DB_BK_PATH, "rb");
                if (fp_src)
                {
                    FILE *fp_dst = fopen(DS_COMFORT_DB_PATH, "wb");
                    if (fp_dst)
                    {
                        uint8_t buf[512];
                        size_t n;
                        while ((n = fread(buf, 1, sizeof(buf), fp_src)) > 0)
                        {
                            fwrite(buf, 1, n, fp_dst);
                        }
                        fclose(fp_dst);
                        LOG_INFO("POWER_CTRL: restored records from %s\n",
                                 DS_COMFORT_DB_BK_PATH);
                    }
                    fclose(fp_src);
                }
            }
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
        LOG_INFO("VOLUME_SET req vol=%u\n", payload_len >= 1 ? payload[0] : 0xFF);
        rsp_len = audio_set_volume(payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_OWNER_REC_START:
        LOG_INFO("OWNER_REC_START req\n");
        rsp_len = audio_handle_uart_cmd(cmd_id, payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;
    case DS_CMD_OWNER_REC_STOP:
        LOG_INFO("OWNER_REC_STOP req\n");
        rsp_len = audio_handle_uart_cmd(cmd_id, payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;
    case DS_CMD_OWNER_REC_PLAY:
        LOG_INFO("OWNER_REC_PLAY req src=%u\n", payload_len >= 1 ? payload[0] : 0xFF);
        rsp_len = audio_handle_uart_cmd(cmd_id, payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;
    case DS_CMD_OWNER_REC_PLAY_STOP:
        LOG_INFO("OWNER_REC_PLAY_STOP req\n");
        rsp_len = audio_handle_uart_cmd(cmd_id, payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;
    case DS_CMD_OWNER_REC_DELETE:
        LOG_INFO("OWNER_REC_DELETE req\n");
        rsp_len = audio_handle_uart_cmd(cmd_id, payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;
    case DS_CMD_OWNER_REC_INFO_GET:
        LOG_INFO("OWNER_REC_INFO_GET req\n");
        rsp_len = audio_handle_uart_cmd(cmd_id, payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;
    case DS_CMD_OWNER_REC_SAVE:
        LOG_INFO("OWNER_REC_SAVE req\n");
        rsp_len = audio_handle_uart_cmd(cmd_id, payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_CALM_MODE_SET:
        LOG_INFO("CALM_MODE_SET req mode=%u\n", payload_len >= 1 ? payload[0] : 0xFF);
        rsp_len = bark_control_set_mode(payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_CALM_STRATEGY_SET:
        LOG_INFO("CALM_STRATEGY_SET req len=%u\n", payload_len);
        rsp_len = bark_control_handle_strategy_cmd(cmd_id, payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;
    case DS_CMD_CALM_STRATEGY_GET:
        LOG_INFO("CALM_STRATEGY_GET req mode=%u\n", payload_len >= 1 ? payload[0] : 0xFF);
        rsp_len = bark_control_handle_strategy_cmd(cmd_id, payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_TIME_SET:
        LOG_INFO("TIME_SET req\n");
        rsp_len = system_time_apply(payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_CALM_RECORD_GET:
        LOG_INFO("CALM_RECORD_GET req\n");
        rsp_len = comfort_store_pull_records(payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_CALM_RECORD_DELETE:
        LOG_INFO("CALM_RECORD_DELETE req len=%u\n", payload_len);
        rsp_len = comfort_store_delete_record_by_id(payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_FACTORY_RESET:
        LOG_INFO("FACTORY_RESET req\n");
        rsp_len = comfort_store_factory_reset(payload, payload_len, rsp, sizeof(rsp));
        uart_proto_send_rsp(cmd_id, seq, rsp, rsp_len);
        return;

    case DS_CMD_CALM_MUSIC_PLAY:
        LOG_INFO("CALM_MUSIC_PLAY req\n");
        if (audio_ao_play_wav_file(DS_CALM_MUSIC_PATH) == 0)
        {
            rsp[0] = DS_UART_STATUS_OK;
        }
        else
        {
            LOG_INFO("CALM_MUSIC_PLAY failed to start playback\n");
            rsp[0] = DS_UART_STATUS_INTERNAL_ERROR;
        }
        uart_proto_send_rsp(cmd_id, seq, rsp, 1);
        return;

    case DS_CMD_CALM_MUSIC_PLAY_STOP:
        LOG_INFO("CALM_MUSIC_PLAY_STOP req\n");
        audio_ao_stop();
        rsp[0] = DS_UART_STATUS_OK;
        uart_proto_send_rsp(cmd_id, seq, rsp, 1);
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

static void uart_on_frame(/* uart_proto RX 回调：仅处理 REQ */
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

void uart_dispatch_on_frame(/* 测试注入，与 RX 路径相同 */
                            uint8_t msg_type,
                            uint8_t cmd_id,
                            uint8_t seq,
                            const uint8_t *payload,
                            uint16_t payload_len)
{
    uart_on_frame(msg_type, cmd_id, seq, payload, payload_len, NULL);
}
