# SigmaStar trade secret
# Copyright (c) [2019~2022] SigmaStar Technology.
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

DEP += common ss_uac

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ai))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ao))

FILE_EXIST = $(shell if [ -d $(BUILD_TOP)/source/$(CHIP)/uvc/uvc_device_demo/ ]; then echo "exist"; else echo "noexist"; fi)
ifeq ($(PRODUCT), usbcam)
#need to use $(BUILD_TOP)/source/$(CHIP)/uvc/uvc_device_demo/resource/gadget-uvc-configfs.sh
$(eval $(call CHECKIF_CONFIG_SET,exist,FILE_EXIST))
endif
