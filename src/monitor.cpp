#include "monitor.h"
#include "config.h"
#include "hotspot.h"
#include "logger.h"
#include "selfdestruct.h"

#include <windows.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")

// 注册网络地址变化通知，变化发生时 hEvent 会被置位。
// 返回 true 表示注册成功（或已有变化发生）。
static bool ArmNetworkNotify(HANDLE hEvent, OVERLAPPED* overlap, HANDLE* hNotify) {
    ResetEvent(hEvent);
    DWORD ret = NotifyAddrChange(hNotify, overlap);
    if (ret == NO_ERROR) {
        // 两次注册之间已有变化发生，立即视为触发
        SetEvent(hEvent);
        return true;
    }
    return ret == ERROR_IO_PENDING;
}

void RunMonitorLoop() {
    // 定期检查计时器：立即触发一次，之后按周期触发
    HANDLE hTimer = CreateWaitableTimerW(NULL, FALSE, NULL);
    if (!hTimer) {
        WriteLog(L"Failed to create waitable timer");
        return;
    }

    LARGE_INTEGER dueTime;
    dueTime.QuadPart = 0;
    if (!SetWaitableTimer(hTimer, &dueTime, CHECK_INTERVAL_SECONDS * 1000, NULL, NULL, FALSE)) {
        WriteLog(L"Failed to set waitable timer");
        CloseHandle(hTimer);
        return;
    }

    // 系统事件：网络地址变化通知（网卡连接/断开、IP 变化等）
    HANDLE hNetEvent = CreateEventW(NULL, TRUE, FALSE, NULL); // 手动复位
    OVERLAPPED overlap = {};
    overlap.hEvent = hNetEvent;
    HANDLE hNotify = NULL;

    bool netNotifyArmed = (hNetEvent != NULL) && ArmNetworkNotify(hNetEvent, &overlap, &hNotify);
    if (!netNotifyArmed) {
        WriteLog(L"Network change notification unavailable, using periodic checks only");
    }

    WriteLog(L"Monitor loop started");

    HANDLE handles[2] = { hTimer, hNetEvent };
    DWORD handleCount = netNotifyArmed ? 2 : 1;

    for (;;) {
        // 挂起等待，期间 CPU 占用为 0
        DWORD wait = WaitForMultipleObjects(handleCount, handles, FALSE, INFINITE);

        if (wait == WAIT_FAILED) {
            WriteLog(L"Wait failed, retrying in 60s");
            Sleep(60000);
        } else if (wait == WAIT_OBJECT_0 + 1) {
            // 网络状态变化：重新挂起通知，并等待网络稳定后再检查
            ArmNetworkNotify(hNetEvent, &overlap, &hNotify);
            Sleep(10000);
        }

        // 每次醒来都检查自毁日期：即使程序常驻跨过了截止日期也能自毁
        if (IsPastDestructDate()) {
            SelfDestruct();
            break;
        }

        CheckAndEnableHotspot();
    }

    CloseHandle(hTimer);
    if (hNetEvent) CloseHandle(hNetEvent);
}
