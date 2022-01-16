#pragma once
#include "Exception.h"
#include <pthread.h>
namespace muduo {
namespace base {

DECLARE_EXCEPTION(MutexLockException, Exception);
class MutexLock final {
public:
    MutexLock(const pthread_mutexattr_t *attr = nullptr);
    ~MutexLock();

public:
    void Lock();
    void UnLock();
    void TryLock();
    inline pthread_mutex_t* GetLock()  { return &m_Lock; }

private:
    mutable pthread_mutex_t m_Lock;
};

class RWLock final {
public:
    RWLock(const pthread_rwlockattr_t *attr = nullptr);
    ~RWLock();

public:
    void RLock();
    void WLock();
    void UnLock();
    void TryRLock();
    void TryWLock();

private:
    mutable pthread_rwlock_t m_Lock;
};

} // namespace base
} // namespace muduo
