#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")

#define REGISTRY_RUN_PATH L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define APP_NAME L"AutoHotspot"

std::wstring GetLogPath() {
    wchar_t path[MAX_PATH];
    GetEnvironmentVariableW(L"TEMP", path, MAX_PATH);
    return std::wstring(path) + L"\\AutoHotspot.log";
}

void WriteLog(const std::wstring& message) {
    std::wstring logPath = GetLogPath();

    time_t now = time(0);
    tm ltm;
    localtime_s(&ltm, &now);

    wchar_t timeStr[64];
    wcsftime(timeStr, 64, L"%Y-%m-%d %H:%M:%S", &ltm);

    HANDLE hFile = CreateFileW(logPath.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, NULL,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        std::wstring logLine = std::wstring(timeStr) + L" - " + message + L"\r\n";
        DWORD written;
        WriteFile(hFile, logLine.c_str(), (DWORD)(logLine.length() * sizeof(wchar_t)), &written, NULL);
        CloseHandle(hFile);
    }
}

std::wstring GetExecutablePath() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return std::wstring(path);
}

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

bool AddToStartup() {
    if (IsAlreadyInStartup()) return true;

    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_RUN_PATH, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        WriteLog(L"Failed to open registry");
        return false;
    }

    std::wstring exePath = GetExecutablePath();

    LONG result = RegSetValueExW(hKey, APP_NAME, 0, REG_SZ,
        (const BYTE*)exePath.c_str(),
        (DWORD)((exePath.length() + 1) * sizeof(wchar_t)));

    RegCloseKey(hKey);

    return (result == ERROR_SUCCESS);
}

bool RunHiddenCommand(const std::wstring& command, DWORD& exitCode) {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    std::wstring cmd = command;

    BOOL success = CreateProcessW(NULL, &cmd[0], NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    if (!success) {
        WriteLog(L"CreateProcess failed");
        return false;
    }

    WaitForSingleObject(pi.hProcess, 15000);
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    WriteLog(L"Exit code: " + std::to_wstring(exitCode));

    return true;
}

bool EnableModernHotspot() {
    WriteLog(L"Trying modern hotspot...");

    std::wstring psCmd =
        L"powershell.exe -ExecutionPolicy Bypass -Command \""
        L"Add-Type -AssemblyName System.Runtime.WindowsRuntime; "
        L"$profile = [Windows.Networking.Connectivity.NetworkInformation]::GetInternetConnectionProfile(); "
        L"if ($profile -eq $null) { exit 2 }; "
        L"$manager = [Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager]::CreateFromConnectionProfile($profile); "
        L"if ($manager.TetheringOperationalState -eq 'On') { exit 0 } "
        L"$op = $manager.StartTetheringAsync(); "
        L"while ($op.Status -eq 0) { Start-Sleep -Milliseconds 200 }; "
        L"if ($op.Status -ne 1) { exit 3 } else { exit 0 }\"";

    DWORD exitCode = 0;
    if (!RunHiddenCommand(psCmd, exitCode)) return false;

    return (exitCode == 0);
}

bool EnableHotspotWithRetry() {
    for (int i = 0; i < 5; i++) {
        WriteLog(L"Attempt " + std::to_wstring(i + 1));

        if (EnableModernHotspot()) {
            WriteLog(L"Hotspot enabled successfully");
            return true;
        }

        WriteLog(L"Retrying in 10s...");
        Sleep(10000);
    }

    WriteLog(L"All attempts failed");
    return false;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    WriteLog(L"=== Program Start ===");

    AddToStartup();

    // 等待系统网络初始化
    WriteLog(L"Waiting for system to stabilize...");
    Sleep(30000);

    EnableHotspotWithRetry();

    WriteLog(L"=== Program End ===");
    return 0;
}
