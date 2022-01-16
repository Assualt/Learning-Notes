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
    m_pCond.reset(new Condition(m_mutex));
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
    // 这个位置是要等线程起来之后,才能进行如下操作
    while (m_pLoop == NULL) {
        m_pCond->Wait();
    }
    return m_pLoop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (m_callback) {
        m_callback(&loop);
    }
    m_pLoop = &loop;
    m_pCond->Notify();

    loop.loop();

    m_pLoop = nullptr;
}