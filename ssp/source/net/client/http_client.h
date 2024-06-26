//
// Created by 侯鑫 on 2024/1/10.
//

#ifndef SSP_TOOLS_HTTP_CLIENT_H
#define SSP_TOOLS_HTTP_CLIENT_H

#include "tcp_client.h"
#include "net/protocol/http_request.h"
#include "net/protocol/http_response.h"
#include "http_uri.h"

namespace ssp::net {

enum HttpType {
    Type_GET,
    Type_POST,
    Type_HEAD
};

class HttpClient : public TcpClient {
public:
    HttpClient();

    ~HttpClient() = default;

public:
    HttpResponse Get(const std::string &url, bool needRedirect = false, bool verbose = false);

    HttpResponse Post(const std::string &url, const std::stringbuf &buf, bool needRedirect = false, bool verbose = false);

    HttpResponse Head(const std::string &url, bool needRedirect = false, bool verbose = false);

    static void SaveToFile(const std::string &file, const HttpResponse &response);

    void InitDefaultHeader();

    template<class T>
    void AddHeader(const std::string &key, const T&val)
    {
        request_.AddHeader(key, val);
    }

private:
    HttpResponse Request(HttpType type, const std::string &url, const std::stringbuf& buf, bool needRedirect,
                         bool verbose);

    std::string GetRequestBuffer(const HttpUrl &url);

    HttpResponse TransBufferToResponse(Buffer &buffer);

private:
    HttpRequest request_;
};

}

#endif //SSP_TOOLS_HTTP_CLIENT_H
