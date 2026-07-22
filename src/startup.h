#pragma once

// 是否已注册开机启动项
bool IsAlreadyInStartup();

// 注册开机启动项（已存在则直接返回 true）
bool AddToStartup();

// 移除开机启动项（不存在也视为成功）
bool RemoveFromStartup();
