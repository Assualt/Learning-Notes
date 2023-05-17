#include "SocketsOp.h"
#include "base/Exception.h"
#include "base/Format.h"
#include "base/Logging.h"
#include "base/System.h"
#include <sys/uio.h>
#include <unistd.h>
using namespace muduo::base;
using muduo::base::FmtString;
namespace muduo::net {

DECLARE_EXCEPTION(SocketException, base::Exception)

int sockets::connect(int sockFd, const struct sockaddr *addr) {
    return ::connect(sockFd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

void sockets::bind(int sockFd, const struct sockaddr *addr) {
    int ret = ::bind(sockFd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        throw SocketException(FmtString("bind address fd[%] error").arg(sockFd).str());
    }
}

void sockets::listen(int sockFd) {
    int ret = ::listen(sockFd, SOMAXCONN);
    if (ret < 0) {
        throw SocketException(FmtString("listen address fd[%] error").arg(sockFd).str());
    }
}

int sockets::accept(int sockFd, struct sockaddr_in6 *addr) {
    socklen_t addrLen = static_cast<socklen_t>(sizeof(*addr));
#if USE_ACCEPT4
#else
    int connFd = ::accept(sockFd, (sockaddr *)addr, &addrLen);
#endif
    if (connFd > 0) {
        return connFd;
    }
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
    return connFd;
}

ssize_t sockets::read(int sockFd, void *buf, size_t count) { return ::read(sockFd, buf, count); }

ssize_t sockets::readv(int sockFd, const struct iovec *iov, int iovCnt) { return ::readv(sockFd, iov, iovCnt); }

ssize_t sockets::write(int sockFd, const void *buf, size_t nWrite) { return ::write(sockFd, buf, nWrite); }

void sockets::close(int sockFd) {
    auto ret = ::close(sockFd);
    if (ret < 0) {
        throw SocketException(
            FmtString("close fd:% failed. ret:% errmsg:%").arg(sockFd).arg(ret).arg(System::GetErrMsg(errno)).str());
    }
}

void sockets::bindOrDie(int sockFd, const struct sockaddr *addr) {
    int ret = ::bind(sockFd, (const sockaddr *)addr, static_cast<socklen_t>(sizeof(struct sockaddr)));
    if (ret < 0) {
        logger.error("sockets::bindOrDie. sockFd is:%d ret:%d errno:%d msg:%s", sockFd, ret, errno,
                     System::GetErrMsg(errno));
        throw SocketException(FmtString("close fd:% failed.").arg(sockFd).str());
    }
}

void sockets::listenOrDie(int sockFd) {
    int ret = ::listen(sockFd, SOMAXCONN);
    if (ret < 0) {
        logger.error("sockets::listenOrDie. sockFd:%d errmsg:%s", sockFd, System::GetErrMsg(errno));
    }
}

int sockets::createNonblockingOrDie(const sa_family_t family) {
#if VALGRIND
    int sockFd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (sockFd < 0) {
        logger.error("sockets::createNonblockingOrDie errmsg:%s", System::GetErrMsg(errno));
    }

    setNonBlockAndCloseOnExec(sockFd);
#else
    int sockFd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockFd < 0) {
        logger.error("sockets::createNonblockingOrDie");
        throw SocketException("sockets::createNonblockingOrDie failed");
    }
#endif
    return sockFd;
}

struct sockaddr_in6 sockets::getLocalAddr(int sockFd) {
    struct sockaddr_in6 localAddr;
    bzero(&localAddr, sizeof localAddr);
    socklen_t addrLen = static_cast<socklen_t>(sizeof localAddr);
    if (::getsockname(sockFd, (struct sockaddr *)(&localAddr), &addrLen) < 0) {
        logger.warning("sockets::getLocalAddr errmsg:%s", System::GetErrMsg(errno));
    }
    return localAddr;
}

void sockets::shutdownWrite(int sockFd) {
    auto ret = ::shutdown(sockFd, SHUT_WR);
    if (ret < 0) {
        logger.error("shut down sockFd error :%d, ret:%d errno:%d", sockFd, ret, errno);
    }
}

} // namespace muduo::net
