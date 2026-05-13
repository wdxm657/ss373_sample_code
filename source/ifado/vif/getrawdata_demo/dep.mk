DEP += common

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_vif))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_isp))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_cus3a))
$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ispalgo))
