//
// Created by 侯鑫 on 2024/1/10.
//

#include "net/client/http_client.h"
#include "base/system.h"

using namespace ssp::base;
using namespace ssp::net;

int main(int argc, char **argv)
{
    if (argc < 2) {
        return 0;
    }

    System::SetThreadName("main");

    std::shared_ptr<LogImpl> _au(new StdoutLog);
    auto &mainLog = Logger::GetLogger();
    mainLog.BasicConfig(LogLevel::Info,
                        "T:(%(appname)-%(threadName))[%(asctime):%(levelname)]%(message)",
                        "filename", "%Y-%m-%d");
    mainLog.AddLogHandle(_au.get());

    HttpClient client;
    client.InitDefaultHeader();
    auto response = client.Get(argv[1], false, true);
    if (!response.IsOk()) {
        std::cout << response.Error() << std::endl;
        return 0;
    }

    if (response.IsTextHtml()) {
        auto [code, msg] = response.GetStatusCode();
        std::cout << "status:" << code << " " << msg << std::endl;
        std::cout << "text:" << std::endl << response.Text() << std::endl;
    }

    return 0;
}