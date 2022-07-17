#include "HttpClient.h"

HttpResponse HttpClient::Get(const string &url, bool needRedirect, bool verbose) {
    Buffer buffer;
    return Request(Type_GET, url, buffer, needRedirect, verbose);
}
HttpResponse HttpClient::Post(const string &url, const Buffer &body, bool needRedirect, bool verbose) {
    return Request(ReqType::Type_POST, url, body, needRedirect, verbose);
}
HttpResponse HttpClient::Request(HttpClient::ReqType type, const string &reqUrl, const Buffer &buff, bool redirect, bool verbose) {
    HttpResponse resp(false);
    if (!conn_.connect(reqUrl)) {
        logger.info("connect the url:%s error", reqUrl);
        return resp;
    }
    return resp;
}
