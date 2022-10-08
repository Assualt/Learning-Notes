//
// Created by xhou on 2022/10/5.
//

#include "redis_cli.h"
#include "net/Endian.h"
#include "net/InetAddress.h"
#include <memory>
using namespace muduo::net;

RedisClient::RedisClient(const std::string &host, uint16_t port, const std::string &authUser,
                         const std::string &authPass, bool useTls, uint32_t timeOut) {
    client_ = std::make_unique<TcpClient>(useTls);
    client_->setTimeOut(timeOut, timeOut, timeOut);
    connected_ = client_->connect(InetAddress(port));
}

RedisClient::~RedisClient() { close(); }

bool RedisClient::isConnected() { return connected_; }

void RedisClient::close() {
    execute("quit");
    if (client_) {
        client_->close();
    }
}

std::string RedisClient::getErrString() { return ""; }

std::vector<std::string> RedisClient::keys() {
    auto res = executeWithResult<std::string>("keys *");
    if (!res.has_value()) {
        return {};
    }

    if (res.value().empty() || res.value().size() == 1) {
        return {};
    }

    return std::vector<std::string>(res.value().begin() + 1, res.value().end());
}

std::string RedisClient::get(const std::string &key) {
    auto val = executeWithResult<std::string>("get " + key);

    if (!val.has_value()) {
        return "";
    }

    if (val->empty()) {
        return "";
    }

    return val.value().front();
}

void RedisClient::set(const std::string &key, const std::string &val) {
    bool execResult = execute("set " + key + " \"" + val + "\"");
    if (!execResult) {
        logger.warning("set for key %s error", key);
    }
}

void RedisClient::del(const std::string &key) {
    bool result = execute("del " + key);
    if (!result) {
        logger.info("del key %s failed", key);
    }
}

void RedisClient::expire(const std::string &key, uint32_t seconds) {
    bool result = execute("expire " + key + " " + std::to_string(seconds));
    if (!result) {
        logger.info("set expire time for key %s failed", key);
    }
}

void RedisClient::expireAt(const std::string &key, time_t time) {
    bool result = execute("expireat " + key + " " + std::to_string(time));
    if (!result) {
        logger.info("set expire time for key %s failed", key);
    }
}

void RedisClient::incr(const std::string &key) { execute("incr " + key); }

bool RedisClient::exists(const std::string &key) { return execute("exists " + key); }

std::string RedisClient::type(const std::string &key) {
    auto ret = executeWithResult<std::string>("type " + key);
    if (!ret.has_value()) {
        return "none";
    }

    if (ret.value().empty()) {
        return "none";
    }

    return ret.value().front();
}

std::vector<std::string> RedisClient::lrange(const std::string &key, int32_t start, int32_t end) {
    std::stringstream ss;
    ss << "lrange \"" << key << "\" " << start << " " << end;
    auto result = executeWithResult<std::string>(ss.str());
    if (!result.has_value()) {
        return {};
    }

    if (result.value().empty() || result.value().size() == 1) {
        return {};
    }

    return {result.value().begin() + 1, result.value().end()};
}

int32_t RedisClient::llen(const std::string &key) {
    auto ret = executeWithResult<std::string>("llen \"" + key + "\"");
    if (!ret.has_value()) {
        return 0;
    }

    if (ret.value().empty()) {
        return 0;
    }

    auto str = ret.value().front();
    return atoi(str.substr(str.find(":") + 1).c_str());
}