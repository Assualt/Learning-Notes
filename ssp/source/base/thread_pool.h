//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_THREAD_POOL_H
#define SSP_TOOLS_THREAD_POOL_H

#include "object.h"
#include "thread.h"
#include "condition.h"
#include <memory>
#include <deque>

namespace ssp::base {
class ThreadPool : Object {
public:
public:
    using Task = std::function<void(uintptr_t)>;

public:
    ThreadPool() = default;

    explicit ThreadPool(std::string name = "ThreadPool");

    ~ThreadPool() override;

    void Dump(std::ostream& os) override;

public:
    void SetMaxQueueSize(int maxSize);

    void SetThreadInitCallBack(const Task &callback);

    void Start(int numThreads, const std::string &prefix = "Thread");

    void Stop();

    const std::string &GetThreadName() const;

    size_t QueueSize() const;

    void Run(Task func);

private:
    void WaitQueueForever();

    bool IsFull();

    void RunInThread(uintptr_t val);

    Task Take();

private:
    std::deque<Task>                     taskQueue_;
    bool                                 isRunning_{false};
    size_t                               maxQueueSize_{};
    std::vector<std::unique_ptr<Thread>> threadVecs_;
    Task                                 threadInitFunc_;
    std::string                          threadPoolName_;
    mutable MutexLock                    mutex_;

    std::unique_ptr<Condition> noFullCond_;
    std::unique_ptr<Condition> notEmptyCond_;

};
}


#endif //SSP_TOOLS_THREAD_POOL_H
