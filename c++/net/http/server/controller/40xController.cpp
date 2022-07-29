#include "40xController.h"

REG_OBJECT(NotFound);
REG_OBJECT(MethodFound);
REG_OBJECT(AuthRequire);
REG_OBJECT(BadRequest);

#define AUTH_REQUIRED                                                                                                                        \
    "<html>\r\n<head>\r\n<title>401 Authorization Required</title>\r\n</head>\r\n<body bgcolor=\"white\">\r\n<center><h1>401 Authorization " \
    "Required</h1></center>\r\n<hr>\r\n<center>httpserver</center>\r\n</body>\r\n</html>\r\n"

#define BAD_REQUEST                                                                                                     \
    "<html>\r\n<head>\r\n<title>401 Bad Request</title>\r\n</head>\r\n<body bgcolor=\"white\">\r\n<center><h1>401 Bad " \
    "Request</h1></center>\r\n<hr>\r\n<center>httpserver</center>\r\n</body>\r\n</html>"

#define METHOD_NOT_ALLOWED                                                                                                                     \
    "<html>\r\n<head>\r\n<title>405 Method not Allowed</title>\r\n</head>\r\n<body bgcolor=\"white\">\r\n<center><h1>405 Method  not Allowed." \
    "</h1></center>\r\n<hr>\r\n<center>httpserver</center>\r\n</body>\r\n</html>"

bool NotFound::onGet(const HttpRequest &req, HttpResponse &res, const HttpConfig &) {
    res.setStatusMessage(HttpStatusCode::k404NotFound, req.getHttpVersion(), "not found");
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
    res.setStatusMessage(HttpStatusCode::k405MethodNotAllow, req.getHttpVersion(), "Method Not Allowed");
    res.addHeader(ContentType, "text/html");
    res.setBody(METHOD_NOT_ALLOWED);
    return true;
}

bool MethodFound::onPost(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool MethodFound::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool AuthRequire::onGet(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    res.setStatusMessage(HttpStatusCode::k401NotAuth, "HTTP/1.1", "Unauthorized");
    res.addHeader(ContentType, "text/html");
    res.addHeader(WwwAuthenticate, cfg.getAuthName());
    res.setBody(AUTH_REQUIRED);
    return true;
}

bool AuthRequire::onPost(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool AuthRequire::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool BadRequest::onGet(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
    res.setStatusMessage(HttpStatusCode::k400BadRequest, req.getHttpVersion(), "Bad Request");
    res.addHeader(ContentType, "text/html; charset=utf8");
    res.setBody(BAD_REQUEST);
    return true;
}

bool BadRequest::onPost(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool BadRequest::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}