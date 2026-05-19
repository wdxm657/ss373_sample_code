DEP += audio

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ao))

APP_REL_PREFIX:= bin
