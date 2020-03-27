#ifndef _XLOCK_H_2020_01_04
#define _XLOCK_H_2020_01_04
#include <pthread.h>
#include "base/xstring.h"
#include "base/xexception.h"
NAMESPACE_BEGIN
DECLARE_EXCEPTION(LockException, XException);
class TLock {
public:
    TLock(void) {
        pthread_mutex_init(&m_lock, NULL);
    }  //初始化所变量
    ~TLock(void) {
        pthread_mutex_destroy(&m_lock);
    }  //销毁锁变量
    void lock(void) {
        int RetCode = pthread_mutex_lock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to mutex lock error. code:%").arg(RetCode).str());
        }
    }  //未能锁住当前变量　抛异常出去
    void release(void) {
        int RetCode = pthread_mutex_unlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to mutex unlock error. code:%").arg(RetCode).str());
        }
    }  //未能释放当前锁变量　抛异常出去
    void trylock(void) {
        int RetCode = pthread_mutex_trylock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to mutex trylock error. code:%").arg(RetCode).str());
        }
    }

protected:
    mutable pthread_mutex_t m_lock;  //定义可变的变量　锁变量
};

class TSpinLock {
    TSpinLock(void) {
        pthread_spin_init(&m_lock, 0);
    }  //初始化所变量
    ~TSpinLock(void) {
        pthread_spin_destroy(&m_lock);
    }  //销毁锁变量
    void lock(void) {
        int RetCode = pthread_spin_lock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to spin lock error. code:%").arg(RetCode).str());
        }
    }  //未能锁住当前变量　抛异常出去
    void release(void) {
        int RetCode = pthread_spin_unlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to spin unlock error. code:%").arg(RetCode).str());
        }
    }  //未能释放当前锁变量　抛异常出去
    bool trylock(void) {
        int RetCode = pthread_spin_trylock(&m_lock);
        if (RetCode == 0) {
            return 0;
        } else if (RetCode == EBUSY) {
            throw LockException(TFmtString("try to spin trylock error. code:%").arg(RetCode).str());
        }
        return false;
    }

protected:
    mutable pthread_spinlock_t m_lock;  //定义可变的变量　锁变量
};

//基类实现　ReadLock WriteLock
class TRWLock {
    TRWLock(void) {
        pthread_rwlock_init(&m_lock, NULL);
    }  //初始化所变量
    ~TRWLock(void) {
        pthread_rwlock_destroy(&m_lock);
    }  //销毁锁变量
    void readLock(void) {
        int RetCode = pthread_rwlock_rdlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to rwlock readlock error. code:%").arg(RetCode).str());
        }
    }
    void writeLock(void) {
        int RetCode = pthread_rwlock_wrlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to rwlock writelock error. code:%").arg(RetCode).str());
        }
    }
    void release(void) {
        int RetCode = pthread_rwlock_unlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to rwlock relaselock error. code:%").arg(RetCode).str());
        }
    }  //未能释放当前锁变量　抛异常出去
    void tryReadlock(void) {
        int RetCode = pthread_rwlock_tryrdlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to rwlock tryReadlock error. code:%").arg(RetCode).str());
        }
    }
    //未能释放当前锁变量　抛异常出去
    void tryWritelock(void) {
        int RetCode = pthread_rwlock_trywrlock(&m_lock);
        if (RetCode) {
            throw LockException(TFmtString("try to rwlock trywritelock error. code:%").arg(RetCode).str());
        }
    }

protected:
    mutable pthread_rwlock_t m_lock;  //定义可变的变量　锁变量
};

template <class type_lock>
class TAutoLockImpl {
public:
    TAutoLockImpl(type_lock &lock) : m_lock(&lock) {
        m_lock->lock();
    }
    void release(void) {
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
    TAutoReadLockImpl(type_lock &lock) : m_lock(&lock) {
        m_lock->readLock();
    }
    void release(void) {
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
    TAutoWriteLockImpl(type_lock &lock) : m_lock(&lock) {
        m_lock->writeLock();
    }
    void release(void) {
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

NAMESPACE_END

#endif
