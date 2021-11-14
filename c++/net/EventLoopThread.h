#pragma once

#include "base/nonecopyable.h"
#include <functional>

using muduo::base::nonecopyable;
namespace muduo {
namespace net {

using ThreadFunc = std::function<void(EventLoop *)>;

class EventLoop;
class EventLoopThread : nonecopyable {
public:
    EventLoopThread(const ThreadFunc &callback = ThreadFunc(), const std::string& name = "");

    ~EventLoopThread();
private:
    void threadFunc();


private:
    EventLoop *m_pLoop;
    bool m_bExited;
    
};

} // namespace net
} // namespace muduo
