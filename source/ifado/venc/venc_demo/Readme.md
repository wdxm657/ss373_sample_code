# venc_demo使用说明

---

## 一、demo场景

* 场景1：venc 出H264码流 功能演示

  ```
  file -> venc -> file

  venc 读1920x1080 nv12格式文件，通过MI_Sys接口灌入VENC模块中，H264码流格式写出文件
  ```

* 场景2：venc 出H265码流 功能演示

  ```
  file -> venc -> file

  venc 读1920x1080 nv12格式文件，通过MI_Sys接口灌入VENC模块中，H265码流格式写出文件
  ```

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/venc/venc_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_venc_venc_demo`获取可执行文件;

---

## 三、运行环境说明

* 板端环境：

    无要求

* dts配置：

    无要求

* 输入文件：

    需要`source/ifado/venc/venc_demo/resource/input/`路径下文件；

    将`resource`目录放置于板子运行路径，demo会读取以下文件

  ```
  ./resource/input/1920_1080_nv12.yuv
  ```

---

## 四、运行说明

将可执行文件prog_venc_venc_demo放到板子上，修改权限777

1. 命令`./prog_venc_venc_demo 0`运行场景1 出H264码流功能演示；
2. 命令`./prog_venc_venc_demo 1`运行场景2 出H265码流功能演示；

---

## 五、运行结果说明

* 效果查看

    运行成功后会在运行路径下创建路径`./out/venc/`写出`.es`码流文件，如为场景1则生成`venc_demo_case0.es`,如为场景2则生成`venc_demo_case1.es`；

  * H264和H265码流可用Intel® Video Pro Analyzer打开分析
  *

* 退出命令

    运行成功直接退出

---
