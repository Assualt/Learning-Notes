#pragma once

#include "base/Condition.h"
#include "base/Logging.h"
#include "base/Mutex.h"
#include "base/Thread.h"
#include "base/nonecopyable.h"
#include <queue>
#include <sstream>

using muduo::base::Condition;
using muduo::base::Logger;
using muduo::base::MutexLock;
using muduo::base::Thread;

class HttpLog : muduo::base::nonecopyable {
public:
    explicit HttpLog(Logger &log);
    ~HttpLog();
    bool Init();

public:
    template <class T> HttpLog &operator<<(const T &val) {
        m_sCmdInStream << val;
        CheckLogBufferOverFlow();
        return *this;
    }

    void LogTaskThread();

private:
    void CheckLogBufferOverFlow();

private:
    Logger                 &m_pLogger;
    std::stringstream       m_sCmdInStream;
    MutexLock               m_mutex;
    std::unique_ptr<Thread> m_pThread{nullptr};
    MutexLock               m_mutexWrite;
    bool                    m_bExit{false};
    std::queue<std::string> m_logQueue;
};