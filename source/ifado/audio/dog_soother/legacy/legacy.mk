# SigmaStar trade secret
# legacy/test_yamnet 独立可执行文件（与 dog_soother 分模块链接）

INC += $(BUILD_TOP)/$(MODULE_PATH)/..
INC += $(BUILD_TOP)/$(MODULE_PATH)/../yamnet
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

# yamnet 源文件在上级 dog_soother/yamnet，本模块仅编 test_yamnet.c，链接共用 .o（见 legacy_post.mk）

EXEFILE := test_yamnet
