//
// Created by 侯鑫 on 2024/1/1.
//

#include "thread_context.h"
#include <utility>
#include "mutex.h"
#include "system.h"

using namespace ssp::base;

namespace {
ThreadStateFunc g_stateChangeHook_ {nullptr};
MutexLock g_stateChangeLock_;
}

ThreadState ThreadContext::GetState() const
{
    return state_;
}

ThreadContext::ThreadContext(std::string name,ThreadFunc func, ThreadBeforeRunHook runFunc, ThreadBeforeExitHook exitHook)
    :threadName_(std::move(name)), func_(std::move(func)), runFunc_(std::move(runFunc)), exitFunc_(std::move(exitHook))
{
}

void ThreadContext::Run()
{
    System::SetThreadName(threadName_);
    if (runFunc_ != nullptr) {
        runFunc_(reinterpret_cast<uintptr_t>(this));
    }

    SetState(RUNNING);
    if (func_ != nullptr) {
        func_(reinterpret_cast<uintptr_t>(this));
    }

    if (exitFunc_ != nullptr) {
        exitFunc_(reinterpret_cast<uintptr_t>(this));
    }

    SetState(EXITED);
}

void ThreadContext::SetState(ThreadState state)
{
    if (state_ != state && g_stateChangeHook_ != nullptr) {
        g_stateChangeHook_(state_, state);
    }

    g_stateChangeLock_.Lock();
    state_ = state;
    g_stateChangeLock_.UnLock();
}

void ThreadContext::SetThreadStateChangeHook(ThreadStateFunc func)
{
    g_stateChangeHook_ = std::move(func);
}

std::string ThreadContext::GetName() const
{
    return threadName_;
}