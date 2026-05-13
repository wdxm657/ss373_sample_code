# 1 概述

本文档用于介绍AOV场景下，MCU替换RTC进行休眠唤醒的软硬件方案MCU部分的实现。

MCU要实现功能：**SOC上下电** 、**唤醒SOC并上报唤醒源**、**存储STR标志和电量高低标志位**

MCU和SOC之间的引脚连接框图如下：

![*](.\mymedia\mcu_soc_pin_show.png "mcu_soc_pin_show.png")

**PA5/PB6** :        MCU给SOC的CPU和DRAM上电下电引脚，拉高供电，拉低断电

**PA15** :              MCU接收来自SOC的休眠通知引脚，下降沿中断

**PA13/PA14** :   MCU和SOC之间UART通讯的引脚

# 2 环境搭建

MCU已经实现了上图中的方案，源码在本目录下。需要将以下源码导入到keil 5 中：

- **Drivers** :              MCU芯片相关的driver源码
- **Inc** :                      头文件路径
- **MF-Config** :        模板工程自带
- **Src** :                     MCU在AOV场景下休眠唤醒主要的源码
- **release_bin**:       Src对应编译好的hex文件

**推荐的源码管理**

![keil_files](.\mymedia\keil_files1.png "keil_files")

源码怎么分布没有统一，编译无问题即可。

# 3 MCU流程

![MCU_flow](.\mymedia\MCU_flow.png "MCU_flow")

MCU存储着唤醒源类型，由于MCU未接入WIFI或PIR唤醒信号，为了模拟WIFI 和 PIR 唤醒流程，MCU存储的唤醒源由SOC在休眠或power off前，提前通过URAT命令来设置好，并在SOC唤醒后上报。因此MCU主流程只需轮询串口命令，记录MCU发过来的唤醒源类型，当收到唤醒源查询命令，就返回之前记录的唤醒源。

MCU同时还需要存储电量高低信息和STR 标志，利用MCU在STR期间不掉电的特性，SOC将STR 标志和电量信息存储在MCU中。当收到设置唤醒源的命令时，MCU内部会自动记录STR flag，当SOC resume或者正常上电时，IPL会通过uart命令来查询是否存在STR flag，以此判断当前是STR resume流程还是正常上电流程，同时IPL会根据电量高低来决定走Purelinux还是Dualos。

SOC 在进入STR后或者执行Power off命令时，会拉低Suspend GPIO（默认为高）来触发MCU Suspend GPIO中断，中断处理只需要根据SOC之前发送过来的电量来决定是只关闭CPU的电，还是CPU和DDR的电都关闭。随后MCU会延迟1S，并重新恢复供电。

# 4 SOC上下电

MCU通过PA5/PA6给SOC的CPU和DDR上下电。

## 4.2 上电阶段

MCU在上电阶段，会拉高PA5、PB6，这样SOC在打开电源开关后即可上电。

## 4.3 下电阶段

在AOV场景下，原本RTC会在SOC休眠的时候拉低各个电源，换成MCU之后，需要SOC来告诉MCU何时应该关闭电源。这里根据电量高低分两种情况：

### 4.3.1高电量：

高电量情况下，SOC会在休眠之前发送当前电量信息，和下一轮期望模拟的唤醒源类型给MCU，然后通过拉低Suspend GPIO（默认为高）来触发MCU suspend中断。

MCU收到这个信号后会立刻拉低PA5来关闭SOC的CPU电源，延时1S后恢复供电，达到周期唤醒的SOC的效果。

SOC被唤醒后会通过uart命令来查询唤醒源，此时MCU将先前记录的唤醒源类型上报给SOC，这样实现1s Timer 周期唤醒和模拟WIFI，PRI唤醒的效果。

### 4.3.2低电量：

低电量情况下SOC会先发送低电量信息和下一轮唤醒源类型给MCU（低电量只有PIR和WIFI唤醒源），随后拉低Suspend GPIO触发MCU Suspend中断，此时MCU会同时拉低PA5/PA6来关闭SOC的CPU和DDR电源，并在延时1S后恢复供电，SOC唤醒后Preload Demo会查询唤醒源，此时MCU将先前记录的唤醒源上报，达到模拟SOC低电量模式下被PIR或Preview唤醒的效果。

# 5 唤醒SOC

## 5.1 timer唤醒

指的是MCU定时唤醒SOC,这是AOV场景中最基本的唤醒方式。

MCU的计时是从收到SOC的休眠通知开始，目前timer唤醒的定时时间在MCU内部写死1s唤醒一次。

Timer唤醒在休眠时候关闭CPU电源，在定时时间后打开CPU电源，这要求SOC端拉低休眠引脚的时间尽可能接近休眠流程的末尾，否则可能出现MCU已经关闭了CPU电源但SOC还没有走完休眠流程的问题。

## 5.2 PIR、用户预览唤醒

参考以下框图，现在Demo实现的是由SOC接收键盘按键来设置PIR和WIFI事件，然后通过UART告知MCU，MCU在下次SOC被唤醒后查询唤醒源时，上报对应的唤醒源类型给SOC。

![PIR_Preview](.\mymedia\PIR_Preview.png)

这里demo中是以键盘输入字符来设置下一轮要模拟的唤醒源，键入的字符参考上图中的Key board 字符命令介绍。

# 6 调试相关

MCU 中存储着STR标志和电量高低标志位，IPL会根据这些信息来决定启动dualos还是purelinux，以及此次上电是正常上电还是STR resume上电

![PIR_Preview](.\mymedia\STRFlag_PowerLevel.png)

因此我们可以在不启动AOV demo的情况下来进行STR验证和Purelinux与Dualos的切换

## 6.1 STR测试

Purelinux启动后在板端下可以通过以下命令在不运行demo的情况下测试STR：

`stty -F /dev/ttyS1 -echo ispeed 9600 ospeed 9600 igncr`

`echo -ne "\x04\x73\xA0\xCB" > /dev/ttyS1`

`echo mem >/sys/power/state`


命令分别为配置UART，发送UART命令，执行STR，其中UART命令中‘\x04’代表命令长度为4 Byte，‘\x73’代表命令为设置sw0，‘\xA0\xCB’ 表示 STR_PASSWORD: 0xCBA


## 6.2 高电量切换低电量

Purelinux启动后可以通过以下命令切换至低电量模式并启动dualos：

`stty -F /dev/ttyS1 -echo ispeed 9600 ospeed 9600 igncr`

`echo -ne "\x04\x73\x01\x00" > /dev/ttyS1`

`reboot`

同理在dualos启动后可以通过以下命令切换至高电量模式并启动purelinux：

`stty -F /dev/ttyS1 -echo ispeed 9600 ospeed 9600 igncr`

`echo -ne "\x04\x73\x03\x00" > /dev/ttyS1`

`reboot`
