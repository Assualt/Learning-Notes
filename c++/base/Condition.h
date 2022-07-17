#pragma once
#include "Mutex.h"
#include "nonecopyable.h"

namespace muduo {
namespace base {

class Condition : nonecopyable {
public:
    explicit Condition(MutexLock &lock);

    ~Condition();

    void Wait();

    [[maybe_unused]] bool WaitForTimeOut(double seconds);

    void Notify();

    void NotifyAll();

private:
    pthread_cond_t m_cond{};
    MutexLock &    m_lock;
};
} // namespace base
} // namespace muduo
