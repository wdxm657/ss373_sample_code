# audio alg使用说明

---

## 一、demo场景


此路径下的demo均为audio alg算法

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译；

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/audio/alg/*`命令进行全部的`audio alg demo`编译，或者`make source/ifado/audio/alg/xxx`指定算法demo;

3. 获取可执行文件：`sample_code/out/arm/app/prog_audio_alg_* `;

---

## 三、运行环境说明

* 无特殊环境要求

---

## 四、运行说明

1. **aec_demo**

   - overview: 回声消除（Acoustic Echo Cancellation，简称AEC），是一种用于抑制远程回声的功能。详细描述可参考aec API文档。

   - Usage:  `./prog_audio_alg_aec_demo`

   - input/output:

     - `input_file1 = resource/input/audio/aec_farend.wav`
       - 设备扬声器的信号源，远程传来的信号

     - `input_file2 = resource/input/audio/aec_nearend.wav`
       - 设备麦克风录到的信号，此信号可能包含声学回声(Acoustic echo)和近端语者(Near-end talker)的信号
     - `output_file = out/audio/aec_processed.wav`
       - 经过回声消除后的音频



2. **apc_demo**
   - overview: APC（Audio Process Chain）音频处理链路，是一个包含降噪、均衡器和自动增益控制的算法组合。APC的主要目的是提高音频质量。详细描述可参考apc API文档。
   - Usage:  `./prog_audio_alg_apc_demo`
   - input/output:
     - `input_file = resource/input/audio/apc_original.wav`
       - 噪音大音质低的音频文件
     - `output_file = out/audio/apc_processed.wav`
       - 经过降噪、自动增益等算法优化过的音频文件



3. **kws_demo**
   - overview: 语音唤醒（Keywords Spotting, KWS）是检测语音流中是否有指定唤醒词的算法。详细描述可参考kws API文档。
   - Usage:  `./prog_audio_alg_kws_demo`
   - input/output:
     - `input_resource: resource/input/audio/kws.graph kws.dict kws_keyword.wav`
       - 编码文件、语音词典和带有"你好问问"的音频文件
     - `input_ipu_model: resource/input/audio/kws_c32m.img`
       - 语音识别模型，需要从`alkaid/project/board/ifado/dla_file/ipu_net/kws`中拷贝至`resource/input/audio/`
     - output: 默认会将key word（你好问问）打印出来
       - 唤醒词支持定制，有需要请联系FAE



4. **mix_demo**
   - overview: 混音(MIX)是将两路音频信号数据混成一路音频信号的算法。详细描述可参考mix API文档。

   - Usage:  `./prog_audio_alg_mix_demo`

   - input/output:

     - `input_file1 = resource/input/audio/mix_signal1.wav`
       - 一路音频信号1

     - `input_file2 = resource/input/audio/mix_signal2.wav`
       - 一路音频信号2
     - `output_file = out/audio/mix_processed.wav`
       - 将两路音频信号混合成一路的音频信号



5. **sed_demo**

   - overview: 声音事件检测（Sound Event Detection, SED）是检测是否有对应声音事件的算法，目前支持检测小孩子哭声。详细描述可参考sed API文档。

   - Usage:  `./prog_audio_alg_sed_demo`

   - input/output:

     - `input_model = resource/input/audio/sed_tcbs.img`
       - ipu 离线网络模型，需要手动从`alkaid/project/board/ifado/dla_file/ipu_net/sed_tcbs.img`拷贝到`resource/input/audio/sed_tcbs.img`

     - `input_audio = resource/input/audio/sed_babycry.wav`
       - 带有小孩哭声的音频文件

     - `output_file = out/audio/sed_processed.txt`
       - 会将检测到小孩哭声的时间点打印出来



6. **src_demo**

   - overview: SRC（Sample Rate Conversion）重采样，用于对音频流做采样频率转换，以获取不同采样频率的音频流。详细描述可参考src API文档。

   - Usage:  `./prog_audio_alg_src_demo`

   - input/output:

     - `input_file = resource/input/audio/src_original.wav`
       - 采样率为48KHz的音频

     - `output_file = out/audio/src_processed.wav `
       - 经过重采样的采样率为8KHz的音频



7. **vad_demo**
   - overview: 语音活动检测算法（Voice Activity Detection, VAD）算法是对通过对输入语音进行处理，检测当前输入是否有语音活动的功能。详细描述可参考vad API文档。
   - Usage:  `./prog_audio_alg_vad_demo`
   - input:
     - `input_file = resource/input/audio/vad_original.wav`
       - 一段朗读数字的音频
   - return:
     - 0：成功
     - 非0：失败



8. **2mic_bf_demo**

    - overview: BF（beamforming）波束形成或空间滤波是一种用于传感器阵列的定向信号处理技术传输或接收。通过这样一种方式组合麦克风阵列中的组件来实现特定角度的信号加强，而其他的则会衰减。波束形成可用于发射端和接收端，以实现空间选择性。此demo为2Mic版本的BF，详细描述可参考bf API文档。

    - Usage:  `./prog_audio_alg_2mic_bf_demo`

    - input/output:

      - `input_file = resource/input/audio/2mic_bf_original.wav`
        - 采样率为16KHz的两颗麦克风阵列

      - `output_file = out/audio/2mic_bf_processed.pcm`
        - `采样率为16KHz的波束形成后的单声道声`



9. **2mic_ssl_demo**

    - overview: SSL（Sound Source Localization）声源定位，用于定位声音来源的方向。此demo为2Mic版本的SSL，详细描述可参考ssl API文档。
    - Usage:  `./prog_audio_alg_2mic_ssl_demo`
    - input/output:
      - `input_file = resource/input/audio/2mic_ssl_original.wav`
        - 采样率为16KHz的两颗麦克风阵列
      - `output_file = out/audio/2mic_ssl_processed.txt`
        - 描述声音来源方向的文本



10. **4mic_bf_demo**
    - overview: BF（beamforming）波束形成或空间滤波是一种用于传感器阵列的定向信号处理技术传输或接收。通过这样一种方式组合麦克风阵列中的组件来实现特定角度的信号加强，而其他的则会衰减。波束形成可用于发射端和接收端，以实现空间选择性。此demo为4Mic版本的BF，详细描述可参考bf API文档。
    - Usage: `./prog_audio_alg_4mic_bf_demo`
    - input/output:
      - `input_file = resource/input/audio/4mic_bf_Chn-01.wav 4mic_bf_Chn-02.wav 4mic_bf_Chn-03.wav 4mic_bf_Chn-04.wav`
        - 默认输入是采样率为16KHz的四颗麦克风阵列
      - `output_file = out/audio/4mic_bf_processed.pcm`
        - 采样率为16KHz的波束形成后的单声道声



11. **4mic_ssl_demo**
    - overview: SSL（Sound Source Localization）声源定位，用于定位声音来源的方向。此demo为4Mic版本的SSL，详细描述可参考ssl API文档。
    - Usage:  `./prog_audio_alg_4mic_ssl_demo`
    - input/output:
      - `input_file = resource/input/audio/4mic_ssl_Chn-01.wav 4mic_ssl_Chn-02.wav 4mic_ssl_Chn-03.wav 4mic_ssl_Chn-04.wav`
        - 默认输入是采样率为16KHz的四颗麦克风阵列
      - `output_file = out/audio/4mic_ssl_processed.txt`
        - 描述声音来源方向的文本




12. **g711_codec_demo**

    - overview: 开源音频编码算法。提供16-bit线性PCM、8-bit A-law/U-law之间的转换。详细可参考github：https://github.com/EasyDarwin/EasyAACEncoder

    - Usage:  `./prog_audio_alg_g711_codec_demo`

    - input/output:

      - `input_file = resource/input/audio/g711_8K_alaw_8bit_MONO_30s.pcm`
        - alaw音频文件
      - `output_file = out/audio/g711_8K_ulaw_8bit_MONO_30s.pcm`
        - ulaw音频文件

      - `operation = con_alaw2ulaw`
        - 编码转换：alaw to ulaw

    - Other operation:`./prog_audio_alg_g711_codec_demo [Enter files and paths] [Output files and paths] [operation]`

      - operation:

        ```
        1.en_linear2alaw
        2.en_linear2ulaw
        3.de_alaw2linear
        4.de_ulaw2linear
        5.con_alaw2ulaw
        6.con_ulaw2alaw
        ```

13. **g726_codec_demo**

    - overview: 开源音频编码算法。可将PCM信号转换为40kbps、32kbps、24kbps、16kbps的ADPCM信号。可参考github：https://github.com/EasyDarwin/EasyAACEncoder

    - Usage:  `./prog_audio_alg_g726_codec_demo`

    - input/output:

      - `input_file = resource/input/audio/g726_8K_16bit_MONO_30s.pcm`
      - 128Kbps音频文件
      - `output_file = out/audio/g726_8K_4bit_MONO_30s.pcm`
        - 32Kbps音频文件

      - `operation = encode_to32k`
        - 编码转换：128Kbps to 32Kbps

    - Other operation:`./prog_audio_alg_g726_codec_demo [Enter files and paths] [Output files and paths] [operation]`

      - operation:

        ```
        1.encode_to16k
        2.encode_to24k
        3.encode_to32k
        4.encode_to40k
        5.decode
        ```



---
