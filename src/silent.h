#pragma once

// 进入完全静默模式：禁止一切可能的错误弹窗。
// 任何致命错误只写日志并让进程静默退出，绝不在前台提示。
// 必须在 WinMain 最开始调用。
void EnterSilentMode();
