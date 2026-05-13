# splice_demo使用说明

---
## 一、demo场景
* 场景1：手动横向拼接JPEG码流 功能演示
    ```
    file -> venc -> splice -> file

    venc 读1920x1080（必须16对齐）nv12格式文件，使用MI_Sys接口灌入到VENC模块中，VENC出四张JEPEG码流进行横向拼接
    ```
* 场景2：手动纵向拼接JPEG码流 功能演示
    ```
    file -> venc -> splice -> file

    venc 读1920x1080（必须16对齐）nv12格式文件，使用MI_Sys接口灌入到VENC模块中，VENC出四张JEPEG码流进行纵向拼接
    ```
* 场景3：手动Z型拼接JPEG码流 功能演示
    ```
    file -> venc -> splice -> file

    venc 读1920x1080（必须16对齐）nv12格式文件，使用MI_Sys接口灌入到VENC模块中，VENC出四张JEPEG码流进行Z型拼接
    ```
---
## 二、编译环境说明
1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/venc/jped_splice/demo`进行编译;

3. 到`sample_code/out/arm/app/prog_venc_jpeg_splice_demo`获取可执行文件;

---
## 三、运行环境说明
* 板端环境：

    无要求

* dts配置：

    无要求

* 输入文件：

    需要`source/ifado/venc/splice_demo/resource/input/`路径下文件；

    将`resource`目录放置于板子运行路径，demo会读取以下文件
    ```
    ./resource/input/1920_1080_nv12.yuv
    ```

---
## 四、运行说明
将可执行文件prog_venc_jpeg_splice_demo放到板子上，修改权限777
1.   命令`./prog_venc_jpeg_splice_demo 0`运行场景1 横向拼接功能演示；
2.   命令`./prog_venc_jpeg_splice_demo 1`运行场景2 纵向拼接功能演示；
3.   命令`./prog_venc_jpeg_splice_demo 2`运行场景3 Z型拼接功能演示；
4.   命令`./prog_venc_jpeg_splice_demo 0/1/2 dump`在上述命令的基础上加上传参`dump`，会dump下四张拼接的原始jpeg文件；在拼接图像效果异常时，可用于判断是拼接前图像异常还是拼接后图像异常；


---
## 五、运行结果说明
* 效果查看

    运行成功后会在运行路径下创建路径`./out/venc/`写出拼接后的文件，`splice_demo_horizontal_splice.jpeg`为场景1写出文件
    `splice_demo_vertical_splice.jpeg`为场景2写出文件，`splice_demo_Z_splice.jpeg`为场景3写出文件。
    如果传参加上`dump`，会在`./out/venc/`路径下产生组成上图的四张原始JPEG图片，例如场景1写出文件如下：
    ```
    splice_demo_horizontal_jpeg_out_0.jpeg
    splice_demo_horizontal_jpeg_out_1.jpeg
    splice_demo_horizontal_jpeg_out_2.jpeg
    splice_demo_horizontal_jpeg_out_3.jpeg
    ```
    可用画图工具直接打开。

* 退出命令

    运行成功直接退出

---