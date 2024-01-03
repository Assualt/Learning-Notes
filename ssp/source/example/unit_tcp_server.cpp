//
// Created by 侯鑫 on 2024/1/3.
//

#include "net/server/tcp_server.h"
#include "net/net_address.h"

using namespace ssp::net;

int main(int argc, char **argv)
{
    std::shared_ptr<LogImpl> _au(new StdoutLog);
    auto &mainLog = Logger::GetLogger();
    mainLog.BasicConfig(LogLevel::Info,
                        "T:(%(appname)-%(thread))[%(asctime):%(levelname)]%(message)",
                        "filename", "%Y-%m-%d");
    mainLog.AddLogHandle(_au.get());

    TcpServer server("tcp_server", InetAddress(8000));
    server.Init(ConfigureManager("./tmp"));
    server.Start();


    return 0;
}