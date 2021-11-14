#pragma once

#include <arpa/inet.h>

namespace muduo {
namespace net {
namespace sockets {

template <typename To, typename From> inline To implicit_cast(From const &f) {
    return f;
}

int     connect(int sockfd, const struct sockaddr *addr);
void    bind(int sockfd, const struct sockaddr *addr);
void    listen(int sockfd);
int     accept(int sockfd, struct sockaddr_in6 *addr);
ssize_t read(int sockfd, void *buf, size_t count);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t nwrite);
void    close(int sockfd);

int  createNonblockingOrDie(const sa_family_t family);
void bindOrDie(int sockfd, const struct sockaddr *addr);
void listenOrDie(int sockfd);

} // namespace sockets
} // namespace net
} // namespace muduo
