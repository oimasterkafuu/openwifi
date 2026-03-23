@echo off
chcp 65001 >nul
echo ==========================================
echo AutoHotspot Manual Build Script
echo ==========================================
echo.
echo 此脚本使用直接调用 cl.exe 的方式编译
echo 请确保已运行 vcvarsall.bat 设置环境变量
echo.

:: 检查 cl.exe 是否可用
where cl.exe >nul 2>&1
if errorlevel 1 (
    echo [错误] 找不到 cl.exe，请先运行以下命令初始化环境：
    echo.
    echo    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    echo.
    echo 根据你的 VS 版本和安装路径调整上述命令
    pause
    exit /b 1
)

echo [信息] 找到编译器: 
cl.exe 2^>^&1 ^| findstr "Microsoft"
echo.

:: 创建输出目录
if not exist "..\build" mkdir "..\build"

:: 编译程序
echo [信息] 开始编译...
echo.

cl.exe /nologo /W3 /O2 /MT /Fe"..\build\AutoHotspot.exe" main.cpp /link /SUBSYSTEM:WINDOWS /ENTRY:WinMain

if errorlevel 1 (
    echo.
    echo [错误] 编译失败
    pause
    exit /b 1
)

echo.
echo ==========================================
echo [成功] 编译完成！
echo 输出文件: build\AutoHotspot.exe
echo ==========================================
echo.

:: 询问是否运行
echo 是否立即运行程序进行测试？(y/n)
set /p RUN="> "
if /i "%RUN%"=="y" (
    echo.
    echo [信息] 启动程序...
    start "" "..\build\AutoHotspot.exe"
)

pause
