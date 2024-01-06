//
// Created by 侯鑫 on 2024/1/7.
//

#include "poller.h"
#include "poller/poll_poller.h"
#include "channel.h"

using namespace ssp::net;

Poller::Poller(EventLoop *loop)
{
}

Poller* Poller::NewPoller(EventLoop *loop)
{
    return new PollPoller(loop);
}
