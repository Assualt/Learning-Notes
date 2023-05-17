#pragma once

#include "base/Condition.h"
#include "base/Mutex.h"
#include "base/Thread.h"
#include "base/nonecopyable.h"
#include <functional>
using namespace muduo::base;
using muduo::base::nonecopyable;

namespace muduo {
namespace net {
class EventLoop;

typedef std::function<void(EventLoop *)> ThreadInitCallback;
class EventLoopThread : nonecopyable {
public:
    EventLoopThread(const ThreadInitCallback &callback = ThreadInitCallback(), const std::string &name = "");

    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunc();

private:
    EventLoop *                m_pLoop;
    bool                       m_bExited;
    Thread                     m_thread;
    ThreadInitCallback         m_callback;
    MutexLock                  m_mutex;
    std::unique_ptr<Condition> m_pCond;
};

} // namespace net
} // namespace muduo
