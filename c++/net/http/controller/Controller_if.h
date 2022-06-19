#pragma once
#include "net/http/HttpRequest.h"
#include "net/http/HttpResponse.h"
#include "net/http/HttpServer.h"
#include <ostream>
#include <string>

using namespace muduo::base;
using namespace muduo::net;

#define REG_PATTERN(pattern, func) HttpServer::getMapper().addRequestMapping(pattern, func);
class IController {
public:
    IController(const std::string &name)
        : name_(name) {
    }

    virtual ~IController() = default;
    const std::string &getName() const {
        return name_;
    }

    virtual bool onRequest(const HttpRequest &, HttpResponse &, const HttpConfig &) = 0;
    virtual void onDump(std::ostream &)                                             = 0;
    virtual void onInit()                                                           = 0;

private:
    std::string name_;
};