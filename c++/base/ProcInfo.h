#pragma once
namespace muduo::base {
namespace ProcInfo {

void RegDefaultGetThreadAction();

void GetAllThreadCallStack();

void ThreadSignalCallback(int sig);

} // namespace ProcInfo

} // namespace muduo::base