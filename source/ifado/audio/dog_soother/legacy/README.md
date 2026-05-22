# legacy

原 YAMNet 独立测试程序 `test_yamnet.c` 保留于此，供算法联调参考。

编译（临时将 `dog_soother.mk` 中 `EXEFILE` 改为 `test_yamnet`，并增加 `SUBDIRS += $(MODULE_PATH)/legacy`，或在本目录单独维护 mk）：

```bash
cd SourceCode/sdk/verify/sample_code
make source/ifado/audio/dog_soother
```

产品固件入口已改为根目录 `main.c` + `app/app_main.c`，目标可执行文件名为 **`dog_soother`**。
