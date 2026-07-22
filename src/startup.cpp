#include "startup.h"
#include "config.h"
#include "logger.h"
#include "util.h"

#include <windows.h>

std::wstring GetStartupPath() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_RUN_PATH, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return L"";
    }

    wchar_t value[MAX_PATH] = {};
    DWORD valueLength = sizeof(value);
    DWORD type;

    std::wstring result;
    if (RegQueryValueExW(hKey, APP_NAME, NULL, &type, (LPBYTE)value, &valueLength) == ERROR_SUCCESS
        && type == REG_SZ) {
        result = value;
    }
    RegCloseKey(hKey);

    return result;
}

bool AddToStartup() {
    std::wstring exePath = GetExecutablePath();

    // 已注册且指向自身路径，无需修改
    if (_wcsicmp(GetStartupPath().c_str(), exePath.c_str()) == 0) return true;

    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_RUN_PATH, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        WriteLog(L"Failed to open registry");
        return false;
    }

    LONG result = RegSetValueExW(hKey, APP_NAME, 0, REG_SZ,
        (const BYTE*)exePath.c_str(),
        (DWORD)((exePath.length() + 1) * sizeof(wchar_t)));

    RegCloseKey(hKey);

    return (result == ERROR_SUCCESS);
}

bool RemoveFromStartup() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_RUN_PATH, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        WriteLog(L"Failed to open registry for removal");
        return false;
    }

    LONG result = RegDeleteValueW(hKey, APP_NAME);
    RegCloseKey(hKey);

    return (result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND);
}
