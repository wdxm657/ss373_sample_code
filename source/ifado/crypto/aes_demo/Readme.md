# aes_demo使用说明

---

## 一、demo场景


对plaintext进行AES_CBC加密后，与期待的加密后数据进行对比；再将加密后的数据解密，与plaintext进行对比；

> aes_demo透过cryptodev中间件，使用ioctl控制/dev/crypto设备。从而能够使用到硬件加速。
> cryptodev-linux 的详细介绍可参考git地址：https://github.com/cryptodev-linux/cryptodev-linux

---

## 二、编译环境说明

1. 在project目录下根据板子（nand/nor，ddr型号等）选择deconfig进行整包编译；

    例如`SSC032A-S01A-S`型号板子，使用nand，qfn128的配置,使用以下deconfig,其他板子型号详细参考用户手册

    `ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig`

    在project目录执行以下命令:

    `make ipc_ifado.spinand.uclibc-9.1.0-ubifs.ssc032a.128.qfn128_defconfig;`

2. 在kernel目录下执行`make menuconfig`，到`Device Drivers-->SStar SoC platform drivers`中开启`SigmaStar Crypto driver`和`Support cryptodev`，开启并save config之后，为避免在整包编译时被默认配置覆盖，需要手动将`kernel/.config`覆盖至`kernel/arch/arm/configs/ifado_ssc032a_s01a_spinand_defconfig`;
3. 再到project目录执行`make clean && make image -j8`;
4. 进到sample_code目录执行`make clean && make source/ifado/crypto/aes_demo`命令进行编译;
5. 到`sample_code/out/arm/app/aes_demo`获取可执行文件;

---

## 三、运行环境说明

* 确认存在节点：/dev/crypto

* 确认驱动支持的功能：

    ```bash
    / # cat /proc/crypto | grep aes | grep driver
    driver       : ctr-aes-infinity
    driver       : cbc-aes-infinity
    driver       : ecb-aes-infinity
    driver       : infinity-aes
    driver       : aes-generic
    ```

    > 带`infinity`的表示支持硬件加速

---

## 四、运行说明

1. 执行`./prog_crypto_aes_demo`即可

---

## 五、运行结果说明

demo将会打印：

    plaintext_raw（原文）：xxxx

    ciphertext（硬件对原文加密后的数据）：xxxx

    ciphertext_expect（硬件对原文加密后的golden数据）：xxxx

    plaintext(de)（硬件对密文解密后的数据）：xxxx

若`硬件对原文加密后的数据`和`硬件对原文加密后的golden数据`不一致，或者，`原文`和`硬件对密文解密后的数据`不一致，则demo返回非零值。

---