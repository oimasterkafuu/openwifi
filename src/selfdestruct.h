#pragma once

// 当前日期是否已超过 config.h 中配置的自毁日期
bool IsPastDestructDate();

// 自毁：移除开机启动项，并删除自身可执行文件。
// 调用后应尽快退出进程。
void SelfDestruct();
