//
// Created by xhou on 2022/9/22.
//

#ifndef MUDUO_BASE_TOOLS_WEBSOCKCONTROLLER_H
#define MUDUO_BASE_TOOLS_WEBSOCKCONTROLLER_H

#include "Controller_if.h"
#include "base/ObjPool.h"

class WebSockController : public IController {
public:
    WebSockController()
        : IController("[WebSocket Controller]", {"/ws", REQ_TYPE::TYPE_GET, false, true}) {}
    ~WebSockController() override = default;
    DECLARE_CONTROLLER

private:
    bool onError(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg);
};
#endif // MUDUO_BASE_TOOLS_WEBSOCKCONTROLLER_H
