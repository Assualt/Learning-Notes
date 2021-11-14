#include "Thread.h"
// #include "Exception.h"
#include "Format.h"
#include "Logging.h"
#include <assert.h>
#include <sys/syscall.h>
#include <unistd.h>
namespace muduo {
namespace base {
namespace detail {

pid_t getTid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

struct ThreadContext {
    using ThreadFunc = muduo::base::Thread::ThreadFunc;
    ThreadFunc  m_func;
    std::string m_strFuncName;
    pid_t *     m_nPid{nullptr};

    ThreadContext(ThreadFunc func, const std::string &name, pid_t *pid)
        : m_func(func)
        , m_strFuncName(name)
        , m_nPid(pid) {
    }
    void run() {
        try {
            m_func();
        } catch (const Exception &e) {
            logger.alert("run in thread occur Exception. msg:%s", e.what());
            abort();
        } catch (...) {
            logger.alert("run in thread occur Unknown exception.");
            abort();
        }
    }
};

void *startThread(void *arg) {
    if (arg == nullptr) {
        return nullptr;
    }
    ThreadContext *_au = static_cast<ThreadContext *>(arg);
    _au->run();
    delete _au;
    return nullptr;
}

} // namespace detail

std::atomic<int32_t> Thread::m_nThreadCnt;

Thread::Thread(ThreadFunc func, const std::string &name)
    : m_IsStarted(false)
    , m_IsJoined(false)
    , m_nThreadId(0)
    , m_nTid(0)
    , m_threadFunc(std::move(func))
    , m_strFunName(name) {
}

Thread::~Thread() {
    if (m_IsStarted && !m_IsJoined) {
        pthread_detach(m_nThreadId);
    }
}

void Thread::setDefaultName() {
    int num = m_nThreadCnt.fetch_add(1);
    if (m_strFunName.empty()) {
        m_strFunName = FmtString("Thread-%").arg(num).str();
    }
}

void Thread::start() {
    if (m_IsStarted) {
        throw Exception("thread is started! Run Failed");
    }
    m_IsStarted                 = true;
    detail::ThreadContext *data = new detail::ThreadContext(m_threadFunc, m_strFunName, &m_nTid);
    auto                   ret  = pthread_create(&m_nThreadId, nullptr, &detail::startThread, data);
}

int Thread::join() {
    if (!m_IsStarted) {
        throw Exception("thread is not started.join failed");
    }
    if (m_IsJoined) {
        throw Exception("thread is joined.");
    }
    m_IsJoined = true;
    return pthread_join(m_nThreadId, nullptr);
}

} // namespace base
} // namespace muduo