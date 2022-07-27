#include "mycontroller1.h"
#include "base/json/json.h"
bool MyController1::onGet(const HttpRequest &req, HttpResponse &res, const HttpConfig &) {
    res.setStatusMessage(HttpStatusCode::k200Ok, req.getHttpVersion(), "OK");
    res.addHeader(ContentType, "application/json");

    json::Json retJson({{"ret", 200}, {"errmsg", ""}, {"data", "data"}, {"postdata", req.getPostParams()},
                        {"query" , req.getQuery()}});

    res.setBody(retJson.dump());
    return true;
}

bool MyController1::onPost(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool MyController1::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

extern "C" int mycontroller1_Entry(std::string *pattern, int *method, bool *needVal, bool *useRegex, uintptr_t *obj) {
    *pattern = "/userinfo\\d+/";
    *method  = 0x1;
    *needVal = false;
    *useRegex = true;
    *obj     = reinterpret_cast<uintptr_t>(new MyController1);
    return 0;
}