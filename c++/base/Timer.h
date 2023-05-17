#pragma once
#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include "nonecopyable.h"
#include <functional>
#include <memory>

namespace muduo::base {

using TimerCallback = std::function<void(uintptr_t, uint32_t)>;

class Timer : nonecopyable {
public:
    /**
     * @brief
     * @param: total 定时器总长度
     * @param: eachDelay 定时器间隔多少时间触发一次
     * @param: cb 定时器回调
     * */
    Timer(uint32_t total, double eachDelay, TimerCallback cb, uintptr_t para1, uint32_t para2);

    void Start();

    void Stop();

    ~Timer() { thread_->Join(); }

private:
    TimerCallback              cb_{nullptr};
    mutable MutexLock          lock_;
    std::unique_ptr<Condition> cond_{nullptr};
    std::unique_ptr<Thread>    thread_{nullptr};

    uint32_t total_{UINT32_MAX};
    double   eachDelay_; // ms

    bool exit_{false};

    struct Params {
        uintptr_t para1;
        uint32_t  para2;
    };

    Params userParam_;
};
} // namespace muduo::base
