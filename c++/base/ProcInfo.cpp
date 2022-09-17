#include "ProcInfo.h"
#include "Backtrace.h"
#include "DirScanner.h"
#include "Format.h"
#include "Logging.h"
#include "System.h"
#include <semaphore.h>
#include <signal.h>
#include <unistd.h> // for getpid

using namespace muduo::base;

namespace {
sem_t    g_sem;
bool     g_inited            = false;
uint32_t g_threadCallbackSig = 47;
} // namespace

void ProcInfo::RegDefaultGetThreadAction() {
    struct sigaction sa;
    sa.sa_handler = ProcInfo::ThreadSignalCallback;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(g_threadCallbackSig, &sa, nullptr);
}

void ProcInfo::ThreadSignalCallback(int sig) {
    logger.info("current threadName:%s recv sig %d and callstack:\n%s", System::GetCurrentThreadName(), sig,
                GetBackCallStack());
    (void)sem_post(&g_sem);
}

void ProcInfo::GetAllThreadCallStack() {
    if (!g_inited) {
        auto ret = sem_init(&g_sem, 0, 0);
        LOG_IF(ret < 0).warning("sem init failed");
        g_inited = true;
    }

    std::string taskPath = FmtString("/proc/%/task").arg(getpid()).str();
    DirScanner  scanner(taskPath.c_str());
    FileAttr    attr;

    while (scanner.Fetch(attr)) {
        attr.SetParentPath(taskPath);
        if (!attr.IsDir()) {
            continue;
        }

        if (attr.GetName() == "." || attr.GetName() == "..") {
            continue;
        }

        auto threadId = atoi(attr.GetName().c_str());

        (void)pthread_kill(threadId, g_threadCallbackSig);

        (void)sem_wait(&g_sem);
    }

    scanner.CloseHandle();
}

// 获取当前进程的所有线程数量
uint32_t ProcInfo::GetCurrentProcThreadNum() {
    auto       scanPath = FmtString("/proc/%/task").arg(System::Pid()).str();
    DirScanner scanner(scanPath.c_str());
    FileAttr   attr;
    uint32_t   threadCnt = 0;
    while (scanner.Fetch(attr)) {
        attr.SetParentPath(scanPath);
        if (!attr.IsDir()) {
            continue;
        }

        if (attr.GetName() == ".." || attr.GetName() == ".") {
            continue;
        }

        auto threadId = std::atoi(attr.GetName().c_str());
        if (threadId != 0) {
            threadCnt++;
        }
    }

    scanner.CloseHandle();
    return threadCnt;
}