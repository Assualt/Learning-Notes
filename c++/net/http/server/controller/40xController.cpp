#include "40xController.h"

REG_OBJECT(NotFound);
REG_OBJECT(MethodFound);
REG_OBJECT(AuthRequire);
REG_OBJECT(BadRequest);

bool NotFound::onGet(const HttpRequest &req, HttpResponse &res, const HttpConfig &) {
    res.setStatusMessage(404, req.getHttpVersion(), "not found");
    res.addHeader(ContentType, "text/html; charset=utf8");
    res.setBody(NOTFOUNDHTML);
    return true;
}

bool NotFound::onPost(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool NotFound::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool MethodFound::onGet(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
    res.setStatusMessage(405, req.getHttpVersion(), "Method Not Allowed");
    res.addHeader(ContentType, "text/html");
    res.setBody(METHODNOTALLOWED);
    return true;
}

bool MethodFound::onPost(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool MethodFound::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool AuthRequire::onGet(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    res.setStatusMessage(401, "HTTP/1.1", "Unauthorized");
    res.addHeader(ContentType, "text/html");
    res.addHeader("WWW-Authenticate", cfg.getAuthName());
    res.setBody(AUTHREQUIRED);
    return true;
}

bool AuthRequire::onPost(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool AuthRequire::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool BadRequest::onGet(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
    res.setStatusMessage(400, req.getHttpVersion(), "Bad Request");
    res.addHeader(ContentType, "text/html; charset=utf8");
    res.setBody(BADREQUEST);
    return true;
}

bool BadRequest::onPost(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool BadRequest::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}