#pragma once

#include "HttpConnection.h"
#include "base/Logging.h"
#include "net/Buffer.h"
#include "net/http/HttpConfig.h"
#include "net/http/HttpRequest.h"
#include "net/http/HttpResponse.h"
#include <iostream>
class HttpClient {
public:
    enum ReqType { Type_GET, Type_POST, Type_PUT, Type_HEAD };

public:
    HttpClient() = default;

    HttpResponse Get(const std::string &url, bool needRedirect = false, bool verbose = false);
    HttpResponse Post(const std::string &url, const Buffer &buf, bool needRedirect = false, bool verbose = false);

private:
    HttpResponse Request(ReqType type, const std::string &reqUrl, const Buffer &buff, bool redirect, bool verbose);

private:
    HttpConnection conn_;
};