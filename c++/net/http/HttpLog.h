#pragma once

#include "base/Condition.h"
#include "base/Logging.h"
#include "base/Mutex.h"
#include "base/Thread.h"
#include "base/nonecopyable.h"
#include <sstream>

using muduo::base::Condition;
using muduo::base::Logger;
using muduo::base::MutexLock;
using muduo::base::Thread;
class HttpLog : muduo::base::nonecopyable {
public:
    HttpLog(Logger &log);
    ~HttpLog() {
        m_Thread->join();
    }

    bool Init();

public:
    HttpLog &operator<<(short);
    HttpLog &operator<<(unsigned short);
    HttpLog &operator<<(int);
    HttpLog &operator<<(unsigned int);
    HttpLog &operator<<(long);
    HttpLog &operator<<(unsigned long);
    HttpLog &operator<<(long long);
    HttpLog &operator<<(unsigned long long);
    HttpLog &operator<<(const char *val);
    HttpLog &operator<<(const std::string &);
    HttpLog &Write(const std::string &);

    void DoTaskObj();
    void CheckBufOverFilled();

private:
    Logger                    &m_Logger;
    std::stringstream          m_CmdInStream;
    MutexLock                  m_mutex;
    std::unique_ptr<Condition> m_Cond{nullptr};
    std::unique_ptr<Thread>    m_Thread{nullptr};
    MutexLock                  m_mutexWrite;
    std::unique_ptr<Condition> m_CondWrite{nullptr};
    bool                       m_bExit{false};
};