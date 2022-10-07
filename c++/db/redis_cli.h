//
// Created by xhou on 2022/10/5.
//

#ifndef MUDUO_BASE_TOOLS_REDIS_CLI_H
#define MUDUO_BASE_TOOLS_REDIS_CLI_H

#include "base/Logging.h"
#include "net/Buffer.h"
#include "net/TcpClient.h"
#include <chrono>
#include <optional>
#include <vector>

#define CTRL "\r\n"

using muduo::net::Buffer;
using muduo::net::TcpClient;
class RedisClient {
public:
    RedisClient(const std::string &host, uint16_t port = 6379, const std::string &authUser = "",
                const std::string &authPass = "", bool useTls = false, uint32_t timeOut = 10);
    ~RedisClient();

    bool isConnected();

    void close();

    std::string getErrString();

    // for string
    void del(const std::string &key);

    void set(const std::string &key, const std::string &val);

    void expire(const std::string &key, uint32_t seconds);

    void expireAt(const std::string &key, time_t time);

    void incr(const std::string &key);

    std::string get(const std::string &key);

    bool exists(const std::string &key);

    std::string type(const std::string &key);

    std::vector<std::string> keys();

private:
    template <class T> std::optional<std::vector<T>> parseResult(Buffer &outBuf) {
        if (strncasecmp(outBuf.peek(), "-ERR", strlen("-ERR")) == 0) {
            return std::nullopt;
        }

        if (strncasecmp(outBuf.peek(), "+OK", strlen("+OK")) == 0) {
            return std::nullopt;
        }

        std::vector<T> result;
        while (true) {
            const char *crlf = outBuf.findCRLF();
            if (crlf == nullptr) {
                break;
            }

            auto val = std::string(outBuf.peek(), crlf - outBuf.peek());
            if (!val.empty() && val.front() == '$' && std::atoi(val.substr(1).c_str()) == -1) {
                return std::nullopt;
            }

            outBuf.retrieveUntil(crlf + 2);
            if (!val.empty() && val.front() == '$') {
                continue;
            }

            std::stringstream ss;
            T                 ret;
            if (!(ss << val && ss >> ret && ss.eof())) {
                continue;
            }

            result.push_back(ret);
        }

        return result;
    }

    template <class T> std::optional<std::vector<T>> executeWithResult(const std::string &cmd) {
        if (!isConnected()) {
            return std::nullopt;
        }
        std::string command = cmd + CTRL;
        Buffer      inBuf;
        inBuf.append(command.c_str(), command.size());
        auto size = client_->sendRequest(inBuf);
        if (size != command.size()) {
            return std::nullopt;
        }

        Buffer outBuf;
        auto   recvSize = client_->recvResponse(outBuf);
        if (recvSize <= 0) {
            return std::nullopt;
        }

        return parseResult<T>(outBuf);
    }

    bool execute(const std::string &cmd) {
        if (!isConnected()) {
            return false;
        }
        std::string command = cmd + CTRL;
        Buffer      inBuf;
        inBuf.append(command.c_str(), command.size());
        auto size = client_->sendRequest(inBuf);
        if (size != command.size()) {
            return false;
        }

        Buffer outBuf;
        auto   recvSize = client_->recvResponse(outBuf);
        if (recvSize <= 0) {
            return false;
        }

        if (strncasecmp(outBuf.peek(), "+OK", strlen("+OK")) == 0) {
            return true;
        }

        if (strncasecmp(outBuf.peek(), ":1", strlen(":1")) == 0) {
            return true;
        }

        return false;
    }

private:
    std::unique_ptr<TcpClient> client_{nullptr};
    bool                       connected_{false};
};

#endif // MUDUO_BASE_TOOLS_REDIS_CLI_H
