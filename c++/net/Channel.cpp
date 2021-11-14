#include "Channel.h"
#include "EventLoop.h"
#include <memory>
#include <poll.h>
namespace muduo {
namespace net {

const int Channel::kNoneEvent  = 0;
const int Channel::kReadEvent  = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : m_pLoop(loop)
    , m_nFD(fd)
    , m_nEvents(0)
    , m_nrecv_events(0)
    , m_bUseLock(false)
    , m_nIndex(-1) {
}

Channel::~Channel() {
}

EventLoop *Channel::ownerLoop() {
    return m_pLoop;
}
int Channel::fd() const {
    return m_nFD;
}

int Channel::events() const {
    return m_nEvents;
}
void Channel::update() {
    m_pLoop->updateChannel(this);
}

void Channel::handleEvent(const base::Timestamp &recvTime) {
    std::shared_ptr<void> guard;
    if (m_bUseLock) {
        guard = m_Lock.lock();
        if (guard)
            handleEventWithGuard(recvTime);
    } else {
        handleEventWithGuard(recvTime);
    }
}

void Channel::handleEventWithGuard(const base::Timestamp &recvTime) {
    if ((m_nrecv_events & POLLHUP) && !(m_nrecv_events & POLLIN)) {
        if (m_FCloseCallback)
            m_FCloseCallback();
    }
    if (m_nrecv_events & POLLNVAL) {
    }
    if (m_nrecv_events & (POLLERR | POLLNVAL)) {
        if (m_FErrorCallback)
            m_FErrorCallback();
    }
    if (m_nrecv_events & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (m_FReadCallback)
            m_FReadCallback(recvTime);
    }
    if (m_nrecv_events & POLLOUT) {
        if (m_FWriteCallback)
            m_FWriteCallback();
    }
}

} // namespace net
} // namespace muduo