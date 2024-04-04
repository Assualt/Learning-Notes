//
// Created by 侯鑫 on 2024/1/10.
//

#include "http_client.h"
#include "http_context.h"
#include "source/net/protocol/http_config.h"
#include "base/timestamp.h"
#include "base/log.h"
#include "base/file.h"

using namespace ssp::net;
using namespace ssp::base;

HttpClient::HttpClient()
    : TcpClient(false)
{}

HttpResponse HttpClient::Head(const std::string &url, bool needRedirect, bool verbose)
{
    return Request(Type_HEAD, url, {}, needRedirect, verbose);
}

HttpResponse HttpClient::Get(const std::string &url, bool needRedirect, bool verbose)
{
    return Request(Type_GET, url, {}, needRedirect, verbose);
}

HttpResponse HttpClient::Post(const std::string &url, const std::stringbuf &buf, bool needRedirect, bool verbose)
{
    return Request(Type_POST, url, buf, needRedirect, verbose);
}

HttpResponse HttpClient::Request(ssp::net::HttpType type, const std::string &url, const std::stringbuf &buf,
                                 bool needRedirect, bool verbose)
{
    HttpUrl u(url);
    if (!u.IsValid()) {
        logger.Warning("url:%s is not normal .", url);
        return HttpResponse{false};
    }

    if (!Connect(u.host_, u.port_, u.scheme_ == "https", verbose)) {
        return HttpResponse{false};
    }

    if (type == Type_GET) {
        request_.SetRequestType("GET");
    } else if (type == Type_POST) {
        request_.SetRequestType("POST");
        request_.SetPostParams(buf.str());
    } else if (type == Type_HEAD) {
        request_.SetRequestType("HEAD");
    }

    auto buffer = GetRequestBuffer(u);
    if (verbose) {
        std::cout << "*    Trying " << u.fullUrl_ << "...." << std::endl
            << "* Connected to " << u.host_ << " (" << u.host_ << ") port " << u.port_ << std::endl;
        std::cout << request_;
    }

    auto ret = Send(buffer.c_str(), buffer.size());
    if (ret <= 0) {
        logger.Info("send data to server failed. write bytes:%d", ret);
        return HttpResponse{false};
    }

    Buffer recvBuffer;
    ret = Read(recvBuffer);
    if (ret <= 0) {
        logger.Info("recv buffer error. ret:%d", ret);
        return HttpResponse{false};
    }

    auto resp = TransBufferToResponse(recvBuffer);
    if (verbose) {
        std::cout << resp;
    }

    return resp;
}

std::string HttpClient::GetRequestBuffer(const HttpUrl &u)
{
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
    return request_.ToHeaderBuffer();
}

HttpResponse HttpClient::TransBufferToResponse(Buffer &buffer)
{
    HttpContext  context;
    if (!context.ParseRequest(&buffer, TimeStamp::Now())) {
        logger.Info("response error");
        return HttpResponse{false};
    }

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
        logger.Info("preSize: left size with body totalSize:%d", size);
    }

    return context.GetResponse();
}

void HttpClient::InitDefaultHeader()
{
    request_.AddHeader(UserAgent, USER_AGENT_DEFAULT);
    request_.AddHeader(Accept, "*/*");
}


void HttpClient::SaveToFile(const std::string &filePath, const HttpResponse& response)
{
    auto file = File::New(filePath, FileOpType::CREATE | FileOpType::TRUNC | FileOpType::RDWR, 0660);
    auto buffer = response.GetOutBuffer();
    file.Write((void *)buffer.peek(), buffer.readableBytes());
    file.Close();
}
