# scl_demo使用说明

---
## 一、demo场景
* 场景1：scl stretchbuf 功能演示
    ```
    file -> scl -> file

    scl 读1920x1080 nv12格式文件，使用MI_SCL_StretchBuf接口，裁剪成720x540 nv12格式写出文件
    ```


---
## 二、编译环境说明
1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/scl/scl_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_scl_scl_demo`获取可执行文件;

---
## 三、运行环境说明
* 板端环境：

    无要求

* dts配置：

    无要求

* 输入文件：

    需要`source/ifado/scl/scl_demo/resource/input/`路径下文件；

    将`resource`目录放置于板子运行路径，demo会读取以下两个文件
    ```
    ./resource/input/1920_1080_nv12.yuv
    ```

---
## 四、运行说明
将可执行文件prog_scl_scl_demo放到板子上，修改权限777
1.   命令`./prog_scl_scl_demo`运行场景1 stretchbuf功能演示；


---
## 五、运行结果说明
* 效果查看

    运行成功后会在运行路径下创建路径`./out/scl/`写出`scl_demo_stretchbuf.yuv`文件；

    可用7yuv软件打开，选择分辨率为720x540，format格式为NV12查看。

    也可以与`resource/output/`路径下标准文件作对比。

* 退出命令

    运行成功自动退出

---