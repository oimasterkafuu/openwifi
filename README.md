# AutoHotspot

Windows 自动热点工具 - 工作日自动开启设备热点。

## 功能特性

- ✅ **开机自启动** - 首次运行自动添加到开机启动项
- ✅ **静默运行** - 无窗口后台运行
- ✅ **智能开启** - 仅周一到周五自动开启热点
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
