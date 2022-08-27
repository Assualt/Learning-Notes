#include "base/nonecopyable.h"

#include <functional>
#include <memory>
#include <vector>

namespace muduo {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : base::nonecopyable {
public:
    typedef std::function<void(EventLoop *)> ThreadInitFunc;

    EventLoopThreadPool(EventLoop *baseLoop, const std::string &arg);
    ~EventLoopThreadPool();
    void SetThreadNum(int numThreads) { m_nThreadNum = numThreads; }
    void start(const ThreadInitFunc &func = ThreadInitFunc());

    EventLoop *getNextLoop();
    EventLoop *getLoopForHash(size_t hashCode);

    std::vector<EventLoop *> getAllLoops();
    bool                     isStart();
    const std::string       &name() const;

private:
    EventLoop                                    *m_pBaseLoop;
    std::string                                   m_strName;
    bool                                          m_bIsStarted;
    int                                           m_nThreadNum;
    int                                           m_nNextNum;
    std::vector<std::unique_ptr<EventLoopThread>> m_vThreads;
    std::vector<EventLoop *>                      m_vLoops;
};
} // namespace net
} // namespace muduo