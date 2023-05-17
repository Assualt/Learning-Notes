#include "HttpClient.h"
#include "HttpUrl.h"
#include "base/StringUtil.h"
#include "net/http/HttpContext.h"
#include <ctime>

using namespace muduo::net;

HttpClient::HttpClient()
    : TcpClient(true) {}

HttpResponse HttpClient::Get(const string &url, bool needRedirect, bool verbose) {
    Buffer buffer;
    return Request(Type_GET, url, buffer, needRedirect, verbose);
}

HttpResponse HttpClient::Post(const string &url, const Buffer &body, bool needRedirect, bool verbose) {
    return Request(ReqType::Type_POST, url, body, needRedirect, verbose);
}

HttpResponse HttpClient::Head(const std::string &url, bool needRedirect, bool verbose) {
    Buffer buffer;
    return Request(ReqType::Type_HEAD, url, buffer, needRedirect, verbose);
}

HttpResponse HttpClient::Request(HttpClient::ReqType type, const string &url, const Buffer &buff, bool needRedirect,
                                 bool verbose) {
    HttpResponse resp(false);
    std::string  reqUrl = url;
    HttpUrl      u(reqUrl);
    bool         switchSSL = false;
    if (u.scheme == "https") {
        switchSSL = true;
    }

    if (!connect(u.host, u.port, switchSSL)) {
        logger.info("connect the url:%s error", reqUrl);
        return resp;
    }

    if (verbose) {
        TcpClient::showTlsInfo();
    }

    if (type == Type_GET) {
        request_.setRequestType("GET");
    } else if (type == Type_POST) {
        request_.setRequestType("POST");
    } else if (type == Type_HEAD) {
        request_.setRequestType("HEAD");
    }

request:
    struct timespec startTime = {0};
    clock_gettime(CLOCK_REALTIME, &startTime);
    size_t writeBytes = sendRequest(GetRequestBuffer(reqUrl));
    if (writeBytes <= 0) {
        logger.info("send data to server failed. write bytes:%d", writeBytes);
        return resp;
    }

    if (verbose) {
        std::cout << request_;
    }

    Buffer respBuf;
    size_t readBytes = recvResponse(respBuf);
    if (readBytes <= 0) {
        logger.info("read data to server failed. write bytes:%d", readBytes);
        return resp;
    }

    resp = TransBufferToResponse(respBuf);
    if (verbose) {
        std::cout << resp;
    }

    timespec endTime;
    clock_gettime(CLOCK_REALTIME, &endTime);

    float costTime =
        (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) / 1000.0 / 1000.0 / 1000.0;
    auto downloadBits = resp.getBody().size() * 1.0;
    if (downloadBits == 0) {
        downloadBits = resp.getBodyBuf().readableBytes() * 1.0;
    }

    auto speed = downloadBits / 1024.0 / costTime;
    logger.info("download %d KB, cost time:%f ms, speed %f kb/s", downloadBits / 1024.0, costTime * 1000, speed);

    if ((resp.getStatusCode() == HttpStatusCode::k302MovedPermanently ||
         resp.getStatusCode() == HttpStatusCode::k301MovedPermanently) &&
        needRedirect) {
        reqUrl = resp.getHeader(Location);
        logger.info("begin to redirect url:%s ", reqUrl);
        if (util::StartsWithIgnoreCase(reqUrl, "http")) { // 说明是一个非当前host的地址,需要重新链接
            TcpClient::close();
            HttpUrl o(reqUrl);
            switchSSL = o.scheme == "https";
            auto ret  = connect(o.host, o.port, switchSSL);
            logger.info("reconnect to url:%s result is %b", reqUrl, ret);
        }
        goto request;
    }

    return resp;
}

bool HttpClient::IsBinaryContentType(const std::string &type) {
    if (strncasecmp(type.c_str(), "text/", 5) == 0) {
        return true;
    } else if (util::EqualsIgnoreCase(type, "application/xml") || util::EqualsIgnoreCase(type, "application/json") ||
               util::EqualsIgnoreCase(type, "application/x-www-form-urlencoded") ||
               util::EqualsIgnoreCase(type, "application/xhtml+xml") ||
               util::EqualsIgnoreCase(type, "application/atom+xml")) {
        return true;
    }
    return false;
}

void HttpClient::TryDecodeBuffer(const HttpRequest &req, HttpResponse &resp) {
    auto           encodeType = req.get(ContentEncoding);
    const auto &   buf        = req.getBodyBuffer();
    MyStringBuffer in, out;
    in.sputn(buf.peek(), buf.readableBytes());
    if (util::EqualsIgnoreCase(encodeType, "gzip")) {
        auto length = ZlibStream::GzipDecompress(in, out);
        logger.info("gzip decode length is %d", length);
    } else if (util::EqualsIgnoreCase(encodeType, "deflate")) {
        auto length = ZlibStream::DeflateDecompress(in, out);
        logger.info("deflate decode length is %d", length);
    } else if (util::EqualsIgnoreCase(encodeType, "zlib")) {
        auto length = ZlibStream::ZlibDeCompress(in, out);
        logger.info("zlib decode length is %d", length);
    } else {
        resp.setBody(buf);
        return;
    }

    auto isText = IsBinaryContentType(req.get(ContentType));
    if (isText) {
        resp.setBody(out.toString());
    } else {
        resp.setBody(buf);
    }
}

HttpResponse HttpClient::TransBufferToResponse(Buffer &buffer) {
    HttpResponse resp(false);
    HttpContext  context;
    if (!context.parseRequest(&buffer, Timestamp())) {
        logger.info("response error");
        return resp;
    }

    auto preSize = context.request().getBodySize();
    if (!context.gotEnd()) {
        int32_t size = 0;
        while (true) {
            if (context.gotEnd()) {
                break;
            }
            auto nRead = recvResponse(buffer);
            if (nRead <= 0) {
                logger.info("recv buf length is nullptr");
                break;
            }
            context.parseRequest(&buffer, Timestamp());
            size += nRead;
        }
        logger.info("preSize:%d left size with body %d, totalSize:%d", preSize, size, context.request().getBodySize());
    }

    HttpRequest req = context.request();
    for (auto &item : req.GetRequestHeader()) {
        resp.addHeader(item.first, item.second);
    }
    resp.setStatusMessage(static_cast<HttpStatusCode>(req.getStatusCode()), req.getHttpVersion(),
                          req.getStatusMessage());

    TryDecodeBuffer(req, resp);
    return resp;
}

Buffer HttpClient::GetRequestBuffer(const std::string &url) {
    HttpUrl     u(url);
    std::string path = u.path;
    if (!u.query.empty()) {
        if (path.back() != '?')
            path.append("?");
        path.append(u.query);
    } else if (path.empty()) {
        path = "/";
    }
    request_.setRequestPath(path);
    request_.setHeader("Host", u.host);

    Buffer      buf;
    std::string header = request_.toStringHeader();
    buf.append(header.c_str(), header.size());
    return buf;
}

void HttpClient::SaveResultToFile(const std::string &resultFile, const HttpResponse &res) {
    std::ofstream fout(resultFile);
    if (res.getBodyBuf().readableBytes() != 0) {
        fout.write(res.getBodyBuf().peek(), res.getBodyBuf().readableBytes());
    } else {
        fout.write(res.getBody().c_str(), res.getBody().size());
    }

    fout.close();
}

void HttpClient::setBasicAuthUserPass(const std::string &user, const std::string &passwd) {
    std::stringstream ss;
    ss << user << ":" << passwd;
    std::string output;
    output.resize(ss.str().size() * 1.5);
    base64::encoder encoder;
    encoder.encode_str(ss.str().c_str(), ss.str().size(), output.data());
    this->setHeader(Authorization, "Basic " + output);
}