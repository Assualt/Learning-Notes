//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_MUTEX_H
#define SSP_TOOLS_MUTEX_H

#include "exception.h"
#include <pthread.h>

namespace ssp::base {


DECLARE_EXCEPTION(MutexLockException, Exception)
class MutexLock final {
public:
    explicit MutexLock(const pthread_mutexattr_t *attr = nullptr);

    ~MutexLock();

public:
    void Lock();

    void UnLock();

    void TryLock();

    inline pthread_mutex_t *GetLock() { return &m_mLock; }

private:
    mutable pthread_mutex_t m_mLock{};
};

class RwLock final {
public:
    explicit RwLock(const pthread_rwlockattr_t *attr = nullptr);

    ~RwLock();

public:
    void RLock();

    void WLock();

    void UnLock();

    void TryRLock();

    void TryWLock();

private:
    mutable pthread_rwlock_t m_rwLock {0};
};

class AutoLock {
public:
    explicit AutoLock(MutexLock &lock)
        : m_lock(lock)
    {
        m_lock.Lock();
    }

    ~AutoLock() { m_lock.UnLock(); }

private:
    MutexLock &m_lock;
};
}

#endif //SSP_TOOLS_MUTEX_H
