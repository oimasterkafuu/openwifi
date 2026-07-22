#pragma once

// ==================== 全局配置 ====================

// 程序标识（注册表启动项名称）
#define APP_NAME L"AutoHotspot"
#define REGISTRY_RUN_PATH L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"

// 定期检查热点的间隔（秒）。
// 间隔期间进程完全挂起等待，CPU 占用为 0。
#define CHECK_INTERVAL_SECONDS 300

// 自毁日期：在此日期之后（不含当天），程序启动时不再开启热点，
// 而是移除开机启动项并删除自身，未来也不再启动。
#define DESTRUCT_YEAR  2027
#define DESTRUCT_MONTH 6
#define DESTRUCT_DAY   20
