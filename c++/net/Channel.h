#pragma once

#include "base/Timestamp.h"
#include "base/nonecopyable.h"
#include <functional>
#include <memory>
using muduo::base::nonecopyable;
namespace muduo {
namespace net {

class EventLoop;
class Channel : nonecopyable {
public:
    using EventCallback     = std::function<void()>;
    using ReadEventCallback = std::function<void(const base::Timestamp &)>;

public:
    Channel(EventLoop *loop, int fd);
    ~Channel();

    void setReadCallback(ReadEventCallback cb) { m_FReadCallback = std::move(cb); }

    void setWriteCallback(EventCallback cb) { m_FWriteCallback = std::move(cb); }

    void setCloseCallback(EventCallback cb) { m_FCloseCallback = std::move(cb); }

    void setErrorCallback(EventCallback cb) { m_FErrorCallback = std::move(cb); }

    void setReadTimeOutCallback(EventCallback cb) { m_ReadTimeoutCallback = std::move(cb); }

    inline int fd() { return m_nFD; }

    inline int events() { return m_nEvents; }

    inline void set_revents(int evt) { m_nrecv_events = evt; }

    EventLoop *getLoop() { return m_pLoop; }

    void setPollEvents(int event) { m_nrecv_events = event; }

    void handleEvent(const base::Timestamp &recvTime);

    EventLoop *ownerLoop();

    int fd() const;

    int events() const;

    int index() const { return m_nIndex; }

    void setIndex(int idx) { m_nIndex = idx; }

    bool isNoneEvent() const { return m_nEvents == kNoneEvent; }

    void enableReading() {
        m_nEvents |= kReadEvent;
        update();
    }

    void disableReading() {
        m_nEvents &= ~kReadEvent;
        update();
    }

    void enableWriting() {
        m_nEvents |= kWriteEvent;
        update();
    }

    void disableWriting() {
        m_nEvents &= ~kWriteEvent;
        update();
    }

    void disableAll() {
        m_nEvents = kNoneEvent;
        update();
    }

    bool isWriting() const { return m_nEvents & kWriteEvent; }

    bool isReading() const { return m_nEvents & kReadEvent; }

    void remove();

    std::string reventsToString() const;

    std::string eventToString() const;

    std::string eventsToString(int fd, int ev) const;

    void doReadTimeOutFunc();

protected:
    void update();

    void handleEventWithGuard(const base::Timestamp &recvTime);

private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *m_pLoop;
    int        m_nFD;
    int        m_nEvents;
    int        m_nIndex;

    int                 m_nrecv_events; // received by poll/epoll
    bool                m_bUseLock;
    std::weak_ptr<void> m_Lock;

    ReadEventCallback m_FReadCallback;
    EventCallback     m_FWriteCallback;
    EventCallback     m_FCloseCallback;
    EventCallback     m_FErrorCallback;
    EventCallback     m_ReadTimeoutCallback;
};

} // namespace net
} // namespace muduo
