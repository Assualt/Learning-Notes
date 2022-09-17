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
    void SetMaxQueueSize(int maxSize);

    void SetThreadInitCallBack(const Task &callback);

    void Start(int numThreads);

    void Stop();

    const std::string &GetThreadName() const;

    size_t QueueSize() const;

    void Run(Task func);

private:
    void WaitQueueForever();

    bool IsFull();

    void RunInThread();

    Task Take();

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
