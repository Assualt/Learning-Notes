#include "mycontroller.h"

bool MyController::onGet(const HttpRequest &req, HttpResponse &res, const HttpConfig &) {
    res.setStatusMessage(200, req.getHttpVersion(), "use my controller=");
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

int mycontroller_Entry(std::string *pattern, int *method, bool *needVal, uintptr_t *obj) {
    *pattern = "/my";
    *method  = 0x1;
    *needVal = false;
    *obj     = reinterpret_cast<uintptr_t>(new MyController);
    return 0;
}