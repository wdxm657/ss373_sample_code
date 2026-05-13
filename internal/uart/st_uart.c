#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "mi_common_datatype.h"
#include "st_common_aov.h"
#include "st_uart.h"

#undef ARRAY_SIZE
#define ARRAY_SIZE(array)    ((int)(sizeof(array) / sizeof(array[0])))

#define DEV_NAME        "/dev/ttyS1" //默认要测试的tty设备

#define COLOR_NONE          "\033[0m"
#define COLOR_GREEN         "\033[0;32m"
#define COLOR_RED           "\033[0;31m"

#define ST_UART_DBG(fmt, args...) ({do{printf(COLOR_GREEN"[DBG]:%s[%d]: "COLOR_NONE, __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})
#define ST_UART_ERR(fmt, args...) ({do{printf(COLOR_RED"[ERR]:%s[%d]: "COLOR_NONE, __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})

#define UART_UT_READ
#define UART_UT_WRITE
static char protocol_data = 0;
static char protocol_pority = 0;
static char protocol_stop = 0;
static char protocol_ctsrts = 0;
static unsigned int baudrate = 0;
//#define _DEBUG_TIME_
#ifdef _DEBUG_TIME_
struct timeval stPreStamp;
struct timeval stCurStamp;
#endif
volatile MI_U8 u8ReadDoneFlag = 0;
char recBuf[10];

int device_fd = -1;
static ST_WakeupType_e gWakeupSource = E_ST_WAKEUP_EMPTY;
int gAckFlag = 0;
ST_OSType_e gOsType = E_ST_OS_PURELINUX;
#ifdef UART_UT_READ
pthread_t readPt;
#endif

/*
* 功能：串口通信协议设定
* 参数：
*     @fd: 文件描述符
*     @databits: 数据位
*     @stopbits: 停止位
*     @parity： 奇偶校验位
* 返回：
*     @0: 成功
*     @-1: 失败
*
*/
int ST_ComSetParity(int fd, int databits, int stopbits, int parity)
{
    struct termios options;

    if (tcgetattr(fd, &options) != 0)
    {
        perror("SetupSerial 1");
        return -1;
    }
    options.c_cflag &= ~CSIZE;

    // 设置数据位数
    switch (databits)
    {
        case 5:
        {
            options.c_cflag |= CS5;
            break;
        }

        case 6:
        {
            options.c_cflag |= CS6;
            break;
        }

        case 7:
        {
            options.c_cflag |= CS7;
            break;
        }

        case 8:
        {
            options.c_cflag |= CS8;
            break;
        }

        default:
        {
            ST_UART_DBG("Unsupported data size\n");
            return -1;
        }
    }

    // 设 置停止位
    switch (stopbits)
    {
        case 1:
        {
            options.c_cflag &= ~CSTOPB;
            break;
        }

        case 2:
        {
            options.c_cflag |= CSTOPB;
            break;
        }

        default:
        {
            ST_UART_DBG("Unsupported stop bits\n");
            return -1;
        }
    }

    // 设置奇偶校验位
    switch (parity)
    {
        case 'n':
        case 'N':
        {
            options.c_cflag &= ~PARENB;    // Clear parity enable
            options.c_iflag &= ~IGNPAR;    // disable parity checking
            break;
        }

        case 'o':
        case 'O':    // 设置为奇效验
        {
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= (IGNPAR | INPCK);    // enable parity checking
            break;
        }

        case 'e':
        case 'E':    // 设置为偶效验
        {
            options.c_cflag |= PARENB;    // Enable parity
            options.c_cflag &= ~PARODD;
            options.c_iflag |= (IGNPAR | INPCK);    // enable parity checking
            break;
        }

        case 'S':
        case 's':  // as no parity
        {
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        }

        default:
        {
            ST_UART_DBG("Unsupported parity\n");
            return -1;
        }
    }

/*
* Enable XON/XOFF flow control on output
* IXOFF  Enable XON/XOFF flow control on input.
* IXANY  (XSI) Typing any character will restart stopped output.  (The default is to allow
*              just the START character to restart output.)
* ICRNL  Translate carriage return to newline on input (unless IGNCR is set)
* IGNCR  Ignore carriage return on input
* INLCR  Translate NL to CR on input
* ONLCR  (XSI) Map NL to CR-NL on output
* OCRNL  Map CR to NL on output
* ICANON Enable canonical mode
* ECHO   Echo input characters.
* ECHOE  If  ICANON is also set, the ERASE character erases the preceding input character,
*            and WERASE erases the preceding word.
* ISIG   When any of the characters INTR, QUIT, SUSP, or DSUSP are received, generate  the
*              corresponding signal
*/

    options.c_iflag &= ~(IXON | IXOFF | IXANY);    // avoid 0x13 stop the termios
    options.c_iflag &= ~(ICRNL | IGNCR | INLCR);
    options.c_oflag &= ~(ONLCR|OCRNL);
    //raw input mode
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //raw output
    options.c_oflag &= ~OPOST;

    if (protocol_ctsrts)
    {
        options.c_cflag |= CRTSCTS;
    }

    tcflush(fd, TCIFLUSH); // Update the options and do it NOW
    options.c_cc[VTIME] = 150; // 15 seconds
    options.c_cc[VMIN] = 0;

    if (tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("SetupSerial 3");
        return -1;
    }

    return 0;
}

/*
* 功能：设定串口波特率
* 参数：
*      @fd: 文件描述符
*      @speed: 波特率
* 返回：void
*/
void ST_ComSetSpeed(int fd, int speed)
{
    int speed_arr[] = {B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800, B2400, B4800, B9600,
                        B19200, B38400, B57600, B115200, B230400, B460800,  B576000, B921600, B1500000,
                        B2000000, B2500000, B3000000, B3500000, B4000000};
    int name_arr[] = {50,  75,    110,  134,    150,  200,    300,  600,    1200,  1800,  2400,  4800,    9600,
                        19200,  38400,  57600,  115200,  230400,  460800,   576000, 921600, 1500000,
                        2000000, 2500000, 3000000, 3500000, 4000000};
    int i = 0;
    struct termios Opt;

    tcgetattr(fd, &Opt);

    for (i = 0;  i < ARRAY_SIZE(speed_arr);  i++)
    {
        if  (speed == name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            if (tcsetattr(fd, TCSANOW, &Opt))
            {
                perror("tcsetattr fd1");
            }
            return;
        }
        tcflush(fd,TCIOFLUSH);
    }
}

ST_WakeupType_e ST_Uart_GetWakeSource()
{
    ST_WakeupType_e wakeupSource = gWakeupSource;
    gWakeupSource = E_ST_WAKEUP_EMPTY;
    if(E_ST_WAKEUP_EMPTY == wakeupSource)
    {
        ST_Uart_WriteCMD(eUartCMDSrc);  //keep polling source type
    }
    return wakeupSource;
}




void ST_Uart_MemsetRecvBuf(void)
{
    memset(recBuf, 0x0, 10);
}

//Send power state high or low to MCU
MI_S32 ST_Uart_WritePowerState(eUartPowerState ePowerState)
{
    MI_S32 ret = 0;
    ret = ST_Uart_WriteCMD(ePowerState + UART_CMD_POWER_STATE_OFFSET);   // eUartPowerState 与 eUartCMD 差值
    return ret;
}

//Send next wakeup source to MCU
MI_S32 ST_Uart_SetWakeupSource(eUartWakeSrcType eWakeupSrc)
{
    MI_S32 ret = 0;
    ret = ST_Uart_WriteCMD(eWakeupSrc + UART_CMD_WAKEUP_SRC_OFFSET);   // eUartWakeSrcType 与 eUartCMD 差值
    return ret;
}

MI_S32 ST_Uart_GetAckFlag(void)
{
    MI_S32 tmpAckFlag = gAckFlag;
    gAckFlag = 0;
    return tmpAckFlag;
}

MI_S32 ST_Uart_DealRecvBuf(char * recBuf)
{
    if(recBuf == NULL)
    {
        return -1;
    }


    if (recBuf[0] == 'T')
    {
        //printf("Timer wakeup source from MCU\n");
        gWakeupSource = E_ST_WAKEUP_TIMER;
        //ST_Uart_WriteCMD(eUartCMDAck);
        return 0;
    }
    else if (recBuf[0] == 'W')
    {
        //printf("WIFI wakeup source from MCU\n");
        gWakeupSource = E_ST_WAKEUP_PREVIEW;
        //ST_Uart_WriteCMD(eUartCMDAck);
        return 0;
    }
    else if (recBuf[0] == 'P')
    {
        //printf("PIR wakeup source from MCU\n");
        gWakeupSource = E_ST_WAKEUP_PIR;
        //ST_Uart_WriteCMD(eUartCMDAck);
        return 0;
    }
    else if (recBuf[0] == 'A')
    {
        //printf("recv buf ack\n");
        gAckFlag = 1;
        return 0;
    }

    int recBufLen = 0;
    recBufLen = recBuf[0];

    if(recBufLen > 9 || recBufLen <= 0)
    {
        // printf("recvBufLen err, %d, 0x%02x.\n", recBufLen, recBuf[0]);
        return -2;
    }

    // printf("recv buf %s, len %d\n", recBuf, recBufLen);

    // for (int i = 0; i < len; i ++)
    // {
    //     if (i % 16 == 0)
    //         printf("\n");
    //     printf("0x%02x ", recBuf[i]);
    // }
    // printf("\nend\n");



    if (strncmp(&recBuf[1], "suspend", recBufLen) == 0)
    {
        printf("enter suspend\n");
        system("echo mem > /sys/power/state");
        // 這裏的suspend的處理要修正，不能直接這樣。
    }
    else if (strncmp(&recBuf[1], "linux", recBufLen) == 0)
    {
        gOsType = E_ST_OS_PURELINUX;
        printf("getos purelinux\n");
    }
    else if (strncmp(&recBuf[1], "dualos", recBufLen) == 0)
    {
        gOsType = E_ST_OS_DUALOS;
        printf("getos dualos\n");
    }
    else if (strncmp(&recBuf[1], "getos", recBufLen) == 0)
    {
        printf("getos request\n");
        ST_OSType_e tmpOsType = ST_Common_AovOSCheck();
        if(tmpOsType == E_ST_OS_DUALOS)
        {
            ST_Uart_WriteCMD(eUartCMDDualos);
        }
        else
        {
            ST_Uart_WriteCMD(eUartCMDPurelinux);
        }
    }
    else if (strncmp(&recBuf[1], "switch", recBufLen) == 0)
    {
        printf("switchos request\n");
        ST_OSType_e tmpOsType = ST_Common_AovOSCheck();
        if(tmpOsType == E_ST_OS_DUALOS)
        {
            system("echo 3 > /sys/class/sstar/rtcpwc/save_in_sw3");
            system("reboot");
        }
        else
        {
            system("echo 3 > /sys/class/sstar/rtcpwc/save_in_sw3");
            system("reboot");
        }
    }
    else if (strncmp(&recBuf[1], "reboot", recBufLen) == 0)
    {
        printf("reboot request\n");
        system("reboot");
    }
    else
    {
        // printf("incorrect rec buf %s.\n", recBuf);
        // for (int i = 0; i < recBufLen; i ++)
        // {
        //     printf("0x%02x ", recBuf[i]);
        // }
        // printf("\n\n");
    }

    return 0;
}

MI_S32 ST_Uart_WriteCMD(eUartCMD eUartCmd)
{
    int *fd = (int *)&device_fd;
    MI_S32 send_len = 0;

    char szBuf[10];
    int lsr = 0;
#ifdef _DEBUG_TIME_
    gettimeofday(&stPreStamp, NULL);
#endif

    memset(szBuf, 0, sizeof(szBuf));
    memset(recBuf, 0, sizeof(recBuf));
    u8ReadDoneFlag = 0;
    switch(eUartCmd)
    {
        case eUartCMDReboot:
            send_len = strlen("reboot") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "reboot", 6);
            break;
        case eUartCMDSwitchOs:
            send_len = strlen("switch") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "switch", 6);
            break;
        case eUartCMDGetOs:
            send_len = strlen("getos") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "getos", 5);
            break;
        case eUartCMDDualos:
            send_len = strlen("dualos") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "dualos", 6);
            break;
        case eUartCMDPurelinux:
            send_len = strlen("linux") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "linux", 5);
            break;
        case eUartCMDSuspend:
            send_len = strlen("suspend") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "suspend", 7);
            break;
        case eUartCMDPowerHigh:
            send_len = strlen("high") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "high", 4);
            break;
        case eUartCMDPowerLow:
            send_len = strlen("low") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "low", 3);
            break;
        case eUartCMDWakeSrcTimer:
            send_len = strlen("timer") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "timer", 5);
            break;
        case eUartCMDWakeSrcWifi:
            send_len = strlen("wifi") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "wifi", 4);
            break;
        case eUartCMDWakeSrcPir:
            send_len = strlen("pir") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "pir", 3);
            break;
        case eUartCMDAck:
            send_len = 2;
            szBuf[0] = send_len;
            szBuf[1] = 'A';
            break;
        case eUartCMDSrc:
            send_len = strlen("getsrc") + 1;
            szBuf[0] = send_len;
            memcpy(szBuf + 1, "getsrc", 6);
            break;
        default:
            printf("incorrect cmd %d.\n", eUartCmd);
            break;
    }

#if 0
     printf("\033[31m writr buf %s \033[0m\n", szBuf);
     for (int i = 0; i < szBuf[0] + 1; i ++)
     {
         printf("0x%02x ", szBuf[i]);
     }
     printf("\n\n");
#endif

    write(*fd, szBuf, send_len);
    do
    {
        ioctl(*fd, TIOCSERGETLSR, &lsr);
    } while(!(lsr & TIOCSER_TEMT));


    return 0;

}

#ifdef UART_UT_READ
/*
* 功能：读线程
*       函数当中通过文件描述符，完成串口读数据动作
* 参数：
*       @args: 文件描述符
* 返回：void *
*/
void *ST_ReadProcess(void *pArgs)
{
    int *fd = (int *)pArgs;
    int ret = 0;
    int len = 0;

    fd_set read_fds;
    struct timeval TimeoutVal;

    while(1)
    {
        FD_ZERO(&read_fds);
        FD_SET(*fd, &read_fds);

        TimeoutVal.tv_sec  = 10;
        TimeoutVal.tv_usec = 0;

        ret = select(*fd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if(ret < 0)
        {
            ST_UART_ERR("select failed!\n");
            usleep(10 * 1000);
            continue;
        }
        else if(ret == 0)
        {
            usleep(1 * 1000);
            continue;
        }
        else
        {
            if(FD_ISSET(*fd, &read_fds))
            {

                //len = read(*fd, recBuf, sizeof(recBuf) -1);

                usleep(1 * 1000);   // too short will reception is incomplete for 38400

                memset(recBuf, 0x0, 10);
                len = read(*fd, recBuf, 10);

#ifdef _DEBUG_TIME_
                gettimeofday(&stCurStamp, NULL);

                printf("read len = %d,pre time:%d s,%d us, current time:%d s,%d us\r\n", len, stPreStamp.tv_sec, stPreStamp.tv_usec, stCurStamp.tv_sec, stCurStamp.tv_usec);
                for (i = 0; i < len; i ++)
                {
                    if (i % 16 == 0)
                        printf("\n");
                    printf("%02x ", recBuf[i]);
                }
                printf("\n");
#endif
                ST_Uart_DealRecvBuf(recBuf);
                u8ReadDoneFlag = 1;
            }
        }
    }

    return NULL;
}
#endif
int ST_UART_INIT(stUartParm* param)
{
    int ret = 0;

    baudrate = param->baudrate;
    protocol_data = param->data;
    protocol_pority = param->parity;
    protocol_stop = param->stop;
    protocol_ctsrts = param->ctsrts;
    printf("init %s, %d, %d, %d, %d, %d\n", param->tty_device, baudrate, protocol_data, protocol_pority, protocol_stop, protocol_ctsrts);


    device_fd = open(param->tty_device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (-1 == device_fd)
    {
        ST_UART_ERR("can not open device %s\n", param->tty_device);
        return -1;
    }

    ST_ComSetSpeed(device_fd, baudrate);

    if (0 != ST_ComSetParity(device_fd, protocol_data, protocol_stop, protocol_pority))
    {
        ST_UART_ERR("ST_ComSetParity error\n");
        return -1;
    }

#ifdef UART_UT_READ
    ret = pthread_create(&readPt, NULL, ST_ReadProcess, (void *)&device_fd);
#endif
#if 0
#ifdef UART_UT_READ
    pthread_join(readPt, NULL);
#endif

    if (device_fd > 0)
    {
        close(device_fd);
        device_fd = -1;
    }
#endif
    return ret;
}

