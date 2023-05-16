//
// Created by Sunshine on 2021/3/9.
//
#pragma once

#include "Channel.h"
#include "base/Mutex.h"
#include "base/Timestamp.h"
#include "base/nonecopyable.h"
#include <atomic>
#include <functional>
#include <vector>

#include "Poller.h"
#include <memory>

using muduo::base::MutexLock;
using muduo::base::nonecopyable;
using muduo::base::Timestamp;

namespace muduo::net {

class EventLoop : nonecopyable {

public:
    typedef std::function<void()>  Functor;
    typedef std::vector<Channel *> ChannelList;

    EventLoop();

    ~EventLoop();

    // Loops forever
    void loop();

    // Quit Loop
    void quit() const;

    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

    [[maybe_unused]] bool hasChannel(Channel *channel);

    void runInLoop(Functor callback);

    void queueInLoop(Functor cb);

    void assertLoopThread();

    void wakeup() const;

    void handleRead() const;

    bool isInLoopThread() const;

    [[maybe_unused]] void printActiveChannels() const;

protected:
    void doPendingFunctors();

    void clearReadTimeoutChannel();

protected:
    bool m_bLooping;
    bool m_bQuit;
    bool m_bEventHanding;

    int m_nWakeUpFD;

    ChannelList              m_vActiveChannels;
    Channel                 *m_pCurrentChannel;
    Timestamp                m_tRecvTimeStamp;
    std::unique_ptr<Poller>  m_Poller{nullptr};
    std::vector<Functor>     m_vPendingFunctors;
    bool                     m_bCallFuncs;
    pid_t                    m_nThreadId;
    std::unique_ptr<Channel> m_wakeupChannel{nullptr};
};
} // namespace muduo::net
