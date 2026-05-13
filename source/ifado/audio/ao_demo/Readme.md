# ao_demo使用说明

---

## 一、demo场景


使用扬声器播放指定路径的音频文件（格式为：单声道、采样率为8KHz、PCM格式）。

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译；

    例如`SSC029A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_iford.spinand.glibc-11.1.0-squashfs.ssc029a.512.bga12_ddr4_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/iford/audio/ao_demo`命令进行编译;

3. 到`sample_code/out/arm/app/prog_audio_ao_demo `获取可执行文件;

---

## 三、运行环境说明

* 板端环境：

    以`SSC029A-S01A-S`为例，确认JP88引脚 PIN1和PIN2使用跳线帽短接，JP89、JP207引脚使用跳线帽短接，再Speaker连接到JP82引脚。

---

## 四、运行说明

- Usage:  `./prog_audio_ao_demo gain x`
  - x: 音量，范围是[-63.5, 64]
  - input_file: `resource/input/ao_8K_16bit_MONO_30s.wav`
  - output: Speaker播放

## 五、运行结果说明

demo将会使用扬声器播放音频文件。

---