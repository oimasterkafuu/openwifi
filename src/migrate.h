#pragma once

// 检查并清除旧版本安装：
// 从注册表读取旧版程序的启动路径，若与自身路径不同则删除旧版 exe。
// 必须在 AddToStartup() 覆盖注册表之前调用。
void ReplaceOldInstallation();
