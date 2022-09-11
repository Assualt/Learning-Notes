#pragma once
#include <iostream>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <syscall.h>

namespace muduo {
namespace base {
namespace System {

inline int mkdir(const std::string &path, int mode) { return ::mkdir(path.c_str(), mode); }

inline int rename(const std::string &srcFilePath, const std::string &dstFileName) {
    return ::rename(srcFilePath.c_str(), dstFileName.c_str());
}

inline int remove(const std::string &path) { return ::remove(path.c_str()); }

inline int rmdir(const std::string &path) { return ::rmdir(path.c_str()); }

inline std::string GetCurrentThreadName() {
    char tempName[ 256 ] = {0};
    (void)prctl(PR_GET_NAME, tempName);
    return tempName;
}

inline void SetThreadName(const std::string &threadName) { (void)::prctl(PR_SET_NAME, threadName.c_str()); }

inline long Tid() { return syscall(SYS_gettid); }

inline long Pid() { return getpid(); }

inline const char *GetErrMsg(int err) { return strerror(err); }

} // namespace System
} // namespace base
} // namespace muduo
