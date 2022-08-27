#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "base/Format.h"

using namespace muduo::net;
using namespace muduo::base;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &arg)
    : m_pBaseLoop(baseLoop)
    , m_strName(arg)
    , m_bIsStarted(false)
    , m_nThreadNum(0)
    , m_nNextNum(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {
    for (auto &loop : m_vLoops) {
        loop->quit();
    }
    m_vLoops.clear();
}

void EventLoopThreadPool::start(const ThreadInitFunc &cb) {
    m_pBaseLoop->assertLoopThread();

    m_bIsStarted = true;
    for (int i = 0; i < m_nThreadNum; ++i) {
        auto threadName = FmtString("%s/%d").arg(m_strName).arg(i).str();
        auto tmp        = std::make_unique<EventLoopThread>(cb, threadName);
        m_vLoops.push_back(tmp->startLoop());
        m_vThreads.push_back(std::move(tmp));
    }
    if (m_nThreadNum == 0 && cb) {
        cb(m_pBaseLoop);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    m_pBaseLoop->assertLoopThread();
    EventLoop *loop = m_pBaseLoop;

    if (!m_vLoops.empty()) {
        // round-robin
        loop = m_vLoops[ m_nNextNum ];
        ++m_nNextNum;
        if (m_nNextNum >= m_vLoops.size()) {
            m_nNextNum = 0;
        }
    }
    return loop;
}

EventLoop *EventLoopThreadPool::getLoopForHash(size_t hashCode) {
    m_pBaseLoop->assertLoopThread();
    EventLoop *loop = m_pBaseLoop;

    if (!m_vLoops.empty()) {
        loop = m_vLoops[ hashCode % m_vLoops.size() ];
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
    m_pBaseLoop->assertLoopThread();
    if (m_vLoops.empty()) {
        return std::vector<EventLoop *>(1, m_pBaseLoop);
    }
    return m_vLoops;
}
