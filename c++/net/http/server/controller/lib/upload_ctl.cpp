#include "upload_ctl.h"

bool UploadController::onGet(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
    res.setStatusMessage(HttpStatusCode::k200Ok, req.getHttpVersion(), "OK");
    res.setBody("Get Request Body for upload");
    return true;
}

bool UploadController::onPost(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
    res.setStatusMessage(HttpStatusCode::k200Ok, req.getHttpVersion(), "OK");
    res.setBody("Get Request Body for upload");
    return true;
}

bool UploadController::onPut(const HttpRequest &, HttpResponse &, const HttpConfig &cfg) {
    return true;
}

extern "C" int upload_Entry(std::string *pattern, int *method, bool *needVal, bool *useRegex, uintptr_t *obj) {
    *pattern = "/upload";
    *method  = 0x3;
    *needVal = false;
    *useRegex = false;
    *obj     = reinterpret_cast<uintptr_t>(new UploadController);
    return 0;
}