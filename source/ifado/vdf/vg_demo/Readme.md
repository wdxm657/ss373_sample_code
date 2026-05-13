# vg_demo使用说明

仅支持在pure linux系统中运行

---

## 一、demo场景

- 场景1：ISP->SCL->VDF->VENC->RGN

  ```

  从ISP灌视频流通过SCL输出到VDF_VG mode 实现电子围栏，并通过VENC上RGN模块显示检测结果。

  参数说明： 该场景内所有分辨率均为`640x360`

  绑定模式：isp->scl->vdf vdf->venc 均为framemode

  ```

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`

    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/vdf/vg_demo`进行编译;

3. 到`sample_code/out/arm/app/prog_vdf_vg_demo`获取可执行文件;

---

## 三、运行环境说明

- 板端环境：

    `SSC032A-S01A-S`型号板子sensor pad0对应CON3;


- dts配置：

  > 例如上述编译环境说明的`SSC032A-S01A-S`型号板子，直接编译使用即可


- 输入文件：

   场景1处理电子围栏所需要的需要yuv视频流 `vg_test_640x360_422yuyv.yuv`可在` "sample_code/source/ifado/vdf/vg_demo/resource/input/vg_test_640x360_422yuyv.yuv" `获取

- 运行此demo程序时需要手动增加以下配置
- /config/modparam.json 配置：

    - ​​​在/config/modparam.json中添加：
```
    "E_MI_MODULE_ID_ISP" :
    {
        "u32DefaultDropNum": 0
    }
```

---

## 四、运行说明

将可执行文件prog_vdf_vg_demo放到板子上，修改权限777

1. 按`./prog_vdf_vg_demo`运行场景1处理电子围栏；

   > 视频流输入路径为`"./resource/input/vg_test_640x360_422yuyv.yuv"`

---

## 五、运行结果说明

1. demo的输出分为两个部分；

   > 检测处理的OSD贴图效果输出为`"./out/vdf/vg_demo_case0.es"`

   > VG检测结果的保存文本`"./out/vdf/vg_result.txt"`