#include "Thread.h"
#include "Format.h"
#include "Logging.h"
#include "ProcInfo.h"
#include "System.h"

using namespace muduo;
using namespace muduo::base;
using namespace muduo::base::detail;

ThreadContext::ThreadContext(ThreadFunc func, const std::string &name, pid_t *pid)
    : m_func(func)
    , m_strThreadName(name)
    , m_nPid(pid) {}

void ThreadContext::Run() {
    System::SetThreadName(m_strThreadName);
    m_func();
}

void *Thread::StartThread(void *arg) {
    if (arg == nullptr) {
        return nullptr;
    }

    // Reg default get the callstack of current thread sigaction
    ProcInfo::RegDefaultGetThreadAction();
    ThreadContext *_au = static_cast<ThreadContext *>(arg);
    _au->Run();
    delete _au;
    return nullptr;
}

std::atomic<int32_t> Thread::m_nThreadCnt;

Thread::Thread(ThreadFunc func, const std::string &name)
    : m_isStarted(false)
    , m_isJoined(false)
    , m_nThreadId(0)
    , m_nTid(0)
    , m_threadFunc(std::move(func))
    , m_strFunName(name) {}

Thread::~Thread() {
    if (m_isStarted && !m_isJoined) {
        pthread_detach(m_nThreadId);
    }
}

void Thread::SetDefaultName() {
    int num = m_nThreadCnt.fetch_add(1);
    if (m_strFunName.empty()) {
        m_strFunName = FmtString("Thread-%").arg(num).str();
    }
}

void Thread::Detached() {
    if (!m_isStarted) {
        throw ThreadException("thread is not started!");
    }

    auto ret = pthread_detach(m_nThreadId);
    LOG_IF(ret != 0).info("pthread_detached failed. ret:%s", ret);
}

void Thread::Start() {
    if (m_isStarted) {
        throw Exception("thread is started! Run Failed");
    }
    m_isStarted            = true;

    ThreadContext *context = new ThreadContext(m_threadFunc, m_strFunName, &m_nTid);
    auto           ret     = pthread_create(&m_nThreadId, nullptr, &Thread::StartThread, context);
    if (ret != 0) {
        logger.alert("pthread_create error ret:%d", ret);
        throw ThreadException("pthread create error");
    }
}

int Thread::Join() {
    if (!m_isStarted) {
        throw ThreadException("thread is not started.join failed");
    }
    if (m_isJoined) {
        throw ThreadException("thread is joined.");
    }
    m_isJoined = true;
    return pthread_join(m_nThreadId, nullptr);
}

bool Thread::IsStarted() { return m_isStarted; }

pid_t Thread::Tid() const { return System::Tid(); }

std::string Thread::Name() { return System::GetCurrentThreadName(); }

int Thread::NumCreated() { return 0; }
