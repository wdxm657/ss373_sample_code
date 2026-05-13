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

.PHONY : all clean gen_exe gen_obj clean_files gen_lib modules_all modules_clean

GCCFLAGS += -Wall -g -pipe -fPIC
ifeq ($(ARCH),arm)
API_EX_CFLAGS+= -mthumb
endif
ifeq ($(DEBUG_ASAN), 1)
GCCFLAGS += -fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address -funwind-tables
endif
CODEDEFINE += -DLINUX_OS -DBUILD_OWNER=\"$(shell whoami)\" -DBUILD_DATE="\"$(shell date +"%Y-%m-%d %T")\""

#Relative to absoluted path
INC := $(patsubst $(MODULE_PATH)/%,$(BUILD_TOP)/$(MODULE_PATH)/%,$(INC))
SRC += $(sort $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c)) $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.cpp)))
#Relative to absoluted path
SRC := $(patsubst $(MODULE_PATH)/%,$(BUILD_TOP)/$(MODULE_PATH)/%,$(SRC))

CXXFLAGS := $(GCCFLAGS) $(LOCAL_CXXFLAGS)
CXXFLAGS += $(CODEDEFINE) -DLINUX_OS -std=gnu++11
CXXFLAGS += $(foreach dir,$(INC),-I$(dir))
ifneq ($(TOOLCHAIN), llvm)
CXXFLAGS += -Wno-psabi
endif

CFLAGS := $(GCCFLAGS) $(LOCAL_CFLAGS)
CFLAGS += $(CODEDEFINE)
CFLAGS += $(foreach dir,$(INC),-I$(dir))

ifeq ($(TOOLCHAIN), llvm)
CFLAGS += -D_GNU_SOURCE
else
CXXFLAGS += -Wno-psabi
endif

OBJS_CXX  := $(patsubst %.cpp,%.user.$(ARCH).o,$(filter %.cpp, $(SRC)))
OBJS      := $(patsubst %.c,%.user.$(ARCH).o,$(filter %.c, $(SRC)))
OBJS_KRN  := $(foreach file,$(patsubst %.c,%.o,$(filter %.c, $(SRC))),$(patsubst $(BUILD_TOP)/$(MODULE_PATH)/%,%,$(file)))
SRC_CHECK := $(filter-out $(realpath $(MODULE_PATH))%,$(foreach file,$(SRC),$(realpath $(file))))
ifneq ($(SRC_CHECK),)
$(warning files->$(SRC_CHECK))
$(error $(MODULE_NAME) include extern src files, please use submodule or parvate submodule for instead.)
endif

ifeq ($(LIB_TYPE), static)
LIB_SUFFIX := a
endif
ifeq ($(LIB_TYPE), dynamic)
LIB_SUFFIX := so
endif
MODULE_OUT:=$(OUT_PATH)/$(ARCH)/modules
LIB_OUT:= $(OUT_PATH)/$(ARCH)/libs/$(LIB_TYPE)
APP_OUT:= $(OUT_PATH)/$(ARCH)/app

DFILES := $(foreach f,$(OBJS_CXX) $(OBJS),$(patsubst %.o,%.d,$(f)))
sinclude $(DFILES)

ifeq ($(KERNEL_MODULE),1)
ifneq ($(PROJ_ROOT), )
KDIR ?= $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)
KO_NAME ?= $(notdir $(MODULE_PATH))

CFLAGS_KRN += -g -Werror -Wall -Wno-unused-result -pipe
CFLAGS_KRN += $(foreach dir,$(INC),-I$(dir))
CFLAGS_KRN += $(CODEDEFINE)

export CFLAGS_KRN
export KO_NAME
export OBJS_KRN

endif

kernel_module:
ifneq ($(PROJ_ROOT),)
ifneq ($(MODULE_PATH),)
	@ln -sf $(BUILD_TOP)/kmake.mk $(BUILD_TOP)/$(MODULE_PATH)/Makefile
	@$(MAKE) -C $(KDIR) M=$(BUILD_TOP)/$(MODULE_PATH) modules
	@mkdir -p $(MODULE_OUT)
	@cp -fv $(MODULE_PATH)/$(KO_NAME).ko $(MODULE_OUT)
endif
endif

kernel_module_clean:
ifneq ($(PROJ_ROOT),)
ifneq ($(MODULE_PATH),)
ifneq ($(wildcard $(BUILD_TOP)/$(MODULE_PATH)/Makefile),)
	@$(MAKE) -C $(KDIR) M=$(BUILD_TOP)/$(MODULE_PATH) clean
	@rm -fv $(BUILD_TOP)/$(MODULE_PATH)/Makefile
endif
endif
endif
endif

$(OBJS):%.user.$(ARCH).o:%.c
	@echo compile $<...
	@$(CC) $(CFLAGS) -MM $< -MT $@ > $(@:.o=.d)
	@$(CC) $(CFLAGS) -c -ffunction-sections -fdata-sections $< -o $@
$(OBJS_CXX):%.user.$(ARCH).o:%.cpp
	@echo compile $<...
	@$(CC) $(CXXFLAGS) -MM $< -MT $@ > $(@:.o=.d)
	@$(CXX) $(CXXFLAGS) -c -ffunction-sections -fdata-sections $< -o $@

gen_exe: modules_all $(OBJS_CXX) $(OBJS)
ifneq ($(OBJS_CXX), )
	@mkdir -p $(APP_OUT)
	@$(CXX) $(CXXFLAGS) -Wl,--gc-sections $(OBJS_CXX) $(OBJS) $(LIBS) -o $(APP_OUT)/$(EXEFILE)
else
ifneq ($(OBJS), )
	@mkdir -p $(APP_OUT)
	@$(CC) $(CFLAGS) -Wl,--gc-sections $(OBJS) $(LIBS) -o $(APP_OUT)/$(EXEFILE) -lstdc++
endif
endif

gen_obj:$(OBJS_CXX) $(OBJS)

%.a: $(OBJS_CXX) $(OBJS)
	@mkdir -p $(LIB_OUT)
	@$(AR) sqD $@ $(OBJS_CXX) $(OBJS)

%.so:$(OBJS_CXX) $(OBJS)
	@mkdir -p $(LIB_OUT)
	@$(CXX) -shared -fPIC -o $@ $(OBJS_CXX) $(OBJS)

gen_lib: $(LIB_OUT)/lib$(LIB_NAME).$(LIB_SUFFIX)

clean_files: modules_clean
	@rm -rf $(OBJS_CXX) $(OBJS) $(OBJS_CXX:.o=.d) $(OBJS:.o=.d)
ifneq ($(EXEFILE), )
	@rm -rf $(APP_OUT)/$(EXEFILE)
endif
