#pragma once

// 检查热点状态，未开启则尝试开启。
// 返回 true 表示热点已处于开启状态（或本次成功开启）。
bool CheckAndEnableHotspot();
