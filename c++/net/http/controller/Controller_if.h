#pragma once
#include "base/ObjPool.h"
#include "net/http/HttpParttern.h"
#include "net/http/HttpRequest.h"
#include "net/http/HttpResponse.h"
#include "net/http/HttpServer.h"
#include <functional>
#include <ostream>
#include <string>

using namespace muduo::base;
using namespace muduo::net;

class IController : Object {
public:
    IController(const std::string &name, const RequestMapper::Key &key)
        : name_(name) {
        HttpServer::getMapper().addRequestObject(key, reinterpret_cast<uintptr_t>(this));
    }

    IController(const std::string &name)
        : name_(name) {
    }

    virtual ~IController() = default;
    const std::string &getName() const {
        return name_;
    }

    bool InitSelf() override {
        return true;
    }
    bool InitOther() override {
        return true;
    }
    bool InitFinish() override {
        return true;
    }

    virtual bool onRequest(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
        std::map<std::string, Func> methodFuncs = {
            {"get", [ this ](const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) { return this->onGet(req, res, cfg); }},
            {"post", [ this ](const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) { return this->onPost(req, res, cfg); }},
            {"put", [ this ](const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) { return this->onPut(req, res, cfg); }},
        };

        logger.info("request path:%s controller name:%s type:%s", req.getRequestPath(), name_, req.getRequestType());
        auto itr = methodFuncs.find(utils::toLower(req.getRequestType()));
        if (itr != methodFuncs.end()) {
            return itr->second(req, res, cfg);
        }
        return onLost(req, res, cfg);
    }

    virtual void onDump(std::ostream &)                                          = 0;
    virtual bool onGet(const HttpRequest &, HttpResponse &, const HttpConfig &)  = 0;
    virtual bool onPost(const HttpRequest &, HttpResponse &, const HttpConfig &) = 0;
    virtual bool onPut(const HttpRequest &, HttpResponse &, const HttpConfig &)  = 0;

private:
    bool onLost(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
        res.setStatusMessage(302, "HTTP/1.1", "req method is not support");
        res.addHeader("Content-Type", "text/html");
        res.addHeader("Content-Length", 0);
        return true;
    }

private:
    std::string name_;
};

#define DECLARE_CONTROLLER                                                            \
public:                                                                               \
    bool InitSelf() override {                                                        \
        return true;                                                                  \
    }                                                                                 \
    bool InitOther() override {                                                       \
        return true;                                                                  \
    }                                                                                 \
    bool InitFinish() override {                                                      \
        return true;                                                                  \
    }                                                                                 \
    void onDump(std::ostream &) override {                                            \
        return;                                                                       \
    }                                                                                 \
                                                                                      \
    bool onGet(const HttpRequest &, HttpResponse &, const HttpConfig &cfg) override;  \
    bool onPost(const HttpRequest &, HttpResponse &, const HttpConfig &cfg) override; \
    bool onPut(const HttpRequest &, HttpResponse &, const HttpConfig &cfg) override;
