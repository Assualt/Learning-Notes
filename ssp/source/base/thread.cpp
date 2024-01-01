//
// Created by 侯鑫 on 2024/1/1.
//

#include "thread.h"
#include "log.h"
#include "system.h"

using namespace ssp::base;

Thread::Thread(const ThreadFunc& func, const std::string &threadName)
    : context_(std::make_unique<ThreadContext>(threadName, func, nullptr, nullptr))
    , name_(threadName)
{
}

Thread::~Thread()
{
    if (threadId_ == 0) {
        context_->SetState(JOINED);
        pthread_join(threadId_, nullptr);
        context_->SetState(EXITED);
    }
}

void Thread::Start()
{
    if (context_->GetState() == ThreadState::RUNNING) {
        return;
    }

    context_->SetState(START_WAIT_RUN);
    auto ret = pthread_create(&threadId_, nullptr, &Thread::StartThread, context_.get());
    if (ret != 0) {
        log_sys.Warning("Start thread for name failed. ret:%u errMsg:%s", ret, System::GetErrMsg(errno));
        context_->SetState(EXITED);
    }
}

void Thread::Join()
{
    if (threadId_ == 0 || context_->GetState() == JOINED) {
        return;
    }

    context_->SetState(JOINED);
    pthread_join(threadId_, nullptr);
    context_->SetState(EXITED);
}

void Thread::Detached()
{
    if (threadId_ == nullptr || context_->GetState() == ThreadState::DETACHED) {
        return;
    }

    context_->SetState(DETACHED);
    pthread_detach(threadId_);
}

void* Thread::StartThread(void *arg)
{
    if (arg == nullptr) {
        return nullptr;
    }

    auto au = reinterpret_cast<ThreadContext*>(arg);
    if (au == nullptr) {
        au->SetState(EXITED);
        return nullptr;
    }

    au->Run();
    return nullptr;
}

ThreadState Thread::GetState()
{
    return context_->GetState();
}

std::string Thread::Name() const
{
    return context_->GetName();
}