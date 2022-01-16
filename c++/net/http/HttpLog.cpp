#include "HttpLog.h"
#include "base/LogHandle.h"
#include "base/Thread.h"

#define MAX_BUF_SIZE 100
using namespace muduo::base;

static std::shared_ptr<LogHandle> RollFileHandler(new RollingFileLogHandle("./", "access.log"));

HttpLog::HttpLog(Logger &log)
    : m_Logger(log) {
    m_Cond.reset(new Condition(m_mutex));
    m_CondWrite.reset(new Condition(m_mutexWrite));
}

bool HttpLog::Init() {
    m_Logger.addLogHandle(RollFileHandler.get());
    m_Logger.SetAppendLF(false);
    m_Thread.reset(new Thread(std::bind(&HttpLog::DoTaskObj, this), "AsyncTask"));
    m_Thread->start();
    return true;
}

void HttpLog::DoTaskObj()
{
    while (!m_bExit) {
        m_Cond->Wait();
        auto str = m_CmdInStream.str();
        m_Logger.debug(str.c_str());
        m_CmdInStream.str("");
        m_CondWrite->Notify();
    }
}

void HttpLog::CheckBufOverFilled()
{
    // check last log contains \n
    std::string str = m_CmdInStream.str();
    if (str.size() < MAX_BUF_SIZE) {
        return;
    }
    m_Cond->Notify();
    m_CondWrite->Wait();
}

HttpLog& HttpLog::operator<<(short val)
{
    m_CmdInStream << val;
    CheckBufOverFilled();
    return *this;
}
HttpLog& HttpLog::operator<<(unsigned short val)
{
    m_CmdInStream << val;
    CheckBufOverFilled();
    return *this;
}
HttpLog& HttpLog::operator<<(int val)
{
    m_CmdInStream << val;
    CheckBufOverFilled();
    return *this;
}
HttpLog& HttpLog::operator<<(unsigned int val)
{
    m_CmdInStream << val;
    CheckBufOverFilled();
    return *this;
}
HttpLog& HttpLog::operator<<(long val)
{
    m_CmdInStream << val;
    CheckBufOverFilled();
    return *this;
}
HttpLog& HttpLog::operator<<(unsigned long val)
{
    m_CmdInStream << val;
    CheckBufOverFilled();
    return *this;
}
HttpLog& HttpLog::operator<<(long long val)
{
    m_CmdInStream << val;
    CheckBufOverFilled();
    return *this;
}
HttpLog& HttpLog::operator<<(unsigned long long val)
{
    m_CmdInStream << val;
    CheckBufOverFilled();
    return *this;
}
HttpLog& HttpLog::operator<<(const char *out)
{
    m_CmdInStream.write(out, strlen(out));
    CheckBufOverFilled();
    return *this;
}
HttpLog& HttpLog::operator<<(const std::string &val)
{
    m_CmdInStream << val;
    CheckBufOverFilled();
    return *this;
}
HttpLog &HttpLog::Write(const std::string &val)
{
    m_CmdInStream << val;
    CheckBufOverFilled();
    return *this;
}