//
// Created by 侯鑫 on 2024/1/1.
//

#include "mutex.h"
#include "format.h"

using namespace ssp::base;

#define CHECK_LOCK_NULL(pointer, val)                                                                                  \
    do {                                                                                                               \
        if (pointer == nullptr) {                                                                                      \
            throw MutexLockException(val);                                                                             \
        }                                                                                                              \
    } while (0)

#define CHECK_LOCK_RET(cond, val)                                                                                      \
    do {                                                                                                               \
        if (cond) {                                                                                                    \
            throw MutexLockException(val);                                                                             \
        }                                                                                                              \
    } while (0)

MutexLock::MutexLock(const pthread_mutexattr_t *attr)
{
    pthread_mutex_init(&m_mLock, attr);
}

void MutexLock::Lock()
{
    CHECK_LOCK_NULL(&m_mLock, "lock is not init, lock error");
    auto ret = pthread_mutex_lock(&m_mLock);
    CHECK_LOCK_RET(ret != 0, FmtString("try % error, ret:%").arg("pthread_mutex_lock").arg(ret).str());
}

void MutexLock::TryLock()
{
    CHECK_LOCK_NULL(&m_mLock, "lock is not init, lock error");
    auto ret = pthread_mutex_trylock(&m_mLock);
    CHECK_LOCK_RET(ret != 0, FmtString("try % error, ret:%").arg("pthread_mutex_try lock").arg(ret).str());
}

void MutexLock::UnLock() {
    CHECK_LOCK_NULL(&m_mLock, "lock is not init, lock error");
    auto ret = pthread_mutex_unlock(&m_mLock);
    CHECK_LOCK_RET(ret != 0, FmtString("try % error, ret:%").arg("pthread_mutex_unlock").arg(ret).str());
}

MutexLock::~MutexLock()
{
    pthread_mutex_destroy(&m_mLock);
}

RwLock::RwLock(const pthread_rwlockattr_t *attr)
{
    pthread_rwlock_init(&m_rwLock, attr);
}

void RwLock::RLock()
{
    CHECK_LOCK_NULL(&m_rwLock, "lock is not init, lock error");
    auto ret = pthread_rwlock_rdlock(&m_rwLock);
    CHECK_LOCK_RET(ret != 0, FmtString("try % error, ret:%").arg("pthread_rwlock_rdlock").arg(ret).str());
}

void RwLock::WLock()
{
    CHECK_LOCK_NULL(&m_rwLock, "lock is not init, lock error");
    auto ret = pthread_rwlock_wrlock(&m_rwLock);
    CHECK_LOCK_RET(ret != 0, FmtString("try % error, ret:%").arg("pthread_rwlock_wrlock").arg(ret).str());
}

void RwLock::UnLock()
{
    CHECK_LOCK_NULL(&m_rwLock, "lock is not init, lock error");
    auto ret = pthread_rwlock_unlock(&m_rwLock);
    CHECK_LOCK_RET(ret != 0, FmtString("try % error, ret:%").arg("pthread_rwlock_unlock").arg(ret).str());
}

void RwLock::TryRLock()
{
    CHECK_LOCK_NULL(&m_rwLock, "lock is not init, trylock error");
    auto ret = pthread_rwlock_tryrdlock(&m_rwLock);
    CHECK_LOCK_RET(ret != 0, FmtString("try % error, ret:%").arg("pthread_rwlock_try read lock").arg(ret).str());
}

void RwLock::TryWLock()
{
    CHECK_LOCK_NULL(&m_rwLock, "lock is not init, trylock error");
    auto ret = pthread_rwlock_trywrlock(&m_rwLock);
    CHECK_LOCK_RET(ret != 0, FmtString("try % error, ret:%").arg("pthread_rwlock_try write lock").arg(ret).str());
}

RwLock::~RwLock()
{
    pthread_rwlock_destroy(&m_rwLock);
}