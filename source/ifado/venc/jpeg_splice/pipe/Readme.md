# jpeg_splice_demo使用说明

---
## 一、demo场景
* 8M打猎相机场景
    ```
       +-----+                                  +------------------+   +---------+
       | VIF |                                  |  SCL Dev1        |   |VENC Dev8|
       +--+--+                                  |                  |   |         |   +----------------------+
          |                                     +------+  2560x1440|   +------+  |   |   Splice 5120x2880   |
          v                    +--------------->|Chn0  +-------------->|Chn0  +---->-------------+----------+
       +--+--+                 |                +------+           |   +------+  |   |Chn0       | Chn1     |
       | ISP |       +-----------------------+  |                  |   |         |   |2560x1440  | 2560x1440|
       +--+--+       |1920x1080| |1920x1080  |  +------+  2560x1440|   +------+  |   |           |          |
          |          |         + |         +--->|Chn1  +-------------->|Chn1  +----->-----------------------+
          v          |           |           |  +------+           |   +------+  |   |Chn2       | Chn3     |
    +-----+------+   +-----------------------+  |                  |   |         |   |2560x1440  | 2560x1440|
    |  User Get  +-->+1920x1080  |1920x1080  |  +------+  2560x1440|   +------+  |   |           |          |
    |            |   |         +--------------->|Chn2  +-------------->|Chn2  +----->+--+--------+----------+
    |  2560x1440 |   |           |           |  +------+           |   +------+  |      ^
    +------------+   +-----------------------+  |                  |   |         |      |
                                 |        |     +------+  2560x1440|   +------+  |      |
                                 |        +---->|Chn3  +-------------->|Chn3  +---------+
                                 |              +------+           |   +------+  |
                                 |              |                  |   |         |   +-------------+
                                 |  2560x1440   +------+  640x360  |   +------+  |   |             |
                                 +------------->|Chn4  +---------------|Chn4  +---------640x360    |
                                                +------+           |   +------+  |   |             |
                                                |                  |   |         |   +-------------+
                                                +------------------+   +---------+

    参数说明：   sensor接pad0，vif使用dev0 chn0 port0，isp使用dev0 chn0 port1
                scl使用dev1 chn{0,1,2,3,4}均为port0，venc使用dev8 chn{0,1,2,3,4}
                vif和isp为realtime mode，scl和venc各个chn一一对应使用frame mode
                isp出2560x1440 Buf在User层crop成4张1920x1080，分别将4张1920x1080 Buf和原2560x1440 Buf输入Scl的5个chn
                scl chn4 将原2560x1440的Buf缩小为640x360，chn0~3 将4张1920x1080的Buf放大为2560x1440
                venc chn4 单独输出一张640x360的jpg图，chn0~3分别输出4张2560x1440的jpg图，然后手动拼接为5120x2880的jpg图
                isp和scl的相关图像格式均为 YUV420NV12

    PS:         若需要使用JPG图拼接功能，必须通过 ST_Common_SetJpegSpliceChnAttr
                接口配置Venc Chn属性，修改输出的JPG图的Marker信息。

    ```

---
## 二、编译环境说明
1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/venc/jpeg_splice/pipe`进行编译;

3. 到`sample_code/out/arm/app/prog_venc_jpeg_splice_pipe`获取可执行文件;

---
## 三、运行环境说明
> DualOs系统中使用的Sensor类型在编译时决定，可查看alkaid deconfig的CONFIG_SENSOR_TYPE参数确认

* 板端环境：

    `SSC032A-S01A-S`型号板子单sensor场景下sensor pad0对应CON3；双sensor场景下sensor pad0对应CON4，sensor pad2对应CON5;

    单sensor场景下在sensor pad0(CON3)位置接上mipi sensor，对应跑的是4lane，可以是imx415，imx307等;

* dts配置：

    mipi snr0 4lane EVB上使用，默认dts已配好，无需修改

    mipi snr0+snr1 2+2lane EVB上使用，默认dts已配好，无需修改

    > 例如上述编译环境说明的`SSC032A-S01A-S`型号板子，直接编译使用即可

* sensor 驱动配置：

    /customer/demo.sh路径修改insmod对应的sensor ko，例如imx415 sensor时，可配置为：
    ```
    insmod /config/modules/5.10/imx415_MIPI.ko chmap=1(跑单sensor)

    ```

* 输入文件：

    场景1可加载指定的iq bin文件，如不指定默认加载/config/iqfile/`sensorname`_api.bin文件；
    部分sensor的iq bin文件可在`/project/board/ifado/iqfile`路径下获取。

---
## 四、运行说明
将可执行文件prog_venc_jpeg_splice_pipe放到板子上，修改权限777

1. 按`./prog_venc_jpeg_splice_pipe index 5 iqbin xxx savenum xx`运行场景；

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

运行后会在当前路径下的`out`文件夹内保存`savenum`后对应的组数的JPG图像

**退出**

    保存对应图像后自动退出