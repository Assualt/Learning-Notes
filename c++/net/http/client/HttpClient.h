#pragma once

#include "HttpConnection.h"
#include "base/Logging.h"
#include "base/crypto/base64.h"
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

    HttpResponse Head(const std::string &url, bool needRedirect = false, bool verbose = false);

    template <class Val> void SetHeader(const std::string &key, const Val &val) { request_.setHeader(key, val); }

    void setUserAgent(const std::string &AgentVal) { this->setHeader(UserAgent, AgentVal); }

    void setContentType(const std::string &ContentTypeVal) { this->setHeader(ContentType, ContentTypeVal); }

    void setAcceptLanguage(const std::string &AcceptLanguageVal) { this->setHeader(AcceptLanguage, AcceptLanguageVal); }

    void setAcceptEncoding(const std::string &AcceptEncodingVal) { this->setHeader(AcceptEncoding, AcceptEncodingVal); }

    void setAccept(const std::string &AcceptVal) { this->setHeader(Accept, AcceptVal); }

    void setConnectTimeout(int timeout) { conn_.setTimeOut(timeout); }

    void setHttpVersion(const std::string version) { this->request_.setHttpVersion(version); }

    template <class T> void setHeader(const std::string &key, const T &val) { this->request_.setHeader(key, val); }

    void setCookie(const std::string &strCookie) { this->setHeader(Cookie, strCookie); }

    void SaveResultToFile(const std::string &resultFile, const HttpResponse &res);

    void setBasicAuthUserPass(const std::string &user, const std::string &passwd);

private:
    HttpResponse Request(ReqType type, const std::string &reqUrl, const Buffer &buff, bool redirect, bool verbose);
    Buffer       GetRequestBuffer(const std::string &url);
    HttpResponse TransBufferToResponse(Buffer &buffer);

private:
    HttpConnection conn_;
    HttpRequest    request_;
};