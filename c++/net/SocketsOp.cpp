#include "SocketsOp.h"
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
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
#if USE_ACCEPT4
#else
    int connfd = ::accept(sockfd, (sockaddr *)addr, &addrlen);
#endif
    if (connfd < 0) {
        int saveError = errno;
        logger.info("sockets::accept error. errno:%d", saveError);
        switch (saveError) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                errno = saveError;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                logger.error("unexpected error of ::accept errno:%d", saveError);
                break;
            default:
                logger.error("unknown error of ::accept errno:%d", saveError);
                break;
        }
        return connfd;
    }
    return connfd;
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

struct sockaddr_in6 sockets::getLocalAddr(int sockfd) {
    struct sockaddr_in6 localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, (struct sockaddr *)(&localaddr), &addrlen) < 0) {
        logger.warning("sockets::getLocalAddr");
    }
    return localaddr;
}

void sockets::shutdownWrite(int sockfd) {
    if (::shutdown(sockfd, SHUT_WR) < 0) {
        logger.error("shut down sockfd error :%d", sockfd);
    }
}

} // namespace net
} // namespace muduo
