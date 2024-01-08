//
// Created by 侯鑫 on 2024/1/3.
//

#include "net/server/tcp_server.h"
#include "net/net_address.h"
#include "net/event_loop.h"
#include "base/system.h"
#include "base/string_utils.h"

using namespace ssp::net;
using namespace ssp::base;

void TcpMsgCallback(const TcpConnectionPtr& conn, std::stringbuf &buffer, const TimeStamp &stamp)
{
    auto str = util::trimRight(buffer.str(), '\n');
    if (util::StartsWithIgnoreCase(str, "Quit")) {
        conn->Close();
        return;
    }

    std::string sendBuffer = stamp.ToFmtString() + " " + util::TrimRight(buffer.str(), std::string("\r\n"))
            + "\r\n";
    conn->Send(sendBuffer.c_str(), static_cast<int32_t>(sendBuffer.size()));
}

int main(int, char **)
{
    System::SetThreadName("main");

    std::shared_ptr<LogImpl> _au(new StdoutLog);
    auto &mainLog = Logger::GetLogger();
    mainLog.BasicConfig(LogLevel::Info,
                        "T:(%(appname)-%(threadName))[%(asctime):%(levelname)]%(message)",
                        "filename", "%Y-%m-%d");
    mainLog.AddLogHandle(_au.get());

    EventLoop loop;
    TcpServer server(&loop, "tcp_server", InetAddress(8000));
    server.Init(ConfigureManager("./tmp"));
    server.Start();

    server.SetMessageCallback(TcpMsgCallback);

    loop.Loop();
    return 0;
}