#pragma once
#include "Condition.h"
#include "Mutex.h"
#include "nonecopyable.h"
#include <memory>
#include <pthread.h>

namespace muduo {
namespace base {

class CountDownLatch : nonecopyable {
public:
    explicit CountDownLatch(int count);

    void Wait();

    void CountDown();

    int GetCount() const;

private:
    mutable MutexLock          m_mutex;
    int                        m_nCount;
    std::unique_ptr<Condition> m_pCond{nullptr};
};
} // namespace base
} // namespace muduo
