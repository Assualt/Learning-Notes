//
// Created by 侯鑫 on 2024/1/1.
//

#include "condition.h"

using namespace ssp::base;

Condition::Condition(MutexLock &lock)
    : lock_(lock) 
{
    pthread_cond_init(&cond_, nullptr);
}

Condition::~Condition()
{
    pthread_cond_destroy(&cond_);
}

void Condition::Wait()
{
    (void)pthread_cond_wait(&cond_, lock_.GetLock());
}

void Condition::Notify()
{
    (void)pthread_cond_signal(&cond_);
}

void Condition::NotifyAll()
{
    (void)pthread_cond_broadcast(&cond_);
}

[[maybe_unused]] bool Condition::WaitForTimeOut(double seconds)
{
    struct timespec absTime {};
    // FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
    clock_gettime(CLOCK_REALTIME, &absTime);

    const int64_t kNanoSecondsPerSecond = 1000000000;
    auto nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

    absTime.tv_sec += static_cast<time_t>((absTime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
    absTime.tv_nsec = static_cast<long>((absTime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

    return (ETIMEDOUT == pthread_cond_timedwait(&cond_, lock_.GetLock(), &absTime));
}
