//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_THREAD_H
#define SSP_TOOLS_THREAD_H

#include "thread_context.h"
#include <string>

namespace ssp::base {

class Thread {
public:
    Thread() = default;

    explicit Thread(const ThreadFunc& func, const std::string &threadName);

    ~Thread();

public:
    void Start();

    [[maybe_unused]] ThreadState GetState();

    void Join();

    [[maybe_unused]] void Detached();

    std::string Name() const;

private:
    static void* StartThread(void *arg);

private:
    std::unique_ptr<ThreadContext> context_ {nullptr};
    pthread_t threadId_ {nullptr};
    std::string name_;
};

}

#endif //SSP_TOOLS_THREAD_H
