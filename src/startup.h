#pragma once

#include <string>

// 注册表中当前记录的启动路径；未注册时返回空字符串
std::wstring GetStartupPath();

// 注册开机启动项（已指向自身路径则直接返回 true，否则覆盖为自身路径）
bool AddToStartup();

// 移除开机启动项（不存在也视为成功）
bool RemoveFromStartup();
