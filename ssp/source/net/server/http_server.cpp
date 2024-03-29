//
// Created by 侯鑫 on 2024/3/27.
//

#include "http_server.h"
#include "base/log.h"
#include "base/string_utils.h"
#include "net/client/http_context.h"

using namespace ssp::net;

HttpServer::HttpServer(const std::string &name, const InetAddress &address)
    : TcpServer(&loop_, name, address, true)
{
    TcpServer::SetMessageCallback([ this ](TcpConnectionPtr conn, std::stringbuf &buffer, const TimeStamp &stamp) {
        OnMessage(conn, buffer, stamp);
    });

    TcpServer::SetConnectionCallback([ this ](TcpConnectionPtr conn) { return OnConnection(conn); });
}

void HttpServer::MainLoop()
{
    loop_.Loop();
}

void HttpServer::StartServer(const ConfigureManager &config)
{
    TcpServer::Init(config);
    TcpServer::Start();
}

bool HttpServer::OnConnection(TcpConnectionPtr conn) 
{
    logger.Info("connection is ready ...");
    return true;
}

void HttpServer::OnMessage(TcpConnectionPtr conn, std::stringbuf &buffer, const TimeStamp &stamp)
{
    auto str = util::trimRight(buffer.str(), '\n');
    

    conn->Send("Hello\r\n", 7);
    conn->Close();
}