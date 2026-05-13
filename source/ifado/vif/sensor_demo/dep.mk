DEP += common vif isp scl venc rtsp_video ss_rtsp live555

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_sensor))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_vif))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_isp))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_scl))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_venc))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_cus3a))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ispalgo))
