# multi_ring使用说明


## 一、demo场景
    vif->isp->scl->main venc->rtsp0
                        |
                        ->sub venc->rtsp1

    参数说明： vif接sensor pad0，isp/scl/main venc使用dev0 chn0,sub venc使用dev0 chn1

    绑定模式：vif->isp->scl为realtime，scl->main venc为hw_ring，main venc->sub venc为hw_ring

    分辨率说明：目前main venc的分辨率为2560*1440，sub venc的分辨率为1920*1080
    ```

---
## 二、编译环境说明
1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/venc/multi_ring`进行编译;

3. 到`sample_code/out/arm/app/prog_venc_multi_ring`获取可执行文件;

---
## 三、运行环境说明
> DualOs系统中使用的Sensor类型在编译时决定，可查看alkaid deconfig的CONFIG_SENSOR_TYPE参数确认

* 板端环境：

    `SSC032A-S01A-S`型号板子sensor pad0对应CON3;

    在sensor pad0位置接上mipi sensor，对应跑的是4lane，可以是imx415，imx307等;

* dts配置：
    mipi snr0+4lane EVB上使用，默认dts已配好，无需修改

    > 例如上述编译环境说明的`SSC032A-S01A-S`型号板子，直接编译使用即可

* sensor 驱动配置：
    /customer/demo.sh路径修改insmod对应的sensor ko，当前 demo可配置为：

    ```
    insmod /config/modules/5.10/imx415_MIPI.ko chmap=1 lane_num=4
    ```

* 输入文件：

    场景可加载指定的iq bin文件，如不指定默认加载/config/iqfile/`sensorname`_api.bin文件；
    部分sensor的iq bin文件可在`/project/board/ifado/iqfile`路径下获取。

---
## 四、运行说明
将可执行文件prog_vif_sensor_demo放到板子上，修改权限777

1.   按`./prog_venc_multi_ring`运行场景multi ring；
> 按`./prog_venc_multi_ring iqbin xxx`运行，可在场景的基础上加载指定iqbin文件，xxx为iqbin文件路径


---
## 五、运行结果说明
* preview效果查看

正常出流会打印rtsp url，因为有两路venc所以会打印两个url，例如以下log：

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