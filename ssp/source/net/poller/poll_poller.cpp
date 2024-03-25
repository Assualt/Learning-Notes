//
// Created by 侯鑫 on 2024/1/6.
//

#include "poll_poller.h"
#include "base/log.h"
#include "base/system.h"
#include "base/timestamp.h"
#include "net/channel.h"

using namespace ssp::net;
using namespace ssp::base;

PollPoller::PollPoller(EventLoop *loop)
    : Poller(loop)
{}

PollPoller::~PollPoller() = default;

TimeStamp PollPoller::Poll(std::chrono::seconds timeout, ChannelList &channels)
{
    auto eventNums = ::poll(&*pollFdList_.begin(), pollFdList_.size(), static_cast<int>(timeout.count()));
    if (eventNums == 0) {
        return TimeStamp::Now();
    } else if (eventNums < 0) {
        log_sys.Warning("poll events with error. errno:%d", System::GetErrMsg(errno));
        return TimeStamp::Now();
    }

    FillActiveChannel(eventNums, channels);
    return TimeStamp::Now();
}

void PollPoller::FillActiveChannel(int32_t numEvents, ChannelList &channels)
{
    for (auto item : pollFdList_) {
        if (item.revents < 0) {
            continue;
        }

        auto iter = channelMapper_.find(item.fd);
        if (iter == channelMapper_.end()) {
            continue;
        }

        iter->second->SetEvents(item.events);
        channels.push_back(iter->second);
    }
}

void PollPoller::UpdateChannel(Channel *channel)
{
    auto idx = channel->Index();
    if (idx < 0) { // first come
        auto &pollFd = pollFdList_.emplace_back();
        pollFd.fd = channel->Fd(),
        pollFd.events = static_cast<short>(channel->Events());
        pollFd.revents = 0;
        channel->SetIndex(static_cast<int32_t>(pollFdList_.size() - 1));
        channelMapper_.emplace(pollFd.fd, channel);
        return;
    }

    pollFdList_[idx].fd = channel->Fd();
    pollFdList_[idx].events = static_cast<short>(channel->Events());
    pollFdList_[idx].revents = 0;
}

void PollPoller::RemoveChannel(ssp::net::Channel *channel)
{
    auto idx = channel->Index();
    channelMapper_.erase(channel->Fd());
    if (idx == pollFdList_.size() - 1) {
        pollFdList_.pop_back();
        return;
    }

    auto endFd = pollFdList_.end()->fd;
    iter_swap(pollFdList_.begin() + idx, pollFdList_.end() - 1);
    if (endFd < 0) {
        endFd = -endFd - 1;
    }

    channelMapper_[endFd]->SetIndex(idx);
    pollFdList_.pop_back();
}