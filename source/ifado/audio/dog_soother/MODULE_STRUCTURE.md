# dog_soother 模块结构

蛋安抚器 SOC 固件，与 `vendor/ble_egg_anfu` MCU 通过 UART（`55 AA` + CRC16）通信。

## 目录

```
dog_soother/
├── main.c                 # 进程入口
├── app/                   # 应用编排、主循环
│   ├── app_main.c
│   └── app_main.h
├── include/               # 全局配置与协议常量
│   ├── app_config.h
│   ├── app_types.h
│   ├── uart_cmd.h
│   └── log.h
├── uart/                  # 串口帧与命令分发
│   ├── uart_proto.c/h
│   └── uart_dispatch.c/h
├── bark/                  # 吠叫识别 + 安抚状态机
│   ├── bark_detect.c/h
│   └── bark_control.c/h
├── media/                 # 音频录放、超声波
│   ├── audio.c/h
│   └── ultrasonic.c/h
├── store/                 # 运行时状态、安抚记录、恢复出厂
│   └── comfort_store.c/h
├── sys/                   # 系统时间与时区
│   └── system_time.c/h
├── yamnet/                # YAMNet 推理（ncnn）
├── legacy/                # 原 test_yamnet 算法样例（不参与默认链接）
├── tools/                 # 参考工具（不参与默认链接）
│   └── uart_tty_demo.c
├── dog_soother.mk
└── dep.mk
```

## 依赖关系

```
main → app_main
app_main → uart_proto, uart_dispatch, comfort_store, bark_control, audio, ultrasonic
uart_dispatch → 各业务模块（按 cmdId 转发）
bark_control → bark_detect, comfort_store, uart_proto
comfort_store → audio
```

## 构建

```bash
cd sdk/verify/sample_code
make source/ifado/audio/dog_soother
# 产物: out/arm/app/dog_soother
```

环境变量 `DS_UART_DEVICE` 可覆盖默认 `/dev/ttyS5`；也可 `-D /dev/ttySx` 传参（见 `app_main.c`）。

## 实现状态

| 模块 | 状态 |
|------|------|
| uart_proto / uart_dispatch | 已实现帧收发与命令分发骨架 |
| comfort_store / STATUS_GET | 已实现默认状态回包 |
| system_time | 已实现 TIME_SET 基础逻辑 |
| audio / ultrasonic / bark_detect | 占位，待接入 MI AI / AO / 超声驱动 |
| bark_control 自动安抚 | 占位，待阶段 4 填充 |

接口字段以 `vendor/ble_egg_anfu/docs/BLE_UART接口设计_V1.md` 为准。
