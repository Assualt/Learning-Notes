#include "mycontroller.h"

bool MyController::onGet(const HttpRequest &req, HttpResponse &res, const HttpConfig &) {
    res.setStatusMessage(200, req.getHttpVersion(), "use my controller");
    res.addHeader(ContentType, "text/html; charset=utf8");
    res.setBody("use my controller");
    return true;
}

bool MyController::onPost(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool MyController::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

int mycontroller_Entry() {
    static MyController ctl;
    return reinterpret_cast<uintptr_t>(&ctl);
}