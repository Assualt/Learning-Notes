#include "HttpLog.h"
#include "base/LogHandle.h"
#include "base/Thread.h"

#define MAX_BUF_SIZE 100
using namespace muduo::base;

static std::shared_ptr<LogHandle> RollFileHandler(new RollingFileLogHandle("./", "access.log"));

HttpLog::HttpLog(Logger &log)
    : m_pLogger(log) {
    m_pCond.reset(new Condition(m_mutex));
    m_pCondWrite.reset(new Condition(m_mutexWrite));
}

HttpLog::~HttpLog() {
    m_pLogger.debug(m_sCmdInStream.str().c_str());
    m_pThread->join();
}

bool HttpLog::Init() {
    m_pLogger.addLogHandle(RollFileHandler.get());
    m_pLogger.SetAppendLF(false);
    m_pThread.reset(new Thread(std::bind(&HttpLog::LogTaskThread, this), "AsyncTask"));
    m_pThread->start();
    return true;
}

void HttpLog::LogTaskThread() {
    while (!m_bExit) {
        m_pCond->Wait();
        auto str = m_sCmdInStream.str();
        m_pLogger.debug(str.c_str());
        m_sCmdInStream.str("");
        m_pCondWrite->Notify();
    }
}

void HttpLog::CheckLogBufferOverFlow() {
    // check last log contains \n
    std::string str = m_sCmdInStream.str();
    if (str.size() < MAX_BUF_SIZE) {
        return;
    }
    m_pCond->Notify();
    m_pCondWrite->Wait();
}