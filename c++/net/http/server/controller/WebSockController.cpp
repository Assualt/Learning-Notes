//
// Created by xhou on 2022/9/22.
//

#include "WebSockController.h"
#include "base/crypto/base64.h"

REG_OBJECT(WebSockController)
bool WebSockController::onGet(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
    auto isWsFlag = (req.get(Connection) == "Upgrade") && (req.get(Upgrade) == "websocket");
    if (!isWsFlag) {
        return onError(req, res, cfg);
    }

    res.setStatusMessage(HttpStatusCode::k101ProtocolSwitch, req.getHttpVersion(), "Switching Protocols");
    res.addHeader(Connection, "Upgrade");

    auto webSocketKey = req.get(SecWebSocketKey) + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";


    res.addHeader(SecWebSocketAccept, "");
    res.addHeader(Upgrade, "websocket");

    return true;
}

bool WebSockController::onError(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
    res.setStatusMessage(HttpStatusCode::k400BadRequest, req.getHttpVersion(), "Bad Request");
    res.setBody("");
    return true;
}

bool WebSockController::onPost(const HttpRequest &request, HttpResponse &response, const HttpConfig &cfg) {
    return onError(request, response, cfg);
}

bool WebSockController::onPut(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
    return onError(req, res, cfg);
}