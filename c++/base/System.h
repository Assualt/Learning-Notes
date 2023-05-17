#pragma once
#include <csignal>
#include <cstdio>
#include <iostream>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <unordered_map>

#define MAX_PATH_SIZE 256
namespace muduo::base::System {

inline int Mkdir(const std::string &path, int mode) { return ::mkdir(path.c_str(), mode); }

inline int Rename(const std::string &srcFilePath, const std::string &dstFileName) {
    return ::rename(srcFilePath.c_str(), dstFileName.c_str());
}

inline int Remove(const std::string &path) { return ::remove(path.c_str()); }

inline bool Access(const std::string &file, int flag) { return access(file.c_str(), flag) == 0; }

inline std::string GetCurrentThreadName() {
    char tempName[ MAX_PATH_SIZE ] = {0};
    (void)prctl(PR_GET_NAME, tempName);
    return tempName;
}

inline std::string CurrentPwd() {
    char tempName[ MAX_PATH_SIZE ] = {0};
    (void)getcwd(tempName, sizeof(tempName));
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
        {SIGIOT, "SIGIOT"},   {SIGUSR1, "SIGUSR1"}, {SIGALRM, "SIGALRM"}, {SIGBUS, "SIGBUS"}};

    auto iter = g_sigNameMapper.find(sig);
    return iter != g_sigNameMapper.end() ? iter->second : "NULL";
}

} // namespace muduo::base::System
