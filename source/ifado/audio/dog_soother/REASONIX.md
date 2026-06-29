# REASONIX — dog_soother (SigmaStar Star373 SOC)

蛋安抚器 SOC 固件，运行于 SigmaStar Star373。通过 UART 与 Telink B80 MCU（`ble_egg_anfu`）通信，完成音频采集、YAMNet 吠叫识别、安抚执行。

## Stack

- **SOC** SigmaStar Star373 (ARM)
- **Language** C + C++ (gnu++11, YAMNet wrapper)
- **Build** Makefile 体系 (`sample_code/Makefile` → `compile.mk` / `kmake.mk`)
- **AI inference** ncnn (`third_part/ncnn/build-star373/install`)
- **FFT** fftw3f (`third_part/fftw-3.3.10/build_star373`)
- **Audio** MI AI / MI AO / MI SYS (SigmaStar 多媒体中间件)
- **Model** YAMNet (`.param` + `.bin`，滑窗 0.96s，步进 0.96s)

## Layout

| Path | Contents |
|------|----------|
| `main.c` | 进程入口，调 `app_main_run()` |
| `app/app_main.c/h` | 模块初始化 + 主线程阻塞 |
| `uart/uart_proto.c/h` | 串口帧协议（`55 AA` + CRC16，115200 8N1） |
| `uart/uart_dispatch.c/h` | 按 cmdId 分发 REQ 到业务模块 |
| `include/uart_cmd.h` | 帧命令常量（与 B80 MCU `app_uart.h` 一致） |
| `include/app_config.h` | 所有可调参数（路径、阈值、增益、超时） |
| `include/app_types.h` | 工作状态枚举、安抚措施类型 |
| `bark/bark_detect.c/h` | YAMNet 滑窗推理线程 |
| `bark/bark_control.c/h` | 安抚状态机（监测→识别→执行→冷却） |
| `media/audio_ai.c/h` | MI AI 采集（16kHz mono） |
| `media/audio_ao.c/h` | MI AO 播放（16kHz WAV） |
| `media/audio.c/h` | 主人录音/播放/音量/10s 自动停录业务 |
| `media/audio_sys.c/h` | MI_SYS 初始化 |
| `media/ultrasonic.c/h` | 超声波发射（25kHz / 30kHz / 双频） |
| `store/comfort_store.c/h` | 运行时状态 + 安抚记录持久化 |
| `store/calm_strategy.c/h` | 安抚策略加载/保存 |
| `sys/system_time.c/h` | 时间同步 |
| `yamnet/yamnet_wrapper.cpp/h` | ncnn YAMNet 推理封装 |
| `legacy/test_yamnet.c` | 原算法独立测试（默认不参与链接） |
| `tools/uart_tty_demo.c` | PC 端 UART 调试工具示例（不参与链接） |

## UART 协议（SOC ↔ B80 MCU）

详见 `include/uart_cmd.h` + `uart/uart_proto.h`。

- **MCU→SOC REQ**：电源控制(`0x10`)、状态查询(`0x11`)、音量(`0x12`)、录音(`0x20-0x25`)、安抚策略(`0x30-0x33`)、时间(`0x32`)、日志(`0x40`)、记录查询(`0x41-0x42`)、恢复出厂(`0x50`)、BLE连接通知(`0x60`)
- **SOC→MCU EVT**：工作状态(`0x80`)、吠叫检测(`0x81`)、措施执行(`0x82`)、安抚结果(`0x83`)、录音完成(`0x84`)、日志数据(`0x85`)、错误(`0x86`)、心跳(`0x87`)

## 构建

```bash
cd <sdk>/verify/sample_code
make source/ifado/audio/dog_soother
# 产物: out/arm/app/dog_soother
```

清理：
```bash
make source/ifado/audio/dog_soother_app_clean
```

运行：
```bash
export DS_UART_DEVICE=/dev/ttyS2       # 或环境变量
# export DS_UART_DEBUG=1                 # 打印收发帧 hex
/customer/sample_code/bin/dog_soother &                         # 默认 /dev/ttyS1
/customer/sample_code/bin/dog_soother -D /dev/ttyS2
```

## 依赖

- `third_part/ncnn/build-star373/install` — ncnn 推理库
- `third_part/fftw-3.3.10/build_star373` — fftw3f
- `yamnet.param` + `yamnet.bin` — 模型文件（路径前缀见 `DS_YAMNET_MODEL_PREFIX`）

## 工作流程

1. 上电后 UART 初始化，发 `DS_EVT_WORK_STATE(0x80)` 通知 MCU
2. MCU 发 `DS_CMD_POWER_CTRL(0x10)=1` 开启识别
3. bark_detect 线程从 AI 采集环形缓冲区滑窗，YAMNet 推理输出 Top-K
4. bark_control 状态机：MONITORING → IDENTIFYING → ACTING（播放音乐/主人录音/超声波）→ RESTING
5. 各状态变更通过 UART EVT 通知 MCU，MCU 再通过 BLE 转发到手机 APP

## 注意事项

- **`app_config.h` 集中控制所有参数**（路径、阈值、增益、超时）— 修改参数先查这里
- **`DS_UART_DEBUG=1** 环境变量可打印完整 UART 帧 hex，联调利器
- **语音增益** `DS_AI_GAIN_DB` 默认 30dB（与 `test_yamnet -g 30` 对齐），过低会导致 YAMNet 全判 Silence
- **滑窗推理**：0.96s 窗长 × 实时读取（20ms 一块），队列满则阻塞
- **模块依赖链**：`main → app_main → uart_proto + uart_dispatch → bark_control → bark_detect + comfort_store + audio`
- **`legacy/` 和 `tools/`** 不参与默认链接，由 `dog_soother_post.mk` 单独构建
- **`.o` / `.d` 文件**是编译产物，勿手动编辑
