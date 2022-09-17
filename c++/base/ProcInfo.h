#pragma once
#include <cstdint>
namespace muduo::base {
namespace ProcInfo {

void RegDefaultGetThreadAction();

void GetAllThreadCallStack();

void ThreadSignalCallback(int sig);

// 获取当前进程的所有线程数量
uint32_t GetCurrentProcThreadNum();

} // namespace ProcInfo

} // namespace muduo::base