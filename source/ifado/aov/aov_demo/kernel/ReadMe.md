# Light Misc Control说明文档

---

## 1、简介

### 1.1 背景介绍

Light MIsc Control是应用于sigmastar平台的控制IR/LED补光灯以及IRcut等外设的驱动程序。提供了控制补光灯的常开、常闭、帧控的三种控制策略，同时也提供了控制IRcut和获取光敏传感器的接口供用户层使用。

### 1.2. 工作原理

帧控模式是在aov场景下的补光灯控制策略，aov场景由于低功耗需求所以希望对灯光的控制尽量准确，最大程度减少灯光开启时间，但又要兼顾sensor曝光时长，所以要做到在sensor曝光之前灯光就已经能够开启并收敛稳定，在sensor曝光完成时灯光会自动关闭。因此本模块采用了kernel pm的resume机制与注册vif framedone中断来完成上述两个动作。

### 1.3. 业务框图

```
resume_noirq
-------------------->    HrTimer/WorkQueue xms delay
                             :
                      || open Ir/LED light
Sensor exposure ----> ||
                      ||
frame start  -------> ||
       :              ||
      32ms@30         ||
       :              ||
frame done callback   ||
--------------------> || close Ir/LED light
```

由于resume_noirq与Sensor exposure之间不存在同步关系，所以这两者之间的时间间隔目前不是绝对恒定，且要保证open light到Sensor exposure之间补光灯能够完成收敛稳定。所以建议将HrTimer的delay时间设置小一些。如果light的收敛时间超过15ms(resume_noirq与Sensor exposure之间时间间隔经验值)，那么此方案将不能保证sensor的曝光效果，因此可能出现前几行过暗的问题。

### 1.4. DTS配置

如需更改lightsensor配置，可参考：

```
light_misc_control {
    compatible = "sstar,light_misc_control";
    lightsensor-i2c = <0>;
    ir-pad = <PAD_PM_IRIN>;
    ircut-pad = <PAD_SAR_GPIO0 PAD_SAR_GPIO0>;
    status = "ok";
};
```

### 1.5. 驱动配置

```
参数说明：

tig_mode：收敛时间
0：100ms(default)
1：400ms

ir_cut_select:ir cut控制逻辑
0：01控制正转ir cut(default)
1：10控制正转ir cut
```

## 2. 代码结构

```
source/$(CHIP)/aov/aov_demo/kernel/
    st_light_misc_control_driver   driver初始化以及接口注册

    light_misc_control_main        业务逻辑实现

    light_misc_control_hw          硬件资源抽象以及控制

    light_misc_control_i2c         i2c驱动

internal/aov/
    light_misc_control_datatype.h  kernel/user mode共用数据结构

    light_misc_control_api.h       user mode api
```

## 3. 结构体说明

### 3.1. SSTAR_Switch_State_e

此枚举类是用来描述开关状态的。

```
typedef enum {
    E_SWITCH_STATE_OFF,     // 表示当前是关闭状态
    E_SWITCH_STATE_ON,      // 表示当前为开启状态
    E_SWITCH_STATE_KEEP,    // 当前是保持状态，用于控制IRcut保持开关状态
    E_SWITCH_STATE_ERROR,   // ERROR
} SSTAR_Switch_State_e;
```

### 3.2. SSTAR_Light_Type_e

此枚举类是用来描述支持灯的种类。目前支持IR灯与LED灯。

```
typedef enum {
    E_LIGHT_TYPE_IR,
    E_LIGHT_TYPE_LED,
    E_LIGHT_TYPE_MAX,
} SSTAR_Light_Type_e;
```

### 3.3. SSTAR_Control_Type_e

此枚举类是用来描述灯光控制方式的。

```
typedef enum {
    E_CONTROL_TYPE_LONG_TERM_OFF,    // 常关
    E_CONTROL_TYPE_LONG_TERM_ON,     // 常开
    E_CONTROL_TYPE_MULTI_FRAME,      // 帧控
    E_CONTROL_TYPE_MAX,
} SSTAR_Control_Type_e;
```

### 3.4. SSTAR_Light_Ctl_Attr_t

此结构体是用来设置灯光控制类型、使用灯光的种类、开启强度、以及延迟开启时间(仅帧控模式下生效)。

```
typedef struct SSTAR_Light_Ctl_Attr_s
{
    SSTAR_Control_Type_e controlType;        // 控制类型
    SSTAR_Light_Type_e   lightType;          // 灯光种类
    int                  delayOpenTimeMs;    // resume_irq时延迟开启灯光时间
    unsigned int         lightIntensity;     // 灯光强度
} SSTAR_Light_Ctl_Attr_t;
```

### 3.5. SSTAR_Light_Misc_Callback_Param_t

此结构体为设置vif framedone callback使用。

```
typedef struct SSTAR_Light_Misc_Callback_Param_s
{
    int           vifDevId;      // 注册对应device的callback
    unsigned long fun_ptr_addr;  // callback fun的函数指针
} SSTAR_Light_Misc_Callback_Param_t;
```

## 4. api介绍

### 4.1. Dev_Light_Misc_Device_GetFd

获取/dev/light_misc的fd。如果对应设备节点不存在请检查ko是否有正确insmod。

```
int Dev_Light_Misc_Device_GetFd(void);
```

### 4.2. Dev_Light_Misc_Device_CloseFd

关闭对应设备节点的fd。请于4.1成对使用。 如遇返回错误请检查dev_fd值是否正确。

```
int Dev_Light_Misc_Device_CloseFd(int dev_fd);
```

### 4.3. Dev_Light_Misc_Device_Init

初始化设备资源。vifDevId为对应设备设置framedone callback。此api目前只支持设置单个vif device默认为vif0。

Vif framedone callback，这个回调函数是VIF一帧处理完成的中断回调函数，在中断里面不允许实现耗时或等待的动作，例如操作I2C等行为。

```
int Dev_Light_Misc_Device_Init(int dev_fd, int vifDevId);
```

### 4.4. Dev_Light_Misc_Device_DeInit

释放设备资源。

```
int Dev_Light_Misc_Device_DeInit(int dev_fd, int vifDevId);
```

### 4.5. Dev_Light_Misc_Device_Set_Attr

设置灯光。

```
int Dev_Light_Misc_Device_Set_Attr(int dev_fd, SSTAR_Light_Ctl_Attr_t * pstAttr);
```

### 4.6. Dev_Light_Misc_Device_Get_Attr

获取灯光设置参数。

```
int Dev_Light_Misc_Device_Get_Attr(int dev_fd, SSTAR_Light_Ctl_Attr_t* pstAttr);
```

### 4.7. Dev_Light_Misc_Device_Get_LightSensor_Value

获取光线传感器数值。

```
int Dev_Light_Misc_Device_Get_LightSensor_Value(int dev_fd);
```

### 4.8. Dev_Light_Misc_Device_Set_Ircut

设置IR CUT状态。

- 不能一直设为E_SWITCH_STATE_OFF/E_SWITCH_STATE_ON的状态，否则会漏电且烧坏电机，最终需要保持E_SWITCH_STATE_KEEP状态。
- E_SWITCH_STATE_OFF/E_SWITCH_STATE_ON切换到E_SWITCH_STATE_KEEP需要等待电机通电时间，具体等待的时间值根据datasheet而定。

```
int Dev_Light_Misc_Device_Set_Ircut(int dev_fd, SSTAR_Switch_State_e eOpenState);
```

### 4.9. Dev_Light_Misc_Device_Get_TIGMode

获取TIG mode值。

- TIG（Time of Integration）是light sensor的一个参数，指在测量光照强度时所采用的积分时间。有关TIG mode的详细说明，请参照light sensor的datasheet。

```
int Dev_Light_Misc_Device_Get_TIGMode(int dev_fd);
```

## 5. debug

### 5.1. echo init > /dev/light_misc

如果没有用户层调用Dev_Light_Misc_Device_Init且需要debug时则需要echo init指令给device。此种情形下帧控功能由于接收不到framedone所以无法自动及时关闭灯光，仅能在powerDown时掉电关闭。

### 5.2. cat /dev/light_misc

```
/ # cat /dev/light_misc
controlType:2 lightType:0 delayOpenTimeMs:20 lightIntensity:0
eIRcutState:0 eIRState:0 eLEDState:0 reciveVifEventCnt:0 doneFlagCnt:0 openLightCnt:0
```

第一行为当前生效的SSTAR_Light_Ctl_Attr_t灯光控制参数。

第二行表示当前工作状态。

1. eIRcutState表示IRcut开启状态。

2. eIRState表示红外灯开启状态。

3. eLEDState表示LED灯开启状态。

4. reciveVifEventCnt表示从vif framdone call回来的次数。

5. doneFlagCnt表示帧控模式下vif framdone call回来并将doneFlag置起来的次数。doneFlag置起表示我们可以进行关灯操作了。可理解为关灯次数。

6. openLightCnt 表示在帧控模式下自动开启灯光的次数。

### 5.3. echo getlux > /dev/light_misc

获取光线传感器值。结果在kmsg中显示。

```
/ # echo getlux > /dev/light_misc
<6>echo command:getlux
/ # <6>lux:171
```

### 5.4. echo setircut on/off > /dev/light_misc

设置IRcut开关状态。

```
echo setircut on > /dev/light_misc
```

### 5.5. echo setlight ir/led on/off > /dev/light_misc

控制IR/LED灯的常开/常闭的状态。

```
echo setlight ir on > /dev/light_misc
```

### 5.6. echo setlight ir/led multi timeMs > /dev/light_misc

控制IR/LED灯的帧控模式以及延时开灯时间。

```
echo setlight ir multi 0 > /dev/light_misc
```

### 5.7. echo debuglog on/off > /dev/light_misc

开启/关闭 帧控模式下开关灯的debug log。由于帧控模式下开关灯均在中断环境运行，所以默认关闭避免debug log在中断中耗时。

```
echo debuglog on > /dev/light_misc
```

## 6. Makefile config

### 6.1. LIGHT_USE_HRTIMER

是否采用hrTimer作为定时触发源。如果否，则采用工作队列的方式延时运行开灯操作。

### 6.2. LIGHT_SUPPORT_MULTI_VIFDEVICE

是否支持多个对vif device的控灯操作。

目前方案仅支持在resume_noirq时统一开灯，以及由于公版只使用了一根gpio故仅支持收到所有vifDevice完成状态后统一关灯。如需单独控制关灯则在收到完成状态后对相应vif device的gpio进行操作即可。

### 6.3. LIGHT_USE_PM_NOTIFY

是否使用pm_notifier通知器获取pm状态。默认否。

### 6.4. LIGHT_SUPPORT_LIGHTSENSOR_POWEROFF

是否支持str时对light sensor进行动态上下电。开启后将节省light sensor在suspend到resume前的功耗。 但是将带来重新上电后会耗费tig mode相对应的收敛时间。请根据需求选择是否支持light sensor断电。
