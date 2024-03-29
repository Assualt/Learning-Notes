#include "base/Logging.h"
#include "base/stream/CompressStream.h"
#include "net/TcpClient.h"
using namespace muduo::net;
using namespace muduo::base;

int main(int, char **) {
    auto &log = Logger::getLogger();
    log.BasicConfig(LogLevel::Debug, "%(asctime) %(message)", nullptr, nullptr);
    log.addLogHandle(new StdOutLogHandle);

    TcpClient client;
    client.setTimeOut(10, 5, 5);
    bool ret = client.connect(InetAddress(22087));
    if (!ret) {
        logger.info("connect 0.0.0.0:8000 failed.");
        return 0;
    }

    std::string str;
    while (true) {
        logger.info("please input:");
        std::cin >> str;
        if (strcasecmp(str.c_str(), "quit") == 0) {
            logger.info("recv quit message. client with exit");
            break;
        }

        if (str.empty()) {
            logger.info("send string is empty");
            continue;
        }

        auto sendSize = client.sendBuf(str.c_str(), str.size());
        logger.info("<<< send buf str:%s. size:%d", str, sendSize);

        Buffer recvBuf;
        auto   recvSize = client.recvResponse(recvBuf);
        if (recvSize <= 0) { // send Error
            logger.info("recv error errno:%d", errno);
            break;
        }

        logger.info(">>> recv buf str:%s. size:%d", std::string(recvBuf.peek(), recvSize), recvSize);
    }

    client.close();
    return 0;
}
