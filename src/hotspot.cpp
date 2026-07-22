#include "hotspot.h"
#include "logger.h"

#include <windows.h>
#include <string>

// 运行隐藏窗口的外部命令，最多等待 15 秒
static bool RunHiddenCommand(const std::wstring& command, DWORD& exitCode) {
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

    return true;
}

bool CheckAndEnableHotspot() {
    // PowerShell 脚本语义：
    //   exit 0 - 热点已开启 / 本次开启成功
    //   exit 2 - 当前没有可用的网络连接（如开机时网络尚未就绪）
    //   exit 3 - 开启失败
    std::wstring psCmd =
        L"powershell.exe -ExecutionPolicy Bypass -Command \""
        L"Add-Type -AssemblyName System.Runtime.WindowsRuntime; "
        L"$null = [Windows.Networking.Connectivity.NetworkInformation,Windows.Networking.Connectivity,ContentType=WindowsRuntime]; "
        L"$null = [Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager,Windows.Networking.NetworkOperators,ContentType=WindowsRuntime]; "
        L"$profile = [Windows.Networking.Connectivity.NetworkInformation]::GetInternetConnectionProfile(); "
        L"if ($profile -eq $null) { exit 2 }; "
        L"$manager = [Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager]::CreateFromConnectionProfile($profile); "
        L"if ($manager.TetheringOperationalState -eq 'On') { exit 0 }; "
        L"$op = $manager.StartTetheringAsync(); "
        L"while ($op.Status -eq 0) { Start-Sleep -Milliseconds 100 }; "
        L"if ($op.Status -ne 1) { exit 3 } else { exit 0 }\"";

    DWORD exitCode = 0;
    if (!RunHiddenCommand(psCmd, exitCode)) return false;

    if (exitCode == 0) {
        WriteLog(L"Hotspot is on");
        return true;
    }

    // 开启失败不立即重试，交给下一轮定期检查即可
    WriteLog(L"Hotspot is off (exit code: " + std::to_wstring(exitCode) + L"), will retry on next check");
    return false;
}
