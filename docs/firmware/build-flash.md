# 编译与烧录

## 编译

```bash
pio run -e xiao_esp32c3
```

## 查找串口

```bash
pio device list
```

## 烧录和监视

```bash
pio run -e xiao_esp32c3 -t upload
pio device monitor -b 115200
```

如果设备没有进入下载模式，按住 BOOT、短按 RESET，然后松开 BOOT 再重试。端口不稳定时，可在对应环境中显式配置 `upload_port` 和 `monitor_port`，但不要把个人机器的端口提交到共享模板。

