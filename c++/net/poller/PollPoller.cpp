#include "PollPoller.h"

#include "base/Logging.h"
#include "net/Channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>

using namespace muduo;
using namespace muduo::net;

PollPoller::PollPoller(EventLoop *loop)
    : Poller(loop) {}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
    // XXX pollfds_ shouldn't change
    int       numEvents  = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    int       savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        logger.info("%d events happened", numEvents);
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        logger.debug(" nothing happened");
    } else {
        if (savedErrno != EINTR) {
            errno = savedErrno;
            logger.info("PollPoller::poll() errno:%d", errno);
        }
    }
    return now;
}

void PollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const {
    for (PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd) {
        if (pfd->revents > 0) {
            --numEvents;
            ChannelMap::const_iterator ch = m_mChannels.find(pfd->fd);
            assert(ch != m_mChannels.end());
            Channel *channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            // pfd->revents = 0;
            activeChannels->push_back(channel);
        }
    }
}

void PollPoller::updateChannel(Channel *channel) {
    Poller::assertInLoopThread();
    logger.info("fd = %d event=%d", channel->fd(), channel->events());
    if (channel->index() < 0) {
        // a new one, add to pollfds_
        assert(m_mChannels.find(channel->fd()) == m_mChannels.end());
        struct pollfd pfd;
        pfd.fd      = channel->fd();
        pfd.events  = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->setIndex(idx);
        m_mChannels[ pfd.fd ] = channel;
    } else {
        // update existing one
        assert(m_mChannels.find(channel->fd()) != m_mChannels.end());
        assert(m_mChannels[ channel->fd() ] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[ idx ];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
        pfd.fd      = channel->fd();
        pfd.events  = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent()) {
            // ignore this pollfd
            pfd.fd = -channel->fd() - 1;
        }
    }
}

void PollPoller::removeChannel(Channel *channel) {
    Poller::assertInLoopThread();
    logger.info("fd = %d", channel->fd());
    assert(m_mChannels.find(channel->fd()) != m_mChannels.end());
    assert(m_mChannels[ channel->fd() ] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd &pfd = pollfds_[ idx ];
    (void)pfd;
    assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());
    size_t n = m_mChannels.erase(channel->fd());
    assert(n == 1);
    (void)n;
    if (static_cast<size_t>(idx) == pollfds_.size() - 1) {
        pollfds_.pop_back();
    } else {
        int channelAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
        if (channelAtEnd < 0) {
            channelAtEnd = -channelAtEnd - 1;
        }
        m_mChannels[ channelAtEnd ]->setIndex(idx);
        pollfds_.pop_back();
    }
}
