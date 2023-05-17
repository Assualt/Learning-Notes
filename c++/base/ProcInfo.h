#pragma once
#include <cstdint>
namespace muduo::base::ProcInfo {

void RegDefaultGetThreadAction();

void GetAllThreadCallStack();

void ThreadSignalCallback(int sig);

// 获取当前进程的所有线程数量
uint32_t GetCurrentProcThreadNum();

} // namespace muduo::base::ProcInfo