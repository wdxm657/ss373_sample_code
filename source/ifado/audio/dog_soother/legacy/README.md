# legacy

原 YAMNet 独立测试程序 `test_yamnet.c`，供算法联调。

## 编译

与产品固件同次构建（推荐）：

```bash
cd SourceCode/sdk/verify/sample_code
make source/ifado/audio/dog_soother
```

产物：

- `out/arm/app/dog_soother` — 产品固件
- `out/arm/app/test_yamnet` — 本目录算法测试

仅编译测试程序：

```bash
make source/ifado/audio/dog_soother/legacy
```

产品入口为上级目录 `main.c` + `app/app_main.c`，勿将 `legacy/` 加入 `dog_soother.mk` 的 `SUBDIRS`（会与 `main.c` 产生双 `main` 冲突）。
