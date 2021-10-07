#pragma once

#include <map>
#include <vector>

#include "base/Timestamp.h"
#include "base/nonecopyable.h"

namespace muduo {
namespace net {

class Channel;
class EventLoop;
using ChannelMap = std::map<int, Channel *>;

class Poller : base::nonecopyable {
public:
    using ChannelList = std::vector<Channel *>;

    Poller(EventLoop *loop);
    virtual base::Timestamp poll(int timeout, ChannelList *activeChannels) = 0;

    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;
    virtual bool hasChannel(Channel *channel) const;

    static Poller *newDefaultPoller(EventLoop *loop);
    void assertInLoopThread();

protected:
    ChannelMap m_mChannels;

protected:
    EventLoop *m_pLoop;
};

} // namespace net
} // namespace muduo