//
// Created by 侯鑫 on 2024/4/21.
//

#ifndef __TIMER_H_
#define __TIMER_H_

#include <csignal>
#include <cstdint>
#include <ctime>
#include <functional>

namespace ssp::base {

using TimerCallback = std::function<void(uintptr_t, uintptr_t)>;

class Timer {
public:
    Timer();

    ~Timer();

    void Start();

    void Stop();

    void Reset();

    void Pause();

    void Resume();

    void Tick();

    void SetInterval(int interval);

    void SetCallback(TimerCallback callback);

private:
    TimerCallback callback_;
    bool isRunning_;
    bool pasued_;
    struct sigevent event_;
};

} // namespace ssp::base

#endif //__TIMER_H_