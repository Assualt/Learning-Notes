//
// Created by 侯鑫 on 2024/4/21.
//

#include "timer.h"

void Timer::start()
{
    start_time_ = std::chrono::steady_clock::now();
}

void Timer::stop()
{
    end_time_ = std::chrono::steady_clock::now();
}

double Timer::elapsed() const
{
    return std::chrono::duration_cast<std::chrono::microseconds>(end_time_ - start_time_).count();
}