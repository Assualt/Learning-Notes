#include "HttpServer.h"
#include "base/Logging.h"

void handleRequest(const HttpRequest &req, HttpResponse &res) {
    std::cout << "Handle Call Back \n";
}

int main(int argc, char const *argv[]) {
    auto &log = Logger::getLogger();
    log.BasicConfig(Logger::Debug, "T:%(process)[%(asctime):%(levelname)][%(filename):%(lineno))] %(message)", "", "");
    std::shared_ptr<LogHandle> _au(new StdOutLogHandle);
    log.addLogHandle(_au.get());

    EventLoop  loop;
    HttpServer server(&loop, InetAddress(8000));
    server.setRequestCallBack(handleRequest);
    server.setThreadNum(10);
    server.start();
    loop.loop();

    return 0;
}
