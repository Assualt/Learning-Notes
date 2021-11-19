#include "EventLoopThread.h"
#include "EventLoop.h"
#include <functional>
using namespace muduo::base;
using namespace muduo::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback &callback, const std::string &name)
    : m_pLoop(nullptr)
    , m_bExited(false)
    , m_thread(std::bind(&EventLoopThread::threadFunc, this), name)
    , m_callback(callback) {
}

EventLoopThread::~EventLoopThread() {
    m_bExited = true;
    if (m_pLoop != nullptr) {
        m_pLoop->quit();
        m_thread.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    m_thread.start();
    EventLoop *loop = nullptr;
    {
        while (m_pLoop == nullptr) {
        }
        loop = m_pLoop;
    }
    return m_pLoop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (m_callback) {
        m_callback(&loop);
    }
    m_pLoop = &loop;

    loop.loop();

    m_pLoop = nullptr;
}