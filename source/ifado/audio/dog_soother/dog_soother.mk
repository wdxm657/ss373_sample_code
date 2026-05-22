# SigmaStar trade secret
# Copyright (c) [2019~2022] SigmaStar Technology.

INC += $(BUILD_TOP)/$(MODULE_PATH)
INC += $(BUILD_TOP)/$(MODULE_PATH)/include
INC += $(BUILD_TOP)/$(MODULE_PATH)/app
INC += $(BUILD_TOP)/$(MODULE_PATH)/uart
INC += $(BUILD_TOP)/$(MODULE_PATH)/bark
INC += $(BUILD_TOP)/$(MODULE_PATH)/media
INC += $(BUILD_TOP)/$(MODULE_PATH)/store
INC += $(BUILD_TOP)/$(MODULE_PATH)/sys
INC += $(BUILD_TOP)/$(MODULE_PATH)/yamnet
INC += $(BUILD_TOP)/third_part/ncnn/build-star373/install/include
INC += $(BUILD_TOP)/third_part/ncnn/build-star373/install/include/ncnn
INC += $(BUILD_TOP)/third_part/fftw-3.3.10/build_star373/include
INC += $(BUILD_TOP)/internal/audio
INC += $(BUILD_TOP)/internal/common

LOCAL_CFLAGS += -DENABLE_AI_INPUT

LOCAL_CXXFLAGS += -std=gnu++11

LIBS += -L$(BUILD_TOP)/third_part/ncnn/build-star373/install/lib -lncnn
LIBS += -L$(BUILD_TOP)/third_part/fftw-3.3.10/build_star373/lib -lfftw3f
LIBS += -lm -lpthread -lstdc++
LIBS += -lmi_common -lmi_ai -lmi_sys

# 各子目录仅编译该目录下 *.c（见 sample_code/compile.mk；根目录由 build.mk 的 SUBDIRS:=$(MODULE_PATH) 覆盖）
SUBDIRS += $(MODULE_PATH)/app
SUBDIRS += $(MODULE_PATH)/uart
SUBDIRS += $(MODULE_PATH)/bark
SUBDIRS += $(MODULE_PATH)/media
SUBDIRS += $(MODULE_PATH)/store
SUBDIRS += $(MODULE_PATH)/sys
SUBDIRS += $(MODULE_PATH)/yamnet

EXEFILE := dog_soother
