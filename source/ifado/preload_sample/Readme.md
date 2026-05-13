# preload_sample使用说明

---
> 此demo用于DualOS场景下的在rtos创建pipeline以及在Linux创建RTSP拉流的sample code

## 一、demo场景

* 此demo用于dualos场景下，rtos创建pipline，vif->isp->scl->venc。其中单sensor环境下vif->isp使用realtime，双sensor环境下vif->isp使用frame mode。isp->scl使用realtime，scl->venc使用ring mode。linux起来后使用prog_preload_sample_linux demo进行拉流preview画面。
* 此demo本身属于AOV的低电量场景，在此场景中会增加相关的AOV逻辑功能，不在此场景中运行时则是正常的preload demo。当前主要是通过判断是否存在IPU文件区分是否是AOV低电量场景。

```
vif->isp->scl->venc    | -> rtsp
此部分pipline在rtos创建 | linux demo只会parse rtsp部分，创建rtsp拉流
```

---
## 二、编译环境说明
1. 在project路径下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译;

    例如`SSC032X-S01A`型号板子，使用nor，qfn88/128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    单sensor：`ipc-rtos_ifado.nor.uclibc-9.1.0-ramdisk.ssc032b.64.qfn88_aov_defconfig`

    双sensor：`ipc-rtos_ifado.nor.uclibc-9.1.0-ramdisk.ssc032a.128.qfn128_dualsnr_aov_defconfig`

    在project目录执行以下命令进行编译;

    单sensor：`ipc-rtos_ifado.nor.uclibc-9.1.0-ramdisk.ssc032b.64.qfn88_aov_defconfig;`

    双sensor：`make ipc-rtos_ifado.nor.uclibc-9.1.0-ramdisk.ssc032a.128.qfn128_dualsnr_aov_defconfig;`

    `make clean && make image -j8`

2. 如果使用其他的defconfig想要使用本demo，需要配置CONFIG_APPLICATION_SAMPLE，可以进入alkaid/project目录下通过`make menuconfig`来配置，进入menuconfig界面之后通过以下路径打开此config：

    `Rtos->Rtos Application Options->Support pipeline demo applications->Support preload_sample application`

    > 此处默认开启了Application Selector功能，Application Selector作为一个APP启动选择程序使用。rtos启动时它会从earlyinit_setting.json或者headers中获取“APP_0_0”中“NAME”字段的值选择要启动的APP，获取“PARAM”获取启动app的参数，从而达到通过json动态配置要的启动app的目的。例如使用以下代码配置来选择启动的使用本preload demo来启动。

    ```
    "APP_0_0": {
        "NAME": "preload_sample",
        "PARAM": ""
    }
    ```

    > 上述提到的通过json动态配置要启动的app需要打开CONFIG_EARLYINIT_SETTING_FS，可以进入alkaid/project目录下通过`make menuconfig`来配置，进入menuconfig界面之后通过以下路径打开此config：

    > `Rtos->Earlyinit Options ptions->Support Dynamically Load Earlyinit Setting From File System`

    > 决定是否要从文件系统读取 json 更新设定（修改 /misc/earlyinit_setting.json 后 reboot 生效）。开启此config之后可以通过修改/misc/earlyinit_setting.json文件中的NAME选项，改成你想要启动的preload的名字，用于动态切换启动时加载的preload程序。此CONFIG在此处已默认打开。

    * 注意：如果修改后 json 格式不对或者没找到 /misc/earlyinit_setting.json，则会使用编译期间生成的 header。

3. 进到sample_code目录执行`make clean && make source/iford/preload_sample/linux`进行编译;

4. 到`sample_code/out/arm/app/prog_preload_sample_linux`获取可执行文件;

---
## 三、运行环境说明
* 板端环境：

    `SSC032X-S01A`型号板子单sensor pad0对应跑的是4lane，双sensor pad0和pad2对应跑的是2lane，默认preload使用sc4336p sensor。

* dts配置：

    单sensor：mipi snr0 4lane在demo board上使用，默认dts已配好，无需修改

    双sensor：mipi snr0 2lane、mipi snr2 2lane在demo board上使用，默认dts已配好，无需修改

    > 例如上述编译环境说明的`SSC032X-S01A`型号板子，直接编译使用即可


---
## 四、运行说明
* image编译出来之后preload_sample程序默认会放在rtos中，板子上电rtos启动之后会自动加载preload_sample程序。
* linux启动之后将可执行文件prog_preload_sample_linux放到板子上，修改权限777。
1.   按`./prog_preload_sample_linux`运行dualos场景对应的拉流demo；

---
## 五、运行结果说明
* 板子启动之后在运行prog_preload_sample_linux demo之前可以通过以下指令去查看rtos出流情况：

```
cat /proc/mi_modules/mi_vif/mi_vif0
cat /proc/mi_modules/mi_isp/mi_isp0
cat /proc/mi_modules/mi_scl/mi_scl0
cat /proc/mi_modules/mi_venc/mi_venc0
```

* preview效果查看

正常出流会打印rtsp url，rtsp接了多个chn会打印多个url，例如以下log：

使用vlc media player 或 potplayer 等视频播放软件，以打开链接方式播放，播放成功可看到sensor画面。
```
rtsp venc dev0, chn 0, type 3, url 6600
=================URL===================
rtsp://your_ipaddr/6600
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
* 上面提供的defconfig都是在AOV场景下使用的，具体实现的AOV场景功能可以参考aov目录下Readme中的低电量模式部分说明。
