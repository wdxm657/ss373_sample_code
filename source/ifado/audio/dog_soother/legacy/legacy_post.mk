# 链接 dog_soother 模块已编译的 yamnet_wrapper（避免 compile.mk 跨目录源码检查）
YAMNET_WRAPPER_OBJ := $(BUILD_TOP)/source/ifado/audio/dog_soother/yamnet/yamnet_wrapper.user.$(ARCH).o

gen_exe: modules_all $(OBJS)
	@if [ ! -f "$(YAMNET_WRAPPER_OBJ)" ]; then \
		echo "missing $(YAMNET_WRAPPER_OBJ), build dog_soother objects first"; \
		exit 1; \
	fi
	@mkdir -p $(APP_OUT)
	@$(CXX) $(CXXFLAGS) -Wl,--gc-sections $(OBJS) $(YAMNET_WRAPPER_OBJ) $(LIBS) -o $(APP_OUT)/$(EXEFILE)
