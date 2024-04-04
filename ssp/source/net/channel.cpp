//
// Created by 侯鑫 on 2024/1/7.
//

#include "channel.h"
#include "event_loop.h"
#include "base/timestamp.h"
#include <poll.h>

using namespace ssp::net;
using namespace ssp::base;

namespace {
constexpr uint32_t READ_EVENTS = POLLIN | POLLPRI;
constexpr uint32_t WRITE_EVENTS = POLLOUT;
constexpr uint32_t NONE_EVENTS = 0;
}

Channel::Channel(EventLoop *loop, Socket *socket)
    : loop_(loop)
    , channelFd_(socket->Fd())
{
}

void Channel::EnableRead()
{
    eventFlag_ |= READ_EVENTS;
    Update();
}

void Channel::EnableWrite()
{
    eventFlag_ |= WRITE_EVENTS;
    Update();
}

void Channel::Disabled()
{
    eventFlag_ |= NONE_EVENTS;
    Update();
}

void Channel::Remove()
{
    loop_->RemoveChannel(this);
}

void Channel::Update()
{
    loop_->UpdateChannel(this);
}

int32_t Channel::Index() const
{
    return index_;
}

void Channel::SetIndex(int32_t val)
{
    index_ = val;
}

int32_t Channel::Fd() const
{
    return channelFd_;
}

int32_t Channel::Events() const
{
    return eventFlag_;
}

void Channel::SetEvents(int32_t event)
{
    rcvEvent_ = event;
}

void Channel::SetChannelCallback(EventCallback readFn, EventCallback writeFn, EventCallback errFn,
                                 EventCallback closeFn)
{
    netFn_.readFn_ = std::move(readFn);
    netFn_.writeFn_ = std::move(writeFn);
    netFn_.errFn_ = std::move(errFn);
    netFn_.closeFn_ = std::move(closeFn);
}

void Channel::HandleEvent(const TimeStamp& stamp) const
{
    if ((rcvEvent_ & POLLHUP) && !(rcvEvent_ & POLLIN)) {
        if (netFn_.closeFn_) {
            netFn_.closeFn_(stamp);
        }
    }

    if (rcvEvent_ & (POLLERR | POLLNVAL)) {
        if (netFn_.errFn_) {
            netFn_.errFn_(stamp);
        }
    }

    if (rcvEvent_ & (POLLIN | POLLPRI
#if defined(LINUX)
    | POLLRDHUP
#endif
    )) {
        if (netFn_.readFn_) {
            netFn_.readFn_(stamp);
        }
    }

    if (rcvEvent_ & POLLOUT) {
        if (netFn_.writeFn_) {
            netFn_.writeFn_(stamp);
        }
    }
}
