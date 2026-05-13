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


#define ARRAY_SIZE(array)    ((int)(sizeof(array) / sizeof(array[0])))

#define DEV_NAME        "/dev/ttyS1" //默认要测试的tty设备
#define TEST_COUNT        10
//#define SUPPORT_485
#define SENDBUF

#define COLOR_NONE          "\033[0m"
#define COLOR_GREEN         "\033[0;32m"
#define COLOR_RED           "\033[0;31m"

#define ST_DBG(fmt, args...) ({do{printf(COLOR_GREEN"[DBG]:%s[%d]: "COLOR_NONE, __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})
#define ST_ERR(fmt, args...) ({do{printf(COLOR_RED"[ERR]:%s[%d]: "COLOR_NONE, __FUNCTION__,__LINE__);printf(fmt, ##args);}while(0);})

#define UART_UT_READ
#define UART_UT_WRITE
static char protocol_data = 0;
static char *protocol_pority = 0;
static char protocol_stop = 0;
static char protocol_ctsrts = 0;
static const char *tty_device = DEV_NAME;
static unsigned int baudrate = 0;
struct timeval stPreStamp;
struct timeval stCurStamp;


static void parse_opts(int argc, char *argv[])
{

    while (1) {
        static const struct option lopts[] = {
            { "device",  1, 0, 'D' },
            { "baudrate",1, 0, 'b' },
            { "data",    1, 0, 'd' },
            { "parity",  1, 0, 'p' },
            { "stop",    1, 0, 's' },
            { "CTSRTS",  1, 0, 'c' },
            { NULL, 0, 0, 0 },
        };
        int c;

        c = getopt_long(argc, argv, "D:b:d:p:s:c:",
                lopts, NULL);

        if (c == -1)
            break;

        switch (c) {
            case 'D':
                tty_device = optarg;
                break;
            case 'b':
                baudrate = atoi(optarg);
                break;
            case 'd':
                protocol_data = atoi(optarg);
                break;
            case 'p':
                protocol_pority = optarg;
                break;
            case 's':
                protocol_stop = atoi(optarg);
                break;
            case 'c':
                protocol_ctsrts = atoi(optarg);
                break;
            default:
                break;
        }
    }
}

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
            ST_DBG("Unsupported data size\n");
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
            ST_DBG("Unsupported stop bits\n");
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
            ST_DBG("Unsupported parity\n");
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
void  *ST_PollingTxDone( void  *args)
{
    int *fd = (int *)args;
    int lsr = 0;
    do
    {
        ioctl(*fd, TIOCSERGETLSR, &lsr);
    }while(!(lsr & TIOCSER_TEMT));
    system("./pull_high.sh");
    return NULL;
}

#ifdef UART_UT_WRITE
void ST_DebugWriteStatus(int fd)
{
    char szBuf[280];
    int i = 0;
    int len = 0;
    int rc=0;
    while(1)
    {
#ifdef SENDBUF
        for (i = 0; i < 280; i ++)
        {
            szBuf[i] = i;
        }
        len = write(fd, szBuf, 280);
#endif
    }

#ifdef SUPPORT_485
    pthread_t thread;
    pthread_attr_t thread_attr;
    struct sched_param param = {.sched_priority = 1};
    pthread_attr_init(&thread_attr);
    pthread_setschedparam(thread, SCHED_RR, &param);
    printf ( "Creating thread\n");
    rc = pthread_create(&thread, &thread_attr, ST_PollingTxDone, (void *)(&fd));
#endif
    ST_DBG("write over, len=%d\n", len);
}
/*
* 功能：写线程
*       函数当中通过文件描述符，完成串口发送动作
* 参数：
*      @args: 文件描述符
* 返回：void
*/
void *ST_WriteProcess(void *args)
{
    int *fd = (int *)args;

    char szBuf[280];
    int i = 0;
    int len = 0;
    int lsr = 0;


    printf("Write Power High:\n");
    gettimeofday(&stPreStamp, NULL);
    
    memset(szBuf, 0, sizeof(szBuf));

    szBuf[0] = strlen("high") + 1;
    memcpy(szBuf + 1, "high", 4);

    len = write(*fd, szBuf, szBuf[0]);
    do
    {
        ioctl(*fd, TIOCSERGETLSR, &lsr);
    } while(!(lsr & TIOCSER_TEMT));

    ST_DBG("write over, len=%d\n", len);
    usleep(5000 * 1000);


    printf("Write Power Low:\n");
    gettimeofday(&stPreStamp, NULL);
    memset(szBuf, 0, sizeof(szBuf));

    szBuf[0] = strlen("low") + 1;
    memcpy(szBuf + 1, "low", 3);
    len = write(*fd, szBuf, szBuf[0]);
    do
    {
        ioctl(*fd, TIOCSERGETLSR, &lsr);
    } while(!(lsr & TIOCSER_TEMT));

    ST_DBG("write over, len=%d\n", len);
    usleep(5000 * 1000);


    printf("Write timer wake up Mode:\n");
    gettimeofday(&stPreStamp, NULL);
    memset(szBuf, 0, sizeof(szBuf));

    szBuf[0] = strlen("timer") + 1;
    memcpy(szBuf + 1, "timer", 5);
    len = write(*fd, szBuf, szBuf[0]);
    do
    {
        ioctl(*fd, TIOCSERGETLSR, &lsr);
    } while(!(lsr & TIOCSER_TEMT));

    ST_DBG("write over, len=%d\n", len);
    usleep(5000 * 1000);

	
    printf("check wakeup source:\n");
    gettimeofday(&stPreStamp, NULL);
    memset(szBuf, 0, sizeof(szBuf));
	
    szBuf[0] = strlen("src") + 1;
    memcpy(szBuf + 1, "src", 3);
    len = write(*fd, szBuf, szBuf[0]);
    do
    {
        ioctl(*fd, TIOCSERGETLSR, &lsr);
    } while(!(lsr & TIOCSER_TEMT));

    ST_DBG("write over, len=%d\n", len);
    usleep(5000 * 1000);

	
    printf("Write wifi wake up Mode:\n");
    gettimeofday(&stPreStamp, NULL);
    memset(szBuf, 0, sizeof(szBuf));

    szBuf[0] = strlen("wifi") + 1;
    memcpy(szBuf + 1, "wifi", 4);
    len = write(*fd, szBuf, szBuf[0]);
    do
    {
        ioctl(*fd, TIOCSERGETLSR, &lsr);
    } while(!(lsr & TIOCSER_TEMT));

    ST_DBG("write over, len=%d\n", len);
    usleep(5000 * 1000);
	ST_DBG("write over, len=%d\n", len);
	usleep(5000 * 1000);

	    printf("check wakeup source:\n");
    gettimeofday(&stPreStamp, NULL);
    memset(szBuf, 0, sizeof(szBuf));
	
    szBuf[0] = strlen("src") + 1;
    memcpy(szBuf + 1, "src", 3);
    len = write(*fd, szBuf, szBuf[0]);
    do
    {
        ioctl(*fd, TIOCSERGETLSR, &lsr);
    } while(!(lsr & TIOCSER_TEMT));

    ST_DBG("write over, len=%d\n", len);
    usleep(5000 * 1000);
	
    printf("Write pir wake up Mode:\n");
	gettimeofday(&stPreStamp, NULL);
	memset(szBuf, 0, sizeof(szBuf));
	szBuf[0] = strlen("pir") + 1;
	memcpy(szBuf + 1, "pir", 3);
	len = write(*fd, szBuf, szBuf[0]);
	do
	{
	  ioctl(*fd, TIOCSERGETLSR, &lsr);
	} while(!(lsr & TIOCSER_TEMT));

	ST_DBG("write over, len=%d\n", len);
	usleep(5000 * 1000);
	
    printf("check wakeup source:\n");
    gettimeofday(&stPreStamp, NULL);
    memset(szBuf, 0, sizeof(szBuf));
	
    szBuf[0] = strlen("src") + 1;
    memcpy(szBuf + 1, "src", 3);
    len = write(*fd, szBuf, szBuf[0]);
    do
    {
        ioctl(*fd, TIOCSERGETLSR, &lsr);
    } while(!(lsr & TIOCSER_TEMT));

    ST_DBG("write over, len=%d\n", len);
    usleep(5000 * 1000);

    return NULL;
}
#endif

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
    char szBuf[1024];
    int len = 0;
    int i = 0;

    fd_set read_fds;
    struct timeval TimeoutVal;

    while(1)
    {
        FD_ZERO(&read_fds);
        FD_SET(*fd, &read_fds);

        TimeoutVal.tv_sec  = 1;
        TimeoutVal.tv_usec = 0;

        ret = select(*fd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if(ret < 0)
        {
            ST_ERR("select failed!\n");
            usleep(10 * 1000);
            continue;
        }
        else if(ret == 0)
        {
            ST_ERR("time out\n");
            usleep(10 * 1000);
            continue;
        }
        else
        {
            if(FD_ISSET(*fd, &read_fds))
            {
                memset(szBuf, 0, sizeof(szBuf));

                len = read(*fd, szBuf, sizeof(szBuf) -1);
                gettimeofday(&stCurStamp, NULL);

                printf("read len = %d,pre time:%d s,%d us, current time:%d s,%d us\r\n", len, stPreStamp.tv_sec, stPreStamp.tv_usec, stCurStamp.tv_sec, stCurStamp.tv_usec);
                for (i = 0; i < len; i ++)
                {
                    if (i % 16 == 0)
                        printf("\n");
                    printf("%c ", szBuf[i]);
                    //printf("%c ", szBuf[i]);
                }
                printf("\n");
            }
        }
    }

    return NULL;
}
#endif
int main(int argc, char **argv)
{
    int fd = -1;
#ifdef UART_UT_READ
    pthread_t readPt;
#endif
#ifdef UART_UT_WRITE
    pthread_t writePt;
#endif
    int ret = 0;

    parse_opts(argc, argv);

    fd = open(tty_device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (-1 == fd)
    {
        ST_ERR("can not open device %s\n", DEV_NAME);
        return -1;
    }

    ST_ComSetSpeed(fd, baudrate);

    if (0 != ST_ComSetParity(fd, protocol_data, protocol_stop, *protocol_pority))
    {
        ST_ERR("ST_ComSetParity error\n");
        return -1;
    }

#ifdef SUPPORT_485
    system("./pull_low.sh");
#endif
    #if 1
#ifdef UART_UT_READ
    ret = pthread_create(&readPt, NULL, ST_ReadProcess, (void *)&fd);
#endif
#ifdef UART_UT_WRITE
    ret = pthread_create(&writePt, NULL, ST_WriteProcess, (void *)&fd);
#endif

#ifdef UART_UT_WRITE
    pthread_join(writePt, NULL);
#endif
#ifdef UART_UT_READ
    pthread_join(readPt, NULL);
#endif
    #else
    ST_DebugWriteStatus(fd);
    #endif

    if (fd > 0)
    {
        close(fd);
        fd = -1;
    }

    return 0;
}
