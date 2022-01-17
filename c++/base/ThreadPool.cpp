#include "ThreadPool.h"
#include "Format.h"
#include "Logging.h"
namespace muduo {
namespace base {

ThreadPool::ThreadPool(const std::string &name)
    : m_strThreadPoolName(name)
    , m_bIsRunning(false)
    , m_nMaxQueueSize(0)
    , m_notEmptyCond(new Condition(m_mutex))
    , m_notFullCond(new Condition(m_mutex)) {
}

ThreadPool::~ThreadPool() {
    if (m_bIsRunning) {
        stop();
    }
}

const std::string &ThreadPool::getThreadName() const {
    return m_strThreadPoolName;
}

void ThreadPool::setThreadInitCallBack(const Task &callback) {
    m_fThreadInitFunc = callback;
}

void ThreadPool::setMaxQueueSize(int maxSize) {
    m_nMaxQueueSize = maxSize;
}

void ThreadPool::start(int numThreads) {
    m_vThreads.reserve(numThreads);
    m_bIsRunning = true;
    for (int i = 0; i < numThreads; ++i) {
        std::string threadName = FmtString("Thread%").arg(i).str();
        m_vThreads.emplace_back(new Thread(std::bind(&ThreadPool::runInThread, this), threadName));
        m_vThreads[ i ]->start();
    }
    if (numThreads == 0 && m_fThreadInitFunc) {
        m_fThreadInitFunc();
    }
}

void ThreadPool::stop() {
    {
        AutoLock myLock(m_mutex);
        m_bIsRunning = false;
        m_notEmptyCond->NotifyAll();
        m_notFullCond->NotifyAll();
    }
    {
        for (auto &thread : m_vThreads) {
            thread->join();
        }
    }
}

size_t ThreadPool::queueSize() const {
    AutoLock myLock(m_mutex);
    return m_dQueue.size();
}

void ThreadPool::run(Task func) {
    AutoLock myLock(m_mutex);
    if (m_vThreads.empty()) {
        func();
    } else {
        while (isFull() && m_bIsRunning) {
            m_notFullCond->Wait();
        }
        if (!m_bIsRunning) {
            return;
        }
        m_dQueue.push_back(std::move(func));
        m_notEmptyCond->Notify();
    }
}

ThreadPool::Task ThreadPool::take() {
    AutoLock myLock(m_mutex);
    while (m_dQueue.empty() && m_bIsRunning) {
        m_notEmptyCond->Wait();
    }
    Task t;
    while (!m_dQueue.empty()) {
        t = m_dQueue.front();
        m_dQueue.pop_front();
        if (m_nMaxQueueSize > 0) {
            m_notEmptyCond->Notify();
        }
    }
    return t;
}

bool ThreadPool::isFull() {
    return m_nMaxQueueSize > 0 && m_dQueue.size() >= m_nMaxQueueSize;
}

void ThreadPool::runInThread() {
    try {
        if (m_fThreadInitFunc) {
            m_fThreadInitFunc();
        }
        while (m_bIsRunning) {
            Task task(take());
            if (task) {
                task();
            }
        }
    } catch (const std::exception &e) {
        logger.alert("cause exception in here. e:%s", e.what());
        abort();
    } catch (...) {
        logger.alert("cause unknown exception in here.");
        throw;
    }
}

} // namespace base
} // namespace muduo