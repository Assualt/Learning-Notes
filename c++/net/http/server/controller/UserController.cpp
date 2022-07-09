#include "UserController.h"
#include "base/json/json.h"

REG_OBJECT(UserController)
bool UserController::onGet(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
    return onPost(req, res, cfg);
}

bool UserController::onPost(const HttpRequest &request, HttpResponse &response, const HttpConfig &cfg) {
    std::string        user = request.getParams("user");
    int                id   = atoi(request.getParams("id").c_str());
    json::Json::object obj;
    auto               kvalmap = request.getAllParams();
    for (auto &iter : kvalmap) {
        obj.insert(iter);
    }
    json::Json retJson({{"ret", 200}, {"errmsg", ""}, {"data", json::Json(obj)}, {"postdata", request.getPostParams()}, 
        {"query" , request.getQuery()}});
    response.setStatusMessage(200, request.getHttpVersion(), "OK");
    response.addHeader(ContentType, "application/json");
    response.setBody(retJson.dump());
    return true;
}

bool UserController::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}