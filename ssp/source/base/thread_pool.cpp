//
// Created by 侯鑫 on 2024/1/1.
//

#include "thread_pool.h"
#include "format.h"
#include "log.h"
#include <utility>
#include <algorithm>

using namespace ssp::base;

ThreadPool::ThreadPool(std::string name)
    : threadPoolName_(std::move(name))
    , isRunning_(false)
    , maxQueueSize_(0)
    , notEmptyCond_(std::make_unique<Condition>( mutex_))
    , noFullCond_(std::make_unique<Condition>(mutex_))
{
}

ThreadPool::~ThreadPool()
{
    if (isRunning_) {
        Stop();
    }
}

const std::string &ThreadPool::GetThreadName() const
{
    return threadPoolName_;
}

void ThreadPool::SetThreadInitCallBack(const Task &callback)
{
    threadInitFunc_ = callback;
}

void ThreadPool::SetMaxQueueSize(int maxSize)
{
    maxQueueSize_ = maxSize;
}

void ThreadPool::Start(int numThreads, const std::string &prefix)
{
    threadVecs_.reserve(numThreads);
    isRunning_ = true;
    for (int i = 0; i < numThreads; ++i) {
        std::string threadName = FmtString("%-%").arg(prefix).arg(i).str();
        threadVecs_.emplace_back(new Thread([this](auto val) { RunInThread(val); }, threadName));
        threadVecs_[ i ]->Start();
    }
    if (numThreads == 0 && threadInitFunc_) {
        threadInitFunc_();
    }
}

void ThreadPool::Stop() 
{
    {
        WaitQueueForever();
        AutoLock myLock(mutex_);
        isRunning_ = false;
        logger.Info("task to notify all");
        notEmptyCond_->NotifyAll();
        noFullCond_->NotifyAll();
    }
    {
        for (auto &thread : threadVecs_) {
            thread->Join();
        }
    }
}

size_t ThreadPool::QueueSize() const
{
    AutoLock myLock(mutex_);
    return taskQueue_.size();
}

void ThreadPool::Run(Task func)
{
    if (threadVecs_.empty()) {
        func();
        return;
    }

    AutoLock myLock(mutex_);
    while (IsFull() && isRunning_) {
        noFullCond_->Wait();
    }
    if (!isRunning_) {
        return;
    }
    taskQueue_.push_back(std::move(func));
    notEmptyCond_->Notify();
}

ThreadPool::Task ThreadPool::Take()
{
    AutoLock myLock(mutex_);
    while (taskQueue_.empty()) {
        if (!isRunning_) {
            break;
        }
        notEmptyCond_->Wait();
    }
    Task t;
    if (!taskQueue_.empty()) {
        t = taskQueue_.front();
        taskQueue_.pop_front();
        if (maxQueueSize_ > 0) {
            noFullCond_->Notify();
        }
    }
    return t;
}

void ThreadPool::WaitQueueForever()
{
    while (isRunning_) {
        size_t size;
        {
            AutoLock lock(mutex_);
            size = taskQueue_.size();
        }
        if (size > 0) {
            notEmptyCond_->Notify();
        } else {
            break;
        }
    }
}

bool ThreadPool::IsFull()
{
    return maxQueueSize_ > 0 && taskQueue_.size() >= maxQueueSize_;
}

void ThreadPool::RunInThread(uintptr_t)
{
    try {
        if (threadInitFunc_) {
            threadInitFunc_();
        }
        while (isRunning_) {
            Task task(Take());
            if (task) {
                task();
            }
        }
        logger.Warning("thread exit ...");
    } catch (const std::exception &e) {
        logger.Alert("cause exception in here. e:%s", e.what());
        abort();
    } catch (...) {
        logger.Alert("cause unknown exception in here.");
        throw;
    }
}

void ThreadPool::Dump(std::ostream& os)
{
    os << ">>> ThreadPool:(" << GetThreadName() << ")" << std::endl;
    os << "   >> thread:" << threadVecs_.size() << " task:" << QueueSize() << std::endl;
    for (uint32_t idx = 0; idx < threadVecs_.size(); ++idx) {
        os << "    [" << ToFixedString(idx, 3, false) << "]"
           << threadVecs_[idx]->Name() << " ==> state:"
           << threadVecs_[idx]->GetState() << std::endl;
    }
}
