#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

// 注册表路径：开机启动项
#define REGISTRY_RUN_PATH L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define APP_NAME L"AutoHotspot"

// 日志文件路径
std::wstring GetLogPath() {
    wchar_t path[MAX_PATH];
    GetEnvironmentVariableW(L"TEMP", path, MAX_PATH);
    std::wstring logPath = std::wstring(path) + L"\\AutoHotspot.log";
    return logPath;
}

// 写入日志
void WriteLog(const std::wstring& message) {
    std::wstring logPath = GetLogPath();
    
    // 获取当前时间
    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);
    
    wchar_t timeStr[64];
    wcsftime(timeStr, 64, L"%Y-%m-%d %H:%M:%S", &ltm);
    
    // 追加写入日志
    HANDLE hFile = CreateFileW(logPath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, NULL, 
                               OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        std::wstring logLine = std::wstring(timeStr) + L" - " + message + L"\r\n";
        DWORD written;
        WriteFile(hFile, logLine.c_str(), (DWORD)(logLine.length() * sizeof(wchar_t)), &written, NULL);
        CloseHandle(hFile);
    }
}

// 获取程序完整路径
std::wstring GetExecutablePath() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return std::wstring(path);
}

// 检查是否已添加到开机启动
bool IsAlreadyInStartup() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_RUN_PATH, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    wchar_t value[MAX_PATH];
    DWORD valueLength = sizeof(value);
    DWORD type;
    
    bool exists = (RegQueryValueExW(hKey, APP_NAME, NULL, &type, (LPBYTE)value, &valueLength) == ERROR_SUCCESS);
    RegCloseKey(hKey);
    
    return exists;
}

// 添加程序到开机启动
bool AddToStartup() {
    if (IsAlreadyInStartup()) {
        WriteLog(L"Already in startup, skipping registry write");
        return true;
    }

    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_RUN_PATH, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        WriteLog(L"Failed to open registry key");
        return false;
    }

    std::wstring exePath = GetExecutablePath();
    
    // 设置注册表值
    LONG result = RegSetValueExW(hKey, APP_NAME, 0, REG_SZ, 
                                  (const BYTE*)exePath.c_str(), 
                                  (DWORD)((exePath.length() + 1) * sizeof(wchar_t)));
    
    RegCloseKey(hKey);
    
    if (result == ERROR_SUCCESS) {
        WriteLog(L"Successfully added to startup: " + exePath);
        return true;
    } else {
        WriteLog(L"Failed to add to startup, error code: " + std::to_wstring(result));
        return false;
    }
}

// 检查今天是星期几（1=周一，7=周日）
int GetDayOfWeek() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    // wDayOfWeek: 0=周日, 1=周一, ..., 6=周六
    // 转换为: 1=周一, ..., 5=周五, 6=周六, 7=周日
    int day = st.wDayOfWeek;
    if (day == 0) day = 7; // 周日转换为7
    return day;
}

// 获取星期几的名称
std::wstring GetDayName(int day) {
    const wchar_t* days[] = { L"", L"周一", L"周二", L"周三", L"周四", L"周五", L"周六", L"周日" };
    return days[day];
}

// 检查是否是工作日（周一到周五）
bool IsWeekday() {
    int day = GetDayOfWeek();
    return (day >= 1 && day <= 5);
}

// 执行命令并隐藏窗口
bool RunHiddenCommand(const std::wstring& command) {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // 隐藏窗口
    
    // 创建可写的命令字符串
    std::wstring cmd = command;
    
    BOOL success = CreateProcessW(NULL, &cmd[0], NULL, NULL, FALSE, 
                                   CREATE_NO_WINDOW | NORMAL_PRIORITY_CLASS,
                                   NULL, NULL, &si, &pi);
    
    if (!success) {
        WriteLog(L"Failed to create process for command: " + command);
        return false;
    }
    
    // 等待命令完成（最多10秒）
    DWORD waitResult = WaitForSingleObject(pi.hProcess, 10000);
    
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    if (waitResult == WAIT_TIMEOUT) {
        WriteLog(L"Command timed out: " + command);
        return false;
    }
    
    WriteLog(L"Command executed with exit code " + std::to_wstring(exitCode) + L": " + command);
    return (exitCode == 0);
}

// 检查热点是否已配置
bool IsHotspotConfigured() {
    // 检查是否有 SSID 和密码配置
    std::wstring checkCmd = L"cmd.exe /c netsh wlan show hostednetwork setting=security";
    
    // 简化处理：直接尝试执行，如果失败后续会记录
    WriteLog(L"Checking hotspot configuration...");
    return true; // 简化逻辑，直接尝试开启
}

// 开启热点（使用 netsh wlan hostednetwork）
bool EnableHotspotHostedNetwork() {
    WriteLog(L"Attempting to enable hotspot using hostednetwork...");
    
    // 先设置允许承载网络（如果还没设置）
    RunHiddenCommand(L"cmd.exe /c netsh wlan set hostednetwork mode=allow");
    
    // 启动承载网络
    bool result = RunHiddenCommand(L"cmd.exe /c netsh wlan start hostednetwork");
    
    if (result) {
        WriteLog(L"Hotspot enabled successfully via hostednetwork");
    } else {
        WriteLog(L"Failed to enable hotspot via hostednetwork");
    }
    
    return result;
}

// 开启 Windows 10/11 移动热点
bool EnableModernHotspot() {
    WriteLog(L"Attempting to enable Windows 10/11 mobile hotspot...");
    
    // 使用 PowerShell 调用 Windows Runtime API
    std::wstring psCmd = L"powershell.exe -ExecutionPolicy Bypass -Command \"" \
        L"Add-Type -AssemblyName System.Runtime.WindowsRuntime; " \
        L"$null = [Windows.Networking.Connectivity.NetworkInformation,Windows.Networking.Connectivity,ContentType=WindowsRuntime]; " \
        L"$null = [Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager,Windows.Networking.NetworkOperators,ContentType=WindowsRuntime]; " \
        L"$profile = [Windows.Networking.Connectivity.NetworkInformation]::GetInternetConnectionProfile(); " \
        L"$manager = [Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager]::CreateFromConnectionProfile($profile); " \
        L"if ($manager.TetheringOperationalState -eq 'On') { " \
        L"    Write-Host 'Already On'; " \
        L"} else { " \
        L"    $op = $manager.StartTetheringAsync(); " \
        L"    while ($op.Status -eq 0) { Start-Sleep -Milliseconds 100; }; " \
        L"    Write-Host ('Result: ' + $op.Status); " \
        L"}\"";
    
    bool result = RunHiddenCommand(psCmd);
    
    if (result) {
        WriteLog(L"Modern hotspot enabled successfully");
    } else {
        WriteLog(L"Failed to enable modern hotspot");
    }
    
    return result;
}

// 尝试开启热点
bool EnableHotspot() {
    WriteLog(L"=== Starting hotspot enable process ===");
    
    // 首先尝试 Windows 10/11 的现代热点 API
    if (EnableModernHotspot()) {
        return true;
    }
    
    // 如果失败，尝试传统的 hostednetwork 方法
    WriteLog(L"Modern hotspot failed, trying legacy method...");
    if (EnableHotspotHostedNetwork()) {
        return true;
    }
    
    WriteLog(L"All hotspot methods failed");
    return false;
}

// 程序入口点 - 使用 WinMain 而不是 main，这样可以避免控制台窗口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 记录启动信息
    WriteLog(L"=== AutoHotspot started ===");
    WriteLog(L"Executable path: " + GetExecutablePath());
    
    // 1. 首次运行：添加到开机启动
    if (!IsAlreadyInStartup()) {
        WriteLog(L"First run detected, adding to startup...");
        if (AddToStartup()) {
            WriteLog(L"Added to startup successfully");
        } else {
            WriteLog(L"Failed to add to startup");
        }
    } else {
        WriteLog(L"Not first run, already in startup");
    }
    
    // 2. 检查是否是工作日
    int dayOfWeek = GetDayOfWeek();
    std::wstring dayName = GetDayName(dayOfWeek);
    bool isWeekday = IsWeekday();
    
    WriteLog(L"Today is " + dayName + L" (day " + std::to_wstring(dayOfWeek) + L")");
    
    if (!isWeekday) {
        WriteLog(L"Today is weekend, skipping hotspot");
        return 0;
    }
    
    WriteLog(L"Today is weekday, proceeding to enable hotspot");
    
    // 3. 开启热点
    if (EnableHotspot()) {
        WriteLog(L"Hotspot operation completed successfully");
    } else {
        WriteLog(L"Hotspot operation failed - this is normal if hotspot is not configured");
        WriteLog(L"Please configure hotspot manually in Settings first:");
        WriteLog(L"  Settings -> Network & Internet -> Mobile hotspot");
    }
    
    WriteLog(L"=== AutoHotspot finished ===");
    return 0;
}
