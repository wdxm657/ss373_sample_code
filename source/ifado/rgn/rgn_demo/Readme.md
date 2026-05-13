# rgn_demo使用说明

---
## 一、demo场景
* 场景1：rgn在scl ouput上贴color cover功能演示
    ```
    file -> scl(rgn attach cover color) -> venc -> rtsp

    scl 读1920x1080 nv12格式文件， scl和venc framemode bind，scl outputport 叠加一个矩形的color cover
    ```

* 场景2：rgn在venc上叠加osd功能演示
    ```
    file -> scl -> venc(rgn attach osd) -> rtsp

    scl 读1920x1080 nv12格式文件，scl和venc framemode bind，venc通道上叠加osd
    ```

* 场景3：rgn在venc上画人脸框frame功能演示
    ```
    file -> scl -> venc(rgn attach cover Frame) -> rtsp

    scl 读1920x1080 nv12格式文件，scl和venc framemode bind，venc通道上贴两个人脸框
    ```

* 场景4：rgn在venc上贴osd并画字符功能演示
    ```
    file -> scl-> venc(rgn attach osd Draw Text) -> rtsp

    scl 读1920x1080 nv12格式文件，scl和venc framemode bind，venc通道上贴osd，写固定字符
    ```

* 场景5：rgn在scl ouput上贴color cover&frame rotate功能演示
    ```
    file -> scl -> venc(rgn attach cover color & frame) -> rtsp

    scl 读1920x1080 nv12格式文件， scl和venc framemode bind，venc通道上贴一个旋转过的矩形color cover和一个旋转过的矩形框color frame。
    ```

---
## 二、编译环境说明
1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行 `make clean && make source/ifado/rgn/rgn_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_rgn_rgn_demo`获取可执行文件;

---
## 三、运行环境说明
* 板端环境：

    无要求

* dts配置：

    无要求

* 输入文件：

    需要`source/ifado/rgn/rgn_demo/resource/input/`路径下文件；

    将`resource`目录放置于板子运行路径，demo会读取以下两个文件
    ```
    ./resource/input/1920_1080_nv12.yuv
    ./resource/input/424x224.i4.yuv
    ```

---
## 四、运行说明
将可执行文件prog_rgn_rgn_demo放到板子上，修改权限777
1.   命令`./prog_rgn_rgn_demo 0`运行场景1 贴color cover功能演示；
2.   命令`./prog_rgn_rgn_demo 1`运行场景2 贴osd功能演示；
3.   命令`./prog_rgn_rgn_demo 2`运行场景3 贴人脸框功能演示；
4.   命令`./prog_rgn_rgn_demo 3`运行场景4 贴osd并画字符演示；
5.   命令`./prog_rgn_rgn_demo 4`运行场景5 贴color cover&frame rotate功能演示；

---
## 五、运行结果说明

* preview效果查看

正常出流会打印rtsp url，例如以下log：

使用vlc media player 或 potplayer 等视频播放软件，以打开链接方式播放，播放成功可看到rgn贴图画面。

```
rtsp venc dev0, chn 0, type 3, url 6600
=================URL===================
rtsp://10.34.16.66/6600
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