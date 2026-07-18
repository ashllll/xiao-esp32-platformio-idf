# 调试

常用命令：

```bash
pio device list
pio device monitor -b 115200
pio run -t clean
pio run -v
```

排查顺序：

1. 确认 USB 数据线具备数据功能。
2. 确认 `-e` 指定的 XIAO 型号正确。
3. 查看串口是否出现，以及是否被其他程序占用。
4. 必要时通过 BOOT/RESET 强制进入下载模式。
5. 检查外围电路是否影响启动配置引脚或供电。

