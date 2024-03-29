#include "net/poller/EPollPoller.h"

#include "base/Logging.h"
#include "net/Channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;
// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

namespace {
const int kNew     = -1;
const int kAdded   = 1;
const int kDeleted = 2;
} // namespace

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        logger.info("EPollPoller::EPollPoller error");
    }
}

EPollPoller::~EPollPoller() { ::close(epollfd_); }

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels) {
    int       numEvents  = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int       savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        logger.debug("%d events happended", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        logger.debug("nothing happend");
    } else {
        // error happens, log uncommon ones
        if (savedErrno != EINTR) {
            errno = savedErrno;
            logger.warning("EPollPoller::poll() errno:%d, events:%d", errno, numEvents);
        }
    }
    return now;
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) {
    assert(static_cast<size_t>(numEvents) <= events_.size());
    std::set<int> eventSet;
    for (int i = 0; i < numEvents; ++i) {
        Channel *channel = static_cast<Channel *>(events_[ i ].data.ptr);
#ifndef NDEBUG
        int                        fd = channel->fd();
        ChannelMap::const_iterator it = m_mChannels.find(fd);
        assert(it != m_mChannels.end());
        assert(it->second == channel);
#endif
        channel->set_revents(events_[ i ].events);
        activeChannels->push_back(channel);
        eventSet.insert(channel->fd());
    }

    m_noEventSet.clear();
    for (auto channel : m_mChannels) {
        if (eventSet.count(channel.second->fd())) {
            continue;
        }

        if (channel.second->fd() == epollfd_) {
            continue;
        }
        m_noEventSet.insert(channel.second->fd());
    }
}

void EPollPoller::updateChannel(Channel *channel) {
    Poller::assertInLoopThread();
    const int index = channel->index();
    logger.debug("channel fd=%d, events=%d index=%d", channel->fd(), channel->events(), index);
    if (index == kNew || index == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew) {
            assert(m_mChannels.find(fd) == m_mChannels.end());
            m_mChannels[ fd ] = channel;
        } else // index == kDeleted
        {
            assert(m_mChannels.find(fd) != m_mChannels.end());
            assert(m_mChannels[ fd ] == channel);
        }

        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        (void)fd;
        assert(m_mChannels.find(fd) != m_mChannels.end());
        assert(m_mChannels[ fd ] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel) {
    Poller::assertInLoopThread();
    int fd = channel->fd();
    assert(m_mChannels.find(fd) != m_mChannels.end());
    assert(m_mChannels[ fd ] == channel);
    // assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = m_mChannels.erase(fd);
    (void)n;
    assert(n == 1);

    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}

void EPollPoller::update(int operation, Channel *channel) {
    struct epoll_event event;
    memset(&event, 0, sizeof event);
    event.events   = channel->events();
    event.data.ptr = channel;
    int fd         = channel->fd();
    LOG_SYSTEM.debug("epoll_ctl op:%s fd:%d event=%s", operationToString(operation), fd, channel->eventToString());
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_SYSTEM.error("epoll_ctl op =, %s fd = %d", operationToString(operation), fd);
        } else {
            LOG_SYSTEM.error("epoll_ctl op =, %s fd = %d", operationToString(operation), fd);
        }
    }
}

const char *EPollPoller::operationToString(int op) {
    switch (op) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
    }
}
