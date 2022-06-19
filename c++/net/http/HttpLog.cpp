#include "HttpLog.h"
#include "base/LogHandle.h"
#include "base/Thread.h"

#define MAX_BUF_SIZE 100
using namespace muduo::base;

static std::shared_ptr<LogHandle> RollFileHandler(new RollingFileLogHandle("./", "access.log"));

HttpLog::HttpLog(Logger &log)
    : m_pLogger(log) {
}

HttpLog::~HttpLog() {
    m_pLogger.debug(m_sCmdInStream.str().c_str());
    m_pThread->Join();
}

bool HttpLog::Init() {
    m_pLogger.addLogHandle(RollFileHandler.get());
    m_pLogger.SetAppendLF(false);
    m_pThread.reset(new Thread(std::bind(&HttpLog::LogTaskThread, this), "AsyncTask"));
    m_pThread->Start();
    return true;
}

void HttpLog::LogTaskThread() {
    while (!m_bExit) {
        if (m_logQueue.empty()) {
            continue;
        }
        m_mutex.Lock();
        auto itr = m_logQueue.front();
        m_logQueue.pop();
        m_mutex.UnLock();
        m_pLogger.debug(itr.c_str());
    }
}

void HttpLog::CheckLogBufferOverFlow() {
    // check last log contains \n
    std::string str = m_sCmdInStream.str();
    if (str.back() == '\n') {
        return;
    }

    m_mutex.Lock();
    m_logQueue.push(str);
    m_sCmdInStream.str("");
    m_mutex.UnLock();
}