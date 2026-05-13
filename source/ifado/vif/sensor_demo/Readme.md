# sensor_demo使用说明

---
## 一、demo场景
* 场景1：单sensor realtime pipeline
    ```
    vif->isp->scl->venc->rtsp

    参数说明： vif接sensor pad0，isp/scl/venc使用dev0 chn0

    绑定模式：vif->isp->scl为realtime，scl->venc为framemode
    ```

* 场景2：双sensor framemode pipeline
    ```
    vif0/vif1->isp->scl->venc->rtsp

    参数说明：vif0接sensor pad0，vif1接sensor pad2，isp/scl/venc使用dev0 chn0和chn1

    绑定模式：vif->isp为framemode,isp->scl为realtime,scl->venc为framemode
    ```

---
## 二、编译环境说明
1. 在project路径下根据板子（nand/nor，ddr型号等）与场景选择deconfig进行整包编译;

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `场景1:ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`
    `场景2:ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_dualsnr_defconfig`
    根据场景在project目录执行以下命令进行编译;

    `场景1:make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `场景2:make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_dualsnr_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/vif/sensor_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_vif_sensor_demo`获取可执行文件;

---
## 三、运行环境说明
> DualOs系统中使用的Sensor类型在编译时决定，可查看alkaid deconfig的CONFIG_SENSOR_TYPE参数确认

* 板端环境：

    `SSC032A-S01A-S`型号板子单sensor场景下sensor pad0对应CON3；双sensor场景下sensor pad0对应CON4，sensor pad2对应CON5;
    单sensor场景下在sensor pad0(CON3)位置接上mipi sensor，对应跑的是4lane，可以是imx415，imx307等;
    双sensor场景下在sensor pad0(CON4)和sensor pad2(CON5)位置接上mipi sensor，对应跑的是2+2lane，可以是imx415，imx307等;
    双sensor场景下需要注意JP143和JP144的跳帽需要插上。

* dts配置：

    mipi snr0 4lane EVB上使用，使用场景1对应的defconfig的情况下，默认dts已配好，无需修改
    mipi snr0+snr1 2+2lane EVB上使用，使用场景2对应的defconfig的情况下，默认dts已配好，无需修改

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
将可执行文件prog_vif_sensor_demo放到板子上，修改权限777

1.   按`./prog_vif_sensor_demo 0`运行场景1单sensor realtime pipeline；
> 按`./prog_vif_sensor_demo 0 iqbin xxx`运行，可在场景1的基础上加载指定iqbin文件，xxx为iqbin文件路径

2.   按`./prog_vif_sensor_demo 1`运行场景2双sensor framemode pipeline；

运行demo后，会打印sensor可选择输入的res，输入你想选择preview的res（sensor resolution），例如imx415sensor，会打印以下log：
* 注意同一型号sensor，跑两路流，请输入同一个res index
* 注意I6DC支持的分辨率上限为2560*1440，请选择不要大于这个分辨率的index
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
5
You select 5 res
Res 0
```

* 注意运行场景2双sensor framemode pipeline时，需要选择带2lane结尾的分辨率才能正常使用。
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
13
You select 13 res
Res 13
```


---
## 五、运行结果说明
* preview效果查看

正常出流会打印rtsp url，单sensor打印一个url，双sensor打印两个url，例如以下log：

使用vlc media player 或 potplayer 等视频播放软件，以打开链接方式播放，播放成功可看到sensor画面。
```
rtsp venc dev0, chn 0, type 3, url 6600
=================URL===================
rtsp://your_ipaddr/6600
=================URL===================
Create Rtsp H265 Session, FPS: 30
rtsp venc dev0, chn 1, type 3, url 6601
=================URL===================
rtsp://your_ipaddr:555/6601
=================URL===================
Create Rtsp H265 Session, FPS: 30
```
>  注意：your_ipaddr如果为0.0.0.0，请先配置网络，配置方法可参考如下：
```
ifconfig eth0 192.168.1.10 netmask 255.255.255.0
route add default gw 192.168.1.1
```


* 退出命令

    输入`q`即可退出demo

---