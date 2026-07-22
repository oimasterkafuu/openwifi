#pragma once

#include <string>

// 日志文件路径：%TEMP%\AutoHotspot.log
std::wstring GetLogPath();

// 追加一行带时间戳的日志
void WriteLog(const std::wstring& message);
