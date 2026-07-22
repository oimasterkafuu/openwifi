#include "selfdestruct.h"
#include "config.h"
#include "logger.h"
#include "startup.h"
#include "util.h"

#include <windows.h>
#include <string>

bool IsPastDestructDate() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    if (st.wYear != DESTRUCT_YEAR) return st.wYear > DESTRUCT_YEAR;
    if (st.wMonth != DESTRUCT_MONTH) return st.wMonth > DESTRUCT_MONTH;
    return st.wDay > DESTRUCT_DAY;
}

void SelfDestruct() {
    WriteLog(L"Self-destruct date passed, removing startup entry and executable");

    // 1. 移除开机启动项，保证未来不再启动
    RemoveFromStartup();

    // 2. 删除自身可执行文件。
    //    进程运行中无法直接删除自己，因此启动一个独立的 cmd 进程，
    //    等待几秒（此时本进程已退出）后再执行删除。
    std::wstring cmd =
        L"cmd.exe /c timeout /t 3 /nobreak >nul & del /f /q \"" + GetExecutablePath() + L"\"";

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (CreateProcessW(NULL, &cmd[0], NULL, NULL, FALSE,
        CREATE_NO_WINDOW | DETACHED_PROCESS, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        WriteLog(L"Failed to spawn cleanup process");
    }
}
