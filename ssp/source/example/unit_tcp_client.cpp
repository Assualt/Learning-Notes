//
// Created by 侯鑫 on 2024/1/8.
//
#include "net/client//tcp_client.h"
#include "base/string_utils.h"
#include "base/system.h"

using namespace ssp::net;
using namespace ssp::base;

int main(int, char **)
{
    System::SetThreadName("main");

    std::shared_ptr<LogImpl> _au(new StdoutLog);
    auto &mainLog = Logger::GetLogger();
    mainLog.BasicConfig(LogLevel::Info,
                        "%(message)",
                        "filename", "%Y-%m-%d");
    mainLog.AddLogHandle(_au.get());


    TcpClient client;
    InetAddress address(8000);
    auto ret = client.Connect(address);
    if (!ret) {
        logger.Info("connect with 8000 failed.");
        return 0;
    }

    std::string line;
    while (true) {
        log_line.Info("[Client]>");
        std::getline(std::cin, line);
        if (util::StartsWithIgnoreCase(line, "quit")) {
            logger.Info("rcv quit message ... exit .");
            break;
        }

        if (line.empty()) {
            continue;
        }

        auto size = client.Send(line.data(), line.size());
        if (size != static_cast<int32_t>(line.size())) {
            logger.Info("send error");
            break;
        }
        std::stringbuf rcvBuffer;
        auto rcvSize = client.Read(rcvBuffer);
        if (rcvSize <= 0) { // send Error
            logger.Info("receive error errno:%d", System::GetErrMsg(errno));
            break;
        }

        logger.Info("[Server:%s]> %d %s\n", address.ToIpPort(), rcvSize, util::TrimRight(rcvBuffer.str(), {"\r\n"}));
    }

    return 0;
}