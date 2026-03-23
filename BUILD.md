# AutoHotspot 构建和使用指南

## 项目说明

AutoHotspot 是一个 Windows 程序，具有以下功能：

1. **首次运行自注册**：程序首次运行时会自动将自己添加到开机启动项
2. **开机静默运行**：开机时不显示任何窗口，在后台运行
3. **工作日自动开热点**：仅在周一到周五自动开启设备热点

## 项目结构

```
openwifi/
├── src/
│   ├── main.cpp          # 主程序源代码
│   ├── build.bat         # 自动查找 VS 并编译
│   └── build_manual.bat  # 手动编译脚本
├── build/                # 编译输出目录（自动创建）
└── BUILD.md              # 本文档
```

## 构建要求

- **操作系统**: Windows 10 或 Windows 11
- **开发环境**: Visual Studio 2017 或更高版本（社区版免费）
  - 安装时必须勾选 **"使用 C++ 的桌面开发"** 工作负载
- **权限**: 普通用户权限即可编译，运行时需要管理员权限才能开启热点

## 构建步骤

### 方法一：使用自动构建脚本（推荐）

1. 打开 **x64 Native Tools Command Prompt for VS 2022**（或你的 VS 版本）
   - 在 Windows 开始菜单中搜索 "x64 Native Tools"
   - 以**管理员身份**运行

2. 切换到项目目录：
   ```cmd
   cd C:\path\to\openwifi\src
   ```

3. 运行构建脚本：
   ```cmd
   build.bat
   ```

4. 编译成功后，可执行文件位于 `build\AutoHotspot.exe`

### 方法二：使用手动构建脚本

1. 初始化编译环境（在普通 CMD 或 PowerShell 中）：
   ```cmd
   "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
   ```
   
   > 根据你的 VS 版本和安装路径调整上述命令

2. 切换到项目目录：
   ```cmd
   cd C:\path\to\openwifi\src
   ```

3. 运行手动构建脚本：
   ```cmd
   build_manual.bat
   ```

### 方法三：手动编译

1. 打开 **x64 Native Tools Command Prompt for VS 2022**

2. 切换到 `src` 目录：
   ```cmd
   cd C:\path\to\openwifi\src
   ```

3. 执行编译命令：
   ```cmd
   cl.exe /nologo /W3 /O2 /MT /Fe"..\build\AutoHotspot.exe" main.cpp /link /SUBSYSTEM:WINDOWS /ENTRY:WinMain
   ```

### 方法四：使用 Visual Studio IDE

1. 打开 Visual Studio
2. 选择 **文件** → **新建** → **项目** → **空项目**
3. 将 `main.cpp` 添加到项目中
4. 右键项目 → **属性**：
   - 配置类型：**应用程序 (.exe)**
   - 子系统：**Windows (/SUBSYSTEM:WINDOWS)**
   - 入口点：**WinMain**
5. 按 **Ctrl+Shift+B** 构建

## 使用方法

### 首次运行

1. **右键** `AutoHotspot.exe`，选择 **"以管理员身份运行"**
   - 管理员权限是开启热点所必需的
   - 首次运行会自动添加到开机启动项

2. 程序将在后台静默运行，不显示任何窗口

3. 查看日志确认运行情况：
   ```
   %TEMP%\AutoHotspot.log
   ```

### 配置热点（重要）

**程序需要系统已配置热点才能正常工作！**

#### Windows 10/11 设置方法：

1. 打开 **设置** → **网络和 Internet** → **移动热点**
2. 配置热点：
   - **共享我的 Internet 连接**：选择你的网络连接（Wi-Fi 或以太网）
   - **编辑** → 设置网络名称（SSID）和密码
3. 手动开启一次热点，确认配置正确
4. 关闭热点

### 验证自动启动

1. 按 `Win + R`，输入 `shell:startup`，回车
2. 确认有 `AutoHotspot` 项
3. 或者查看注册表：
   ```
   HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
   ```

### 查看日志

日志文件位置：
```
%TEMP%\AutoHotspot.log
```

示例日志内容：
```
2026-03-23 09:00:00 - === AutoHotspot started ===
2026-03-23 09:00:00 - Executable path: C:\...\AutoHotspot.exe
2026-03-23 09:00:00 - Not first run, already in startup
2026-03-23 09:00:00 - Today is 周一 (day 1)
2026-03-23 09:00:00 - Today is weekday, proceeding to enable hotspot
2026-03-23 09:00:01 - Hotspot enabled successfully
```

## 卸载/移除

### 方法一：使用注册表编辑器
1. 按 `Win + R`，输入 `regedit`，回车
2. 导航到：
   ```
   HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
   ```
3. 删除 `AutoHotspot` 值

### 方法二：使用命令行
```cmd
reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v AutoHotspot /f
```

### 清理日志
```cmd
del %TEMP%\AutoHotspot.log
```

## 故障排除

### 程序无法开启热点

1. **检查日志** `%TEMP%\AutoHotspot.log`
2. **确认热点已配置**：在设置中手动开启一次热点
3. **确认无线网卡支持承载网络**：
   ```cmd
   netsh wlan show drivers
   ```
   查看 "支持的承载网络" 是否为 "是"

4. **Windows 版本限制**：
   - Windows 10/11 家庭版可能不支持某些热点功能
   - 某些网卡驱动可能不支持热点

### 程序没有自动启动

1. 检查注册表是否添加成功
2. 检查杀毒软件是否拦截
3. 确保程序路径不包含特殊字符

### 周末也开启了热点

1. 检查系统日期和时间设置是否正确
2. 查看日志中的 "Today is" 记录

## 技术说明

### 实现原理

1. **自启动**：写入注册表 `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`
2. **隐藏窗口**：使用 `WinMain` 入口点和 `/SUBSYSTEM:WINDOWS`
3. **工作日检测**：使用 Windows API `GetLocalTime()` 获取星期
4. **开启热点**：
   - 首选：Windows Runtime API (`Windows.Networking.NetworkOperators`)
   - 备选：`netsh wlan hostednetwork` 命令

### 安全说明

- 程序仅修改当前用户的注册表（HKCU），不影响其他用户
- 日志文件仅存储在用户的临时目录
- 程序不收集或传输任何数据

## 许可证

本项目仅供学习和个人使用。
