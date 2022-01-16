#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "base/Logging.h"
#include "net/Buffer.h"
#include <iostream>
class HttpClient {
public:
    enum ReqType { Type_GET, Type_POST, Type_PUT, Type_HEAD };

public:
    HttpClient() = default;
    void Request(ReqType type, const std::string &reqUrl, Buffer &buff);

private:
    std::string requestUrl;

    HttpRequest  request;
    HttpResponse response;
};