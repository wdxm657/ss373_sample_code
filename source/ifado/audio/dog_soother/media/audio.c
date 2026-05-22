#define LOG_TAG "audio"
#include "log.h"

#include "audio.h"
#include "app_config.h"
#include "app_types.h"
#include "uart_cmd.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static uint8_t g_volume = 30;
static uint8_t g_recording;

static int owner_file_exists(void)
{
    return access(DS_OWNER_PCM_PATH, F_OK) == 0;
}

int audio_init(void)
{
    g_recording = 0;
    return 0;
}

void audio_deinit(void)
{
}

void audio_tick(void)
{
    /* TODO: 录音超时 10s 自动 stop 并上报 DS_EVT_OWNER_REC */
}

void audio_refresh_owner_info(uint8_t *exist, uint8_t *duration_sec)
{
    if (exist)
    {
        *exist = owner_file_exists() ? 1 : 0;
    }
    if (duration_sec)
    {
        *duration_sec = 0;
    }
}

void audio_delete_owner_rec(void)
{
    unlink(DS_OWNER_PCM_PATH);
}

uint16_t audio_set_volume(
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap)
{
    if (!rsp || rsp_cap < 2 || !payload || payload_len < 1)
    {
        return 0;
    }
    g_volume = payload[0];
    rsp[0] = DS_UART_STATUS_OK;
    rsp[1] = g_volume;
    return 2;
}

uint16_t audio_handle_uart_cmd(
    uint8_t cmd_id,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *rsp,
    uint16_t rsp_cap)
{
    (void)payload;
    (void)payload_len;
    if (!rsp || rsp_cap < 1)
    {
        return 0;
    }

    switch (cmd_id)
    {
    case DS_CMD_OWNER_REC_START:
        g_recording = 1;
        LOG_INFO("owner rec start (stub)\n");
        rsp[0] = DS_UART_STATUS_OK;
        return 1;

    case DS_CMD_OWNER_REC_STOP:
        g_recording = 0;
        rsp[0] = DS_UART_STATUS_OK;
        return 1;

    case DS_CMD_OWNER_REC_PLAY:
    case DS_CMD_OWNER_REC_PLAY_STOP:
        rsp[0] = DS_UART_STATUS_OK;
        return 1;

    case DS_CMD_OWNER_REC_DELETE:
        audio_delete_owner_rec();
        rsp[0] = DS_UART_STATUS_OK;
        return 1;

    case DS_CMD_OWNER_REC_INFO_GET:
        rsp[0] = DS_UART_STATUS_OK;
        rsp[1] = owner_file_exists() ? 1 : 0;
        rsp[2] = 0;
        return 3;

    default:
        rsp[0] = DS_UART_STATUS_PARAM_ERROR;
        return 1;
    }
}
