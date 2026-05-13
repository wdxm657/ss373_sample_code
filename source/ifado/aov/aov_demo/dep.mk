DEP += aov common rgn ss_font vif isp scl venc dla audio uart vdf rtsp_video ss_rtsp live555

$(eval $(call CHECKIF_CONFIG_SET,y,CONFIG_ENABLE_POWER_SAVE_AOV))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ipu))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_sensor))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_vif))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_isp))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_scl))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_venc))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_cus3a))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ispalgo))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_rgn))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ai))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ao))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_vdf))

ifeq ($(CONFIG_ENABLE_POWER_SAVE_AOV),y)
APP_REL_PREFIX:= bin
endif
