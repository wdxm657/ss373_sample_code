#!/bin/sh

USB_DEVICE_DIR=/sys/kernel/config/usb_gadget/s-star
USB_CONFIGS_DIR=/sys/kernel/config/usb_gadget/s-star/configs/default.1
USB_FUNCTIONS_DIR=/sys/kernel/config/usb_gadget/s-star/functions

FUNCTION_ENALBE=0
PREVIEW_ENALBE=0
# stream number
UVC_FUNCTION_ENABLE=0
UAC_FUNCTION_ENABLE=0
RNDIS_FUNCTION_ENABLE=0
HID_FUNCTION_ENABLE=0

#uvc
UVC_STREAM_INDEX=0
UVC_STREAMING_MAX_PACKET_ARRAY="3072,1024,1024"
UVC_STREAMING_MAX_BURST_ARRAY="13,13,13"
UVC_STREAMING_INTERVAL_ARRAY="1,1,1,1,1,1"
UVC_STREAMING_NAME_ARRAY="UVC Camera 0,UVC Camera 1,UVC Camera 2"
UVC_STREAMING_FORMAT_YUV_ENABLE=1
UVC_STREAMING_FORMAT_NV12_ENABLE=1
UVC_STREAMING_FORMAT_MJPEG_ENABLE=1
UVC_STREAMING_FORMAT_H264_ENABLE=1
UVC_STREAMING_FORMAT_H265_ENABLE=1
UVC_STREAMING_FORMAT_AV1_ENABLE=1
UVC_VERSION=0x0110  #uvc1.5: 0x0150  uvc1.1:0x0110
UVC_MODE=0x0        #0x0 :isoc mode  o0x1:bulk mode

#remote_wake_up_des
REMOTE_WAKE_UP_ENABLE=0

USB_DEVICE_PID=0x0102
USB_DEVICE_VID=0x1d6b
MANUFACTURER="Linux Foundation"
PRODUCT="USB GADGET"
SERIAL_NUM="0123"
CONFIGURATION="composite device"
###########################################UVC CONFIG######################################################
# bBitsPerPixel(uncompressed/framebase)/bDefaultFrameIndex/bmaControls/guidFormat(uncompressed/framebase)
config_uvc_format_yuyv()
{
    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv
    echo 0x10 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/bBitsPerPixel
    echo 0x01 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/bDefaultFrameIndex
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/bmaControls
    echo -ne \\x59\\x55\\x59\\x32\\x00\\x00\\x10\\x00\\x80\\x00\\x00\\xaa\\x00\\x38\\x9b\\x71 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/guidFormat
}

config_uvc_format_nv12()
{
    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12
    echo 0x0C > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/bBitsPerPixel
    echo 0x01 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/bDefaultFrameIndex
    echo 0x04 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/bmaControls
    echo -ne \\x4e\\x56\\x31\\x32\\x00\\x00\\x10\\x00\\x80\\x00\\x00\\xaa\\x00\\x38\\x9b\\x71 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/guidFormat
}

config_uvc_format_mjpeg()
{
    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default
    echo 0x01 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/bDefaultFrameIndex
    echo 0x04 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/bmaControls
}

config_uvc_format_h264()
{
    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264
    echo 0x10 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/bBitsPerPixel
    echo 0x01 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/bDefaultFrameIndex
    echo 0x04 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/bmaControls
    echo -ne \\x48\\x32\\x36\\x34\\x00\\x00\\x10\\x00\\x80\\x00\\x00\\xaa\\x00\\x38\\x9b\\x71 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/guidFormat
}

config_uvc_format_h265()
{
    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265
    echo 0x10 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/bBitsPerPixel
    echo 0x01 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/bDefaultFrameIndex
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/bmaControls
    echo -ne \\x48\\x32\\x36\\x35\\x00\\x00\\x10\\x00\\x80\\x00\\x00\\xaa\\x00\\x38\\x9b\\x71 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/guidFormat
}

config_uvc_format_av1()
{
    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1
    echo 0x10 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/bBitsPerPixel
    echo 0x01 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/bDefaultFrameIndex
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/bmaControls
    echo -ne \\x41\\x56\\x30\\x31\\x00\\x00\\x10\\x00\\x80\\x00\\x00\\xaa\\x00\\x38\\x9b\\x71 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/guidFormat
}

# bmCapabilities/dwDefaultFrameInterval/dwFrameInterval/dwMaxBitRate/dwMaxVideoFrameBufferSize(uncompressed/mjpeg)/dwMinBitRate/wHeight/wWidth
config_uvc_frame_yuyv()
{
    UVC_FRAME_WIDTH=$1
    UVC_FRAME_HEIGHT=$2
    UVC_FRAME_INTERVAL=$3

    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/bmCapabilities
    echo $UVC_FRAME_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/dwDefaultFrameInterval
    echo -e "333333\n666666\n1000000" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/dwFrameInterval
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*30)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*2)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/dwMaxVideoFrameBufferSize
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/dwMinBitRate
    echo $UVC_FRAME_HEIGHT > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/wHeight
    echo $UVC_FRAME_WIDTH > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv/${UVC_FRAME_HEIGHT}p/wWidth
}

config_uvc_frame_nv12()
{
    UVC_FRAME_WIDTH=$1
    UVC_FRAME_HEIGHT=$2
    UVC_FRAME_INTERVAL=$3

    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/bmCapabilities
    echo $UVC_FRAME_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/dwDefaultFrameInterval
    echo -e "333333\n666666\n1000000" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/dwFrameInterval
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*12*30)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    echo | awk "{print $UVC_FRAME_WIDTH*$UVC_FRAME_HEIGHT*3/2}" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/dwMaxVideoFrameBufferSize
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*12*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/dwMinBitRate
    echo $UVC_FRAME_HEIGHT > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/wHeight
    echo $UVC_FRAME_WIDTH > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12/${UVC_FRAME_HEIGHT}p/wWidth
}

config_uvc_frame_mjpeg()
{
    UVC_FRAME_WIDTH=$1
    UVC_FRAME_HEIGHT=$2
    UVC_FRAME_INTERVAL=$3

    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/bmCapabilities
    echo $UVC_FRAME_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/dwDefaultFrameInterval
    echo -e "333333\n666666\n1000000" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/dwFrameInterval
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*30)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*2)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/dwMaxVideoFrameBufferSize
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/dwMinBitRate
    echo $UVC_FRAME_HEIGHT > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/wHeight
    echo $UVC_FRAME_WIDTH > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default/${UVC_FRAME_HEIGHT}p/wWidth
}

config_uvc_frame_h264()
{
    UVC_FRAME_WIDTH=$1
    UVC_FRAME_HEIGHT=$2
    UVC_FRAME_INTERVAL=$3

    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/bmCapabilities
    echo $UVC_FRAME_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/dwDefaultFrameInterval
    echo -e "333333\n666666\n1000000" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/dwFrameInterval
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*30)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/dwMinBitRate
    echo $UVC_FRAME_HEIGHT > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/wHeight
    echo $UVC_FRAME_WIDTH > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264/${UVC_FRAME_HEIGHT}p/wWidth
}

config_uvc_frame_h265()
{
    UVC_FRAME_WIDTH=$1
    UVC_FRAME_HEIGHT=$2
    UVC_FRAME_INTERVAL=$3

    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/bmCapabilities
    echo $UVC_FRAME_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/dwDefaultFrameInterval
    echo -e "333333\n666666\n1000000" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/dwFrameInterval
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*30)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/dwMinBitRate
    echo $UVC_FRAME_HEIGHT > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/wHeight
    echo $UVC_FRAME_WIDTH > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265/${UVC_FRAME_HEIGHT}p/wWidth
}

config_uvc_frame_av1()
{
    UVC_FRAME_WIDTH=$1
    UVC_FRAME_HEIGHT=$2
    UVC_FRAME_INTERVAL=$3

    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/${UVC_FRAME_HEIGHT}p
    echo 0x00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/${UVC_FRAME_HEIGHT}p/bmCapabilities
    echo $UVC_FRAME_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/${UVC_FRAME_HEIGHT}p/dwDefaultFrameInterval
    echo -e "333333\n666666\n1000000" > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/${UVC_FRAME_HEIGHT}p/dwFrameInterval
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*30)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/${UVC_FRAME_HEIGHT}p/dwMaxBitRate
    echo $((UVC_FRAME_WIDTH*UVC_FRAME_HEIGHT*16*10)) > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/${UVC_FRAME_HEIGHT}p/dwMinBitRate
    echo $UVC_FRAME_HEIGHT > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/${UVC_FRAME_HEIGHT}p/wHeight
    echo $UVC_FRAME_WIDTH > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1/${UVC_FRAME_HEIGHT}p/wWidth
}

config_uvc_yuv()
{
    config_uvc_format_yuyv
    if [ $PREVIEW_ENALBE -eq 0 ] ; then
        config_uvc_frame_yuyv 320 240 333333
        config_uvc_frame_yuyv 640 480 333333
        config_uvc_frame_yuyv 1280 720 333333
        config_uvc_frame_yuyv 1920 1080 333333
        config_uvc_frame_yuyv 2560 1440 333333
        config_uvc_frame_yuyv 2592 1944 333333
        config_uvc_frame_yuyv 3840 2160 666666
    else
        if [ $UVC_STREAM_INDEX -eq 0 ] ; then
            config_uvc_frame_yuyv 320 240 333333
            config_uvc_frame_yuyv 640 480 333333
            config_uvc_frame_yuyv 1280 720 333333
            config_uvc_frame_yuyv 1920 1080 333333
            config_uvc_frame_yuyv 2560 1440 333333
            config_uvc_frame_yuyv 2592 1944 333333
            config_uvc_frame_yuyv 3840 2160 666666
        else
            config_uvc_frame_yuyv 320 240 333333
        fi
    fi

    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/yuyv ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/yuyv
}

config_uvc_nv12()
{
    config_uvc_format_nv12

    if [ $PREVIEW_ENALBE -eq 0 ] ; then
        config_uvc_frame_nv12 320 240 333333
        config_uvc_frame_nv12 640 480 333333
        config_uvc_frame_nv12 1280 720 333333
        config_uvc_frame_nv12 1920 1080 333333
        config_uvc_frame_nv12 2560 1440 333333
        config_uvc_frame_nv12 2592 1944 333333
        config_uvc_frame_nv12 3840 2160 666666
    else
        if [ $UVC_STREAM_INDEX -eq 0 ] ; then
            config_uvc_frame_nv12 320 240 333333
            config_uvc_frame_nv12 640 480 333333
            config_uvc_frame_nv12 1280 720 333333
            config_uvc_frame_nv12 1920 1080 333333
            config_uvc_frame_nv12 2560 1440 333333
            config_uvc_frame_nv12 2592 1944 333333
            config_uvc_frame_nv12 3840 2160 666666
        else
            config_uvc_frame_nv12 320 240 333333
        fi
    fi

    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/uncompressed/nv12 ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/nv12
}

config_uvc_mjpeg()
{
    config_uvc_format_mjpeg
    if [ $PREVIEW_ENALBE -eq 0 ] ; then
        config_uvc_frame_mjpeg 320 240 333333
        config_uvc_frame_mjpeg 640 480 333333
        config_uvc_frame_mjpeg 1280 720 333333
        config_uvc_frame_mjpeg 1920 1080 333333
        config_uvc_frame_mjpeg 2560 1440 333333
        config_uvc_frame_mjpeg 2592 1944 333333
        config_uvc_frame_mjpeg 3840 2160 333333
    else
        if [ $UVC_STREAM_INDEX -eq 0 ] ; then
            config_uvc_frame_mjpeg 320 240 333333
            config_uvc_frame_mjpeg 640 480 333333
            config_uvc_frame_mjpeg 1280 720 333333
            config_uvc_frame_mjpeg 1920 1080 333333
            config_uvc_frame_mjpeg 2560 1440 333333
            config_uvc_frame_mjpeg 2592 1944 333333
            config_uvc_frame_mjpeg 3840 2160 333333
        else
            config_uvc_frame_mjpeg 320 240 333333
        fi
    fi

    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/mjpeg/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/mjpeg
}

config_uvc_h264()
{
    config_uvc_format_h264
    if [ $PREVIEW_ENALBE -eq 0 ] ; then
        config_uvc_frame_h264 320 240 333333
        config_uvc_frame_h264 640 480 333333
        config_uvc_frame_h264 1280 720 333333
        config_uvc_frame_h264 1920 1080 333333
        config_uvc_frame_h264 2560 1440 333333
        config_uvc_frame_h264 2592 1944 333333
        config_uvc_frame_h264 3840 2160 333333
    else
        if [ $UVC_STREAM_INDEX -eq 0 ] ; then
            config_uvc_frame_h264 320 240 333333
            config_uvc_frame_h264 640 480 333333
            config_uvc_frame_h264 1280 720 333333
            config_uvc_frame_h264 1920 1080 333333
            config_uvc_frame_h264 2560 1440 333333
            config_uvc_frame_h264 2592 1944 333333
            config_uvc_frame_h264 3840 2160 333333
        else
            config_uvc_frame_h264 320 240 333333
        fi
    fi

    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h264 ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/h264
}

config_uvc_h265()
{
    config_uvc_format_h265
    if [ $PREVIEW_ENALBE -eq 0 ] ; then
        config_uvc_frame_h265 320 240 333333
        config_uvc_frame_h265 640 480 333333
        config_uvc_frame_h265 1280 720 333333
        config_uvc_frame_h265 1920 1080 333333
        config_uvc_frame_h265 2560 1440 333333
        config_uvc_frame_h265 2592 1944 333333
        config_uvc_frame_h265 3840 2160 333333
    else
        if [ $UVC_STREAM_INDEX -eq 0 ] ; then
            config_uvc_frame_h265 320 240 333333
            config_uvc_frame_h265 640 480 333333
            config_uvc_frame_h265 1280 720 333333
            config_uvc_frame_h265 1920 1080 333333
            config_uvc_frame_h265 2560 1440 333333
            config_uvc_frame_h265 2592 1944 333333
            config_uvc_frame_h265 3840 2160 333333
        else
            config_uvc_frame_h265 320 240 333333
        fi
    fi

    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/h265 ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/h265
}

config_uvc_av1()
{
    config_uvc_format_av1
    if [ $PREVIEW_ENALBE -eq 0 ] ; then
        config_uvc_frame_av1 320 240 333333
        config_uvc_frame_av1 640 480 333333
        config_uvc_frame_av1 1280 720 333333
        config_uvc_frame_av1 1920 1080 333333
        config_uvc_frame_av1 2560 1440 333333
        config_uvc_frame_av1 2592 1944 333333
        config_uvc_frame_av1 3840 2160 333333
    else
        if [ $UVC_STREAM_INDEX -eq 0 ] ; then
            config_uvc_frame_av1 320 240 333333
            config_uvc_frame_av1 640 480 333333
            config_uvc_frame_av1 1280 720 333333
            config_uvc_frame_av1 1920 1080 333333
            config_uvc_frame_av1 2560 1440 333333
            config_uvc_frame_av1 2592 1944 333333
            config_uvc_frame_av1 3840 2160 333333
        else
            config_uvc_frame_av1 320 240 333333
        fi
    fi

    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/framebase/av1 ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default/av1
}

config_uvc()
{
    UVC_STREAMING_MAX_PACKET=`echo $UVC_STREAMING_MAX_PACKET_ARRAY | awk -F ',' '{print $'$((UVC_STREAM_INDEX+1))'}'`
    UVC_STREAMING_MAX_BURST=`echo $UVC_STREAMING_MAX_BURST_ARRAY | awk -F ',' '{print $'$((UVC_STREAM_INDEX+1))'}'`
    UVC_STREAMING_INTERVAL=`echo $UVC_STREAMING_INTERVAL_ARRAY | awk -F ',' '{print $'$((UVC_STREAM_INDEX+1))'}'`
    UVC_STREAMING_NAME=`echo $UVC_STREAMING_NAME_ARRAY | awk -F ',' '{print $'$((UVC_STREAM_INDEX+1))'}'`

    # 配置speed/name
    # streaming_maxpacket/streaming_maxburst/streaming_interval/bulk_streaming_ep
    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}
    echo $UVC_STREAMING_MAX_PACKET > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming_maxpacket
    echo $UVC_STREAMING_MAX_BURST > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming_maxburst
    echo $UVC_STREAMING_INTERVAL > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming_interval
    echo -n $UVC_STREAMING_NAME > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming_name
    echo $UVC_MODE > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/bulk_streaming_ep

    # 配置control
    # bcdUVC/dwClockFrequency
    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/header/default
    echo $UVC_VERSION > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/header/default/bcdUVC
    echo 0x02DC6C00 > ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/header/default/dwClockFrequency
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/header/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/class/fs/default
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/header/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/control/class/ss/default

    # 配置streaming
    mkdir ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default

    if [ $UVC_STREAMING_FORMAT_YUV_ENABLE -gt 0 ]; then
        config_uvc_yuv
    fi
    if [ $UVC_STREAMING_FORMAT_MJPEG_ENABLE -gt 0 ]; then
        config_uvc_mjpeg
    fi
    if [ $UVC_STREAMING_FORMAT_NV12_ENABLE -gt 0 ]; then
        config_uvc_nv12
    fi
    if [ $UVC_STREAMING_FORMAT_H264_ENABLE -gt 0 ]; then
        config_uvc_h264
    fi
    if [ $UVC_STREAMING_FORMAT_H265_ENABLE -gt 0 ]; then
        config_uvc_h265
    fi

    if [ $UVC_STREAMING_FORMAT_AV1_ENABLE -gt 0 ]; then
        config_uvc_av1
    fi

    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/class/fs/default
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/class/hs/default
    ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/header/default ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX}/streaming/class/ss/default
}

###########################################RNDIS CONFIG####################################################
config_rndis()
{
    # dev_addr/host_addr/qmult/class/subclass/protocol
    mkdir ${USB_FUNCTIONS_DIR}/rndis.instance0
}

config_uac()
{
    # audio_play_mode/req_number
    # c_srate/c_chmask/c_ssize/c_mpsize
    # p_srate/p_chmask/p_ssize/p_mpsize
    mkdir ${USB_FUNCTIONS_DIR}/uac1.instance0
    echo 0x03  > ${USB_FUNCTIONS_DIR}/uac1.instance0/req_number # speaker is 0x01, mic is 0x02, mic&&speaker is 0x03
    echo 0x01  > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_chmask # capture channel mask, 1chn is 0x01, 2chn is 0x03
    echo 48000 > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_srate # capture samplrate
    echo 0x02  > ${USB_FUNCTIONS_DIR}/uac1.instance0/c_ssize # capture sample size, 16bit is 0x02, only 16bit can set
    echo 0x01  > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_chmask # playback channel mask
    echo 48000 > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_srate # playback samplrate
    echo 0x02  > ${USB_FUNCTIONS_DIR}/uac1.instance0/p_ssize # playback sample size
}

###########################################HID CONFIG######################################################
config_hid()
{
    # protocol/report_desc/report_length/subclass
    case $HID_FUNCTION_ENABLE in
    1)
        mkdir ${USB_FUNCTIONS_DIR}/hid.instance0
        echo 0x01 > ${USB_FUNCTIONS_DIR}/hid.instance0/protocol
        echo -ne \\x05\\x01\\x09\\x06\\xa1\\x01\\x05\\x07\\x19\\xe0\\x29\\xe7\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x08\\x81\\x02\\x95\\x01\\x75\\x08\\x81\\x03\\x95\\x05\\x75\\x01\\x05\\x08\\x19\\x01\\x29\\x05\\x91\\x02\\x95\\x01\\x75\\x03\\x91\\x03\\x95\\x06\\x75\\x08\\x15\\x00\\x25\\x65\\x05\\x07\\x19\\x00\\x29\\x65\\x81\\x00\\xc0 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_desc
        echo 0x3F > ${USB_FUNCTIONS_DIR}/hid.instance0/report_length
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/subclass
        ;;
    2)
        mkdir ${USB_FUNCTIONS_DIR}/hid.instance0
        echo 0x02 > ${USB_FUNCTIONS_DIR}/hid.instance0/protocol
        echo -ne \\x05\\x01\\x09\\x02\\xa1\\x01\\x09\\x01\\xa1\\x00\\x05\\x09\\x19\\x01\\x29\\x03\\x15\\x00\\x25\\x01\\x95\\x08\\x75\\x01\\x81\\x02\\x05\\x01\\x09\\x30\\x09\\x31\\x09\\x38\\x15\\x81\\x25\\x7f\\x75\\x08\\x95\\x03\\x81\\x06\\xc0\\xc0 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_desc
        echo 0x2E > ${USB_FUNCTIONS_DIR}/hid.instance0/report_length
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/subclass
        ;;
    3)
        mkdir ${USB_FUNCTIONS_DIR}/hid.instance0
        echo 0x01 > ${USB_FUNCTIONS_DIR}/hid.instance0/protocol
        echo -ne \\x05\\x01\\x09\\x06\\xa1\\x01\\x05\\x07\\x19\\xe0\\x29\\xe7\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x08\\x81\\x02\\x95\\x01\\x75\\x08\\x81\\x03\\x95\\x05\\x75\\x01\\x05\\x08\\x19\\x01\\x29\\x05\\x91\\x02\\x95\\x01\\x75\\x03\\x91\\x03\\x95\\x06\\x75\\x08\\x15\\x00\\x25\\x65\\x05\\x07\\x19\\x00\\x29\\x65\\x81\\x00\\xc0 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_desc
        echo 0x3F > ${USB_FUNCTIONS_DIR}/hid.instance0/report_length
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/subclass

        mkdir ${USB_FUNCTIONS_DIR}/hid.instance1
        echo 0x02 > ${USB_FUNCTIONS_DIR}/hid.instance1/protocol
        echo -ne \\x05\\x01\\x09\\x02\\xa1\\x01\\x09\\x01\\xa1\\x00\\x05\\x09\\x19\\x01\\x29\\x03\\x15\\x00\\x25\\x01\\x95\\x08\\x75\\x01\\x81\\x02\\x05\\x01\\x09\\x30\\x09\\x31\\x09\\x38\\x15\\x81\\x25\\x7f\\x75\\x08\\x95\\x03\\x81\\x06\\xc0\\xc0 > ${USB_FUNCTIONS_DIR}/hid.instance1/report_desc
        echo 0x2E > ${USB_FUNCTIONS_DIR}/hid.instance1/report_length
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance1/subclass
        ;;
    4)
        # Demo Kit
        mkdir ${USB_FUNCTIONS_DIR}/hid.instance0
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/protocol
        echo -ne   \\x06\\x00\\xFF\\x09\\x01\\xa1\\x01\\x09\\x03\\x15\\x00\\x26\\x00\\xFF\\x75\\x08\\x96\\x00\\x04\\x81\\x02\\x09\\x04\\x15\\x00\\x26\\x00\\xFF\\x75\\x08\\x96\\x00\\x04\\x91\\x02\\xc0 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_desc
        echo 0x400 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_length
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/subclass
        ;;
    5)
        # HID Sensor
        mkdir ${USB_FUNCTIONS_DIR}/hid.instance0
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/protocol
        echo -ne \\x05\\x20\\x09\\x01\\xA1\\x01\\x85\\x01\\x05\\x20\\x09\\x11\\xA1\\x00\\x05\\x20\\x0A\\x09\\x03\\x15\\x00\\x25\\x02\\x75\\x08\\x95\\x01\\xA1\\x02\\x0A\\x30\\x08\\x0A\\x31\\x08\\x0A\\x32\\x08\\xB1\\x00\\xC0\\x0A\\x1F\\x03\\x15\\x00\\x25\\x01\\x75\\x01\\x95\\x01\\xA1\\x02\\x0A\\xC0\\x09\\xB1\\x03\\xC0\\x75\\x07\\x95\\x01\\xB1\\x03\\x0A\\x16\\x03\\x15\\x00\\x25\\x05\\x75\\x08\\x95\\x01\\xA1\\x02\\x0A\\x40\\x08\\x0A\\x41\\x08\\x0A\\x42\\x08\\x0A\\x43\\x08\\x0A\\x44\\x08\\x0A\\x44\\x08\\xB1\\x00\\xC0\\x0A\\x19\\x03\\x15\\x00\\x25\\x05\\x75\\x08\\x95\\x01\\xA1\\x02\\x0A\\x50\\x08\\x0A\\x51\\x08\\x0A\\x52\\x08\\x0A\\x53\\x08\\x0A\\x54\\x08\\x0A\\x55\\x08\\xB1\\x00\\xC0\\x0A\\x01\\x02\\x15\\x00\\x25\\x06\\x75\\x08\\x95\\x01\\xA1\\x02\\x0A\\x00\\x08\\x0A\\x01\\x08\\x0A\\x02\\x08\\x0A\\x03\\x08\\x0A\\x04\\x08\\x0A\\x05\\x08\\x0A\\x06\\x08\\xB1\\x00\\xC0\\x0A\\x0E\\x03\\x15\\x00\\x27\\xFF\\xFF\\xFF\\x7F\\x75\\x20\\x95\\x01\\x55\\x00\\xB1\\x02\\x0A\\x04\\x03\\x15\\x00\\x27\\xFF\\xFF\\xFF\\x7F\\x75\\x20\\x95\\x01\\x55\\x00\\xB1\\x02\\x0A\\xB1\\x24\\x16\\x01\\x80\\x26\\xFF\\x7F\\x75\\x10\\x95\\x01\\x55\\x0D\\xB1\\x02\\x0A\\xB1\\x34\\x16\\x01\\x80\\x26\\xFF\\x7F\\x75\\x10\\x95\\x01\\x55\\x0D\\xB1\\x02\\x0A\\xB2\\x14\\x16\\x01\\x80\\x26\\xFF\\x7F\\x75\\x10\\x95\\x01\\x55\\x00\\xB1\\x02\\x0A\\xB1\\x74\\x16\\x01\\x80\\x26\\xFF\\x7F\\x75\\x10\\x95\\x01\\x55\\xFD\\xB1\\x02\\x05\\x20\\x0A\\x01\\x02\\x15\\x00\\x25\\x06\\x75\\x08\\x95\\x01\\xA1\\x02\\x0A\\x00\\x08\\x0A\\x01\\x08\\x0A\\x02\\x08\\x0A\\x03\\x08\\x0A\\x04\\x08\\x0A\\x05\\x08\\x0A\\x06\\x08\\x81\\x02\\xC0\\x0A\\x02\\x02\\x15\\x00\\x25\\x05\\x75\\x08\\x95\\x01\\xA1\\x02\\x0A\\x10\\x08\\x0A\\x11\\x08\\x0A\\x12\\x08\\x0A\\x13\\x08\\x0A\\x14\\x08\\x0A\\x15\\x08\\x81\\x02\\xC0\\x0A\\xB1\\x04\\x15\\x00\\x25\\x01\\x75\\x08\\x95\\x01\\x55\\x00\\x81\\x02\\x0A\\xB2\\x04\\x16\\x00\\x00\\x26\\xFF\\x7F\\x75\\x10\\x95\\x01\\x55\\x0D\\x81\\x02\\xC0\\xC0 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_desc
        echo 0x16 > ${USB_FUNCTIONS_DIR}/hid.instance0/report_length
        echo 0x00 > ${USB_FUNCTIONS_DIR}/hid.instance0/subclass
        echo 0x01 > ${USB_FUNCTIONS_DIR}/hid.instance0/no_out_endpoint
        ;;
    esac
}

config_clear()
{
    if [ ! -d $USB_DEVICE_DIR ];then
        return
    fi

    echo "" > ${USB_DEVICE_DIR}/UDC

    for i in `ls ${USB_CONFIGS_DIR}/ | grep ".instance"`
    do
        rm ${USB_CONFIGS_DIR}/$i
    done

    if [ -d $USB_CONFIGS_DIR/strings/0x409 ]; then
        rmdir  $USB_CONFIGS_DIR/strings/0x409
    fi
    if [ -d $USB_CONFIGS_DIR ]; then
        rmdir  $USB_CONFIGS_DIR
    fi

    for i in `ls $USB_FUNCTIONS_DIR | grep .instance`; do
        if [ -n `echo $i | grep uvc` ]; then
            rm -f $USB_FUNCTIONS_DIR/$i/control/class/fs/default
            rm -f $USB_FUNCTIONS_DIR/$i/control/class/ss/default
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/class/fs/default
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/class/hs/default
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/class/ss/default
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/header/default/yuyv
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/header/default/nv12
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/header/default/mjpeg
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/header/default/h264
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/header/default/h265
            rm -f $USB_FUNCTIONS_DIR/$i/streaming/header/default/av1

            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/yuyv ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/yuyv/*p
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/yuyv
            fi
            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/nv12 ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/nv12/*p
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/uncompressed/nv12
            fi
            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/mjpeg/default ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/mjpeg/default/*p
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/mjpeg/default
            fi
            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/framebase/h264 ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/framebase/h264/*p
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/framebase/h264
            fi
            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/framebase/h265 ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/framebase/h265/*p
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/framebase/h265
            fi

            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/framebase/av1 ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/framebase/av1/*p
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/framebase/av1
            fi

            if [ -d $USB_FUNCTIONS_DIR/$i/control/header/default ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/control/header/default
            fi
            if [ -d $USB_FUNCTIONS_DIR/$i/streaming/header/default ]; then
                rmdir $USB_FUNCTIONS_DIR/$i/streaming/header/default
            fi

        fi
        rmdir $USB_FUNCTIONS_DIR/$i
    done

    if [ -d $USB_DEVICE_DIR/strings/0x409 ]; then
        rmdir  $USB_DEVICE_DIR/strings/0x409
    fi
    rmdir $USB_DEVICE_DIR
}

usage()
{
    echo -e "Usage:./gadget-configfs.sh -x NUM\n"
    echo -e "\t -a        UVC"
    echo -e "\t -d        UAC"
    echo -e "\t -b        UVC close H265 and av1 format"
    echo -e "\t -c        add rndis"
    echo -e "\t -e        add HID"
    echo -e "\t -f        add remote wake up"
    echo -e "\t -n        add uvc name, Examples: ./gadget-configfs.sh -n usb1,usb2"
    echo -e "\t -m        uvc mode, Examples: ./gadget-configfs.sh -m 0 ,0 :isoc mode,1:bulk mode"
    echo -e "\t -z        NO CLASS"
    echo -e "\t Examples: ./gadget-configfs.sh -a1        ---> one UVC device"
}

main()
{
    while getopts ":a:b:c:d:e:f:g:h:i:j:n:m:z" opt
    do
        case $opt in
        a)
            UVC_FUNCTION_ENABLE=$OPTARG
            ;;
        b)
            PREVIEW_ENALBE=$OPTARG
            UVC_STREAMING_FORMAT_H265_ENABLE=0
            UVC_STREAMING_FORMAT_AV1_ENABLE=0
            ;;
        c)
            RNDIS_FUNCTION_ENABLE=$OPTARG
            ;;
        d)
            UAC_FUNCTION_ENABLE=$OPTARG
            ;;
        e)
            HID_FUNCTION_ENABLE=$OPTARG
            ;;
        f)
            REMOTE_WAKE_UP_ENABLE=$OPTARG
            ;;
        n)
            UVC_STREAMING_NAME_ARRAY=$OPTARG
            ;;
        m)
            UVC_MODE=$OPTARG
            if [ $UVC_MODE -eq 0 ] ; then
                UVC_MODE=0x0
            else
                UVC_MODE=0x1
            fi
            ;;
        z)
            config_clear
            exit 0
            ;;
        ?)
            usage
            exit 1
            ;;
        esac
    done

    if [ $UVC_FUNCTION_ENABLE -eq 0 ] && [ $UAC_FUNCTION_ENABLE -eq 0 ] && [ $RNDIS_FUNCTION_ENABLE -eq 0 ] && [ $HID_FUNCTION_ENABLE -eq 0 ]; then
        usage
        exit 1
    fi


    config_clear
    echo "UVC:$UVC_FUNCTION_ENABLE"
    echo "UAC:$UAC_FUNCTION_ENABLE"
    echo "RNDIS:$RNDIS_FUNCTION_ENABLE"
    echo "HID:$HID_FUNCTION_ENABLE"
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
    if [ $REMOTE_WAKE_UP_ENABLE -eq 1 ]; then
        echo 0xE0 > ${USB_CONFIGS_DIR}/bmAttributes
    else
        echo 0xC0 > ${USB_CONFIGS_DIR}/bmAttributes
    fi

    # 配置strings
    # manufacturer/product/serialnumber/configuration
    echo "$MANUFACTURER" > ${USB_DEVICE_DIR}/strings/0x409/manufacturer
    echo "$PRODUCT" > ${USB_DEVICE_DIR}/strings/0x409/product
    #echo "$SERIAL_NUM" > ${USB_DEVICE_DIR}/strings/0x409/serialnumber
    echo "$CONFIGURATION" > ${USB_CONFIGS_DIR}/strings/0x409/configuration

    # functions
    if [ $RNDIS_FUNCTION_ENABLE -gt 0 ]; then
        config_rndis
        ln -s ${USB_FUNCTIONS_DIR}/rndis.instance0 ${USB_CONFIGS_DIR}/rndis.instance0
    fi

    # functions
    while [ $UVC_STREAM_INDEX -lt $UVC_FUNCTION_ENABLE ]
    do
        config_uvc
        ln -s ${USB_FUNCTIONS_DIR}/uvc.instance${UVC_STREAM_INDEX} ${USB_CONFIGS_DIR}/uvc.instance${UVC_STREAM_INDEX}
        let UVC_STREAM_INDEX++
    done

    if [ $UAC_FUNCTION_ENABLE -gt 0 ]; then
        config_uac
        ln -s ${USB_FUNCTIONS_DIR}/uac1.instance0 ${USB_CONFIGS_DIR}/uac1.instance0
    fi

    # functions
    case $HID_FUNCTION_ENABLE in
    1)
        config_hid
        ln -s ${USB_FUNCTIONS_DIR}/hid.instance0 ${USB_CONFIGS_DIR}/hid.instance0
        ;;
    2)
        config_hid
        ln -s ${USB_FUNCTIONS_DIR}/hid.instance0 ${USB_CONFIGS_DIR}/hid.instance0
        ;;
    3)
        config_hid
        ln -s ${USB_FUNCTIONS_DIR}/hid.instance0 ${USB_CONFIGS_DIR}/hid.instance0
        ln -s ${USB_FUNCTIONS_DIR}/hid.instance1 ${USB_CONFIGS_DIR}/hid.instance1
        ;;
    4)
        config_hid
        ln -s ${USB_FUNCTIONS_DIR}/hid.instance0 ${USB_CONFIGS_DIR}/hid.instance0
        ;;
    5)
        config_hid
        ln -s ${USB_FUNCTIONS_DIR}/hid.instance0 ${USB_CONFIGS_DIR}/hid.instance0
        ;;
    esac

    # 配置device
    # UDC/bDeviceClass/bDeviceProtocol/bDeviceSubClass/bMaxPacketSize0/bcdDevice/bcdUSB/idProduct/idVendor
    echo 0xef > ${USB_DEVICE_DIR}/bDeviceClass
    echo 0x01 > ${USB_DEVICE_DIR}/bDeviceProtocol
    echo 0x02 > ${USB_DEVICE_DIR}/bDeviceSubClass
    echo 0x00 > ${USB_DEVICE_DIR}/bMaxPacketSize0
    echo 0x0419 > ${USB_DEVICE_DIR}/bcdDevice
    echo 0x0200 > ${USB_DEVICE_DIR}/bcdUSB
    echo $USB_DEVICE_PID > ${USB_DEVICE_DIR}/idProduct
    echo $USB_DEVICE_VID > ${USB_DEVICE_DIR}/idVendor
    UDC=`ls /sys/class/udc/ | head -n 1`
    echo $UDC > ${USB_DEVICE_DIR}/UDC
}


main $@
