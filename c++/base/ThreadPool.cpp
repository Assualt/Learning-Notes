#include "Format.h"
#include "Logging.h"
#include "ThreadPool.h"
namespace muduo {
namespace base {

std::mutex ThreadPool::g_ThreadMutex;

ThreadPool::ThreadPool(const std::string &name)
    : m_strThreadPoolName(name)
    , m_bIsRunning(false)
    , m_nMaxQueueSize(0) {
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
        std::lock_guard<std::mutex> guard(g_ThreadMutex);
        m_bIsRunning = false;
        m_notEmptyCond.notify_all();
        m_notFullCond.notify_all();
    }
    {
        for (auto &thread : m_vThreads) {
            thread->join();
        }
    }
}

size_t ThreadPool::queueSize() const {
    std::lock_guard<std::mutex> guard(g_ThreadMutex);
    return m_dQueue.size();
}

void ThreadPool::run(Task func) {
    std::unique_lock<std::mutex> guard(g_ThreadMutex);
    if (m_vThreads.empty()) {
        func();
    } else {
        while (isFull() && m_bIsRunning) {
            m_notFullCond.wait(guard);
        }
        if (!m_bIsRunning) {
            return;
        }
        m_dQueue.push_back(std::move(func));
        m_notEmptyCond.notify_one();
    }
}

ThreadPool::Task ThreadPool::take() {
    std::unique_lock<std::mutex> guard(g_ThreadMutex);
    while (m_dQueue.empty() && m_bIsRunning) {
        m_notEmptyCond.wait(guard);
    }
    Task t;
    while (!m_dQueue.empty()) {
        t = m_dQueue.front();
        m_dQueue.pop_front();
        if (m_nMaxQueueSize > 0) {
            m_notEmptyCond.notify_one();
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