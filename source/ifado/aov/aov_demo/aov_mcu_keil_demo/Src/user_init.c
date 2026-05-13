#include "user_init.h"

typedef enum{
        RTCS = 0,
        PIR,
        WIFI,
} WakeSource;

typedef enum{
        POWER_H = 0,
        POWER_L,
} PowerLevel;

typedef enum{
        SOC_ON = 0,
        SOC_OFF,
} SOCStatus;


uint8_t sWakeSrc = 0;
uint8_t sPowerLevel = 0;
uint8_t sSocStatus;
uint8_t sRTCValueLow;
uint8_t sRTCValueHigh;

void ClockInit(uint32_t clock)
{
    switch (clock)
    {
        case FL_RCC_RCHF_FREQUENCY_8MHZ:
            FL_RCC_RCHF_WriteTrimValue(RCHF8M_TRIM);
            break;

        case FL_RCC_RCHF_FREQUENCY_16MHZ:
            FL_RCC_RCHF_WriteTrimValue(RCHF16M_TRIM);
            break;

        case FL_RCC_RCHF_FREQUENCY_24MHZ:
            FL_RCC_RCHF_WriteTrimValue(RCHF24M_TRIM);
            break;

        default:
            FL_RCC_RCHF_WriteTrimValue(RCHF8M_TRIM);
            break;
    }

    FL_RCC_RCHF_SetFrequency(clock);
    FL_RCC_SetSystemClockSource(FL_RCC_SYSTEM_CLK_SOURCE_RCHF);
}

static void SystickInit(void)
{
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}
#if 0
void FoutInit(void)
{
    FL_GPIO_InitTypeDef init = {0};

    init.pin = FL_GPIO_PIN_11;
    init.mode = FL_GPIO_MODE_DIGITAL;
    init.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    init.pull = DISABLE;
    FL_GPIO_Init(GPIOD, &init);

    FL_GPIO_SetFOUT0(GPIO, FL_GPIO_FOUT0_SELECT_AHBCLK_DIV64);
}
#endif

#if(DEBUG_EN == 1)
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
    int handle;
};
FILE __stdout;

//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
    x = x;
}

//重定义fputc函数
int fputc(int ch, FILE *f)
{
    FL_UART_WriteTXBuff(UART4, (uint8_t)ch);
    while(FL_UART_IsActiveFlag_TXBuffEmpty(UART4) != SET);
    return ch;
}

void DebugUartInit(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_UART_InitTypeDef    defaultInitStruct;

    GPIO_InitStruct.pin = FL_GPIO_PIN_0;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = DISABLE;
    GPIO_InitStruct.remapPin = DISABLE;

    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.pin = FL_GPIO_PIN_1;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = DISABLE;
    GPIO_InitStruct.remapPin = DISABLE;

    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    defaultInitStruct.clockSrc = 0;
    defaultInitStruct.baudRate = 38400;
    defaultInitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    defaultInitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
    defaultInitStruct.parity = FL_UART_PARITY_EVEN;
    defaultInitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;

    FL_UART_Init(UART4, &defaultInitStruct);

    printf("\r\nUart4 debug enabled\r\n");
}
#endif

void UserInit(void)
{
    SystickInit();

#if(DEBUG_EN == 1)
    //LedInit();
    DebugUartInit();
#endif
}

void DelayUs(uint32_t count)
{
    count = (uint64_t)FL_RCC_GetSystemClockFreq() * count / 1000000;
    count = count > 16777216 ? 16777216 : count;

    SysTick->LOAD = count - 1;
    SysTick->VAL = 0;
    while (!((SysTick->CTRL >> 16) & 0x1));
}

void DelayMs(uint32_t count)
{
    while (count--)
    {
        DelayUs(1000);
    }
}

void GPIO_IRQHandler(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;
    if(FL_GPIO_IsActiveFlag_EXTI(GPIO,AOV_SUSPEND_EXTI_LINE))
    {
        FL_GPIO_ClearFlag_EXTI(GPIO,AOV_SUSPEND_EXTI_LINE);

#if 0
        if(sWakeSrc == WIFI)
        {
            FL_GPIO_ResetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);

            DelayMs(AOV_POWER_OFF_TIME_WIFI_MS);

            FL_GPIO_ClearFlag_EXTI(GPIO,AOV_SUSPEND_EXTI_LINE);
            FL_GPIO_SetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);
        }
#endif

        GPIO_InitStruct.pin = AOV_UART_TX_GPIO_PIN|AOV_UART_RX_GPIO_PIN;
        GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
        GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.pull = DISABLE;
        GPIO_InitStruct.remapPin = DISABLE;

        FL_GPIO_Init(AOV_UART_TX_GPIO_GROUP, &GPIO_InitStruct);
        FL_UART_DisableTX(AOV_UART_NUMBER);
        if(sPowerLevel == POWER_H)
        {
            FL_GPIO_ResetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);

            DelayMs(AOV_POWER_OFF_TIME_HIGH_MS);

            FL_GPIO_ClearFlag_EXTI(GPIO,AOV_SUSPEND_EXTI_LINE);
            FL_GPIO_SetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);
        }
        else if(sPowerLevel == POWER_L)
        {
            FL_GPIO_ResetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);
            FL_GPIO_ResetOutputPin(AOV_SRAM_PWR_CTR_GPIO_GROUP, AOV_SRAM_PWR_CTR_GPIO_PIN);

            DelayMs(AOV_POWER_OFF_TIME_LOW_MS);

            FL_GPIO_ClearFlag_EXTI(GPIO,AOV_SUSPEND_EXTI_LINE);
            FL_GPIO_SetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);
            FL_GPIO_SetOutputPin(AOV_SRAM_PWR_CTR_GPIO_GROUP, AOV_SRAM_PWR_CTR_GPIO_PIN);
        }
        GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
        FL_GPIO_Init(AOV_UART_TX_GPIO_GROUP, &GPIO_InitStruct);

        FL_UART_EnableTX(AOV_UART_NUMBER);
    }
}
void AOV_GPIO_Init(void)
{
    FL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    //PA5     SYS_PWR_CTR
    //FL_GPIO_ResetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);
    GPIO_InitStruct.pin = AOV_SYS_PWR_CTR_GPIO_PIN;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = DISABLE;
    GPIO_InitStruct.remapPin = DISABLE;
    FL_GPIO_Init(AOV_SYS_PWR_CTR_GPIO_GROUP, &GPIO_InitStruct);

    //PA6     SRAM_PWR_CTR
    //FL_GPIO_ResetOutputPin(AOV_SRAM_PWR_CTR_GPIO_GROUP, AOV_SRAM_PWR_CTR_GPIO_PIN);
    GPIO_InitStruct.pin = AOV_SRAM_PWR_CTR_GPIO_PIN;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = DISABLE;
    GPIO_InitStruct.remapPin = DISABLE;
    FL_GPIO_Init(AOV_SRAM_PWR_CTR_GPIO_GROUP, &GPIO_InitStruct);

    //PA15 GPIO IRQ    SUSPEND_GPIO    input from SOC
    FL_GPIO_ResetOutputPin(AOV_SUSPEND_GPIO_GROUP, AOV_SUSPEND_GPIO_PIN);
    GPIO_InitStruct.pin = AOV_SUSPEND_GPIO_PIN;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = DISABLE;
    GPIO_InitStruct.remapPin = DISABLE;
    FL_GPIO_Init(AOV_SUSPEND_GPIO_GROUP, &GPIO_InitStruct);

    FL_RCC_EnableGroup1OperationClock(FL_RCC_GROUP1_OPCLK_EXTI);

    FL_GPIO_SetTriggerEdge(GPIO,AOV_SUSPEND_EXTI_LINE,FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE);
    FL_GPIO_SetExtiLine8(GPIO,AOV_SUSPEND_EXTI_LINE_PIN);
    FL_GPIO_EnableDigitalFilter(GPIO, AOV_SUSPEND_EXTI_LINE);
    FL_GPIO_SetTriggerEdge(GPIO,AOV_SUSPEND_EXTI_LINE,FL_GPIO_EXTI_TRIGGER_EDGE_FALLING);
    FL_GPIO_ClearFlag_EXTI(GPIO,AOV_SUSPEND_EXTI_LINE);

    NVIC_DisableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn,2);
    NVIC_EnableIRQ(GPIO_IRQn);
}

void AOV_RESET_SYS_SRAM_POWER(void)
{
    FL_GPIO_ResetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);
    FL_GPIO_ResetOutputPin(AOV_SRAM_PWR_CTR_GPIO_GROUP, AOV_SRAM_PWR_CTR_GPIO_PIN);

    FL_GPIO_SetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);
    FL_GPIO_SetOutputPin(AOV_SRAM_PWR_CTR_GPIO_GROUP, AOV_SRAM_PWR_CTR_GPIO_PIN);
}

/***************************************************************** I2C *****************************************************************/
#define    I2CREAD         1    //I2C读操作
#define    I2CWRITE        0    //I2C写操作

#define    STARTBIT        0
#define    RESTARTBIT      1
#define    STOPBIT         2

#define DEVICE_P12T42     0xE8         //power 12v to 4.2v 的器件地址
#define ADDRLEN_P12T42    1            //power 12v to 4.2v的地址长度

uint8_t I2C_Send_Bit(uint8_t BIT_def )
{
    switch(BIT_def)
    {
        case STARTBIT:
            FL_I2C_Master_EnableI2CStart(I2C);
            while(!FL_I2C_Master_IsActiveFlag_Start(I2C));
            break;

        case RESTARTBIT:
            FL_I2C_Master_EnableI2CRestart(I2C);
            while(!FL_I2C_Master_IsActiveFlag_Start(I2C));
            break;

        case STOPBIT:
            FL_I2C_Master_EnableI2CStop(I2C);
            while(!FL_I2C_Master_IsActiveFlag_Stop(I2C));
            break;

        default:
            break;
    }

    return 0; //ok

}

uint8_t I2C_Send_Byte( uint8_t x_byte )
{

    FL_I2C_Master_WriteTXBuff(I2C,x_byte);

    while(!FL_I2C_Master_IsActiveFlag_TXComplete(I2C));
    FL_I2C_Master_ClearFlag_TXComplete(I2C);

    if(!FL_I2C_Master_IsActiveFlag_NACK(I2C))
    {
        return 0;
    }
    else
    {
        FL_I2C_Master_ClearFlag_NACK(I2C);
        return 1;
    }

}

uint8_t I2C_Receive_Byte( uint8_t *x_byte )
{
    //i2c en, rcen
    FL_I2C_Master_EnableRX(I2C);
    while(!FL_I2C_Master_IsActiveFlag_RXComplete(I2C));
    FL_I2C_Master_ClearFlag_RXComplete(I2C);
    *x_byte=FL_I2C_Master_ReadRXBuff(I2C);
    return 0;
}

uint8_t Sendaddr( uint8_t Device, uint16_t Addr, uint8_t AddrLen, uint8_t Opt )
{
    uint8_t result, Devi_Addr;

    Devi_Addr = Device;


    //-------------- start bit -------------
    result = I2C_Send_Bit( STARTBIT );//发送起始位
    if( result != 0 ) return 1; //failure.

    //-------------- disable read -------------
    FL_I2C_Master_DisableRX(I2C);
    //-------------- device addr -------------
    result = I2C_Send_Byte( Devi_Addr );//发送器件地址
    if( result != 0 ) return 2; //failure.

    //--------------- data addr --------------
    if(AddrLen == 2)
    {
        result = I2C_Send_Byte( Addr>>8 );//发送数据地址高8位
        if( result != 0 ) return 3; //failure.
    }
    result = I2C_Send_Byte( Addr>>0 );//发送数据地址低8位
    if( result != 0 ) return 3; //failure.

    if( Opt == I2CREAD ) //读操作
    {
        result = I2C_Send_Bit( RESTARTBIT );//发送重起始位
        if( result != 0 ) return 5; //failure.

        result = I2C_Send_Byte( Devi_Addr|1 );//发送器件地址，读取
        if( result != 0 ) return 5; //failure.
    }

    return 0; //ok
}


uint8_t Wait_for_end( uint8_t Device )
{
    uint8_t result, Devi_Addr;

    Devi_Addr = Device;

    SysTick->LOAD = 0x1000000-1;
    SysTick->VAL = 0;
    Do_DelayStart(); //需要5ms的内部写周期
    {
        I2C_Send_Bit( STARTBIT );    //发送起始位

        result = I2C_Send_Byte( Devi_Addr ); //发送器件地址

        I2C_Send_Bit( STOPBIT ); //发送停止位

        if( result == 0 ) return 0; //设置地址成功，写周期结束

    }While_DelayMsEnd(5);

    return 1; //设置地址失败
}

uint8_t I2C_Write_Bottom(uint8_t Device, uint16_t Addr, uint8_t AddrLen, uint8_t *Buf, uint8_t Len)
{
    uint8_t k, n, status;

    if(Len > 128) return 0xFF;//一次最多操作128字节

    for( k=0; k<1; k++ )//每遍最多写3次
    {
        status = 0;
        if( Sendaddr( Device, Addr, AddrLen, I2CWRITE ) )
        {
          status = 1; //写入失败
        }
        else
        {
            for( n=0; n<Len; n++ )
            {
                //发送一个字节
                if( I2C_Send_Byte( Buf[n] ) )
                {
                  status = 1;
                  break;
                } //写入失败
            }
        }
        //发送停止位
        if( I2C_Send_Bit( STOPBIT ) )
            status = 1;

        if( status == 0 )
        {
          Wait_for_end(Device);
          break;
        } //写正确
    }

    return( status );
}


uint8_t I2C_Read_Bottom(uint8_t Device, uint16_t Addr, uint8_t AddrLen, uint8_t *Buf, uint8_t Len)
{
    uint8_t k, n, status;

    if(Len > 128) return 0xFF;//一次最多操作128字节

    for( k=0; k<3; k++ )//每遍最多读3次
    {
        status = 0;
        if( Sendaddr( Device, Addr, AddrLen, I2CREAD ) )
      status = 1; //写入失败
        else
        {
            for( n=0; n<Len; n++ )
            {
                if( n < (Len-1) )
                    FL_I2C_Master_SetRespond(I2C,FL_I2C_MASTER_RESPOND_ACK);
                else
                    FL_I2C_Master_SetRespond(I2C,FL_I2C_MASTER_RESPOND_NACK);

                //接受一个字节
                if( I2C_Receive_Byte( Buf+n ) )
        {
          status = 1;
          break;
        }

            }
        }
        //发送停止位
        if( I2C_Send_Bit( STOPBIT ) )
      status = 1;    //失败

        if( status == 0 ) break;//读正确
    }

    return( status );
}


void AOV_I2C_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_I2C_MasterMode_InitTypeDef    defaultInitStruct;

    GPIO_InitStruct.pin = FL_GPIO_PIN_11;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = DISABLE;
    GPIO_InitStruct.remapPin = DISABLE;

    FL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    GPIO_InitStruct.pin = FL_GPIO_PIN_12;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = DISABLE;
    GPIO_InitStruct.remapPin = DISABLE;

    FL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    defaultInitStruct.clockSource = FL_RCC_I2C_CLK_SOURCE_RCHF;
    defaultInitStruct.baudRate = 40000;

    FL_I2C_MasterMode_Init(I2C,&defaultInitStruct );


    FL_I2C_Master_EnableIT_Start(I2C);
}


uint8_t AOV_I2C_P12To42_Enable(void)
{
    uint8_t Device = DEVICE_P12T42;
    uint8_t AddrLen = ADDRLEN_P12T42;
    uint8_t Len = 0;
    uint8_t Result = 0;
    uint8_t i = 0;
    uint8_t RegAddr[6] = {0x06, 0x07, 0x08, 0x09, 0x0A, 0x0C};
    uint8_t RegValue[6] = {0x3F, 0x6D, 0x3A, 0x04, 0x21, 0x22};

    uint8_t RegValue_R_Test[6];

    Len = 1;
    for(i = 0; i < 6; i++)
    {
        Result = I2C_Write_Bottom(Device, RegAddr[i], AddrLen, &RegValue[i], Len);
    }

    memset(RegValue_R_Test, 0, 6);
    for(i = 0; i < 6; i++)
    {
        Result = I2C_Read_Bottom(Device, RegAddr[i], AddrLen, &RegValue_R_Test[i], Len);
    }

    return Result;
}



/***************************************************************** UART0 *****************************************************************/
uint8_t rxData[8] = {0};
int rxCount = 0;
int rxDone = 0;
int cmdLen = 0;

void UART0_IRQHandler(void)
{
    if((ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART0))
        &&(SET == FL_UART_IsActiveFlag_RXBuffFull(UART0)))
    {
        rxData[rxCount] = FL_UART_ReadRXBuff(UART0);
        if(rxCount == 0)
        {
            if(rxData[0] == UART_PD_CMD_SEQ_HEAD)
            {
                cmdLen = UART_PD_CMD_SEQ_LEN;
            }
            else if((uint8_t)rxData[0] > AOV_MAX_UART_CMD_LEN)
            {
                return;
            }
            else
            {
                cmdLen = rxData[0];
            }
        }
#if (DEBUG_EN == 1)
        FL_UART_WriteTXBuff(UART4, rxData[rxCount]);
        while(SET != FL_UART_IsActiveFlag_TXBuffEmpty(UART4));
#endif
        rxCount++;
        if(rxCount >= cmdLen)
        {
#if (DEBUG_EN == 1)
            rxData[rxCount] = '\0';
#endif
            rxCount = 0;
            rxDone = 1;
        }
        else{
            rxDone = 0;
        }
    }

}

void AOV_UART_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_UART_InitTypeDef    defaultInitStruct;

    GPIO_InitStruct.pin = AOV_UART_TX_GPIO_PIN;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = DISABLE;
    GPIO_InitStruct.remapPin = DISABLE;

    FL_GPIO_Init(AOV_UART_TX_GPIO_GROUP, &GPIO_InitStruct);

    GPIO_InitStruct.pin = AOV_UART_RX_GPIO_PIN;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = DISABLE;
    GPIO_InitStruct.remapPin = DISABLE;

    FL_GPIO_Init(AOV_UART_RX_GPIO_GROUP, &GPIO_InitStruct);

    defaultInitStruct.clockSrc = FL_RCC_UART0_CLK_SOURCE_APB1CLK;
    defaultInitStruct.baudRate =     AOV_UART_BAUDRATE;
    defaultInitStruct.dataWidth =     AOV_UART_DATA_BIT;
    defaultInitStruct.stopBits =     AOV_UART_STOP;
    defaultInitStruct.parity =         AOV_UART_PARITY;
    defaultInitStruct.transferDirection = AOV_UART_DIRECTION_TX_RX;

    FL_UART_Init(AOV_UART_NUMBER, &defaultInitStruct);

    NVIC_DisableIRQ(AOV_UART_INTERRUPT_NUMBER);
    NVIC_SetPriority(AOV_UART_INTERRUPT_NUMBER,2);
    NVIC_EnableIRQ(AOV_UART_INTERRUPT_NUMBER);

    sWakeSrc = RTCS;
    sPowerLevel = POWER_H;
    sSocStatus = SOC_ON;
    sRTCValueLow = 0;
    sRTCValueHigh = 0;
    rxCount = 0;
    rxDone = 0;
    cmdLen = 0;
    memset(rxData,0x00,sizeof(rxData));
}

void Uart0_4_RxTx(void)
{
    uint8_t TestTxData[10] = {'U', 'R', 'T', 'E', 'S', 'B', 'E', 'G', 'I', 10};//{0x55,0x4A,0x01};
    uint8_t i;
    volatile uint8_t tmp08;

    for(i=0; i<10; i++)
    {
        FL_UART_WriteTXBuff(UART0, TestTxData[i]);
        while(SET != FL_UART_IsActiveFlag_TXBuffEmpty(UART0));
    }

    for(i=0; i<10; i++)
    {
        FL_UART_WriteTXBuff(UART4, TestTxData[i]);
        while(SET != FL_UART_IsActiveFlag_TXBuffEmpty(UART4));
    }

    #if 0
    while(1)
    {
        if(SET == FL_UART_IsActiveFlag_RXBuffFull(UART0))
        {
            tmp08 = FL_UART_ReadRXBuff(UART0);
            FL_UART_WriteTXBuff(UART0, tmp08+1);
            TestTxData[8] = tmp08;
            for(i=0; i<10; i++)
            {
                FL_UART_WriteTXBuff(UART0, TestTxData[i]);
                while(SET != FL_UART_IsActiveFlag_TXBuffEmpty(UART0));
            }
        }
    }
    #endif

}

void PowerDownOnSOCSRAM()
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    NVIC_DisableIRQ(GPIO_IRQn);

    GPIO_InitStruct.pin = AOV_UART_TX_GPIO_PIN|AOV_UART_RX_GPIO_PIN;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = DISABLE;
    GPIO_InitStruct.remapPin = DISABLE;

    FL_GPIO_Init(AOV_UART_TX_GPIO_GROUP, &GPIO_InitStruct);
    FL_UART_DisableTX(AOV_UART_NUMBER);
    if(sPowerLevel == POWER_H)
    {
        FL_GPIO_ResetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);
        DelayMs(AOV_POWER_OFF_TIME_HIGH_MS);
        GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
        FL_GPIO_Init(AOV_UART_TX_GPIO_GROUP, &GPIO_InitStruct);
        FL_UART_EnableTX(AOV_UART_NUMBER);
        FL_GPIO_SetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);
    }
    else if(sPowerLevel == POWER_L)
    {
        FL_GPIO_ResetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);
        FL_GPIO_ResetOutputPin(AOV_SRAM_PWR_CTR_GPIO_GROUP, AOV_SRAM_PWR_CTR_GPIO_PIN);
        DelayMs(AOV_POWER_OFF_TIME_LOW_MS);
        GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
        FL_GPIO_Init(AOV_UART_TX_GPIO_GROUP, &GPIO_InitStruct);
        FL_UART_EnableTX(AOV_UART_NUMBER);
        FL_GPIO_SetOutputPin(AOV_SYS_PWR_CTR_GPIO_GROUP, AOV_SYS_PWR_CTR_GPIO_PIN);
        FL_GPIO_SetOutputPin(AOV_SRAM_PWR_CTR_GPIO_GROUP, AOV_SRAM_PWR_CTR_GPIO_PIN);
    }

    FL_GPIO_ClearFlag_EXTI(GPIO,AOV_SUSPEND_EXTI_LINE);
    NVIC_SetPriority(GPIO_IRQn,2);
    NVIC_EnableIRQ(GPIO_IRQn);
}



void SOC_Monitor(void)
{
    char wakeSrcFlag = 'T';
    uint8_t strFlag = 0;

    FL_UART_EnableIT_RXBuffFull(AOV_UART_NUMBER);

    while(1)
    {
        DelayMs(1);
        if(rxDone == 1)
        {
#if (DEBUG_EN == 1)
            printf("Rx: %s\r\n", (char *)rxData);
#endif
            //Uart command trigger SOC power down/on
            if((rxData[0] == UART_PD_CMD_SEQ_HEAD) &&
                (rxData[1] == UART_PD_CMD_SEQ_LEN) &&
                (rxData[2] == UART_PD_CMD_SEQ_CMD_TYPE) &&
                (rxData[3] == UART_PD_CMD_SEQ_DATA_CHECKSUM))
            {
                PowerDownOnSOCSRAM();
            }
            else if(NULL != strstr((char *)rxData, "high"))
            {
                if(POWER_L == sPowerLevel)  //Back to timer wake up if power change from low to high
                {
                    sWakeSrc = RTCS;
                    sPowerLevel = POWER_H;
                }
                sRTCValueLow = ((sRTCValueLow&(~AOV_POWER_LEVEL_MASK)) | (AOV_POWER_LEVEL_HIGH|AOV_POWER_LEVEL_BIT_IS_VALID));
                FL_UART_WriteTXBuff(AOV_UART_NUMBER, 'H');
            }
            else if(NULL != strstr((char *)rxData, "low"))
            {
                if(POWER_H == sPowerLevel)  //Change to PIR wake up if power change from High to low
                {
                    sWakeSrc = PIR;
                    sPowerLevel = POWER_L;
                }
                sRTCValueLow = ((sRTCValueLow&(~AOV_POWER_LEVEL_MASK)) | AOV_POWER_LEVEL_BIT_IS_VALID);
                FL_UART_WriteTXBuff(AOV_UART_NUMBER, 'L');
            }
            else if(NULL != strstr((char *)rxData, "timer"))
            {
                sWakeSrc = RTCS;
                strFlag = 1;
                FL_UART_WriteTXBuff(AOV_UART_NUMBER, 'T');
            }
            else if(NULL != strstr((char *)rxData, "wifi"))
            {
                sWakeSrc = WIFI;
                strFlag = 1;
                FL_UART_WriteTXBuff(AOV_UART_NUMBER, 'W');
            }
            else if(NULL != strstr((char *)rxData, "pir"))
            {
                sWakeSrc = PIR;
                strFlag = 1;
                FL_UART_WriteTXBuff(AOV_UART_NUMBER, 'P');
            }
            else if((AOV_UART_SET_STR_PASSWORD_CMD_LEN == cmdLen) && (AOV_UART_SET_STR_PASSWORD_CMD_HEAD == rxData[1]))
            {
                sRTCValueLow = rxData[2];
                sRTCValueHigh = rxData[3];
            }
            else if((AOV_UART_GET_STR_PASSWORD_CMD_LEN == cmdLen) && (AOV_UART_GET_STR_PASSWORD_CMD_HEAD == rxData[1]))
            {
                FL_UART_WriteTXBuff(AOV_UART_NUMBER, sRTCValueLow);

#if (DEBUG_EN == 1)
                printf("TX:0x%x ",sRTCValueLow);
#endif
                while(SET != FL_UART_IsActiveFlag_TXBuffEmpty(AOV_UART_NUMBER));
                FL_UART_WriteTXBuff(AOV_UART_NUMBER, sRTCValueHigh);

#if (DEBUG_EN == 1)
                printf("0x%x\n",sRTCValueHigh);
#endif
            }
            else if(NULL != strstr((char *)rxData, "getsrc"))
            {
                if(WIFI == sWakeSrc)
                {
                    wakeSrcFlag = 'W';
                }
                else if(PIR == sWakeSrc)
                {
                    wakeSrcFlag = 'P';
                }
                else
                {
                    wakeSrcFlag = 'T';
                }
                FL_UART_WriteTXBuff(AOV_UART_NUMBER, wakeSrcFlag);

#if (DEBUG_EN == 1)
                printf("TX:0x%x \n",wakeSrcFlag);
#endif
            }

            if(1 == strFlag)
            {
                if(POWER_H == sPowerLevel)
                {
                    sRTCValueLow = (sRTCValueLow&(~AOV_STR_PASSWORD_LOW_MASK))|AOV_STR_PASSWORD_LOW;
                    sRTCValueHigh = AOV_STR_PASSWORD_HIGH;
                }
                else
                {
                    sRTCValueLow = sRTCValueLow&(~AOV_STR_PASSWORD_LOW_MASK);
                    sRTCValueHigh = 0x00;
                }
                strFlag = 0;
            }
#if (DEBUG_EN == 1)
            printf("PL %d, WS %d, WF %c\r\n", sPowerLevel, sWakeSrc, wakeSrcFlag);
#endif
            cmdLen = 0;
            rxDone = 0;
        }
    }

}


