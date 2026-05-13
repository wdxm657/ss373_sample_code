# SigmaStar trade secret
# Copyright (c) [2019~2020] SigmaStar Technology.
# All rights reserved.
#
# Unless otherwise stipulated in writing, any and all information contained
# herein regardless in any format shall remain the sole proprietary of
# SigmaStar and be kept in strict confidence
# (SigmaStar Confidential Information) by the recipient.
# Any unauthorized act including without limitation unauthorized disclosure,
# copying, use, reproduction, sale, distribution, modification, disassembling,
# reverse engineering and compiling of the contents of SigmaStar Confidential
# Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
# rights to any and all damages, losses, costs and expenses resulting therefrom.
#

# ifeq ($(BB_CHIP_ID),infinity6f)
#     CHIP := souffle
# else ifeq ($(BB_CHIP_ID),infinity6dw)
#     CHIP := iford
# endif



#-------------------------------------------------------------------------------
# Description of some variables owned by the library
#-------------------------------------------------------------------------------

PATH_C +=\
    $(PATH_usb_gadget_preload)/src

PATH_H +=\
    $(PATH_include)\
    $(PATH_include)/isp\
    $(PATH_include)/ipu\
    $(PATH_include)/isp/$(CHIP)\
    $(PATH_include)/ipu/$(CHIP)\
    $(PATH_usb_gadget_preload)/pub\
    $(PATH_cust_isp)/inc\
    $(PATH_system)MsWrapper/pub \
    $(PATH_system)cam_fs_wrapper/pub \
    $(PATH_system)cam_dev_wrapper/pub \
    $(PATH_system)cam_drv_poll/pub \
    $(PATH_arm)/pub \
    $(PATH_dualos_camera)/pub\
    $(PATH_env_util)/pub\
    $(PATH_kernel_hal)/pub\
    $(PATH_miu)/pub \
    $(PATH_miu_hal)/pub \
    $(PATH_sys)/pub\
    $(PATH_cam_os_wrapper)/pub\
    $(PATH_MsWrapper)/inc\
    $(PATH_memmang)/rtk_heap/inc\
    $(PATH_sensor_early_init)/drv/pub\
    $(PATH_dualos)/pub\
    $(PATH_dualos_hal)/inc \
    $(PATH_initcall)/pub\
    $(PATH_mmupte_hal)/inc\
    $(PATH_mi_interface)/include/ai\
    $(PATH_mi_interface)/include/ao\
    $(PATH_mi_interface)/include/aio\
    $(PATH_mi_interface)/include/common\
    $(PATH_mi_interface)/include/divp\
    $(PATH_mi_interface)/include/internal/rtos\
    $(PATH_mi_interface)/include/ipu\
    $(PATH_mi_interface)/include/ipu/$(CHIP)\
    $(PATH_mi_interface)/include/ive\
    $(PATH_mi_interface)/include/ldc\
    $(PATH_mi_interface)/include/md\
    $(PATH_mi_interface)/include/od\
    $(PATH_mi_interface)/include/rgn\
    $(PATH_mi_interface)/include/scl\
    $(PATH_mi_interface)/include/sed\
    $(PATH_mi_interface)/include/sensor\
    $(PATH_mi_interface)/include/shadow\
    $(PATH_mi_interface)/include/sys\
    $(PATH_mi_interface)/include/vdf\
    $(PATH_mi_interface)/include/vg\
    $(PATH_mi_interface)/include/venc\
    $(PATH_mi_interface)/include/vpe\
    $(PATH_mi_interface)/include/mipitx\
    $(PATH_mi_interface)/include/gfx\
    $(PATH_mi_interface)/include/disp\
    $(PATH_mi_interface)/include/panel\
    $(PATH_mi_interface)/include/vif\
    $(PATH_mi_interface)/include/vdisp\
    $(PATH_mi_interface)/include/isp\
    $(PATH_mi_interface)/include/isp/$(CHIP)\
    $(PATH_mi_interface)/include/cus3a\
    $(PATH_mi_interface)/include/fb\
    $(PATH_mi_impl)/ai\
    $(PATH_mi_impl)/ao\
    $(PATH_mi_impl)/common/hal_common\
    $(PATH_mi_impl)/divp\
    $(PATH_mi_impl)/ipu\
    $(PATH_mi_impl)/ldc\
    $(PATH_mi_impl)/rgn\
    $(PATH_mi_impl)/rgn/hal_common\
    $(PATH_mi_impl)/sensor/mi/inc\
    $(PATH_mi_impl)/sensor/drv/inc\
    $(PATH_mi_impl)/sensor/drv/pub\
    $(PATH_mi_impl)/sensor/hal/chip/$(CHIP)\
    $(PATH_mi_impl)/sensor/hal/pub\
    $(PATH_mi_impl)/shadow\
    $(PATH_mi_impl)/sys\
    $(PATH_mi_impl)/sys/hal_common\
    $(PATH_mi_impl)/venc\
    $(PATH_mi_impl)/venc/hal_common\
    $(PATH_mi_impl)/vpe\
    $(PATH_mi_impl)/mipitx\
    $(PATH_mi_impl)/disp\
    $(PATH_mi_impl)/vif/mi\
    $(PATH_mi_impl)/vif/drv/inc\
    $(PATH_mi_impl)/vif/drv/pub\
    $(PATH_mi_impl)/vif/hal/pub\
    $(PATH_mi_impl)/vif/hal/chip/$(CHIP)\
    $(PATH_mi_impl)/isp\
    $(PATH_mi_impl)/isp/hal_common\
    $(PATH_mi_impl)/isp/hal_common/$(CHIP)\
    $(PATH_mi_impl)/scl\
    $(PATH_earlyinit_rtos_api)/pub\
    $(PATH_application_selector)/pub
ifeq ($(call FIND_COMPILER_OPTION, CONFIG_USB_GADGET_UVC_SUPPORT), TRUE)
PATH_H +=\
    $(PATH_usbhost)/msos/pub\
    $(PATH_flash)/\
    $(PATH_application)/pipeline_demo/common/inc\
    $(PATH_usb_gadget_preload)/pub\
    $(PATH_usb_gadget_app)/class/inc\
    $(PATH_cust_usb_gadget)/core/inc\
    $(PATH_cust_usb_gadget)/class/inc\
    $(PATH_cust_usb_gadget)/dbg/inc
PATH_C +=\
    $(PATH_application)/pipeline_demo/common/src

SRC_C_LIST +=\
    st_common.c\
    uvc_app_cli.c
endif

ifeq ($(call FIND_COMPILER_OPTION, CONFIG_USB_GADGET_UAC_SUPPORT), TRUE)
PATH_H +=\
    $(PATH_usb_gadget_preload)/pub\
    $(PATH_usb_gadget_app)/class/inc\
    $(PATH_cust_usb_gadget)/core/inc\
    $(PATH_cust_usb_gadget)/class/inc\
    $(PATH_cust_usb_gadget)/dbg/inc
SRC_C_LIST +=\
    uac_app.c\
    uac_audio.c\
    pCam_handler_audio.c\
    usb_ac_fu.c\
    uac_unit_app_custom.c
endif

ifeq ($(call FIND_COMPILER_OPTION, CONFIG_USB_GADGET_CDC_SUPPORT), TRUE)
PATH_H +=\
    $(PATH_usb_gadget_preload)/pub\
    $(PATH_usb_gadget_app)/class/inc\
    $(PATH_cust_usb_gadget)/core/inc\
    $(PATH_cust_usb_gadget)/class/inc\
    $(PATH_cust_usb_gadget)/dbg/inc
SRC_C_LIST +=\
    cdc_app.c\
	st_cdc_acm.c
endif


SRC_C_LIST +=\
    usb_gadget_preload.c\
    CameraSetting.c
