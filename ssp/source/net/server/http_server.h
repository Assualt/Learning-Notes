//
// Created by 侯鑫 on 2024/3/27.
//

#ifndef SSP_TOOLS_HTTP_SERVER_H
#define SSP_TOOLS_HTTP_SERVER_H

#include "tcp_server.h"
#include "net/event_loop.h"
#include "net/net_address.h"
#include "net/tcp_connection.h"
#include "base/timestamp.h"
#include "base/config.h"

namespace ssp::net {

class HttpServer : public TcpServer {
public:
    explicit HttpServer(const std::string &name, const InetAddress &address);

    bool OnConnection(TcpConnectionPtr connection);

    void StartServer(const ssp::base::ConfigureManager &config);

    void OnMessage(TcpConnectionPtr connection, std::stringbuf &buffer, const TimeStamp &recvTime);

    void MainLoop();

private:
    EventLoop loop_;
};
}

#endif // SSP_TOOLS_HTTP_SERVER_H