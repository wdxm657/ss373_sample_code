INC += $(BUILD_TOP)/internal/common
SUBDIRS += ./

LIBS += -lmi_common -lAEC_LINUX

MODULE_REL_FILES +=  $(BUILD_TOP)/source/$(CHIP)/audio/alg/aec_demo/resource