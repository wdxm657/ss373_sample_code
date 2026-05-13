# usb_gadget_preload demo使用说明
支持在dualos系统中运行

---

## 一、demo场景

* 场景功能:uvc出流

  ```
  vif->isp->scl->venc->uvc_video
             |->uvc_video

  参数说明： vif接sensor pad0，会通过scl或venc输出视频流。
  其中可以支持的格式有：YUYV, NV12, H264, H265, Mjpeg

  demo会根据播放器的选择来初始化scl模块的output port，以及venc模块的Dev和Channel。
  ```

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

    例如`SSC032B-S01A-S`型号板子，使用nand，qfn88的配置,使用以下defconfig,其他板子型号详细参考用户手册

    `usbcam-rtos_ifado.spinand.uclibc-9.1.0-ramdisk.ssc032b.64.qfn88_demo_defconfig` 或

    在project目录执行以下命令进行编译;

    `make usbcam-rtos_ifado.spinand.uclibc-9.1.0-ramdisk.ssc032b.64.qfn88_demo_defconfig`
    `make clean && make image -j8`

2. 在project编译rtos阶段，会将该路径下的demo编译为rtos的lib，在rtos preload阶段将会运行该demo;

---

## 三、运行环境说明

* 板端环境：

    在`SSC032B-S01A-S`板端sensor pad0(CON3插口)位置接上mipi sensor，对应跑的是4lane或2lane，可以是imx415，imx307等;

* dts配置：

    mipi snr0 4lane EVB上使用，默认dts已配好，无需修改

    例如上述编译环境说明的`SSC032B-S01A-S`型号板子，直接编译使用即可

* app编译启动配置(如使用上述defconfig，无需重新配置，直接编译即可)：

    通过project menuconfig enable usb_gadget_preload application；

    选择 Rtos -> Rtos Application Options -> Support pipeline demo applications -> [*] Support usb_gadget_preload application

```
CONFIG_CONFIG_APPLICATION_USB_GADGET_PRELOAD=y
```

* earlyinit setting json 配置sensor设定和启动的preload app ：

```
CONFIG_CONFIG_EARLYINIT_SETTING_JSON="single_sensor_realtime_usb_gadget_preload.json"
```

---

## 四、运行说明

* 运行方法
usb_gadget_preload demo在开机启动时会自动运行，参数设定在/misc/PreloadSetting.txt文件。

* preview效果查看
1. 运行demo后查看PC端的，设备管理器->照相机下是否有对应的UVC设备如：Webcam;
2. 使用potplayer 软件，打开->设备设置->摄像头；
3. 设备：选择Webcam；格式：选择场景中支持的码流格式和的分辨率；输出：选择捕获
4. 点击 ：打开设备，即可看到对应码流分辨率的画面

```
成功获取Mjpeg，1920x1080@30fps码流log（log在rtos端查看）：
Webcam-UVC: start preview 0, info(format:MJPG, Width:1920, High:1080)
```

* 注意事项
1. 如果potplayer 没有检测到video设备，请先确定设备管理器端是否识别，其次可以点击‘检索更新’刷新；

---
