#ifndef _XLOCK_H_2020_01_04
#define _XLOCK_H_2020_01_04
#include <pthread.h>
#include "base/tstring.h"
#include "base/texception.h"
NAMESPACE_BEGIN
DECLARE_EXCEPTION(LockException, TException);
class TLock {
public:
    TLock() {
        pthread_mutex_init(&m_lock, nullptr);
    }  //初始化所变量
    ~TLock() {
        pthread_mutex_destroy(&m_lock);
    }  //销毁锁变量
    void lock() {
        int RetCode = pthread_mutex_lock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to mutex lock error. code:%").arg(RetCode).str());
        }
    }  //未能锁住当前变量　抛异常出去
    void release() {
        int RetCode = pthread_mutex_unlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to mutex unlock error. code:%").arg(RetCode).str());
        }
    }  //未能释放当前锁变量　抛异常出去
    void trylock() {
        int RetCode = pthread_mutex_trylock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to mutex trylock error. code:%").arg(RetCode).str());
        }
    }

protected:
    mutable pthread_mutex_t m_lock{};  //定义可变的变量　锁变量
};

class TSpinLock {
    TSpinLock() {
        pthread_spin_init(&m_lock, 0);
    }  //初始化所变量
    ~TSpinLock() {
        pthread_spin_destroy(&m_lock);
    }  //销毁锁变量
    void lock() {
        int RetCode = pthread_spin_lock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to spin lock error. code:%").arg(RetCode).str());
        }
    }  //未能锁住当前变量　抛异常出去
    void release() {
        int RetCode = pthread_spin_unlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to spin unlock error. code:%").arg(RetCode).str());
        }
    }  //未能释放当前锁变量　抛异常出去
    bool trylock() {
        int RetCode = pthread_spin_trylock(&m_lock);
        if (RetCode == 0) {
            return 0;
        } else if (RetCode == EBUSY) {
            throw LockException(TFmtString("try to spin trylock error. code:%").arg(RetCode).str());
        }
        return false;
    }

protected:
    mutable pthread_spinlock_t m_lock{};  //定义可变的变量　锁变量
};

//基类实现　ReadLock WriteLock
class TRWLock {
    TRWLock() {
        pthread_rwlock_init(&m_lock, nullptr);
    }  //初始化所变量
    ~TRWLock() {
        pthread_rwlock_destroy(&m_lock);
    }  //销毁锁变量
    void readLock() {
        int RetCode = pthread_rwlock_rdlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to rwlock readlock error. code:%").arg(RetCode).str());
        }
    }
    void writeLock() {
        int RetCode = pthread_rwlock_wrlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to rwlock writelock error. code:%").arg(RetCode).str());
        }
    }
    void release() {
        int RetCode = pthread_rwlock_unlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to rwlock relaselock error. code:%").arg(RetCode).str());
        }
    }  //未能释放当前锁变量　抛异常出去
    void tryReadlock() {
        int RetCode = pthread_rwlock_tryrdlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to rwlock tryReadlock error. code:%").arg(RetCode).str());
        }
    }
    //未能释放当前锁变量　抛异常出去
    void tryWritelock() {
        int RetCode = pthread_rwlock_trywrlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to rwlock trywritelock error. code:%").arg(RetCode).str());
        }
    }

protected:
    mutable pthread_rwlock_t m_lock{};  //定义可变的变量　锁变量
};

template <class type_lock>
class TAutoLockImpl {
public:
    explicit TAutoLockImpl(type_lock &lock) : m_lock(&lock) {
        m_lock->lock();
    }
    void release() {
        m_lock->release();
        m_lock = nullptr;
    }
    ~TAutoLockImpl() {
        release();
    }

protected:
    type_lock *m_lock;
};
template <class type_lock>
class TAutoReadLockImpl {
public:
    explicit TAutoReadLockImpl(type_lock &lock) : m_lock(&lock) {
        m_lock->readLock();
    }
    void release() {
        m_lock->release();
        m_lock = nullptr;
    }
    ~TAutoReadLockImpl() {
        release();
    }

protected:
    type_lock *m_lock;
};
template <class type_lock>
class TAutoWriteLockImpl {
public:
    explicit TAutoWriteLockImpl(type_lock &lock) : m_lock(&lock) {
        m_lock->writeLock();
    }
    void release() {
        m_lock->release();
        m_lock = nullptr;
    }
    ~TAutoWriteLockImpl() {
        release();
    }

protected:
    type_lock *m_lock;
};
typedef TAutoReadLockImpl<TRWLock> TAutoReadLock;
typedef TAutoWriteLockImpl<TRWLock> TAutoWriteLock;
typedef TAutoLockImpl<TLock> TAutoLock;

NAMESPACE_END  // namespace hmt

#endif
