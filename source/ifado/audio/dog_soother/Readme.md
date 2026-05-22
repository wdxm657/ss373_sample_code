# dog_soother 使用说明

蛋安抚器 SOC 固件（模块化）。目录说明见 [MODULE_STRUCTURE.md](./MODULE_STRUCTURE.md)。

---

## 文件说明

| 路径                   | 说明                             |
| ---------------------- | -------------------------------- |
| `main.c` / `app/`      | 产品入口与主循环                 |
| `uart/`                | MCU 串口协议（`55 AA` + CRC16）  |
| `bark/`                | 吠叫识别与安抚状态机             |
| `media/`               | 录音/播放、超声波                |
| `store/`               | 状态与安抚记录                   |
| `sys/`                 | 时间同步                         |
| `yamnet/`              | YAMNet 推理库                    |
| `legacy/test_yamnet.c` | 原算法独立测试（默认不参与链接） |
| `dog_soother.mk`       | 模块编译配置                     |
| `dep.mk`               | 模块依赖                         |

---

## 依赖

- **ncnn**: `sample_code/third_part/ncnn/build-star373/install`
- **fftw**: `sample_code/third_part/fftw-3.3.10/build_star373`
- **模型**: `yamnet.param` + `yamnet.bin`（路径前缀不含后缀）

---

## 编译（sample_code Makefile）

在已配置好 SDK / `current.configs` 的环境下：

```bash
cd SourceCode/sdk/verify/sample_code
make source/ifado/audio/dog_soother
```

产物：`out/arm/app/dog_soother`

清理：

```bash
make source/ifado/audio/dog_soother_app_clean
```

---

## 运行（产品固件 dog_soother）

```bash
./dog_soother
# 或指定串口
./dog_soother -D /dev/ttyS1
export DS_UART_DEVICE=/dev/ttyS1
export DS_UART_DEBUG=1   # 打印收发帧 hex
```

阶段 1 联调：MCU 发 `SOC_STATUS_GET(0x11)` 应收到 9 字节 RSP；SOC 启动后会主动发 `WORK_STATE_EVT(0x80)`。

## 音频（MI AI / MI AO）

- **AI 采集**：`media/audio_ai.c`（对齐 `ai_demo` + `legacy/test_yamnet.c`，16kHz mono，默认增益 -10dB）
- **AO 播放**：`media/audio_ao.c`（对齐 `ao_demo`，16kHz，WAV 文件播放）
- **业务**：`media/audio.c` 主人录音写 `DS_OWNER_PCM_PATH`、播放、音量、10s 自动停录发 `0x84 EVT`
- **识别线程**：`bark/bark_detect.c` 从 detect 队列滑窗（0.96s / 0.48s hop），`yamnet_inference_topk` 后 **printf Top-K**（与 legacy/test_yamnet 一致）
- **主线程**：`app_main` 仅 `pause()` 等待退出，不再 `audio_tick` / `bark_control_tick`

板端日志示例：`yamnet ready`、`--- window #0 ...`、`[069] Dog` 等。

## 运行（legacy 算法测试 test_yamnet）

```bash
./test_yamnet -m <model_prefix> -I <wav|pcm|ain> [options]
```

| 参数              | 说明                                      |
| ----------------- | ----------------------------------------- |
| `-m, --model`     | 模型路径前缀（不含 `.param`/`.bin`）      |
| `-I, --input`     | 输入类型：`wav` / `pcm` / `ain`（麦克风） |
| `-i, --in-file`   | 输入文件（`wav`/`pcm` 必填）              |
| `-s, --seconds`   | `ain` 采集时长（秒），默认 10             |
| `-g, --gain`      | `ain` 增益（dB），默认 -10                |
| `-t, --threshold` | 猫狗阈值，默认 0.25                       |

**格式要求**（均为 16kHz、16-bit）：

- **wav**：标准 PCM WAV，单声道或立体声（自动混为单声道）
- **pcm**：裸 `s16le` 单声道，无文件头
- **ain**：板载 MI AI 麦克风（`dog_soother.mk` 默认 `-DENABLE_AI_INPUT`）

示例：

```bash
./test_yamnet -m ./models/yamnet -I wav -i dog_bark.wav
./test_yamnet -m ./models/yamnet -I pcm -i test.pcm
./test_yamnet -m ./models/yamnet -I ain -s 10 -g -10
```

对整段音频按 **0.96s 窗长、0.48s 步进**（50% 重叠）滑动识别，例如 `0.00–0.96s`、`0.48–1.44s`、`0.96–1.92s` …

- **线程 1**：按 **1x 实时** 流式读取（每 20ms 一块，对齐墙钟），凑满 0.96s 才入队；队列满则阻塞，不会一次塞满多个窗  
- **线程 2**：取窗推理，打印 Top-5 与 **RTF**（&lt;1 表示快于实时）

每窗输出示例：

```
--- window #0 [0.00 - 0.96] s | infer 85.32 ms | RTF 0.0889 ---
  1. [069] Dog  72.50%
  ...
```
