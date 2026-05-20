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

BUILD_TOP:=$(shell pwd)
ALKAID_PATH ?= $(BUILD_TOP)/../../..
OUT_PATH:=$(BUILD_TOP)/out
ALKAID_PROJ:=$(ALKAID_PATH)/project
ALKAID_PROJ_CONFIG:=$(ALKAID_PATH)/project/configs/current.configs
3PARTY_PATH:=$(BUILD_TOP)/../3rdparty
IMAGE_PATH := $(BUILD_TOP)/release

define CHECKIF_CONFIG_UNSET
	$(eval CHECK_DEMO := $(filter-out %sample_code, $(subst sample_code/,sample_code , $(dir $(lastword $(MAKEFILE_LIST))))))
	$(eval CHECK_CFG := $(value $(2)))
ifeq ($(CHECK_CFG),$(1))
	FILTER_CHECK+="\033[1;41;43m$(CHECK_DEMO) didn't compiled, because $(2) is set to $(1)!!!\033[0m\n"
	MODULES_FILTER+=$(CHECK_DEMO)
endif
endef

define CHECKIF_CONFIG_SET
	$(eval CHECK_DEMO := $(filter-out %sample_code, $(subst sample_code/,sample_code , $(dir $(lastword $(MAKEFILE_LIST))))))
	$(eval CHECK_CFG := $(value $(2)))
ifneq ($(CHECK_CFG),$(1))
	FILTER_CHECK+="\033[1;41;43m$(CHECK_DEMO) didn't compiled, because $(2) is not set to $(1)!!!\033[0m\n"
	MODULES_FILTER+=$(CHECK_DEMO)
endif
endef

include $(BUILD_TOP)/mi_dep.mk

LIBS_PATH := ../common internal
APP_PATH  := source/$(CHIP)
DEMO_PATH := reserve


DEMO_LIST := audio crypto cv disp dla isp ive ldc rgn scl vdf venc vif uart aov cm4 preload_sample rpmsg
ifeq ($(PRODUCT), usbcam)
DEMO_LIST += uvc uac
endif

ifeq ($(DUAL_OS), on)
DEMO_LIST += preload_linux
DEMO_LIST := $(filter-out nuc, $(DEMO_LIST))
endif
ifeq ($(CM4_DEMO), y)
DEMO_LIST += cm4
endif

DEMO_PATH := $(foreach m,$(DEMO_LIST),$(APP_PATH)/$(m))

MODULES := $(patsubst $(BUILD_TOP)/%/,%,$(foreach m,$(shell find $(foreach n, $(DEMO_PATH), $(BUILD_TOP)/$(n)) -type d),$(dir $(wildcard $(m)/$(notdir $(m)).mk))))
#$(warning MODULES := $(MODULES))

ifneq ($(MODULES_IN),)
-include $(foreach mod,$(MODULES_IN),$(BUILD_TOP)/$(mod)/dep.mk)
else
-include $(foreach mod,$(MODULES),$(BUILD_TOP)/$(mod)/dep.mk)
endif

#Absoluted path to related.
LIBS_PATH := $(patsubst $(BUILD_TOP)/%,%,$(sort $(LIBS_PATH)))
APP_PATH  := $(patsubst $(BUILD_TOP)/%,%,$(sort $(APP_PATH)))

#User self's sub modules has itself's 'mk' files which is wrong that makes the root Makefile to recognize it as APP module.
#So that, filter out the modules by user setting and user self's sub modules in dep.mk.
#MODULES := $(filter-out $(sort $(MODULES_FILTER)) $(shell find $(LIBS_PATH) -type d), $(MODULES))

MODULES_CLEAN := $(foreach m,$(MODULES),$(m)_clean)
MODULES_APP_ALL := $(foreach m,$(MODULES),$(m)_app_all)
MODULES_APP_CLEAN := $(foreach m,$(MODULES),$(m)_app_clean)
MODULES_APP_INSTALL := $(foreach m,$(MODULES),$(m)_install)

#Filter out the files in $(LIBS_PATH)
INTERNAL_ITEMS := $(filter-out $(filter-out %/,$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*/))), $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*)))
INTERNAL_LIBS  := $(filter $(notdir $(INTERNAL_ITEMS)), $(sort $(DEP)))
MODULES_KERNEL := $(patsubst $(BUILD_TOP)/%/,%,$(foreach m,$(shell find $(foreach n, $(LIBS_PATH) $(APP_PATH), $(BUILD_TOP)/$(n)) -type d),$(dir $(wildcard $(m)/module.mk))))
MODULES_KERNEL := $(patsubst ./%,%,$(MODULES_KERNEL))
MODULES_KERNEL := $(sort $(MODULES_KERNEL))

MODULES_KERNEL_ALL := $(foreach m,$(MODULES_KERNEL),$(m)_module_all)
MODULES_KERNEL_CLEAN := $(foreach m,$(MODULES_KERNEL),$(m)_module_clean)
MODULES_OBJS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_all)
MODULES_OBJS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_clean)
MODULES_LIBS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_all)
MODULES_LIBS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_clean)

export PROJ_ROOT CHIP PRODUCT BOARD TOOLCHAIN TOOLCHAIN_VERSION TOOLCHAIN_REL KERNEL_VERSION CUSTOMER_TAILOR CUSTOMER_OPTIONS MOD_PREFIX INTERFACE_ENABLED INTERFACE_DISABLED MHAL_ENABLED MHAL_DISABLED DUAL_OS CHIP_ALIAS MY_TOOLCHAIN
export BUILD_TOP
export OUT_PATH IMAGE_PATH
export ALKAID_PROJ
export ALKAID_PROJ_CONFIG
export TARGET_OUT
export LIBS_PATH
export ARCH CROSS_COMPILE
export CM4_DEMO
.PHONY: all lib obj clean install depend_internal depend_internal_clean $(MODULES) $(MODULES_CLEAN) $(MODULES_APP_ALL) $(MODULES_APP_CLEAN) $(MODULES_OBJS_ALL) $(MODULES_OBJS_CLEAN) $(MODULES_LIBS_ALL) $(MODULES_LIBS_CLEAN)

all:
ifneq ($(MODULES_KERNEL_ALL), )
	@$(MAKE) $(MODULES_KERNEL_ALL)
endif
	@$(MAKE) MODULES_IN="$(MODULES)" depend_internal
ifneq ($(MODULES_APP_ALL), )
	@$(MAKE) $(MODULES_APP_ALL)
endif

clean:
	@$(MAKE) $(foreach mod,$(MODULES),$(patsubst %,%_app_clean,$(mod)))
	@$(MAKE) MODULES_IN="$(MODULES)" depend_internal_clean
ifneq ($(MODULES_KERNEL_CLEAN), )
	@$(MAKE) $(MODULES_KERNEL_CLEAN)
endif
	@rm $(OUT_PATH) -rf
	@rm $(IMAGE_PATH) -rf

install:
	@rm -rvf $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/sample_code
	@mkdir -p $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/sample_code/
	@$(MAKE) $(MODULES_APP_INSTALL)
	@if [ -d $(BUILD_TOP)/for_board ]; then \
		mkdir -p $(IMAGE_PATH)/bin; \
		cp -rvf $(BUILD_TOP)/for_board/* $(IMAGE_PATH)/bin/; \
		find $(IMAGE_PATH)/bin -name '*.sh' -exec chmod 755 {} +; \
	fi;
	@if [ -d $(IMAGE_PATH) ]; then  \
		cp -rvf $(IMAGE_PATH)/* $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/bin/sample_code/;  \
	fi;

depend_internal:
ifeq ($(findstring $(MODULES_IN),$(MODULES_FILTER)),)
ifneq ($(DEP), )
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_obj_all)
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_lib_all)
endif
endif

depend_internal_clean:
ifneq ($(DEP), )
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_obj_clean)
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_lib_clean)
endif

$(MODULES_APP_INSTALL):
	@$(MAKE) MODULE_NAME=$(strip $(subst /,_,$(foreach m,$(APP_PATH),$(patsubst $(m)/%,%,$(filter $(m)/%,$(patsubst %_install,%,$@)))))) MODULE_PATH=$(patsubst %_install,%,$@) -f ./build.mk module_install

$(MODULES):
	@$(MAKE) MODULES_IN=$@ depend_internal
	@$(MAKE) MODULES_IN=$@ $(patsubst %,%_app_all,$@)

$(MODULES_CLEAN):
	@$(MAKE) $(patsubst %_clean,%_app_clean,$@)
	@$(MAKE) MODULES_IN=$(patsubst %_clean,%,$@) depend_internal_clean

app_all:
ifeq ($(FILTER_PATH),)
	@$(MAKE) MODULE_NAME=$(MODULE_NAME) MODULE_PATH=$(MODULE_PATH) -f ./build.mk gen_exe;
else
	@for str in $(FILTER_CHECK);do \
		if [[ $$str == *"$$FILTER_PATH"* ]]; then \
			echo -n -e $$str; \
		fi \
	done
endif

$(MODULES_APP_ALL):
	@$(MAKE) MODULE_NAME=$(strip $(subst /,_,$(foreach m,$(APP_PATH),$(patsubst $(m)/%,%,$(filter $(m)/%,$(patsubst %_app_all,%,$@)))))) MODULE_PATH=$(patsubst %_app_all,%,$@) FILTER_PATH=$(if $(filter $(patsubst %_app_all,%,$@),$(foreach m,$(MODULES_FILTER),$(patsubst %/,%,$(m)))),$(patsubst %_app_all,%,$@),) app_all;

$(MODULES_APP_CLEAN):
	@$(MAKE) MODULE_NAME=$(strip $(subst /,_,$(foreach m,$(APP_PATH),$(patsubst $(m)/%,%,$(filter $(m)/%,$(patsubst %_app_clean,%,$@)))))) MODULE_PATH=$(patsubst %_app_clean,%,$@) -f ./build.mk clean_files

$(MODULES_OBJS_ALL):
	@$(MAKE) MODULE_PATH=$(filter $(foreach m,$(LIBS_PATH),$(m)/$(patsubst %_obj_all,%,$@)),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./build.mk gen_obj

$(MODULES_OBJS_CLEAN):
	@$(MAKE) MODULE_PATH=$(filter $(foreach m,$(LIBS_PATH),$(m)/$(patsubst %_obj_clean,%,$@)),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./build.mk clean_files

$(MODULES_KERNEL_ALL):
	@$(MAKE) MODULE_PATH=$(patsubst %_module_all,%,$@) KERNEL_MODULE=1 -f ./build.mk kernel_module

$(MODULES_KERNEL_CLEAN):
	@$(MAKE) MODULE_PATH=$(patsubst %_module_clean,%,$@) KERNEL_MODULE=1 -f ./build.mk kernel_module_clean

$(MODULES_LIBS_ALL):
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_all,%,$@) MODULE_PATH=$(filter $(foreach m,$(LIBS_PATH),$(m)/$(patsubst %_lib_all,%,$@)),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=static -f ./build.mk gen_lib
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_all,%,$@) MODULE_PATH=$(filter $(foreach m,$(LIBS_PATH),$(m)/$(patsubst %_lib_all,%,$@)),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=dynamic -f ./build.mk gen_lib

$(MODULES_LIBS_CLEAN):
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_clean,%,$@) MODULE_PATH=$(filter $(foreach m,$(LIBS_PATH),$(m)/$(patsubst %_lib_clean,%,$@)),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=static -f ./build.mk clean_files
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_clean,%,$@) MODULE_PATH=$(filter $(foreach m,$(LIBS_PATH),$(m)/$(patsubst %_lib_clean,%,$@)),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=dynamic -f ./build.mk clean_files
