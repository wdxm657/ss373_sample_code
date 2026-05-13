# ai_demo使用说明

---

## 一、demo场景


使用麦克风录制10s时长的音频，并dump到指定路径。

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译；

    例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/iford/audio/ai_demo`命令进行编译;

3. 到`sample_code/out/arm/app/prog_audio_ai_demo `获取可执行文件;

---

## 三、运行环境说明

* 板端环境：

    以`SSC029A-S01A-S`为例，确认JP43、JP140、JP206、JP208引脚使用跳线帽短接，再将amic焊接在电阻R338和R343两端。

---

## 四、运行说明

- Usage:  `./prog_audio_ai_demo gain x`
  - x`: 音量，范围是[-63.5, 64]
  - input: Amic附近声源
  - output_file: `out/audio/ai_dump.wav`

---

## 五、运行结果说明

demo将会使用麦克风录制10s时长的音频（单声道、采样率为8KHz、PCM格式），并dump出来。

---