#include "Poller.h"
#include "Channel.h"
#include "EventLoop.h"
#include "poller/EPollPoller.h"
#include "poller/PollPoller.h"
#include <stdlib.h>
namespace muduo {
namespace net {

Poller::Poller(EventLoop *loop)
    : m_pLoop(loop) {
}

bool Poller::hasChannel(Channel *channel) const {
    ChannelMap::const_iterator it = m_mChannels.find(channel->fd());
    return it != m_mChannels.end() && it->second == channel;
}

Poller *Poller::newDefaultPoller(EventLoop *loop) {
    if (::getenv("MUDUO_USE_POLL")) {
        return new PollPoller(loop);
    } else {
        return new EPollPoller(loop);
    }
}

void Poller::assertInLoopThread() {
    m_pLoop->assertLoopThread();
}

Poller::ChannelList Poller::getEventTimeoutChannel() {
    ChannelList noEventList;
    for (auto &fd : m_noEventSet) {
        if (m_mChannels.find(fd) != m_mChannels.end()) {
            noEventList.push_back(m_mChannels[ fd ]);
        }
    }

    return noEventList;
}

} // namespace net
} // namespace muduo
