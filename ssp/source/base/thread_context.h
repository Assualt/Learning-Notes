//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_THREAD_CONTEXT_H
#define SSP_TOOLS_THREAD_CONTEXT_H

#include <functional>
#include <string>

namespace ssp::base {

using ThreadFunc = std::function<void(uintptr_t)>;
using ThreadBeforeRunHook = std::function<void(uintptr_t)>;
using ThreadBeforeExitHook = std::function<void(uintptr_t)>;
using ThreadStateFunc = std::function<void(uint32_t, uint32_t)>;

enum ThreadState : uint32_t {
    NOT_START,
    START_WAIT_RUN,
    RUNNING,
    JOINED,
    DETACHED,
    EXITED,
};

class ThreadContext {
public:
    ThreadContext() = default;
    ~ThreadContext() = default;

    explicit ThreadContext(std::string name, ThreadFunc func, ThreadBeforeRunHook beforeFunc = nullptr, ThreadBeforeExitHook exitHook = nullptr);

    void Run();

    void SetState(ThreadState state);

    std::string GetName() const;

    [[nodiscard]] ThreadState GetState() const;

    static void SetThreadStateChangeHook(ThreadStateFunc func);

private:
    ThreadFunc func_ {nullptr};
    ThreadBeforeRunHook runFunc_ {nullptr};
    ThreadBeforeExitHook exitFunc_ {nullptr};
    ThreadState state_{ThreadState::NOT_START};
    std::string threadName_;
};

}


#endif //SSP_TOOLS_THREAD_CONTEXT_H
