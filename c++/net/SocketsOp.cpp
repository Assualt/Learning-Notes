#include "SocketsOP.h"
#include "base/Format.h"
#include "base/Logging.h"
#include <sys/uio.h>
#include <unistd.h>
using muduo::base::FmtString;
namespace muduo {
namespace net {

int sockets::connect(int sockfd, const struct sockaddr *addr) {
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}
void sockets::bind(int sockfd, const struct sockaddr *addr) {
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        // throw
        // throw SocketException(FmtString("bind address fd[%] error").arg(sockfd).str());
    }
}
void sockets::listen(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        // throw
        // throw SocketException(FmtString("listen address fd[%] error").arg(sockfd).str());
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

void sockets::bindOrDie(int sockfd, const struct sockaddr *addr) {
    int ret = ::bind(sockfd, (const sockaddr *)addr, static_cast<socklen_t>(sizeof(struct sockaddr)));
    if (ret < 0) {
        logger.error("sockets::bindOrDie. sockfd is:%d", sockfd);
    }
}

void sockets::listenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) {
        logger.error("sockets::listenOrdie. sockfd:%d", sockfd);
    }
}

int sockets::createNonblockingOrDie(const sa_family_t family) {
#if VALGRIND
    int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        logger.error("sockets::createNonblockingOrDie");
    }

    setNonBlockAndCloseOnExec(sockfd);
#else
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0) {
        logger.error("sockets::createNonblockingOrDie");
    }
#endif
    return sockfd;
}

} // namespace net
} // namespace muduo
