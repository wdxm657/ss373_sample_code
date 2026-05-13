DEP += common scl rgn ss_font venc rtsp_video ss_rtsp live555

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_scl))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_venc))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_rgn))
