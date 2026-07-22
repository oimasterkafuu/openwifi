@echo off
chcp 65001 >nul
echo ==========================================
echo AutoHotspot Build Script
echo ==========================================
echo.

:: 查找 Visual Studio 的 MSVC 编译器
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo [错误] 未找到 vswhere.exe，请确保安装了 Visual Studio 2017 或更高版本
    pause
    exit /b 1
)

:: 获取 VS 安装路径
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VSINSTALLPATH=%%i"
)

if not defined VSINSTALLPATH (
    echo [错误] 未找到 Visual Studio C++ 工具链
    pause
    exit /b 1
)

echo [信息] 找到 Visual Studio: %VSINSTALLPATH%

:: 调用 vcvarsall.bat 设置环境变量
call "%VSINSTALLPATH%\VC\Auxiliary\Build\vcvarsall.bat" x64

if errorlevel 1 (
    echo [错误] 无法初始化编译环境
    pause
    exit /b 1
)

echo [信息] 编译环境初始化成功
echo.

:: 创建输出目录
if not exist "..\build" mkdir "..\build"

:: 编译程序
echo [信息] 开始编译...
echo.

cl.exe /nologo /W3 /O2 /MT /Fe"..\build\AutoHotspot.exe" *.cpp /link /SUBSYSTEM:WINDOWS /ENTRY:WinMain

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
pause
