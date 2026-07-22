#include "silent.h"
#include "logger.h"

#include <windows.h>
#include <stdlib.h>

// CRT 非法参数（如空指针）：静默返回，让调用方按失败处理
static void __cdecl SilentInvalidParameterHandler(const wchar_t*, const wchar_t*,
    const wchar_t*, unsigned int, uintptr_t) {
}

// 纯虚函数调用：无法恢复，静默退出
static void __cdecl SilentPureCallHandler() {
    WriteLog(L"Pure virtual function call, exiting silently");
    _exit(3);
}

// 未处理异常（如访问违例）：记录后静默退出，不触发 WER 弹窗
static LONG WINAPI SilentUnhandledExceptionFilter(EXCEPTION_POINTERS* info) {
    wchar_t buf[128];
    if (info && info->ExceptionRecord) {
        wsprintfW(buf, L"Unhandled exception 0x%08lX, exiting silently",
            info->ExceptionRecord->ExceptionCode);
    } else {
        wsprintfW(buf, L"Unhandled exception, exiting silently");
    }
    WriteLog(buf);
    ExitProcess((UINT)-1);
}

void EnterSilentMode() {
    // 禁止系统关键错误弹窗（无磁盘、GP 错误框、文件打开错误框等）
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX |
        SEM_NOALIGNMENTFAULTEXCEPT | SEM_NOOPENFILEERRORBOX);

    // 崩溃时走自己的过滤器，静默退出
    SetUnhandledExceptionFilter(SilentUnhandledExceptionFilter);

    // abort() 不弹 "Microsoft Visual C++ Runtime Error" 对话框
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    // CRT 非法参数与纯虚调用不弹窗
    _set_invalid_parameter_handler(SilentInvalidParameterHandler);
    _set_purecall_handler(SilentPureCallHandler);
}
