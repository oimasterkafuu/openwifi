#include "migrate.h"
#include "logger.h"
#include "startup.h"
#include "util.h"

#include <windows.h>
#include <string>

void ReplaceOldInstallation() {
    std::wstring oldPath = GetStartupPath();
    if (oldPath.empty()) return;

    // 注册表指向自身（例如覆盖安装到同一位置），无需处理
    if (_wcsicmp(oldPath.c_str(), GetExecutablePath().c_str()) == 0) return;

    if (GetFileAttributesW(oldPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        WriteLog(L"Old installation not found on disk: " + oldPath);
        return;
    }

    WriteLog(L"Removing old installation: " + oldPath);

    // 旧版本是一次性程序，运行后很快退出，通常可直接删除
    if (DeleteFileW(oldPath.c_str())) {
        WriteLog(L"Old installation removed");
        return;
    }

    // 删除失败（旧版本可能正在运行）：启动独立 cmd 进程，
    // 每 5 秒重试一次，最多 5 分钟
    WriteLog(L"Direct delete failed, spawning background retry task");
    std::wstring cmd =
        L"cmd.exe /c for /l %i in (1,1,60) do ("
        L"del /f /q \"" + oldPath + L"\" >nul 2>&1 & "
        L"if not exist \"" + oldPath + L"\" exit /b 0 & "
        L"timeout /t 5 /nobreak >nul)";

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (CreateProcessW(NULL, &cmd[0], NULL, NULL, FALSE,
        CREATE_NO_WINDOW | DETACHED_PROCESS, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        WriteLog(L"Failed to spawn delete retry task");
    }
}
