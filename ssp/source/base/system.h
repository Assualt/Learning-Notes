//
// Created by 侯鑫 on 2023/12/26.
//

#ifndef SSP_TOOLS_SYSTEM_H
#define SSP_TOOLS_SYSTEM_H

#include <cstdint>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <pwd.h>

namespace ssp::base::System {

inline int MakeDir(const std::string &path, int mode)
{
    return mkdir(path.c_str(), mode);
}

inline int Rename(const std::string &srcFilePath, const std::string &dstFileName)
{
    return rename(srcFilePath.c_str(), dstFileName.c_str());
}

inline int Remove(const std::string &path)
{
    return remove(path.c_str());
}

inline int RemoveDir(const std::string &path)
{
    return rmdir(path.c_str());
}

inline bool Access(const std::string &file, int32_t flag)
{
    return access(file.c_str(), flag) == 0;
}

inline std::string GetCurrentThreadName()
{
    char tempName[ PATH_MAX ] = {0};
#ifdef LINUX
    (void)prctl(PR_GET_NAME, tempName);
#else
    pthread_getname_np(pthread_self(), tempName, PATH_MAX);
#endif
    return tempName;
}

inline std::string CurrentPwd()
{
    char tempName[ PATH_MAX ] = {0};
    (void)getcwd(tempName, sizeof(tempName));
    return tempName;
}

inline void SetThreadName(const std::string &threadName)
{
#ifdef LINUX
    (void)::prctl(PR_SET_NAME, threadName.c_str());
#else
    pthread_setname_np(threadName.c_str());
#endif
}

inline long Tid()
{
#ifdef LINUX
    return ::syscall(SYS_gettid);
#else
    return 0;
#endif
}

inline long Pid()
{
    return ::getpid();
}

inline const char *GetErrMsg(int err)
{
    return strerror(err);
}

inline const char *GetSigName(int sig) {
    static std::unordered_map<int, const char *> g_sigNameMapper = {
            {SIGINT, "SIGINT"},   {SIGSEGV, "SIGSEGV"}, {SIGHUP, "SIGHUP"},   {SIGPIPE, "SIGPIPE"},
            {SIGQUIT, "SIGQUIT"}, {SIGKILL, "SIGKILL"}, {SIGTERM, "SIGTERM"}, {SIGCHLD, "SIGCHLD"},
            {SIGIOT, "SIGIOT"},   {SIGUSR1, "SIGUSR1"}, {SIGALRM, "SIGALRM"}, {SIGBUS, "SIGBUS"},
            {SIGABRT, "SIGABRT"}
    }
    ;

    auto iter = g_sigNameMapper.find(sig);
    return iter != g_sigNameMapper.end() ? iter->second : "NULL";
}

inline const char *GetUname()
{
    return getpwuid(getuid())->pw_name;
}

inline const char *BaseName(const char *path)
{
#ifdef LINUX
    return basename(path);
#else
    return "";
#endif
}


}


#endif //SSP_TOOLS_SYSTEM_H
