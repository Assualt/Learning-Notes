#pragma once

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include "nonecopyable.h"
#include <deque>
#include <functional>
#include <mutex>
#include <vector>
namespace muduo {
namespace base {

class ThreadPool : nonecopyable {
public:
    using Task = std::function<void()>;

public:
    explicit ThreadPool(const std::string &name = "ThreadPool");
    ~ThreadPool();

public:
    void setMaxQueueSize(int maxSize);
    void setThreadInitCallBack(const Task &callback);

    void start(int numThreads);
    void stop();

    const std::string &getThreadName() const;
    size_t             queueSize() const;

    void run(Task func);

private:
    bool isFull();
    void runInThread();
    Task take();

private:
    std::deque<Task>                     m_dQueue;
    bool                                 m_bIsRunning;
    size_t                               m_nMaxQueueSize;
    std::vector<std::unique_ptr<Thread>> m_vThreads;
    Task                                 m_fThreadInitFunc;
    std::string                          m_strThreadPoolName;
    mutable MutexLock                    m_mutex;

    std::unique_ptr<Condition> m_notFullCond;
    std::unique_ptr<Condition> m_notEmptyCond;
};

} // namespace base
} // namespace muduo
