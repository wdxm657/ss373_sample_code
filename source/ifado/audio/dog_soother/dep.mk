DEP += audio common

$(eval $(call CHECKIF_CONFIG_SET,enable,interface_ai))

APP_REL_PREFIX:= bin
