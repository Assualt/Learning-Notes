#pragma once

#include "Exception.h"
#include "nonecopyable.h"
#include <atomic>
#include <functional>
#include <memory>
#include <pthread.h>
using muduo::base::nonecopyable;

namespace muduo {
namespace base {
using ThreadFunc = std::function<void()>;
namespace detail {
DECLARE_EXCEPTION(ThreadException, Exception);

struct ThreadContext {
public:
    ThreadFunc  m_func{nullptr};
    std::string m_strThreadName;
    pid_t *     m_nPid{nullptr};

public:
    ThreadContext(ThreadFunc func, const std::string &name, pid_t *pid);
    void Run();
};
} // namespace detail

class Thread : nonecopyable {
public:
    explicit Thread(ThreadFunc, const std::string &name = "");
    ~Thread();

public:
    void               Start();
    int                Join();
    void               Detached();
    bool               IsStarted();
    pid_t              Tid() const;
    const std::string &Name() const;

public:
    static int   NumCreated();
    static void *StartThread(void *arg);

private:
    void SetDefaultName();

    bool        m_isStarted{false};
    bool        m_isJoined{false};
    pthread_t   m_nThreadId{0};
    pid_t       m_nTid{0};
    ThreadFunc  m_threadFunc{nullptr};
    std::string m_strFunName;

    static std::atomic<int32_t> m_nThreadCnt;
};

} // namespace base
} // namespace muduo
