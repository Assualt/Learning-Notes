//
// Created by xhou on 2022/8/28.
//

#include "Timer.h"
#include "Timestamp.h"
using namespace muduo::base;

Timer::Timer(uint32_t total, double eachDelay, TimerCallback cb, uintptr_t para1, uint32_t para2)
    : cond_(std::make_unique<Condition>(lock_))
    , cb_(cb)
    , total_(total)
    , eachDelay_(eachDelay) {
    userParam_.para1 = para1;
    userParam_.para2 = para2;
}

void Timer::Start() {
    auto predFunc = [this]() {
        auto begin = Timestamp::now();
        while (!exit_) {
            cb_(userParam_.para1, userParam_.para2);
            cond_->WaitForTimeOut(eachDelay_);
            auto curNow = Timestamp::now();
            if (total_ != UINT32_MAX && curNow.seconds() - begin.seconds() >= total_) {
                exit_ = true; // 退出
            }
        }
    };

    thread_ = std::make_unique<Thread>(predFunc);

    thread_->Start();
}

void Timer::Stop() { exit_ = true; }