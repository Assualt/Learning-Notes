//
// Created by 侯鑫 on 2024/1/3.
//

#include "event_loop.h"
#include "channel.h"
#include <chrono>

using namespace ssp::net;

std::vector<EventLoop> EventLoop::loopList_(100);

void EventLoop::Loop()
{
    ChannelList eventList{};
    state_ = APPLIED;
    quit_ = false;
    while (!quit_) {
        eventList.clear();
        auto stamp = poller_->Poll(std::chrono::seconds(1), eventList);
        DispatchEvent(stamp, eventList);
    }

    state_ = FREE;
    logger.Info("loop --> in exit");
}

void EventLoop::Quit()
{
    quit_ = true;
}

void EventLoop::DispatchEvent(const TimeStamp& stamp, ChannelList &channels)
{
    for (auto channel : channels) {
        channel->HandleEvent(stamp);
        channel->SetEvents(0);
    }
}

EventLoop::EventLoop()
    : poller_(Poller::NewPoller(this))
{
}

EventLoop::~EventLoop() = default;

void EventLoop::UpdateChannel(Channel *channel)
{
    poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(ssp::net::Channel *channel)
{
    poller_->RemoveChannel(channel);
}

LoopState EventLoop::GetState() const
{
    return state_;
}

EventLoop *EventLoop::ApplyLoop()
{
    for (auto & idx : loopList_) {
        if (idx.GetState() == FREE) {
            return &idx;
        }
    }

    return nullptr;
}