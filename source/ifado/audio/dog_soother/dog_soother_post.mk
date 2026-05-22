# 与 dog_soother 同次 make 时顺带编译 legacy/test_yamnet（独立模块，避免双 main 链接冲突）
LEGACY_MODULE := source/ifado/audio/dog_soother/legacy

# 先编出 dog_soother 各 .o（含 yamnet），再链接 test_yamnet
.PHONY: build_legacy_exe
build_legacy_exe: modules_all
	@$(MAKE) MODULES_IN=$(LEGACY_MODULE) $(LEGACY_MODULE)_app_all

gen_exe: build_legacy_exe
