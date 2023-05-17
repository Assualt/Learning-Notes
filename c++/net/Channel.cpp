#include "Channel.h"
#include "EventLoop.h"
#include <memory>
#include <poll.h>
#include <sstream>
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
    , m_nIndex(-1) {}

Channel::~Channel() {}

EventLoop *Channel::ownerLoop() { return m_pLoop; }
int        Channel::fd() const { return m_nFD; }

int  Channel::events() const { return m_nEvents; }
void Channel::update() { m_pLoop->updateChannel(this); }

void Channel::remove() { m_pLoop->removeChannel(this); }

void Channel::doReadTimeOutFunc() {
    if (m_ReadTimeoutCallback) {
        m_ReadTimeoutCallback();
    }
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

std::string Channel::reventsToString() const { return eventsToString(m_nFD, m_nrecv_events); }

std::string Channel::eventToString() const { return eventsToString(m_nFD, m_nEvents); }

std::string Channel::eventsToString(int fd, int ev) const {
    std::stringstream oss;
    oss << fd << ": ";
    if (ev & POLLIN)
        oss << "IN ";
    if (ev & POLLPRI)
        oss << "PRI ";
    if (ev & POLLOUT)
        oss << "OUT ";
    if (ev & POLLHUP)
        oss << "HUP ";
    if (ev & POLLRDHUP)
        oss << "RDHUP ";
    if (ev & POLLERR)
        oss << "ERR ";
    if (ev & POLLNVAL)
        oss << "NVAL ";
    return oss.str();
}

} // namespace net
} // namespace muduo