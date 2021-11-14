#pragma once

#include "nonecopyable.h"
#include <atomic>
#include <functional>
#include <memory>
#include <pthread.h>
using muduo::base::nonecopyable;

namespace muduo {
namespace base {

class Thread : nonecopyable {
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc, const std::string &name = "");
    ~Thread();

public:
    void               start();
    int                join();
    bool               isStarted();
    pid_t              tid() const;
    const std::string &name() const;

    static int numCreated();

private:
    void setDefaultName();

    bool        m_IsStarted;
    bool        m_IsJoined;
    pthread_t   m_nThreadId;
    pid_t       m_nTid;
    ThreadFunc  m_threadFunc;
    std::string m_strFunName;

    static std::atomic<int32_t> m_nThreadCnt;
};

} // namespace base
} // namespace muduo
