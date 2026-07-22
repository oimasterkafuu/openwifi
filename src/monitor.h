#pragma once

// 常驻监控循环，正常情况下不返回。
// 等待期间进程挂起，CPU 占用为 0；仅在以下情况醒来：
//   1. 定期检查计时器到期（CHECK_INTERVAL_SECONDS）
//   2. 系统网络状态发生变化
// 醒来后发现超过自毁日期时，执行自毁并返回。
void RunMonitorLoop();
