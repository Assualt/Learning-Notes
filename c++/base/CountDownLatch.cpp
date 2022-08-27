#include "CountDownLatch.h"

using namespace muduo;
using namespace muduo::base;

CountDownLatch::CountDownLatch(int count)
    : m_nCount(count)
    , m_pCond(new Condition(m_mutex)) {}

void CountDownLatch::Wait() {
    AutoLock myLock(m_mutex);
    while (m_nCount > 0) {
        m_pCond->Wait();
    }
}

void CountDownLatch::CountDown() {
    AutoLock myLock(m_mutex);
    --m_nCount;
    if (m_nCount == 0) {
        m_pCond->NotifyAll();
    }
}

int CountDownLatch::GetCount() const {
    AutoLock myLock(m_mutex);
    return m_nCount;
}
