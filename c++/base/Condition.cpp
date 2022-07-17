#include "Condition.h"

using namespace muduo;
using namespace muduo::base;

Condition::Condition(MutexLock &lock)
    : m_lock(lock) {
    pthread_cond_init(&m_cond, nullptr);
}

Condition::~Condition() {
    pthread_cond_destroy(&m_cond);
}

void Condition::Wait() {
    (void)pthread_cond_wait(&m_cond, m_lock.GetLock());
}

void Condition::Notify() {
    (void)pthread_cond_signal(&m_cond);
}

void Condition::NotifyAll() {
    (void)pthread_cond_broadcast(&m_cond);
}

[[maybe_unused]] bool Condition::WaitForTimeOut(double seconds) {
    struct timespec abstime {};
    // FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
    clock_gettime(CLOCK_REALTIME, &abstime);

    const int64_t kNanoSecondsPerSecond = 1000000000;
    auto          nanoseconds           = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

    abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
    abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

    return (ETIMEDOUT == pthread_cond_timedwait(&m_cond, m_lock.GetLock(), &abstime));
}
