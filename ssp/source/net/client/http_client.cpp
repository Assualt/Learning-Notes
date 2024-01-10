//
// Created by 侯鑫 on 2024/1/10.
//

#include "http_client.h"
#include "http_context.h"
#include "http_uri.h"
#include "base/timestamp.h"
#include "base/log.h"

using namespace ssp::net;
using namespace ssp::base;

HttpClient::HttpClient()
    : TcpClient(true)
{}

HttpResponse HttpClient::Head(const std::string &url, bool needRedirect, bool verbose)
{
    return Request(Type_HEAD, url, {}, needRedirect, verbose);
}

HttpResponse HttpClient::Get(const std::string &url, bool needRedirect, bool verbose)
{
    return Request(Type_POST, url, {}, needRedirect, verbose);
}

HttpResponse HttpClient::Post(const std::string &url, const std::stringbuf &buf, bool needRedirect, bool verbose)
{
    return Request(Type_POST, url, buf, needRedirect, verbose);
}

HttpResponse HttpClient::Request(ssp::net::HttpType type, const std::string &url, const std::stringbuf &buf,
                                 bool needRedirect, bool verbose)
{
    if (type == Type_GET) {
        request_.SetRequestType("GET");
    } else if (type == Type_POST) {
        request_.SetRequestType("POST");
    } else if (type == Type_HEAD) {
        request_.SetRequestType("HEAD");
    }

    auto buffer = GetRequestBuffer(url);
    auto ret = Send(buffer.str().c_str(), buffer.str().size());
    if (ret <= 0) {
        logger.Info("send data to server failed. write bytes:%d", ret);
        return {};
    }

    std::stringbuf recvBuffer;
    ret = Read(recvBuffer);

    HttpResponse response;

    return response;
}

std::stringbuf HttpClient::GetRequestBuffer(const std::string &url)
{
    HttpUrl u(url);
    std::string path = u.path_;
    if (!u.query_.empty()) {
        if (path.back() != '?')
            path.append("?");
        path.append(u.query_);
    } else if (path.empty()) {
        path = "/";
    }
    request_.SetRequestPath(path);
    request_.AddHeader("Host", u.host_);

    request_.ToHeaderBuffer();
    return std::stringbuf{};
}


HttpResponse HttpClient::TransBufferToResponse(Buffer &buffer)
{
    HttpResponse resp(false);
    HttpContext  context;
    if (!context.ParseRequest(&buffer, TimeStamp::Now())) {
        logger.Info("response error");
        return resp;
    }

    auto preSize = context.Request().GetBodySize();
    if (!context.GotEnd()) {
        int32_t size = 0;
        while (true) {
            if (context.GotEnd()) {
                break;
            }

            auto nRead = Read(buffer);
            if (nRead <= 0) {
                logger.Info("read buf length is nullptr");
                break;
            }
            context.ParseRequest(&buffer, TimeStamp::Now());
            size += nRead;
        }
//        logger.Info("preSize:%d left size with body %d, totalSize:%d", preSize, size, context.request().getBodySize());
    }

//    for (auto &item : req.GetRequestHeader()) {
//        resp.AddHeader(item.first, item.second);
//    }
    return resp;
}