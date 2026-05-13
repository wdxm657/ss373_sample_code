# getrawdata_demo使用说明

该demo仅支持在VIF和ISP以Realtime绑定时使用，Framemode时可以通过SYS接口（MI_SYS_ChnOutputPortGetBuf）直接获取VIF raw data，无需参考此demo。

## 一、demo场景

VIF工作在realtime mode下，可以通过此demo获取sensor raw data（模仿IQ tool抓取raw data）。

通过开两个telnet实现进程1运行realtime pipeline，进程2使用getrawdata_demo抓取sensor raw data

* 进程1：

  ```
  vif->isp->scl realtime pipeline
  ```

    可跑任意的vif->isp realtime绑定相关APP，也可使用source/ifado/vif/sensor_demo
* 进程2：

  ```
  运行prog_vif_getrawdata_demo抓取raw data
  ```

---

## 二、编译环境说明

1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译；

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令进行编译;

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`
    `make clean && make image -j8`

2. 进到sample_code目录执行`make clean && make source/ifado/vif/getrawdata_demo`命令进行编译;

3. 到`sample_code/out/arm/app/prog_vif_getrawdata_demo`获取可执行文件;

---

## 三、运行环境说明

> DualOs系统中使用的Sensor类型在编译时决定，可查看alkaid deconfig的CONFIG_SENSOR_TYPE参数确认

* 板端环境：

    在sensor pad0位置接上mipi sensor，可支持4lane输出，可以是imx415，imx307等；

* dts配置：

    mipi snr0 4lane EVB上使用，默认dts已配好，无需修改；

    > 例如上述编译环境说明的`SSC032A-S01A-S`型号板子，直接编译使用即可

* sensor 驱动配置：

    /customer/demo.sh路径修改insmod对应的sensor ko，例如imx415跑单sensor时，可配置为：

  ```
  insmod /config/modules/5.10/imx415_MIPI.ko chmap=1 lane_num=4
  ```

* 输入文件：

    无

---

## 四、运行说明

1. 先在telnet1把正常出流的APP跑起来，此APP需要为vif->isp realtime绑定的；

2. 然后在telnet2将可执行文件prog_vif_getrawdata_demo放到板子上，修改权限777，按`./prog_vif_getrawdata_demo`命令运行；

---

## 五、运行结果说明

在prog_vif_getrawdata_demo运行路径下，out/vif/目录生成getrawdata_demo_vif.raw文件(已经转换成16bpp)；

文件可用7yuv打开，选择Bayer RGGB/GRBG/GBRG/BGGR 16-bit format打开查看sensor抓取的raw图。

---