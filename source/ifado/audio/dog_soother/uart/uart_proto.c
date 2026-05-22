#define LOG_TAG "uart_proto"
#include "log.h"

#include "uart_proto.h"
#include "uart_cmd.h"
#include "app_config.h"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static int g_uart_fd = -1;
static pthread_t g_rx_thread;
static volatile int g_rx_running;
static volatile int g_rx_started;
static ds_uart_frame_cb_t g_rx_cb;
static void *g_rx_user;
static pthread_mutex_t g_tx_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_uart_debug;

static int uart_env_debug_enabled(void)
{
    const char *env = getenv("DS_UART_DEBUG");
    return (env && env[0] == '1');
}

static void uart_hex_log(const char *prefix, const uint8_t *data, uint16_t len)
{
    if (!g_uart_debug || !data || len == 0)
    {
        return;
    }
    char line[128];
    uint16_t pos = 0;
    int n = snprintf(line, sizeof(line), "%s[%u]: ", prefix, len);
    if (n < 0)
    {
        return;
    }
    pos = (uint16_t)n;
    for (uint16_t i = 0; i < len && pos + 3 < sizeof(line); i++)
    {
        n = snprintf(line + pos, sizeof(line) - pos, "%02X ", data[i]);
        if (n < 0)
        {
            break;
        }
        pos = (uint16_t)(pos + (uint16_t)n);
    }
    LOG_DEBUG("%s\n", line);
}

static uint16_t uart_crc16_ibm(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static speed_t uart_baud_to_speed(unsigned int baudrate)
{
    switch (baudrate)
    {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
    default:
        return B115200;
    }
}

static int uart_configure_port(int fd, unsigned int baudrate)
{
    struct termios tio;
    if (tcgetattr(fd, &tio) != 0)
    {
        return -1;
    }

    cfmakeraw(&tio);
    speed_t spd = uart_baud_to_speed(baudrate);
    cfsetispeed(&tio, spd);
    cfsetospeed(&tio, spd);

    tio.c_cflag |= (CLOCAL | CREAD);
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~CRTSCTS;

    tio.c_iflag &= ~(IXON | IXOFF | IXANY);
    tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tio.c_oflag &= ~OPOST;

    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tio) != 0)
    {
        return -2;
    }
    tcflush(fd, TCIOFLUSH);
    return 0;
}

static int uart_send_frame(uint8_t msg_type, uint8_t cmd_id, uint8_t seq, const uint8_t *payload, uint16_t payload_len)
{
    if (g_uart_fd < 0)
    {
        return -1;
    }
    if (payload_len > DS_UART_MAX_PAYLOAD)
    {
        return -2;
    }

    uint8_t frame[8 + DS_UART_MAX_PAYLOAD + 2];
    uint16_t idx = 0;
    frame[idx++] = 0x55;
    frame[idx++] = 0xAA;
    frame[idx++] = DS_UART_PROTO_VER;
    frame[idx++] = msg_type;
    frame[idx++] = cmd_id;
    frame[idx++] = seq;
    frame[idx++] = (uint8_t)(payload_len & 0xFF);
    frame[idx++] = (uint8_t)((payload_len >> 8) & 0xFF);
    if (payload_len && payload)
    {
        memcpy(&frame[idx], payload, payload_len);
        idx += payload_len;
    }
    uint16_t crc = uart_crc16_ibm(&frame[2], (uint16_t)(6 + payload_len));
    frame[idx++] = (uint8_t)(crc & 0xFF);
    frame[idx++] = (uint8_t)((crc >> 8) & 0xFF);

    pthread_mutex_lock(&g_tx_mutex);
    ssize_t wr = write(g_uart_fd, frame, idx);
    pthread_mutex_unlock(&g_tx_mutex);

    if (wr != (ssize_t)idx)
    {
        LOG_ERROR("uart write failed %zd/%u errno=%d\n", wr, idx, errno);
        return -3;
    }

    uart_hex_log("uart_tx", frame, idx);
    return 0;
}

static void uart_try_parse(uint8_t *buf, uint16_t *len)
{
    while (*len >= 10)
    {
        uint16_t start = 0;
        while ((start + 1) < *len)
        {
            if (buf[start] == 0x55 && buf[start + 1] == 0xAA)
            {
                break;
            }
            start++;
        }
        if ((start + 1) >= *len)
        {
            *len = 0;
            return;
        }
        if (start > 0)
        {
            memmove(buf, &buf[start], *len - start);
            *len = (uint16_t)(*len - start);
        }
        if (*len < 10)
        {
            return;
        }

        if (buf[2] != DS_UART_PROTO_VER)
        {
            LOG_ERROR("bad proto ver 0x%02x, drop 1 byte\n", buf[2]);
            memmove(buf, buf + 1, *len - 1);
            *len = (uint16_t)(*len - 1);
            continue;
        }

        uint16_t payload_len = (uint16_t)(buf[6] | (buf[7] << 8));
        uint16_t frame_len = (uint16_t)(8 + payload_len + 2);
        if (payload_len > DS_UART_MAX_PAYLOAD || frame_len > DS_UART_RX_BUF_SIZE)
        {
            LOG_ERROR("invalid frame len payload=%u\n", payload_len);
            memmove(buf, buf + 1, *len - 1);
            *len = (uint16_t)(*len - 1);
            continue;
        }
        if (*len < frame_len)
        {
            return;
        }

        uint16_t recv_crc = (uint16_t)(buf[frame_len - 2] | (buf[frame_len - 1] << 8));
        uint16_t calc_crc = uart_crc16_ibm(&buf[2], (uint16_t)(6 + payload_len));
        if (recv_crc == calc_crc)
        {
            uart_hex_log("uart_rx", buf, frame_len);
            if (g_rx_cb)
            {
                g_rx_cb(buf[3], buf[4], buf[5], &buf[8], payload_len, g_rx_user);
            }
        }
        else
        {
            LOG_ERROR("uart crc err recv=0x%04x calc=0x%04x cmd=0x%02x\n",
                      recv_crc,
                      calc_crc,
                      buf[4]);
        }

        if (*len > frame_len)
        {
            memmove(buf, &buf[frame_len], *len - frame_len);
        }
        *len = (uint16_t)(*len - frame_len);
    }
}

static void *uart_rx_thread(void *arg)
{
    (void)arg;
    uint8_t buf[DS_UART_RX_BUF_SIZE];
    uint16_t len = 0;
    uint8_t chunk[64];

    while (g_rx_running)
    {
        ssize_t rd = read(g_uart_fd, chunk, sizeof(chunk));
        if (rd > 0)
        {
            for (ssize_t i = 0; i < rd; i++)
            {
                if (len < sizeof(buf))
                {
                    buf[len++] = (uint8_t)chunk[i];
                }
                else
                {
                    LOG_ERROR("uart rx overflow, reset buffer\n");
                    len = 0;
                }
            }
            uart_try_parse(buf, &len);
        }
        else if (rd < 0 && errno != EAGAIN && errno != EINTR)
        {
            LOG_ERROR("uart read err: %s\n", strerror(errno));
            usleep(20000);
        }
        else
        {
            usleep(2000);
        }
    }
    return NULL;
}

int uart_proto_open(const char *device_path, unsigned int baudrate)
{
    if (g_uart_fd >= 0)
    {
        return 0;
    }

    g_uart_debug = uart_env_debug_enabled();

    int fd = open(device_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        LOG_ERROR("open %s failed: %s\n", device_path, strerror(errno));
        return -1;
    }

    if (uart_configure_port(fd, baudrate) != 0)
    {
        LOG_ERROR("configure %s failed\n", device_path);
        close(fd);
        return -2;
    }

    g_uart_fd = fd;
    LOG_INFO("uart opened %s @ %u (debug=%d)\n", device_path, baudrate, g_uart_debug);
    return 0;
}

int uart_proto_start_rx(void)
{
    if (g_uart_fd < 0)
    {
        return -1;
    }
    if (g_rx_started)
    {
        return 0;
    }

    g_rx_running = 1;
    if (pthread_create(&g_rx_thread, NULL, uart_rx_thread, NULL) != 0)
    {
        g_rx_running = 0;
        return -2;
    }
    g_rx_started = 1;
    LOG_INFO("uart rx thread started\n");
    return 0;
}

int uart_proto_init(const char *device_path, unsigned int baudrate)
{
    if (uart_proto_open(device_path, baudrate) != 0)
    {
        return -1;
    }
    return uart_proto_start_rx();
}

void uart_proto_deinit(void)
{
    if (g_rx_started)
    {
        g_rx_running = 0;
        pthread_join(g_rx_thread, NULL);
        g_rx_started = 0;
    }
    if (g_uart_fd >= 0)
    {
        close(g_uart_fd);
        g_uart_fd = -1;
    }
}

int uart_proto_send_rsp(uint8_t cmd_id, uint8_t seq, const uint8_t *payload, uint16_t payload_len)
{
    return uart_send_frame(DS_UART_MSG_RSP, cmd_id, seq, payload, payload_len);
}

int uart_proto_send_evt(uint8_t cmd_id, uint8_t seq, const uint8_t *payload, uint16_t payload_len)
{
    return uart_send_frame(DS_UART_MSG_EVT, cmd_id, seq, payload, payload_len);
}

void uart_proto_set_rx_callback(ds_uart_frame_cb_t cb, void *user_data)
{
    g_rx_cb = cb;
    g_rx_user = user_data;
}
