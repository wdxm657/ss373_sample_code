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

.PHONY: module_install module_ext_install

include $(BUILD_TOP)/mi_dep.mk
ifeq ($(KERNEL_MODULE), 1)
ifneq ($(PROJ_ROOT), )
ifneq ($(MODULE_PATH),)
SUBDIRS:=$(MODULE_PATH)
include $(MODULE_PATH)/module.mk
#Change the relative path to the absolute path in $(INC).
INC += $(PROJ_ROOT)/release/include
INC += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/uapi/mstar
INC += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/drivers/sstar/include
endif
endif
else #KERNEL_MODULE=1
LINK_TYPE ?= static
INTER_LINK_TYPE ?= static

ifneq ($(MODULE_PATH),)
EXEFILE:=prog_$(MODULE_NAME)
LIB_NAME:=$(MODULE_NAME)
SUBDIRS:=$(MODULE_PATH)
#$(notdir $(MODULE_PATH)).mk is for app use only
-include $(MODULE_PATH)/$(notdir $(MODULE_PATH)).mk
-include $(MODULE_PATH)/dep.mk
#lib.mk used for internal libs, common libs, app self use libs
-include $(MODULE_PATH)/lib.mk
LIBS := $(foreach m,$(DEP),-l$(m)) $(LIBS)
INC += $(filter $(foreach d,$(DEP),%$(d)), $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*)))
endif

INC += $(BUILD_TOP)/common
INC += $(ALKAID_PROJ)/release/include

# DEBUG config is shared with different toolchain
DEBUG_ASAN:=$(shell if [[ "$(DEBUG)X" != "X"  ]] && ((( $(DEBUG) & 256 )) || (( $(DEBUG) & 1 ))); then echo "1"; fi)
DEBUG_GCOV:=$(shell if [[ "$(DEBUG)X" != "X"  ]] && ((( $(DEBUG) & 512 ))); then echo "1"; fi)
ifeq ($(DEBUG_ASAN), 1)
TARGET_REL_FOLDER := debug
else
TARGET_REL_FOLDER := release
endif
ifeq ($(PRODUCT), android)
ifeq ($(ARCH), arm64)
TARGET_LIB_FOLDER := $(LINK_TYPE)/lib64
else
TARGET_LIB_FOLDER := $(LINK_TYPE)/lib
endif
else
TARGET_LIB_FOLDER := $(LINK_TYPE)
endif
# End of DEBUG config

ifneq ($(PROJ_ROOT), ) # Start of 'PROJ_ROOT'
INC  += $(PROJ_ROOT)/release/include
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/uapi/mstar
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/drivers/sstar/include
LIBS += -L$(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/mi_libs/$(LINK_TYPE)
LIBS += -L$(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/3rd_party_libs/$(TARGET_LIB_FOLDER)
LIBS += -L$(PROJ_ROOT)/release/chip/$(CHIP)/sigma_common_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(TARGET_REL_FOLDER)/$(TARGET_LIB_FOLDER)
LIBS += -lmi_sys -lmi_common
LIBS +=  -lcam_fs_wrapper -lcam_os_wrapper
ifeq ($(DEBUG_GCOV), 1)
LIBS += -lgcov
endif
ifeq ($(DUAL_OS), on)
CODEDEFINE += -DLINUX_FLOW_ON_DUAL_OS
endif
endif # End of PROJ_ROOT

# Add prebuild_libs here because prebuild_libs may use x86 libs.
INC  += $(foreach m,$(PREBUILD_LIBS),$(BUILD_TOP)/../prebuild_libs/$(m)/include/)
LIBS += $(foreach m,$(PREBUILD_LIBS),-L$(BUILD_TOP)/../prebuild_libs/$(m)/$(ARCH)/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(TARGET_REL_FOLDER)/$(LINK_TYPE)/)

ifneq ($(TOOLCHAIN), llvm)
LIBS += -lrt -lpthread -lm -ldl
else
LIBS += -lm -ldl
endif
endif #!KERNEL_MODULE

CODEDEFINE += -DTRANS_BUFFER=480 -DSOCKET_ADDR='"/tmp/cmd_base"'

LIBS += -L$(OUT_PATH)/$(ARCH)/libs/$(INTER_LINK_TYPE)/
MODULE_RELEASE_PACKAGE ?= off
#APP_REL_PREFIX ?= $(MODULE_NAME)

CODEDEFINE += -D_MCU_UART_

ifeq ($(BOARD),032B)
CODEDEFINE += -D_QFN88_BOARD_TYPE_
endif

module_install: module_ext_install
ifneq ($(APP_REL_PREFIX), )
	@mkdir -p $(IMAGE_PATH)/$(APP_REL_PREFIX)/
	@cp -vrf $(OUT_PATH)/$(ARCH)/app/$(EXEFILE) $(IMAGE_PATH)/$(APP_REL_PREFIX)/
ifneq ($(DEBUG_ASAN), 1)
	@$(STRIP) -s $(IMAGE_PATH)/$(APP_REL_PREFIX)/$(EXEFILE)
endif
ifneq ($(MODULE_REL_FILES), )
	$(foreach n,$(MODULE_REL_FILES),cp -rfvd $(n) $(IMAGE_PATH)/$(APP_REL_PREFIX)/;)
endif
ifneq ($(MODULE_REL_LIB), )
	@mkdir -p $(IMAGE_PATH)/$(APP_REL_PREFIX)/lib/
	$(foreach n,$(MODULE_REL_LIB),cp -rfvd $(n) $(IMAGE_PATH)/$(APP_REL_PREFIX)/lib;)
ifneq ($(DEBUG_ASAN), 1)
	@$(STRIP) --strip-unneeded $(IMAGE_PATH)/$(APP_REL_PREFIX)/lib/*
endif
endif
ifneq ($(MODULE_REL_BIN), )
	@mkdir -p $(IMAGE_PATH)/$(APP_REL_PREFIX)/bin/
	$(foreach n,$(MODULE_REL_BIN),cp -rfvd $(n) $(IMAGE_PATH)/$(APP_REL_PREFIX)/bin;)
ifneq ($(DEBUG_ASAN), 1)
	@$(STRIP) --strip-unneeded $(IMAGE_PATH)/$(APP_REL_PREFIX)/bin/*
endif
endif
ifeq ($(MODULE_RELEASE_PACKAGE), on)
	@rm -rfv $(IMAGE_PATH)/$(APP_REL_PREFIX).sqfs
	@mksquashfs $(APP_REL_PREFIX) $(IMAGE_PATH)/$(APP_REL_PREFIX).sqfs -comp xz -all-root
endif
endif

include $(BUILD_TOP)/compile.mk
-include $(MODULE_PATH)/$(notdir $(MODULE_PATH))_post.mk
