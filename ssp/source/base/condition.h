//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_CONDITION_H
#define SSP_TOOLS_CONDITION_H

#include "mutex.h"

namespace ssp::base {
class Condition {
public:
    explicit Condition(MutexLock &lock);

    void Wait();

    [[maybe_unused]] bool WaitForTimeOut(double seconds);

    void Notify();

    void NotifyAll();

public:
    ~Condition();

private:
    pthread_cond_t cond_{};
    MutexLock &    lock_;
};
}

#endif //SSP_TOOLS_CONDITION_H
