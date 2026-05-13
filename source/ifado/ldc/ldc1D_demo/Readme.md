# ldc1D_demo使用说明

---
## 一、demo场景
* LDC一维矫正模式————ldc1D pipeline
    ```
      +----------+      +----------+      +----------+                            +------+
      |          |      |          |      |          |  Chn0 port0  2560x1440     |      |
      | VIF Dev0 +------> ISP Dev0 +------> SCL Dev1 +----------------------------> VENC |
      |          |      |  (ldc1D) |      |          |                            |      |
      +----------+      +----------+      +-----+----+                            +------+

    参数说明： vif 接sensor pad0，isp/venc使用dev0 chn0,scl使用dev1 chn0

    绑定模式：vif->isp（ldc1D）->scl->venc均为framemode
    ```

---
## 二、编译环境说明
1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/ldc/ldc1D_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_ldc_ldc1d_demo`获取可执行文件;

---
## 三、运行环境说明
> DualOs系统中使用的Sensor类型在编译时决定，可查看alkaid deconfig的CONFIG_SENSOR_TYPE参数确认

* 板端环境：

    `SSC032A-S01A-S`型号板子单sensor场景下sensor pad0对应CON3；双sensor场景下sensor pad0对应CON4，sensor pad2对应CON5;

    单sensor场景下在sensor pad0(CON3)位置接上mipi sensor，对应跑的是4lane，可以是imx415，imx307等;

    双sensor场景下在sensor pad0(CON4)和sensor pad2(CON5)位置接上mipi sensor，对应跑的是2+2lane，可以是imx415，imx307等;

    一维LDC参数依赖于镜头，不同镜头需要重新进行标定, 可通过工具`CVTool`生成，工具使用可参考相关文档。

    一维LDC参数：
    ```
    stIspLdcAttr.u32CenterXOffset   = 16018;
    stIspLdcAttr.u32CenterYOffset   = 8098;
    stIspLdcAttr.u32Alpha           = 307;
    stIspLdcAttr.u32Beta            = 153;
    stIspLdcAttr.u32CropLeft        = 0;
    stIspLdcAttr.u32CropRight       = 0;
    ```

* dts配置：

    mipi snr0 4lane EVB上使用，默认dts已配好，无需修改
    mipi snr0+snr1 2+2lane EVB上使用，默认dts已配好，无需修改

    > 例如上述编译环境说明的`SSC032A-S01A-S`型号板子，直接编译使用即可

* sensor 驱动配置：

    /customer/demo.sh路径修改insmod对应的sensor ko，例如imx415 sensor时，可配置为：
    ```
    insmod /config/modules/5.10/imx415_MIPI.ko chmap=1(跑单sensor)
    insmod /config/modules/5.10/imx415_MIPI.ko chmap=7 lane_num=2(跑双sensor)
    ```

* 输入文件：

    场景1可加载指定的iq bin文件，如不指定默认加载/config/iqfile/`sensorname`_api.bin文件；
    部分sensor的iq bin文件可在`/project/board/ifado/iqfile`路径下获取。

---
## 四、运行说明
将可执行文件prog_ldc_ldc1d_demo放到板子上，修改权限777

1. 按`./prog_ldc_ldc1d_demo index 5`运行场景1单sensor；

运行demo后，会打印sensor可选择输入的res，输入需要的3840*2160@30fps需求的对应index 5，imx415sensor，会打印以下log：

```
  index 0, Crop(0,0,3840,2160), outputsize(3860,2250), maxfps 20, minfps 3, ResDesc 3840x2160@20fps

  index 1, Crop(0,0,3072,2048), outputsize(3096,2190), maxfps 30, minfps 3, ResDesc 3072x2048@30fps

  index 2, Crop(0,0,3072,1728), outputsize(3096,1758), maxfps 30, minfps 3, ResDesc 3072x1728@30fps

  index 3, Crop(0,0,2592,1944), outputsize(2616,1974), maxfps 30, minfps 3, ResDesc 2592x1944@30fps

  index 4, Crop(0,0,2944,1656), outputsize(2976,1686), maxfps 30, minfps 3, ResDesc 2944x1656@30fps

  index 5, Crop(0,0,2560,1440), outputsize(2592,1470), maxfps 30, minfps 3, ResDesc 2560x1440@30fps

  index 6, Crop(0,0,1920,1080), outputsize(1920,1080), maxfps 60, minfps 3, ResDesc 1920x1080@60fps

  index 7, Crop(12,16,3840,2160), outputsize(3864,2192), maxfps 30, minfps 3, ResDesc 3840x2160@30fps

  index 8, Crop(12,16,3840,2160), outputsize(3864,2192), maxfps 60, minfps 3, ResDesc 3840x2160@60fps

  index 9, Crop(0,0,3840,2160), outputsize(3864,2250), maxfps 5, minfps 3, ResDesc 3840x2160@5fps

  5
```

---

## 五、运行结果说明

**preview效果查看**

正常出流会打印rtsp url，单sensor打印一个url，例如以下log：

使用vlc media player 或 potplayer 等视频播放软件，以打开链接方式播放，播放成功可看到sensor画面。

```
rtsp venc dev0, chn 0, type 3, url 6600

=================URL===================

rtsp://xxx.xxx.xxx.xxx/6600

=================URL===================

Create Rtsp H265 Session, FPS: 30

press q to exit
```

>  注意：your_ipaddr如果为0.0.0.0，请先配置网络，配置方法可参考如下：

```
ifconfig eth0 192.168.1.10 netmask 255.255.255.0

route add default gw 192.168.1.1
```

**退出命令**

    输入`q`即可退出demo
