# AutoHotspot

Windows 自动热点工具 - 工作日自动开启设备热点。

## 功能特性

- ✅ **开机自启动** - 首次运行自动添加到开机启动项
- ✅ **常驻监控** - 热点被系统关闭后自动重新打开
- ✅ **极低 CPU 占用** - 事件驱动 + 定期检查，等待期间 CPU 占用为 0
- ✅ **定时自毁** - 2027-06-20 之后启动时自动清除启动项并删除自身
- ✅ **详细日志** - 记录每次运行状态

## 快速开始

### 1. 构建

在 Windows 上打开 **x64 Native Tools Command Prompt**，然后：

```cmd
cd src
build.bat
```

详细构建说明见 [BUILD.md](BUILD.md)。

### 2. 配置热点

首次使用前，先在 Windows 设置中配置热点：

**设置 → 网络和 Internet → 移动热点**

设置 SSID 和密码，并手动测试一次。

### 3. 运行

右键 `build\AutoHotspot.exe` → **以管理员身份运行**

管理员权限是开启热点所必需的。

## 日志查看

```
%TEMP%\AutoHotspot.log
```

## 卸载

```cmd
reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v AutoHotspot /f
```

## 文档

- [详细构建和使用指南](BUILD.md)
