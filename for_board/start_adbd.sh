#!/bin/sh
USB_DEVICE_DIR=/sys/kernel/config/usb_gadget/s-star/
USB_CONFIGS_DIR=/sys/kernel/config/usb_gadget/s-star/configs/default.1
USB_FUNCTIONS_DIR=/sys/kernel/config/usb_gadget/s-star/functions

config_adb()
{
    #no attributes, all parameters are set through FunctioFS
    mkdir ${USB_FUNCTIONS_DIR}/ffs.adb
    ln -s ${USB_FUNCTIONS_DIR}/ffs.adb ${USB_CONFIGS_DIR}/ffs.adb
}

# main
if [ -d /sys/kernel/config/usb_gadget ]
then
	umount /sys/kernel/config
fi

mount -t configfs none /sys/kernel/config
mkdir $USB_DEVICE_DIR
mkdir $USB_CONFIGS_DIR
mkdir ${USB_DEVICE_DIR}/strings/0x409
mkdir ${USB_CONFIGS_DIR}/strings/0x409

# 配置configs
# MaxPower/bmAttributes
echo 0x02 > ${USB_CONFIGS_DIR}/MaxPower
echo 0xC0 > ${USB_CONFIGS_DIR}/bmAttributes

# 配置strings
# manufacturer/product/serialnumber/configuration
echo "Linux Foundation" > ${USB_DEVICE_DIR}/strings/0x409/manufacturer
echo "ADB gadget" > ${USB_DEVICE_DIR}/strings/0x409/product
echo "0123" > ${USB_DEVICE_DIR}/strings/0x409/serialnumber
echo "ADB" > ${USB_CONFIGS_DIR}/strings/0x409/configuration

# 配置function
config_adb

# 挂载并运行adbd应用
mkdir -p /dev/usb-ffs/adb
mount -o uid=2000,gid=2000 -t functionfs adb /dev/usb-ffs/adb
/customer/sample_code/bin/adbd &
sleep 2

# 配置device
# UDC/bDeviceClass/bDeviceProtocol/bDeviceSubClass/bMaxPacketSize0/bcdDevice/bcdUSB/idProduct/idVendor
echo 0xef > ${USB_DEVICE_DIR}/bDeviceClass
echo 0x01 > ${USB_DEVICE_DIR}/bDeviceProtocol
echo 0x02 > ${USB_DEVICE_DIR}/bDeviceSubClass
echo 0x00 > ${USB_DEVICE_DIR}/bMaxPacketSize0
echo 0x0419 > ${USB_DEVICE_DIR}/bcdDevice
echo 0x0200 > ${USB_DEVICE_DIR}/bcdUSB
echo 0x0102 > ${USB_DEVICE_DIR}/idProduct
echo 0x1d6b > ${USB_DEVICE_DIR}/idVendor
UDC=`ls /sys/class/udc/ | awk '{print $1}'`
echo $UDC > ${USB_DEVICE_DIR}/UDC

