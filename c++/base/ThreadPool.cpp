#include "ThreadPool.h"
#include "Format.h"
#include "Logging.h"
using namespace muduo;
using namespace muduo::base;

ThreadPool::ThreadPool(const std::string &name)
    : m_strThreadPoolName(name)
    , m_bIsRunning(false)
    , m_nMaxQueueSize(0)
    , m_notEmptyCond(new Condition(m_mutex))
    , m_notFullCond(new Condition(m_mutex)) {}

ThreadPool::~ThreadPool() {
    if (m_bIsRunning) {
        Stop();
    }
}

const std::string &ThreadPool::GetThreadName() const { return m_strThreadPoolName; }

void ThreadPool::SetThreadInitCallBack(const Task &callback) { m_fThreadInitFunc = callback; }

void ThreadPool::SetMaxQueueSize(int maxSize) { m_nMaxQueueSize = maxSize; }

void ThreadPool::Start(int numThreads) {
    m_vThreads.reserve(numThreads);
    m_bIsRunning = true;
    for (int i = 0; i < numThreads; ++i) {
        std::string threadName = FmtString("Thread%").arg(i).str();
        m_vThreads.emplace_back(new Thread(std::bind(&ThreadPool::RunInThread, this), threadName));
        m_vThreads[ i ]->Start();
    }
    if (numThreads == 0 && m_fThreadInitFunc) {
        m_fThreadInitFunc();
    }
}

void ThreadPool::Stop() {
    {
        WaitQueueForever();
        AutoLock myLock(m_mutex);
        m_bIsRunning = false;
        m_notEmptyCond->NotifyAll();
        m_notFullCond->NotifyAll();
    }
    {
        for (auto &thread : m_vThreads) {
            thread->Join();
        }
    }
}

size_t ThreadPool::QueueSize() const {
    AutoLock myLock(m_mutex);
    return m_dQueue.size();
}

void ThreadPool::Run(Task func) {
    if (m_vThreads.empty()) {
        func();
    } else {
        AutoLock myLock(m_mutex);
        while (IsFull() && m_bIsRunning) {
            m_notFullCond->Wait();
        }
        if (!m_bIsRunning) {
            return;
        }
        m_dQueue.push_back(std::move(func));
        m_notEmptyCond->Notify();
    }
}

ThreadPool::Task ThreadPool::Take() {
    AutoLock myLock(m_mutex);
    while (m_dQueue.empty()) {
        m_notEmptyCond->Wait();
    }
    Task t;
    if (!m_dQueue.empty()) {
        t = m_dQueue.front();
        m_dQueue.pop_front();
        if (m_nMaxQueueSize > 0) {
            m_notFullCond->Notify();
        }
    }
    return t;
}

void ThreadPool::WaitQueueForever() {
    while (m_bIsRunning) {
        int size;
        {
            AutoLock lock(m_mutex);
            size = m_dQueue.size();
        }
        if (size > 0) {
            m_notEmptyCond->Notify();
        } else {
            break;
        }
    }
}

bool ThreadPool::IsFull() { return m_nMaxQueueSize > 0 && m_dQueue.size() >= m_nMaxQueueSize; }

void ThreadPool::RunInThread() {
    try {
        if (m_fThreadInitFunc) {
            m_fThreadInitFunc();
        }
        while (m_bIsRunning) {
            Task task(Take());
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