//
// Created by 侯鑫 on 2024/1/1.
//

#ifndef SSP_TOOLS_SOCKET_OPS_H
#define SSP_TOOLS_SOCKET_OPS_H

#include <arpa/inet.h>

namespace ssp::net::sockets {

int CreateSocket(int32_t domain = AF_INET, int32_t type = SOCK_STREAM, int32_t protocol = 0);

int Connect(int sockFd, const struct sockaddr *addr);

void Bind(int sockFd, const struct sockaddr *addr);

void Listen(int sockFd);

int Accept(int sockFd, struct sockaddr_in6 *addr);

int Read(int sockFd, void *buf, size_t count);

ssize_t ReadV(int sockFd, const struct iovec *iov, int iovCnt);

int Write(int sockFd, const void *buf, size_t nWrite);

void Close(int sockFd);

int CreateNonblockingOrDie(sa_family_t family);

void BindOrDie(int sockFd, const struct sockaddr *addr);

void ListenOrDie(int sockFd);

struct sockaddr_in6 GetLocalAddr(int sockFd);

void ShutdownWrite(int sockFd);

}


#endif //SSP_TOOLS_SOCKET_OPS_H
