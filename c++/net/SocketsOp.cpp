#include "SocketsOP.h"
#include "base/Exception.h"
#include "base/Format.h"
#include <sys/uio.h>
#include <unistd.h>
using muduo::base::FmtString;
namespace muduo {
namespace net {

DECLARE_EXCEPTION(SocketException, base::Exception);

int sockets::connect(int sockfd, const struct sockaddr *addr) {
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}
void sockets::bind(int sockfd, const struct sockaddr *addr) {
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        // throw
        throw SocketException(FmtString("bind address fd[%] error").arg(sockfd).str());
    }
}
void sockets::listen(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        // throw
        throw SocketException(FmtString("listen address fd[%] error").arg(sockfd).str());
    }
}
int sockets::accept(int sockfd, struct sockaddr_in6 *addr) {
    return 0;
}
ssize_t sockets::read(int sockfd, void *buf, size_t count) {
    return ::read(sockfd, buf, count);
}
ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt) {
    return ::readv(sockfd, iov, iovcnt);
}
ssize_t sockets::write(int sockfd, const void *buf, size_t nwrite) {
    return ::write(sockfd, buf, nwrite);
}
void sockets::close(int sockfd) {
    if (::close(sockfd) < 0) {
        // throw
    }
}
} // namespace net
} // namespace muduo
