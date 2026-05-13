# sha_demo使用说明

---

## 一、demo场景


1. 对plaintext进行SHA2_256后，与期待的哈希数据进行对比；

2. plaintext内容为plaintext1+plaintext2。对plaintext1进行SHA2_256哈希，再update plaintext2，将结果与plaintext对比；

> sha_demo透过cryptodev中间件，使用ioctl控制/dev/crypto设备。从而能够使用到硬件加速。
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
4. 进到sample_code目录执行`make clean && make source/ifado/crypto/sha_demo`命令进行编译;
5. 到`sample_code/out/arm/app/sha_demo`获取可执行文件;

---

## 三、运行环境说明

* 确认存在节点：/dev/crypto

* 确认驱动支持的功能：

    ```bash
    / # cat /proc/crypto | grep sha | grep driver
    driver       : pkcs1pad(rsa-generic,sha256)
    driver       : sha256-infinity
    driver       : drbg_nopr_hmac_sha256
    driver       : drbg_nopr_hmac_sha512
    driver       : drbg_nopr_hmac_sha384
    driver       : drbg_nopr_hmac_sha1
    driver       : drbg_pr_hmac_sha256
    driver       : drbg_pr_hmac_sha512
    driver       : drbg_pr_hmac_sha384
    driver       : drbg_pr_hmac_sha1
    driver       : sha384-generic
    driver       : sha512-generic
    driver       : sha224-generic
    driver       : sha256-generic
    ```

    > 带`infinity`的表示支持硬件加速

---

## 四、运行说明

1. 执行`./prog_crypto_sha_demo`即可

---

## 五、运行结果说明

demo将会打印：

    The digest（硬件对原文哈希后的数据）：xxxx

    The expected（硬件对原文哈希后的golden数据）：xxxx

若`硬件对原文哈希后的数据`和`硬件对原文哈希后的golden数据`不一致，则demo返回非零值。

---