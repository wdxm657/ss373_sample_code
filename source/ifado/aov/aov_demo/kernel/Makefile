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


KDIR?=$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)

SRC_FILE:= st_light_misc_control_driver.c light_misc_control_main.c light_misc_control_hw.c light_misc_control_i2c.c

SIGMASTAR_INC_DIR +=        $(KDIR)/include/ \
                            $(KDIR)/drivers/sstar/include/ \
                            $(KDIR)/drivers/sstar/include/$(CHIP)

SIGMASTAR_INC_DIR +=        $(BUILD_TOP)/internal/aov/ \
                            $(PROJ_ROOT)/release/include

APP_DIR:= $(OUT_PATH)/$(ARCH)/app
KONAME:= light_misc_control

MODULE_REL_BIN :=  $(BUILD_TOP)/source/$(CHIP)/aov/aov_demo/kernel/$(KONAME).ko

EXTRA_CFLAGS+= $(foreach n,$(SIGMASTAR_INC_DIR),-I$(n))
EXTRA_CFLAGS+= -DLIGHT_USE_HRTIMER
EXTRA_CFLAGS+= -DLIGHT_SUPPORT_MULTI_VIFDEVICE
# EXTRA_CFLAGS+= -DLIGHT_USE_PM_NOTIFY
# EXTRA_CFLAGS+= -DLIGHT_SUPPORT_LIGHTSENSOR_POWEROFF

obj-m       := $(KONAME).o
light_misc_control-objs+= $(patsubst %.c, %.o, $(SRC_FILE))

$(KONAME).ko:
ifneq ($(findstring aarch64,$(CROSS_COMPILE)),)
	$(MAKE) -f Makefile -C $(KDIR) M=$(BUILD_TOP)/$(MODULE_PATH)/kernel modules $(KBUILD_CONFIGS) ARCH=arm64
else
	$(MAKE) -f Makefile -C $(KDIR) M=$(BUILD_TOP)/$(MODULE_PATH)/kernel modules $(KBUILD_CONFIGS) ARCH=arm
endif

modules_all: $(KONAME).ko
	@mkdir -p $(APP_DIR)
	@cp -vrf $(shell find $(MODULE_PATH)/kernel -name *.ko) $(APP_DIR)/

modules_clean:
ifneq ($(findstring aarch64,$(CROSS_COMPILE)),)
	$(MAKE) -f Makefile -C $(KDIR) M=$(BUILD_TOP)/$(MODULE_PATH)/kernel clean ARCH=arm64
else
	$(MAKE) -f Makefile -C $(KDIR) M=$(BUILD_TOP)/$(MODULE_PATH)/kernel clean ARCH=arm
endif
	@rm -rf $(APP_DIR)/$(KONAME).ko
