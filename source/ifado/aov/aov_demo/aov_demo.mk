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

INC += $(BUILD_TOP)/internal/common
INC += $(BUILD_TOP)/internal/vif
INC += $(BUILD_TOP)/internal/isp
INC += $(BUILD_TOP)/internal/scl
INC += $(BUILD_TOP)/internal/venc
INC += $(BUILD_TOP)/internal/dla
INC += $(BUILD_TOP)/internal/rgn
INC += $(BUILD_TOP)/internal/aov
INC += $(BUILD_TOP)/internal/audio
INC += $(BUILD_TOP)/internal/vdf
INC += $(BUILD_TOP)/internal/rtsp_video
INC += $(BUILD_TOP)/../common/ss_rtsp
INC += $(BUILD_TOP)/../common/list
INC += $(BUILD_TOP)/../common/ss_font

INC  += ./
SUBDIRS += ./
LIBS += -lmi_common

LIBS += -lsstaralgo_det -lmi_ipu
LIBS += -lmi_sensor
LIBS += -lmi_vif
LIBS += -lmi_isp
LIBS += -lmi_scl
LIBS += -lmi_venc
LIBS += -lcus3a
LIBS += -lispalgo
LIBS += -lmi_rgn
LIBS += -lmi_ai
LIBS += -lmi_ao
LIBS += -lmi_vdf -lmi_shadow -lMD_LINUX -lOD_LINUX -lVG_LINUX -lmi_ive

include $(MODULE_PATH)/kernel/Makefile
