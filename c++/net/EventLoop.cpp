#include "CurrentThread.h"
#include "EventLoop.h"
#include "SocketsOp.h"
#include "base/Logging.h"
#include <algorithm>
#include <cassert>
#include <sys/eventfd.h>
#include <unistd.h>

namespace muduo::net {
using muduo::net::Channel;

constexpr int kPollTimeMs   = 10000;
constexpr int kNoEventTimes = 2;

int CreateEventFd() {
    int evtFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtFd < 0) {
        abort();
    }
    return evtFd;
}

EventLoop::EventLoop()
    : m_bLooping(false)
    , m_bQuit(false)
    , m_bEventHanding(false)
    , m_nWakeUpFD(CreateEventFd())
    , m_nThreadId(CurrentThread::tid())
    , m_pCurrentChannel(nullptr)
    , m_bCallFuncs(false)
    , m_Poller(Poller::newDefaultPoller(this))
    , m_wakeupChannel(new Channel(this, m_nWakeUpFD)) {

    m_wakeupChannel->setReadCallback([this](const base::Timestamp &) { handleRead(); });
    m_wakeupChannel->enableReading();
}

EventLoop::~EventLoop() { ::close(m_nWakeUpFD); }

void EventLoop::runInLoop(Functor cb) {
    if (m_nThreadId == CurrentThread::tid()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb) {
    m_vPendingFunctors.push_back(std::move(cb));
    if (m_nThreadId == CurrentThread::tid()) {
        wakeup();
    }
}

void EventLoop::quit() const {
    if (m_nThreadId == CurrentThread::tid()) {
        wakeup();
    }
}

void EventLoop::wakeup() const {
    uint64_t one = 1;
    ssize_t  n   = sockets::write(m_nWakeUpFD, &one, sizeof one);
    if (n != sizeof one) {
        logger.info("EventLoop::wakeup() writes  bytes instead of 8");
    }
}

void EventLoop::clearReadTimeoutChannel() {
    static int readTimeoutCnt = 0;
    if (!m_vActiveChannels.empty()) {
        readTimeoutCnt = 0;
        return;
    }
    readTimeoutCnt++;
    if (readTimeoutCnt >= kNoEventTimes) { //
        auto channelList = m_Poller->getEventTimeoutChannel();
        for (Channel *channel : channelList) {
            channel->doReadTimeOutFunc();
        }
    }
}

void EventLoop::loop() {
    m_bLooping = true;
    m_bQuit    = false;
    assertLoopThread();
    while (!m_bQuit) {
        m_vActiveChannels.clear();
        m_tRecvTimeStamp = m_Poller->poll(kPollTimeMs, &m_vActiveChannels);
        m_bEventHanding  = true;
        clearReadTimeoutChannel();
        for (Channel *channel : m_vActiveChannels) {
            m_pCurrentChannel = channel;
            m_pCurrentChannel->handleEvent(m_tRecvTimeStamp);
        }
        m_pCurrentChannel = nullptr;
        m_bEventHanding   = false;
        doPendingFunctors();
    }
    m_bLooping = false;
    logger.alert("Event stop looping");
}

void EventLoop::assertLoopThread() {
    if (m_nThreadId != CurrentThread::tid()) {
        logger.error("thread id %s is not match current thread id:%s", m_nThreadId, CurrentThread::tid());
    }
}

void EventLoop::updateChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertLoopThread();
    m_Poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertLoopThread();
    if (m_bEventHanding) {
        assert(m_pCurrentChannel == channel ||
               std::find(m_vActiveChannels.begin(), m_vActiveChannels.end(), channel) == m_vActiveChannels.end());
    }
    m_Poller->removeChannel(channel);
}

[[maybe_unused]] bool EventLoop::hasChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    assertLoopThread();
    return m_Poller->hasChannel(channel);
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> funcs;
    m_bCallFuncs = true;
    { funcs.swap(m_vPendingFunctors); }
    {
        std::for_each(funcs.begin(), funcs.end(), [](auto func) { func(); });
    }
    m_bCallFuncs = false;
}

void EventLoop::handleRead() const {
    uint64_t one     = 1;
    ssize_t  readNum = sockets::read(m_nWakeUpFD, &one, sizeof one);
    if (readNum != sizeof one) {
        logger.info("EventLoop::handleRead() reads %d bytes instead of 8", readNum);
    }
}

[[maybe_unused]] void EventLoop::printActiveChannels() const {
    for (auto channel : m_vActiveChannels) {
        logger.info("[channel: %s]", channel->reventsToString());
    }
}

bool EventLoop::isInLoopThread() const { return m_nThreadId == CurrentThread::tid(); }

} // namespace muduo::net