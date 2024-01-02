//
// Created by 侯鑫 on 2024/1/1.
//

#include "socket_ops.h"
#include "base/log.h"
#include "base/exception.h"
#include "base/format.h"
#include "base/system.h"

#include <sys/socket.h>
#include <sys/uio.h> // read v

using namespace ssp::base;
using namespace ssp::net;

DECLARE_EXCEPTION(SocketException, Exception)

int sockets::Connect(int sockFd, const struct sockaddr *addr)
{
    return ::connect(sockFd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

void sockets::Bind(int sockFd, const struct sockaddr *addr)
{
    int ret = ::bind(sockFd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0) {
        throw SocketException(FmtString("bind address fd[%] error").arg(sockFd).str());
    }
}

void sockets::Listen(int sockFd)
{
    int ret = ::listen(sockFd, SOMAXCONN);
    if (ret < 0) {
        throw SocketException(FmtString("listen address fd[%] error").arg(sockFd).str());
    }
}

int sockets::Accept(int sockFd, struct sockaddr_in6 *addr)
{
    auto addrLen = static_cast<socklen_t>(sizeof(*addr));
#if USE_ACCEPT4
#else
    int connFd = ::accept(sockFd, (sockaddr *)addr, &addrLen);
#endif
    if (connFd > 0) {
        return connFd;
    }
    int saveError = errno;
    logger.Info("sockets::accept error. errno:%d", saveError);
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
            logger.Error("unexpected error of ::accept errno:%d", saveError);
            break;
        default:
            logger.Error("unknown error of ::accept errno:%d", saveError);
            break;
    }
    return connFd;
}

ssize_t sockets::Read(int sockFd, void *buf, size_t count)
{
    return ::read(sockFd, buf, count);
}

ssize_t sockets::ReadV(int sockFd, const struct iovec *iov, int iovCnt)
{
    return readv(sockFd, iov, iovCnt);
}

ssize_t sockets::Write(int sockFd, const void *buf, size_t nWrite)
{ 
    return ::write(sockFd, buf, nWrite); 
}

void sockets::Close(int sockFd)
{
    auto ret = ::close(sockFd);
    if (ret < 0) {
        throw SocketException(
                FmtString("close fd:% failed. ret:% errmsg:%").arg(sockFd).arg(ret).arg(System::GetErrMsg(errno)).str());
    }
}

void sockets::BindOrDie(int sockFd, const struct sockaddr *addr)
{
    int ret = ::bind(sockFd, (const sockaddr *)addr, static_cast<socklen_t>(sizeof(struct sockaddr)));
    if (ret < 0) {
        logger.Error("sockets::bindOrDie. sockFd is:%d ret:%d errno:%d msg:%s", sockFd, ret, errno,
                     System::GetErrMsg(errno));
        throw SocketException(FmtString("close fd:% failed.").arg(sockFd).str());
    }
}

void sockets::ListenOrDie(int sockFd)
{
    int ret = ::listen(sockFd, SOMAXCONN);
    if (ret < 0) {
        logger.Error("sockets::listenOrDie. sockFd:%d errmsg:%s", sockFd, System::GetErrMsg(errno));
    }
}

int sockets::CreateNonblockingOrDie(const sa_family_t family)
{
    int type = SOCK_STREAM;
#ifdef LINUX
    type |= (SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    int sockFd = ::socket(family, type, IPPROTO_TCP);
    if (sockFd < 0) {
        logger.Error("sockets::createNonblockingOrDie");
        throw SocketException("sockets::createNonblockingOrDie failed");
    }
    return sockFd;
}

struct sockaddr_in6 sockets::GetLocalAddr(int sockFd)
{
    struct sockaddr_in6 localAddr{};
    bzero(&localAddr, sizeof localAddr);
    auto addrLen = static_cast<socklen_t>(sizeof localAddr);
    if (::getsockname(sockFd, (struct sockaddr *)(&localAddr), &addrLen) < 0) {
        logger.Warning("sockets::getLocalAddr errmsg:%s", System::GetErrMsg(errno));
    }
    return localAddr;
}

void sockets::ShutdownWrite(int sockFd)
{
    auto ret = ::shutdown(sockFd, SHUT_WR);
    if (ret < 0) {
        logger.Error("shut down sockFd error :%d, ret:%d errno:%d", sockFd, ret, errno);
    }
}

int sockets::CreateSocket(int32_t domain, int32_t type, int32_t protocol)
{
    return socket(domain, type, protocol);
}