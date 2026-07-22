#include "logger.h"

#include <windows.h>
#include <ctime>

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
