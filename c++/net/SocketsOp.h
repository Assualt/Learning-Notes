#pragma once

#include <arpa/inet.h>

namespace muduo {
namespace net {
namespace sockets {

template <typename To, typename From> inline To implicit_cast(From const &f) { return f; }

int connect(int sockFd, const struct sockaddr *addr);

void bind(int sockFd, const struct sockaddr *addr);

void listen(int sockFd);

int accept(int sockFd, struct sockaddr_in6 *addr);

ssize_t read(int sockFd, void *buf, size_t count);

ssize_t readv(int sockFd, const struct iovec *iov, int iovCnt);

ssize_t write(int sockFd, const void *buf, size_t nWrite);

void close(int sockFd);

int createNonblockingOrDie(const sa_family_t family);

void bindOrDie(int sockFd, const struct sockaddr *addr);

void listenOrDie(int sockFd);

struct sockaddr_in6 getLocalAddr(int sockFd);

void shutdownWrite(int sockFd);

#ifdef USING_OPENSSL
#endif

} // namespace sockets
} // namespace net
} // namespace muduo
