//
// Created by 侯鑫 on 2024/1/10.
//

#include "net/client/http_client.h"
#include "base/system.h"

using namespace ssp::base;
using namespace ssp::net;

int main(int argc, char **argv)
{
    System::SetThreadName("main");

    std::shared_ptr<LogImpl> _au(new StdoutLog);
    auto &mainLog = Logger::GetLogger();
    mainLog.BasicConfig(LogLevel::Info,
                        "%(message)",
                        "filename", "%Y-%m-%d");
    mainLog.AddLogHandle(_au.get());

    HttpClient client;
    client.Get("https://www.baidu.com");

    return 0;
}