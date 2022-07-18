#include "HttpClient.h"
#include "net/http/HttpContext.h"

using namespace muduo::net;

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

    if (type == Type_GET) {
        request_.setRequestType("GET");
    } else if (type == Type_POST) {
        request_.setRequestType("POST");
    }

    size_t writeBytes = conn_.send(GetRequestBuffer(reqUrl));
    if (writeBytes <= 0) {
        logger.info("send data to server failed. write bytes:%d", writeBytes);
        return resp;
    }

    if (verbose) {
        logger.info("request Header:\n%s", request_.toStringHeader());
    }

    Buffer respBuf;
    size_t readBytes = conn_.recv(respBuf);
    if (readBytes <= 0) {
        logger.info("read data to server failed. write bytes:%d", readBytes);
        return resp;
    }

    if (verbose) {
        logger.info("response body:\n%s size:%d", respBuf.peek(), respBuf.readableBytes());
    }

    return TransBufferToResponse(respBuf);
}

HttpResponse HttpClient::TransBufferToResponse(Buffer &buffer) {
    HttpResponse resp(false);
    HttpContext context;
    if (!context.parseRequest(&buffer, Timestamp())) {
        logger.info("response error");
        return resp;
    }

    if (context.gotAll()) {
        HttpRequest req = context.request();
        req.setHttpVersion(req.getHttpVersion());
        req.setRequestType(req.getRequestPath());
        for (auto &item : req.GetHeaderMap()) {
            resp.addHeader(item.first, item.second);
        }

        resp.setStatusCode(static_cast<HttpResponse::HttpStatusCode>(req.getStatusCode()));
        resp.setStatusMessage(req.getStatusMessage());
        resp.setBody(req.getPostParams());
    }

    return resp;
}

Buffer HttpClient::GetRequestBuffer(const std::string &url) {
    HttpUrl     u(url);
    std::string path = u.path;
    if (!u.query.empty()) {
        if (path.back() != '?')
            path.append("?");
        path.append(u.query);
    }
    request_.setRequestPath(path);
    request_.setHeader("Host", u.host);

    Buffer      buf;
    std::string header = request_.toStringHeader();
    buf.append(header.c_str(), header.size());
    return buf;
}