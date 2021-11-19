#include <functional>
#include <memory>
namespace muduo {
namespace base {
class Timestamp;
}
using base::Timestamp;
namespace net {

class EventLoop;
class TcpConnection;
class Buffer;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

using ThreadInitCallback    = std::function<void(EventLoop *)>;
using ConnectionCallback    = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback       = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;
using CloseCallback         = std::function<void(const TcpConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;
} // namespace net
} // namespace muduo