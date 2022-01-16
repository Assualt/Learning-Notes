//
// Created by Sunshine on 2021/3/9.
//
#pragma once

#include "base/Timestamp.h"
#include "base/nonecopyable.h"
#include <atomic>
#include <functional>
#include <vector>

#include "Poller.h"
#include <memory>
using muduo::base::nonecopyable;
using muduo::base::Timestamp;
namespace muduo {
namespace net {

class Channel;
class TimerQueue;
class EventLoop : nonecopyable {

public:
    typedef std::function<void()>  Functor;
    typedef std::vector<Channel *> ChannelList;
    EventLoop();
    ~EventLoop();

    // Loops forever
    void loop();

    // Quit Loop
    void quit();

    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    void runInLoop(Functor callback);
    void queueInLoop(Functor cb);
    void assertLoopThread();
    void wakeup();
    void handleRead();

    bool isInLoopThread() const;
    void printActiveChannels() const;

protected:
    void doPendingFunctors();

protected:
    bool m_bLooping;
    bool m_bQuit;
    bool m_bEventHanding;

    int m_nWakeUpFD;

    ChannelList              m_vActiveChannels;
    Channel *                m_pCurrentChannel;
    Timestamp                m_tRecvTimeStamp;
    std::unique_ptr<Poller>  m_Poller;
    std::vector<Functor>     m_vPendingFunctors;
    bool                     m_bCallFuncs;
    pid_t                    m_nThreadId;
    std::unique_ptr<Channel> m_wakeupChannel;
};
} // namespace net
} // namespace muduo
