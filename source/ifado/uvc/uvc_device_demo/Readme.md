# uvc_demo使用说明
仅支持在pure linux系统中运行

---

## 一、demo场景

* 场景1：单路uvc出流

  ```
  vif->isp->scl->venc->uvc_video0

  参数说明： vif接sensor pad0，会通过/dev/video0节点输出视频流。
  其中可以支持的格式以及分辨率有：H264, H265 , Mjpeg
  支持的分辨率有：
              320x240@30fps
              640x480@30fps
              1280x720@30fps
              1920x1080@30fps
              2560x1440@30fps
  demo会根据播放器的选择的自动选择venc模块的Dev和Channel。

  绑定模式：vif->isp->scl->venc 均为framemode
  ```

* 场景2：双路uvc出流

  ```
  vif->isp->scl->venc->uvc_video0
                   |->uvc_video1

  参数说明： vif接sensor pad0，会通过/dev/video0和/dev/video1节点输出视频流。
  其中可以支持的格式以及分辨率有：H264, H265 ，Mjpeg
  支持的分辨率有：
              320x240@30fps
              640x480@30fps
              1280x720@30fps
              1920x1080@30fps
              2560x1440@30fps
  demo会根据播放器的选择的自动选择venc模块的Dev和Channel。

  绑定模式：vif->isp->scl->venc 均为framemode
  ```

* 场景3：三路uvc出流

  ```
  vif->isp->scl->venc->uvc_video0
                   |->uvc_video1
                   |->uvc_video2

  参数说明： vif接sensor pad0，会通过/dev/video0，/dev/video1以及/dev/video2节点输出视频流。
  其中可以支持的格式以及分辨率有：H264, H265 ，Mjpeg
  支持的分辨率有：
              320x240@30fps
              640x480@30fps
              1280x720@30fps
              1920x1080@30fps
              2560x1440@30fps
  demo会根据播放器的选择的自动选择venc模块的Dev和Channel。

  绑定模式：vif->isp->scl->venc 均为framemode
  ```

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

    例如`SSC032A-S01A-S`型号板子，使用nand，ddr4的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `usbcam_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig` 或

    在project目录执行以下命令进行编译;

    `make usbcam_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/uvc/uvc_device_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_uvc_uvc_device_demo`获取可执行文件;

4. 获取ko文件，kernel下menuconfig配置推荐为下：
    usb2.0配置

   ```
   Device Drivers --->
       <M> Multimedia support --->
           Media device types --->
               [*] Cameras and video grabbers
           Media core support --->
               <M> Video4linux core
               [*] Media Controller API
       [*] USB support --->
           <M> USB Gadget Support --->
               USB Peripheral Controller --->
                   <M> Sstar USB 2.0 Dvice Controller

   Device Drivers
       [*]USB support
           <M> Support for Host-side USB
           <M> USB Gadget Support  --->
           <空> USB Gadget functions configurable through configfs
           USB Gadget precomposed configurations ->
               <M> USB Sigmastar Gadget
                   [*] Include configuration with UVC (video)
    将生成以下ko文件：
    usb-common.ko
    udc-core.ko mc.ko
    videodev.ko
    videobuf2-common.ko
    videobuf2-v4l2.ko
    videobuf2-memops.ko v
    ideobuf2-vmalloc.ko
    videobuf2-dma-sg.ko
    udc-msb250x.ko
    libcomposite.ko
    usb_f_uvc.ko
    g_sstar_gadget.ko
   ```

---

## 三、运行环境说明

* 板端环境：

    在`SSC032A-S01A-S`板端sensor pad0(CON3插口)位置接上mipi sensor，对应跑的是4lane或2lane，可以是imx415，imx307等;

* dts配置：

    mipi snr0 4lane EVB上使用，默认dts已配好，无需修改

  > 例如上述编译环境说明的`SSC032A-S01A-S`型号板子，直接编译使用即可

* sensor 驱动配置：

    /customer/demo.sh路径修改insmod对应的sensor ko，例如imx415跑sensor时，可配置为：

  ```
  insmod /config/modules/5.10/imx415_MIPI.ko chmap=1 lane_num=4
  ```

    /customer/demo.sh路径修改insmod对应的sensor ko，例如imx307跑sensor时，可配置为：

  ```
  insmod /config/modules/5.10/imx307_MIPI.ko chmap=1 lane_num=2 hdr_lane_num=2 mclk=36M
  ```

* UVC驱动配置：

    按顺序insmod uvc驱动所需要的驱动

    注意：insmod之前需要先将/customer/demo.sh文件中关于USB的驱动先注释掉
    usb2.0

  ```
  insmod usb-common.ko
  insmod usbcore.ko
  insmod udc-core.ko
  insmod mc.ko
  insmod videodev.ko
  insmod videobuf2-common.ko
  insmod videobuf2-v4l2.ko
  insmod videobuf2-memops.ko
  insmod videobuf2-vmalloc.ko
  insmod videobuf2-dma-sg.ko
  insmod udc-msb250x.ko
  insmod libcomposite.ko
  insmod usb_f_uvc.ko
  ```

* uvc-configfs脚本说明：

    脚本路径：`sample_code/source/ifado/uvc/uvc_device_demo/resource/gadget-uvc-configfs.sh`
    脚本作用：通过ConfigFs的方式可以快速配置uvc设备

* 输入文件：

    场景可加载指定的iq bin文件，如不指定默认加载/config/iqfile/`sensorname`_api.bin文件；
    部分sensor的iq bin文件可在`/project/board/ifado/iqfile`路径下获取。

---

## 四、运行说明

将可执行文件prog_uvc_uvc_device_demo和sample_code/source/ifado/uvc/resource/uvc_device_demo/目录下的gadget-uvc-configfs.sh脚本的放到板子上(如果默认编译为USBCAM，则demo和脚本会默认打包到开发板/customer/sample_code/bin/目录下)，修改权限777

1. 将usb接线到PC端（usb2.0板端丝印为：CON7 USB,去除JP64的供电跳帽）
2. 运行配置脚本文件
   - 创建一路UVC设备`./gadget-uvc-configfs.sh -a1`
   - 创建两路UVC设备`./gadget-uvc-configfs.sh -a2`
   - 创建三路UVC设备`./gadget-uvc-configfs.sh -a3`

其他参数说明：

> 按`./gadget-uvc-configfs.sh -a2 -n usb1,usb2`运行，可在配置两路UVC的基础上选择两路UVC设备的名字。
> 按`./gadget-uvc-configfs.sh -a2 -m 0`运行，可在配置两路UVC的基础上选择和UVC控制端的传输方式，0：USB_ISOC_MODE，1：USB_BULK_MODE。

注意：

当前的配置方式是脚本配置，请修改板端的/customer/demo.sh文件，注释掉g_sstar_gadget模块的insmod

配置成功会在/dev/路径下生产`/dev/video0 or /dev/video1 or /dev/video2`；

清除当前UVC配置为`./gadget-uvc-configfs.sh -z`；

如果prog_uvc_uvc_device_demo选择了全分辨率的编译选项，请把脚本中对于相关分辨率的使能置起，例：UVC_STREAMING_FORMAT_YUV_ENABLE=1，UVC_STREAMING_FORMAT_NV12_ENABLE=1

如果demo把数据的处理方式变为：UVC_MEMORY_USERPTR，那么需要在加载videobuf2-common驱动的时候加上额外参数，例：insmod /config/modules/5.10/videobuf2-common.ko use_mi_buf=1

3. 运行demo
   - 单路UVC出流`./prog_uvc_uvc_device_demo 0`
   - 双路UVC出流`./prog_uvc_uvc_device_demo 1`
   - 三路UVC出流`./prog_uvc_uvc_device_demo 2`

其他参数说明：

> 按`./prog_uvc_uvc_device_demo 1 iqbin xxx`运行，可在场景2的基础上加载指定iqbin文件，xxx为iqbin文件路径
> 按`./prog_uvc_uvc_device_demo 1 index 0`运行，可在场景2的基础上指定选择sensor的index列表
> 按`./prog_uvc_uvc_device_demo 1 mode 0`运行，可在场景2的基础上选择和UVC控制端的传输方式，0：USB_ISOC_MODE，1：USB_BULK_MODE。
> 按`./prog_uvc_uvc_device_demo 1 hdr 0`运行，可在场景2的基础上选择sensor 模式，0：linear，1：hdr。

如不添加index参数，运行demo后，会打印sensor可选择输入的res，输入你想选择preview的res（sensor resolution），例如imx415sensor，会打印以下log：

```
index 0, Crop(0,0,3840,2160), outputsize(3860,2250), maxfps 20, minfps 3, ResDesc 3840x2160@20fps
index 1, Crop(0,0,3072,2048), outputsize(3096,2190), maxfps 30, minfps 3, ResDesc 3072x2048@30fps
index 2, Crop(0,0,3072,1728), outputsize(3096,1758), maxfps 30, minfps 3, ResDesc 3072x1728@30fps
index 3, Crop(0,0,2592,1944), outputsize(2616,1974), maxfps 30, minfps 3, ResDesc 2592x1944@30fps
index 4, Crop(0,0,2944,1656), outputsize(2976,1686), maxfps 30, minfps 3, ResDesc 2944x1656@30fps
index 5, Crop(4,4,2560,1440), outputsize(2592,1470), maxfps 30, minfps 3, ResDesc 2560x1440@30fps
index 6, Crop(0,6,1920,1080), outputsize(1920,1080), maxfps 60, minfps 3, ResDesc 1920x1080@60fps
index 7, Crop(0,0,3840,2160), outputsize(3864,2192), maxfps 30, minfps 3, ResDesc 3840x2160@30fps
index 8, Crop(12,16,3840,2160), outputsize(3864,2192), maxfps 60, minfps 3, ResDesc 3840x2160@60fps
index 9, Crop(0,0,3840,2160), outputsize(3864,2250), maxfps 10, minfps 3, ResDesc 3840x2160@10fps_2lane
index 10, Crop(0,0,2688,1520), outputsize(2688,1520), maxfps 30, minfps 3, ResDesc 2688x1520@30fps_2lane
index 11, Crop(0,0,1920,1080), outputsize(1920,1080), maxfps 20, minfps 3, ResDesc 1920x1080@20fps_2lane
index 12, Crop(0,0,1920,1080), outputsize(1920,1080), maxfps 30, minfps 3, ResDesc 1920x1080@30fps_2lane
index 13, Crop(4,4,2560,1440), outputsize(2592,1470), maxfps 15, minfps 3, ResDesc 2592x1470@15fps_2lane
choice which resolution use, cnt 14
0
You select 0 res
Res 0
```

---

## 五、运行结果说明

* preview效果查看
1. 运行demo后查看PC端的，设备管理器->照相机下是否有对应的UVC设备如：UVC Camera 0，UVC Camera 1；
2. 使用potplayer 软件，打开->设备设置->摄像头；
3. 设备：选择UVC Camera 0或者UVC Camera 1；格式：选择场景中描述的支持码流格式和的分辨率；输出：选择捕获
4. 点击 ：打开设备，即可看到对应码流分辨率的画面

```
成功获取Mjpeg，1920x1080@30fps码流log：
(UVC_StartCapture 551)exec function pass
Capture u32Width: 1920, u32height: 1080, format: MJPEG
UVC: 3 buffers alloc.
UVC: Start video stream Successfully.
```

* 注意事项
1. 如果potplayer 没有检测到video设备，请先确定设备管理器端是否识别，其次可以点击‘检索更新’刷新；

2. demo端的maxpack，mult，burst，intf参数应该跟脚本的配置项对应；
* 退出命令

    输入`q`即可退出demo

---

## 六、参数修改说明

* uvc-configfs脚本参数说明

  ```
  UVC_STREAMING_NAME_ARRAY：
  配置每一路UVC设备的名字             例：配置两路uvc的名字：UVC_STREAMING_NAME_ARRAY="UVC Camera 0,UVC Camera 1"

  UVC_STREAMING_MAX_PACKET_ARRAY：
  配置每一路uvc设备的maxpacket属性值      例：配置两路uvc的maxpacket为3072和1024：UVC_STREAMING_MAX_PACKET_ARRAY="3072,1024"

  UVC_STREAMING_MAX_BURST_ARRAY：
  配置每一路uvc设备的burst属性值          例：配置两路uvc的brust为13和13：UVC_STREAMING_MAX_BURST_ARRAY="13,13"

  UVC_STREAMING_INTERVAL_ARRAY：
  配置每一路uvc设备的interval属性值       例：配置两路uvc的interval为1：UVC_STREAMING_INTERVAL_ARRAY="1,1"

  UVC_MODE：
  配置uvc设备的传输模式                例:UVC_MODE = 0x0 :isoc mode  UVC_MODE = 0x1:bulk mode

  UVC_STREAMING_FORMAT_YUV_ENABLE:
  是否使能YUV数据格式                   例:UVC_STREAMING_FORMAT_YUV_ENABLE =0：关闭，UVC_STREAMING_FORMAT_YUV_ENABLE=1：打开

  UVC_STREAMING_FORMAT_NV12_ENABLE:
  是否使能NV12数据格式                  例:UVC_STREAMING_FORMAT_NV12_ENABLE =0：关闭，UVC_STREAMING_FORMAT_NV12_ENABLE=1：打开

  UVC_STREAMING_FORMAT_MJPEG_ENABLE:
  是否使能Mjpeg数据格式                 例:UVC_STREAMING_FORMAT_MJPEG_ENABLE =0：关闭，UVC_STREAMING_FORMAT_MJPEG_ENABLE=1：打开

  UVC_STREAMING_FORMAT_MJPEG_ENABLE:
  是否使能Mjpeg数据格式                 例:UVC_STREAMING_FORMAT_MJPEG_ENABLE =0：关闭，UVC_STREAMING_FORMAT_MJPEG_ENABLE=1：打开

  UVC_STREAMING_FORMAT_H264_ENABLE:
  是否使能H264数据格式                  例:UVC_STREAMING_FORMAT_H264_ENABLE =0：关闭，UVC_STREAMING_FORMAT_H264_ENABLE=1：打开

  UVC_STREAMING_FORMAT_H265_ENABLE:
  是否使能H265数据格式                  例:UVC_STREAMING_FORMAT_H265_ENABLE =0：关闭，UVC_STREAMING_FORMAT_H265_ENABLE=1：打开

  UVC_VERSION=0x0110：
  选择uvc版本                         例:UVC_VERSION =0x0150：uvc1.5版本，UVC_VERSION=0x0110：uvc1.1版本
  ```

* 例：增加一组H264码流的分辨率1088x1920@30fps
1. 修改gadget-uvc-configfs.sh脚本

   ```
   UVC_STREAMING_FORMAT_H264_ENABLE=1      //确保H264码流有Enable
   ```

config_uvc_h264()
{
    config_uvc_format_h264
    config_uvc_frame_h264 320 240 333333
    config_uvc_frame_h264 640 480 333333
    config_uvc_frame_h264 1280 720 333333
    add+++++++++++++++++++++++++++++++++
    config_uvc_frame_h264 1088 1920 333333
    ++++++++++++++++++++++++++++++++++++
    config_uvc_frame_h264 1920 1080 333333
    config_uvc_frame_h264 2560 1440 333333
    config_uvc_frame_h264 3840 2160 333333
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264 ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/h264
}

```
2.   修改demo-code
```

需要修改：/sdk/verify/common/ss_uvc/ss_uvc_datatype.h

static const struct uvc_frame_info uvc_frames_h264[] = {
    {  320, 240, { 333333, 666666, 1000000,  0 }, },
    {  640, 480, { 333333, 666666, 1000000,  0 }, },
    { 1280, 720, { 333333, 666666, 1000000,  0 }, },
    add+++++++++++++++++++++++++++++++++
    { 1088, 1920, { 333333, 666666, 1000000,  0 }, },
    ++++++++++++++++++++++++++++++++++++
    { 1920,1080, { 333333, 666666, 1000000,  0 }, },
    { 2560,1440, { 333333, 666666, 1000000,  0 }, },
    { 3840,2160, { 333333, 666666, 1000000,  0 }, },
    { 0, 0, { 0, }, },
};

```
注意：上面两处增加的分辨率在整个分辨率表中的位置应该一致，防止出现播放器选择的分辨率和实际出流不对等的情况
* 例：增加出yuyv格式的数据
1.   修改gadget-uvc-configfs.sh脚本
```

UVC_STREAMING_FORMAT_YUV_ENABLE=1       //确保开启yuyv数据格式

config_uvc_yuv()                       //确认此函数中有相关分辨率的定义
{
    config_uvc_format_yuyv
    config_uvc_frame_yuyv 320 240 333333
    config_uvc_frame_yuyv 640 480 333333
    config_uvc_frame_yuyv 1280 720 333333
    config_uvc_frame_yuyv 1920 1080 333333
    config_uvc_frame_yuyv 2560 1440 333333
    config_uvc_frame_yuyv 3840 2160 666666
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/yuyv
}

```
2.   修改demo-code
```

需要修改：/sdk/verify/common/ss_uvc/ss_uvc_datatype.h

static const struct uvc_format_info uvc_formats[] = {
    add+++++++++++++++++++++++++++++++++
    { V4L2_PIX_FMT_YUYV,  uvc_frames_yuyv },    //确认数组uvc_frames_yuyv下有相关分辨率的定义
    ++++++++++++++++++++++++++++++++++++
    { V4L2_PIX_FMT_MJPEG, uvc_frames_mjpg },
    { V4L2_PIX_FMT_H264,  uvc_frames_h264 },
    { V4L2_PIX_FMT_H265,  uvc_frames_h265 },
};
static const struct uvc_format_info uvc_still_formats[] = {
    add+++++++++++++++++++++++++++++++++
    { V4L2_PIX_FMT_YUYV,  uvc_frames_yuyv },    //确认数组uvc_frames_yuyv下有相关分辨率的定义
    ++++++++++++++++++++++++++++++++++++
    { V4L2_PIX_FMT_MJPEG, uvc_still_frames_mjpg },
    { V4L2_PIX_FMT_H264,  uvc_still_frames_h264 },
    { V4L2_PIX_FMT_H265,  uvc_still_frames_h265 },
};


```
注意：脚本和code增加的数据格式在数据格式队列中应该位置相同，防止出现播放器选择的格式和实际格式不对等的情况
```
