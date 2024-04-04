//
// Created by 侯鑫 on 2024/1/28.
//

#include "kqueue_poller.h"
#include "base/log.h"
#include "base/system.h"
#include <unistd.h>

using namespace ssp::net;
using namespace ssp::base;

namespace {
constexpr uint32_t KQUEUE_MAX_EVT_SIZE = 100;
constexpr uint32_t KQUEUE_WAIT_TIMEOUT_SECONDS = 1;
}

KQueuePoller::KQueuePoller(EventLoop *loop)
    : Poller(loop)
{
    Start();
}

KQueuePoller::~KQueuePoller()
{
    if (kq_ != -1) {
        close(kq_);
    }

    if (socket_) {
        socket_->Close();
    }
}

void KQueuePoller::Start()
{
    kq_ = kqueue();
    if (kq_ == -1) {
        return;
    }

    socket_ = std::make_unique<Socket>();
    struct kevent event;
    EV_SET(&event, kq_, EVFILT_READ, EV_ADD, 0, 0, nullptr);
    if (kevent(kq_, &event, 1, nullptr, 0, nullptr) == -1) {
        log_sys.Warning("kevent listen failed. err:%s", System::GetErrMsg(errno));
    }

    events_.resize(KQUEUE_MAX_EVT_SIZE);
}

TimeStamp KQueuePoller::Poll(std::chrono::seconds, ChannelList &list)
{
    auto timeout = timespec{.tv_sec = KQUEUE_WAIT_TIMEOUT_SECONDS};
    auto readyCnt = kevent(kq_, nullptr, 0, events_.data(), KQUEUE_MAX_EVT_SIZE, &timeout);
    if (readyCnt == 0) {
        return TimeStamp::Now(); // no events
    } else if (readyCnt < 0) {
        log_sys.Warning("k event with error %s", System::GetErrMsg(errno));
        return TimeStamp::Now();
    }

    FillActiveChannel(readyCnt, list);
    return TimeStamp::Now();
}

void KQueuePoller::FillActiveChannel(int32_t nums, ChannelList &channels)
{
}

void KQueuePoller::UpdateChannel(ssp::net::Channel *channel)
{

}

void KQueuePoller::RemoveChannel(ssp::net::Channel *channel)
{

}