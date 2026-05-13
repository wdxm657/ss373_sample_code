# classification_demo使用说明

---

## 一、demo功能演示

- 目标分类

  ```
  +---------+      +---------+      +---------+                     +--------+     +--------+
  |   VIF   | ---> |   ISP   | ---> |   SCL   | ---port0:Source---> |  VENC  | --> |  RTSP  |
  +---------+      +---------+      +----+----+                     +--------+     +--------+
                                         |                               ^
                                         |                               |
                                         |                               |
                                         |                               |
                                         |         +--------+       +----+---+
                                  port1:Scaled---> |  CLS   | ----> |  RGN   |
                                                   +--------+       +--------+
  ```

  - 将dla的cls功能串进pipeline，scl port0和port1分别将来自vif的原始分辨率的bufferinfo和经由scl缩放成ipu网络模型支持的分辨率大小的bufferinfo送给cls处理，得到画面中的目标分类信息后，使用rgn的osd功能，attach到venc绘制出相应的单词。
  - 根据指定的检测模型文件不同，demo可以检测人宠/行人目标，并用红色字体标识出来。
  - 当键入字符"q"时，进程退出。


---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译；

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/dla/classification_demo`命令进行编译;

3. 到`sample_code/out/arm/app/prog_dla_classification_demo`获取可执行文件;

---

## 三、运行环境说明

> DualOs系统中使用的Sensor类型在编译时决定，可查看alkaid deconfig的CONFIG_SENSOR_TYPE参数确认

- 板端环境：

  `SSC032A-S01A-S`型号板子sensor pad0对应CON3，在sensor pad0位置接上mipi sensor即可，此demo使用的是imx415

- dts配置：

  默认dts已配好，无需修改

- sensor 驱动配置：

  默认已加载好imx415_MIPI.ko，无需修改

  ```
  insmod /config/modules/5.10/imx415_MIPI.ko chmap=1
  ```

---

## 四、运行说明

- 将可执行文件`prog_dla_classification_demo`放到板子上，修改权限为777
- 使用sensor为imx415

- 运行命令`./prog_dla_classification_demo index 6 model x`进入目标分类（由于字体库为非矢量库，选择高分辨率出流时，可能会出现字体过小，显示效果不好，推荐使用index 6预览，即1920x1080分辨率）

  - input：
    - `model `：ipu网络模型文件路径。需要手动从`alkaid/project/board/ifado/dla_file/ipu_net/hc36.img  hpc36.img`拷贝出来，运行demo时，使用`model`参数指定路径。模型的详细规格见`alkaid/project/board/ifado/dla_file/ipu_net/README.txt`及doc文档。

  - output：
    - 无

---

## 五、运行结果说明

- 效果查看

  demo运行起来后，正常出流会打印rtsp url，使用vlc media player 或 potplayer 等视频播放软件，以打开链接方式播放，播放成功可看到sensor画面。url例如：`rtsp://your_ipaddr/6600`


---
