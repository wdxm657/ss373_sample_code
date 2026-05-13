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
ifeq ($(SENSOR_SUPPORT_AOV_DUALSNR_RT),y)
CFLAGS+= -DCONFIG_SENSOR_SUPPORT_AOV_DUALSNR_RT
endif

PATH_C +=\
    $(PATH_preload_sample)/src

PATH_H +=\
    $(PATH_mi_interface)/include/common\
    $(PATH_mi_interface)/include/internal/rtos\
    $(PATH_mi_interface)/include/sys\
    $(PATH_include)\
    $(PATH_mi_impl)/common/hal_common\
    $(PATH_mi_impl)/sys/hal_common\
    $(PATH_earlyinit_rtos_api)/pub\
    $(PATH_preload_sample)/pub\
    $(PATH_application_selector)/pub

SRC_C_LIST +=\
    preload_sample_rtos.c \
    preload_sample_rtos_common.c \
    light_sensor.c