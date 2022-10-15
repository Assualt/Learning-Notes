#pragma once

#include "base/Logging.h"
#include "base/crypto/base64.h"
#include "net/Buffer.h"
#include "net/TcpClient.h"
#include "net/http/HttpConfig.h"
#include "net/http/HttpRequest.h"
#include "net/http/HttpResponse.h"
#include <iostream>

using muduo::net::TcpClient;
class HttpClient : TcpClient {
public:
    enum ReqType { Type_GET, Type_POST, Type_PUT, Type_HEAD };

public:
    HttpClient();

    HttpResponse Get(const std::string &url, bool needRedirect = false, bool verbose = false);

    HttpResponse Post(const std::string &url, const Buffer &buf, bool needRedirect = false, bool verbose = false);

    HttpResponse Head(const std::string &url, bool needRedirect = false, bool verbose = false);

    template <class Val> void SetHeader(const std::string &key, const Val &val) { request_.setHeader(key, val); }

    void setUserAgent(const std::string &AgentVal) { this->setHeader(UserAgent, AgentVal); }

    void setContentType(const std::string &ContentTypeVal) { this->setHeader(ContentType, ContentTypeVal); }

    void setAcceptLanguage(const std::string &AcceptLanguageVal) { this->setHeader(AcceptLanguage, AcceptLanguageVal); }

    void setAcceptEncoding(const std::string &AcceptEncodingVal) { this->setHeader(AcceptEncoding, AcceptEncodingVal); }

    void setAccept(const std::string &AcceptVal) { this->setHeader(Accept, AcceptVal); }

    void setConnectTimeout(int timeout) { TcpClient::setTimeOut(timeout, 0, 0); }

    void setHttpVersion(const std::string &version) { this->request_.setHttpVersion(version); }

    template <class T> void setHeader(const std::string &key, const T &val) { this->request_.setHeader(key, val); }

    void setCookie(const std::string &strCookie) { this->setHeader(Cookie, strCookie); }

    void SaveResultToFile(const std::string &resultFile, const HttpResponse &res);

    void setBasicAuthUserPass(const std::string &user, const std::string &passwd);

private:
    HttpResponse Request(ReqType type, const std::string &reqUrl, const Buffer &buff, bool redirect, bool verbose);
    Buffer       GetRequestBuffer(const std::string &url);
    HttpResponse TransBufferToResponse(Buffer &buffer);
    static bool  IsBinaryContentType(const std::string &type);

    void TryDecodeBuffer(const HttpRequest &req, HttpResponse &resp);

private:
    HttpRequest request_;
    std::string lastRequestHost_;
};