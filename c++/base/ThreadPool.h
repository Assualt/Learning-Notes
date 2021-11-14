#pragma once

#include "Thread.h"
#include "nonecopyable.h"
#include <condition_variable>
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
    static std::mutex                    g_ThreadMutex;
    std::condition_variable              m_notFullCond;
    std::condition_variable              m_notEmptyCond;
};

} // namespace base
} // namespace muduo
