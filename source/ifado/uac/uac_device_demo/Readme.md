# uac_demo使用说明

---

## 一、demo场景

* 场景1：uac做audio in设备

  ```
  audio_in->uac->host

  参数说明：audio in的参数包含samplerate，channelmode
  ```

* 场景2：uac做audio out设备

  ```
  host->uac->audio_out

  参数说明： audio out的参数包含samplerate，channelmode
  ```

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

    例如`SSC032A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `usbcam_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make usbcam_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/uac/uac_device_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_uac_uac_device_demo`获取可执行文件;

4. 获取ko文件，kernel下menuconfig配置推荐为下：

   ```
    音频架构的支持：
    Device Drivers --->
        <M> Sound card support--->
           Advance Linux Sound Architecture--->
               [*] PCM timer interface
    生成驱动：soundcore.ko snd.ko snd-timer.ko snd-pcm.ko

    Gadget Uac的支持：
    Device Drivers --->
        USB support --->
            USB Gadget Support --->
                USB Gadget Drivers --->
                    [*] Include configuration with UAC (Audio)
    生成驱动：u_audio.ko usb_f_uac1.ko

    将生成以下ko文件：
    1. soundcore.ko
    2. snd.ko
    3. snd-timer.ko
    4. snd-pcm.ko
    5. u_audio.ko
    6. usb_f_uac1.ko (依赖于libcomposite.ko)

   ```
---

## 三、运行环境说明

> 该demo仅适合在pure linux上执行

* 板端环境：

    在`SSC032A-S01A-S`板端sensor pad0(CON3插口)位置接上mipi sensor，对应跑的是4lane或2lane，可以是imx415，imx307等;
    在`JP84`接上mic，在`JP141`接上扬声器

* dts配置：

    mipi snr0 4lane EVB上使用，默认dts已配好，无需修改

  > 例如上述编译环境说明的`SSC032A-S01A-S`型号板子，直接编译使用即可

* sensor 驱动配置：

    无需配置

* UAC驱动配置：

    按顺序insmod uac驱动所需要的驱动

    注意：insmod之前需要先将/customer/demo.sh文件中关于USB的驱动先注释掉
    usb2.0

  ```
  insmod soundcore.ko
  insmod snd.ko
  insmod snd-timer.ko
  insmod snd-pcm.ko
  insmod u_audio.ko
  insmod usb_f_uac1.ko

  ```

* uvc-configfs脚本说明：

    脚本路径：`sample_code/source/ifado/uvc/uvc_device_demo/resource/gadget-uvc-configfs.sh`
    脚本作用：通过ConfigFs的方式可以快速配置uac设备
    ```
    ./gadget-uvc-configfs.sh -d1 #打开一个uac的设备
    ```


---

## 四、运行说明

将可执行文件prog_uac_uac_device_demo和sample_code/source/ifado/uvc/resource/uvc_device_demo/目录下的gadget-uvc-configfs.sh脚本的放到板子上，修改权限777

1. 将usb接线到PC端（usb2.0板端丝印为：P0_USB2.0,usb3.0板端丝印为：USB3.0+P1）
2. 运行配置脚本文件
   - 创建UAC设备`./gadget-uvc-configfs.sh -d1`

注意：

清除当前UAC配置为`./gadget-uvc-configfs.sh -z`；

3. 运行demo
   - 设置UAC为audio in的设备`./prog_uac_uac_device_demo -d1`
   - 设置UAC为audio out的设备`./prog_uac_uac_device_demo -D1`
   '1' 这里表示audio interface的枚举

---

## 五、运行结果说明

* audio_in 效果查看
1. 运行demo后查看PC端的，设置->声音->选择输入的设备->Capture Input terminal (AC interface)
2. 使用potplayer 软件，打开->设备设置->摄像头->音频录制设备
3. 设备：选择Capture Input terminal(AC interface)
4. 点击 ：打开设备，即可看到对应码流分辨率的画面

* audio_out 效果查看
1. 运行demo后查看PC端的，设置->声音->选择输出的设备->扬声器（AC interface）
2. 在右小角声音选择扬声器（AC interface）
3. 在PC端播放任意一个声音文件

* 退出命令

    输入`CTRL-C`即可退出demo

---

## 六、参数修改说明

* uvc-configfs脚本参数说明

  ```
    playback数据流向: device->host
    capture数据流向: host->device
    playback or capture是相对alsa节点来说的，如果我们的设备从mic采集声音数据后送到alsa去播放，那就是playback，如果是通过Alsa取数据送到喇叭播放，那就是capture。

    1. uac_function_enable
    0： 关闭uac功能， 1： 开启喇叭功能， 2：开启麦克风功能， 3：开启喇叭与麦克风

    2. p_srate/c_srate
    p_srate：麦克风设备的采样率
    c_srate: 喇叭设备的采样率

    3. p_chmask/c_chmask
    p_chmask：麦克风设备的通道数
    c_chmask：喇叭设备的通道数

    4. p_mpsize/c_mpsize
    p_mpsize：麦克风设备接口的endpoint maxpacket
    c_mpsize：喇叭设备接口的endpoint maxpacket
  ```
