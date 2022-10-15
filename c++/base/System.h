#pragma once
#include <csignal>
#include <cstdio>
#include <iostream>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <unordered_map>

namespace muduo::base::System {

inline int mkdir(const std::string &path, int mode) { return ::mkdir(path.c_str(), mode); }

inline int rename(const std::string &srcFilePath, const std::string &dstFileName) {
    return ::rename(srcFilePath.c_str(), dstFileName.c_str());
}

inline int remove(const std::string &path) { return ::remove(path.c_str()); }

inline std::string GetCurrentThreadName() {
    char tempName[ 256 ] = {0};
    (void)prctl(PR_GET_NAME, tempName);
    return tempName;
}

inline void SetThreadName(const std::string &threadName) { (void)::prctl(PR_SET_NAME, threadName.c_str()); }

inline long Tid() { return ::syscall(SYS_gettid); }

inline long Pid() { return ::getpid(); }

inline const char *GetErrMsg(int err) { return strerror(err); }

inline const char *GetSigName(int sig) {
    static std::unordered_map<int, const char *> g_sigNameMapper = {
        {SIGINT, "SIGINT"},   {SIGSEGV, "SIGSEGV"}, {SIGHUP, "SIGHUP"},   {SIGPIPE, "SIGPIPE"},
        {SIGQUIT, "SIGQUIT"}, {SIGKILL, "SIGKILL"}, {SIGTERM, "SIGTERM"}, {SIGCHLD, "SIGCHLD"},
        {SIGIOT, "SIGIOT"},   {SIGUSR1, "SIGUSR1"}, {SIGALRM, "SIGALRM"}};

    auto iter = g_sigNameMapper.find(sig);
    return iter != g_sigNameMapper.end() ? iter->second : "NULL";
}

} // namespace muduo::base::System
