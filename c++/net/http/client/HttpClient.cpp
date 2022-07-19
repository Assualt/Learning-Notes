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
        std::cout << request_;
    }

    Buffer respBuf;
    size_t readBytes = conn_.recv(respBuf);
    if (readBytes <= 0) {
        logger.info("read data to server failed. write bytes:%d", readBytes);
        return resp;
    }

    resp = TransBufferToResponse(respBuf);
    if (verbose) {
        std::cout << resp;
    }

    return resp;
}

HttpResponse HttpClient::TransBufferToResponse(Buffer &buffer) {
    HttpResponse resp(false);
    HttpContext  context;
    if (!context.parseRequest(&buffer, Timestamp())) {
        logger.info("response error");
        return resp;
    }

    if (context.gotAll()) {
        HttpRequest req = context.request();
        for (auto &item : req.GetRequestHeader()) {
            resp.addHeader(item.first, item.second);
        }

        resp.setStatusMessage(static_cast<HttpResponse::HttpStatusCode>(req.getStatusCode()), req.getHttpVersion(), req.getStatusMessage());

        if (req.get(ContentType).find("text/") != std::string::npos) {
            auto encodingType = req.get(ContentEncoding);
            if (strcasecmp(encodingType.c_str(), "gzip") == 0) { //
                auto           buf = req.getBodyBuffer();
                MyStringBuffer in, out;
                in.sputn(buf.peek(), buf.readableBytes());
                ZlibStream::GzipDecompress(in, out);
                resp.setBody(out.toString());
            } else {
                if (!req.getPostParams().empty()) {
                    resp.setBody(req.getPostParams());
                } else {
                    resp.setBody(std::string(req.getBodyBuffer().peek(), req.getBodyBuffer().readableBytes()));
                }
            }
        }
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