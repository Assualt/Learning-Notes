#include "DefaultController.h"
#include "base/DirScanner.h"
#include "base/Logging.h"
using namespace muduo::base;

REG_OBJECT(DefaultController)
REG_OBJECT(FileIndex)

bool DefaultController::onGet(const HttpRequest &request, HttpResponse &response, const HttpConfig &config) {
    auto        tmplateHtml = config.getDirentTmplateHtml();
    std::string currentDir  = config.getServerRoot();
    if (currentDir.back() != '/')
        currentDir += "/";
    if (request.getRequestPath().front() == '/')
        currentDir += request.getRequestPath().substr(1);
    else
        currentDir += request.getRequestPath();
    tmplateHtml.append("<script>start(\"");
    tmplateHtml.append(currentDir);
    tmplateHtml.append("\");</script><script>onHasParentDirectory();</script>");

    DirScanner scanner(currentDir.c_str());
    FileAttr   attr;
    while (scanner.Fetch(attr)) {
        std::stringstream out;
        attr.SetParentPath(currentDir);
        out << "<script>addRow(\"" << attr.GetName() << "\",\"" << attr.GetName() << "\",";
        if (attr.IsDir()) {
            out << "1," << attr.GetSize() << ",\"4096 B\",";
        } else {
            out << "0," << attr.GetSize() << ",\"" << utils::toSizeString(attr.GetSize()) << "\",";
        }
        out << attr.GetModifyTime().seconds() << ",\"" << attr.GetModifyTime().toFormattedString("%Y/%m/%d %H:%M:%S") << "\");</script>\r\n";
        tmplateHtml.append(out.str());
    }
    response.setStatusMessage(200, "HTTP/1.1", "OK");
    response.setBody(tmplateHtml);
    response.addHeader(ContentType, "text/html");
    response.addHeader("Date", utils::toResponseBasicDateString());
    return true;
    return true;
}

bool DefaultController::onPost(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
    return onGet(req, res, cfg);
}

bool DefaultController::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}

bool FileIndex::onGet(const HttpRequest &request, HttpResponse &response, const HttpConfig &config) {
    // handle File
    std::string strRequestPath = FmtString("%/%").arg(config.getServerRoot()).arg(request.getRequestFilePath()).str();
    if (!utils::FileIsBinary(strRequestPath)) {
        std::string result = utils::loadFileString(strRequestPath);
        response.setBody(result);
        logger.info("load %s file bytes:%d, errno:%d", strRequestPath, result.size(), errno);
        if (result.size() <= 0) {
            response.setStatusMessage(404, request.getHttpVersion(), "not found");
        } else {
            response.setStatusMessage(200, request.getHttpVersion(), "OK", request.get(AcceptEncoding));
        }
    } else {
        MyStringBuffer mybuf;
        int            n = utils::loadBinaryStream(strRequestPath, mybuf);
        logger.info("load %s file bytes:%d", strRequestPath, n);
        auto buf = std::make_unique<char[]>(n);
        mybuf.sgetn(buf.get(), n);
        response.setBodyStream(buf.get(), n, HttpResponse::EncodingType::Type_Gzip);
        response.setStatusMessage(200, request.getHttpVersion(), "OK", request.get(AcceptEncoding));
    }
    response.addHeader(ContentType, utils::FileMagicType(strRequestPath));
    struct stat st;
    if (stat(strRequestPath.c_str(), &st) != -1)
        response.addHeader("Last-Modified", utils::toResponseBasicDateString(st.st_mtime));

    return true;
}

bool FileIndex::onPost(const HttpRequest &req, HttpResponse &res, const HttpConfig &cfg) {
    res.setStatusMessage(401, "HTTP/1.1", "Unauthorized=>on");
    res.addHeader(ContentType, "text/html");
    res.addHeader("WWW-Authenticate", cfg.getAuthName());
    res.setBody("on Post");
    return true;
}

bool FileIndex::onPut(const HttpRequest &, HttpResponse &res, const HttpConfig &cfg) {
    return true;
}