#pragma once

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
    EventLoop *        m_pLoop;
    bool               m_bExited;
    Thread             m_thread;
    ThreadInitCallback m_callback;
};

} // namespace net
} // namespace muduo
