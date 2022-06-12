#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServer.h"
#include "HttpUtils.h"
#include "base/Backtrace.h"
#include "base/DirScanner.h"
#include "base/Format.h"
#include "base/Logging.h"
#include <dirent.h>
#include <signal.h>
using std::string;
using namespace muduo;
extern char favicon[ 555 ];

bool IndexPatter(const HttpRequest &request, HttpResponse &response, HttpConfig &config) {
    std::string resultString;
    response.setStatusMessage(200, request.getRequestType(), "OK");
    response.addHeader(ContentType, "text/html; charset=utf-8");
    response.setBody(resultString);
    return true;
}

bool NotFoundIndexPatter(const HttpRequest &request, HttpResponse &response, HttpConfig &config) {
    size_t nRead = 0;
    response.setStatusMessage(404, request.getHttpVersion(), "not found");
    response.addHeader(ContentType, "text/html; charset=utf8");
    return true;
}

bool BadRequestIndexPattern(const HttpRequest &request, HttpResponse &response, HttpConfig &config) {
    response.setStatusMessage(400, request.getHttpVersion(), "Bad Request");
    response.addHeader(ContentType, "text/html; charset=utf8");
    response.setBody(BADREQUEST);
    return true;
}

bool DefaultIndexPattern(const HttpRequest &request, HttpResponse &response, HttpConfig &config) {
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
        response.setBodyStream(mybuf, HttpResponse::EncodingType::Type_Gzip);
        response.setStatusMessage(200, request.getHttpVersion(), "OK", request.get(AcceptEncoding));
    }
    response.addHeader(ContentType, utils::FileMagicType(strRequestPath));
    struct stat st;
    if (stat(strRequestPath.c_str(), &st) != -1)
        response.addHeader("Last-Modified", utils::toResponseBasicDateString(st.st_mtime));

    return true;
}

bool AuthRequiredIndexPattern(const HttpRequest &request, HttpResponse &response, HttpConfig &config) {
    response.setStatusMessage(401, "HTTP/1.1", "Unauthorized");
    response.addHeader(ContentType, "text/html");
    response.addHeader("WWW-Authenticate", config.getAuthName());
    response.setBody(AUTHREQUIRED);
    return true;
}

bool ListDirIndexPatter(const HttpRequest &request, HttpResponse &response, HttpConfig &config) {
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
}
/// login/{user}/{id}
// bool AuthLoginIndexPattern(const HttpRequest &request, HttpResponse &response, HttpConfig &config) {
//     std::string        user = request.getParams("user");
//     int                id   = atoi(request.getParams("id").c_str());
//     json::Json::object obj;
//     auto               kvalmap = request.getAllParams();
//     for (auto &iter : kvalmap) {
//         obj.insert(iter);
//     }
//     json::Json retJson({{"ret", 200}, {"errmsg", ""}, {"data", json::Json(obj)}, {"postdata", request.getPostParams()}});

//     response.setStatusMessage(200, request.getHttpVersion(), "OK");
//     response.addHeader(ContentType, "application/json");
//     response.addHeader(ContentLength, retJson.dump().size());
//     response.setBody(retJson.dump());
//     return true;
// }

bool MethodAllowIndexPatterm(const HttpRequest &request, HttpResponse &response, HttpConfig &config) {
    response.setStatusMessage(405, request.getHttpVersion(), "Method Not Allowed");
    response.addHeader(ContentType, "text/html");
    std::string methodAllowString = METHODNOTALLOWED;
    response.setBody(methodAllowString);
    return true;
}

bool IndexFaviconPattern(const HttpRequest &req, HttpResponse &resp, HttpConfig &config) {
    resp.setStatusMessage(200, req.getHttpVersion(), "OK");
    resp.addHeader(ContentType, "image/x-icon");
    resp.addHeader("Accept-Ranges", "bytes");
    resp.setBody(favicon);
    return true;
}

void registerIndexPattern(HttpServer &server) {
    auto &mapper = server.getMapper();
    mapper.addRequestMapping({"/favicon.ico"}, std::move(IndexFaviconPattern));
    mapper.addRequestMapping({"/index"}, std::move(IndexPatter));
    mapper.addRequestMapping({"/404"}, std::move(NotFoundIndexPatter));
    mapper.addRequestMapping({"/#/"}, std::move(DefaultIndexPattern));
    mapper.addRequestMapping({"/#//"}, std::move(ListDirIndexPatter));
    mapper.addRequestMapping({"/405"}, std::move(MethodAllowIndexPatterm));
    mapper.addRequestMapping({"/401"}, std::move(AuthRequiredIndexPattern));
    mapper.addRequestMapping({"/400"}, std::move(BadRequestIndexPattern));
    // mapper.addRequestMapping({"/login/{user}/{id}", "POST", true}, std::move(AuthLoginIndexPattern));
}

void RegisterSignalHandle(HttpServer &server) {
    auto handle = [](int sig, uintptr_t param) {
        if (sig == SIGSEGV) {
            auto callstack = GetBackCallStack();
            logger.info("callstack is:\n%s", callstack.c_str());
        }
        auto server = reinterpret_cast<HttpServer *>(param);
        // server->Exit();
        exit(0);
    };
    server.RegSignalCallback(SIGINT, reinterpret_cast<uintptr_t>(&server), handle);
    server.RegSignalCallback(SIGHUP, reinterpret_cast<uintptr_t>(&server), handle);
    server.RegSignalCallback(SIGSEGV, reinterpret_cast<uintptr_t>(&server), handle);
}

int main(int argc, char const *argv[]) {
    auto &log = Logger::getLogger();
    log.BasicConfig(Logger::Info, "T:%(tid)(%(asctime))[%(appname):%(levelname)][%(filename):%(lineno)] %(message)", "", "");
    log.setAppName("app");
    log.addLogHandle(new StdOutLogHandle);
    log.addLogHandle(new RollingFileLogHandle("./", "http_server.log"));

    EventLoop  loop;
    HttpServer server(&loop, InetAddress(8100));
    RegisterSignalHandle(server);
    server.SetThreadNum(10);
    registerIndexPattern(server);
    server.Start();
    loop.loop();

    return 0;
}

char favicon[ 555 ] = {
    '\x89', 'P',    'N',    'G',    '\xD',  '\xA',  '\x1A', '\xA',  '\x0',  '\x0',  '\x0',  '\xD',  'I',    'H',    'D',    'R',    '\x0',  '\x0',  '\x0',  '\x10', '\x0',  '\x0',  '\x0',  '\x10',
    '\x8',  '\x6',  '\x0',  '\x0',  '\x0',  '\x1F', '\xF3', '\xFF', 'a',    '\x0',  '\x0',  '\x0',  '\x19', 't',    'E',    'X',    't',    'S',    'o',    'f',    't',    'w',    'a',    'r',
    'e',    '\x0',  'A',    'd',    'o',    'b',    'e',    '\x20', 'I',    'm',    'a',    'g',    'e',    'R',    'e',    'a',    'd',    'y',    'q',    '\xC9', 'e',    '\x3C', '\x0',  '\x0',
    '\x1',  '\xCD', 'I',    'D',    'A',    'T',    'x',    '\xDA', '\x94', '\x93', '9',    'H',    '\x3',  'A',    '\x14', '\x86', '\xFF', '\x5D', 'b',    '\xA7', '\x4',  'R',    '\xC4', 'm',
    '\x22', '\x1E', '\xA0', 'F',    '\x24', '\x8',  '\x16', '\x16', 'v',    '\xA',  '6',    '\xBA', 'J',    '\x9A', '\x80', '\x8',  'A',    '\xB4', 'q',    '\x85', 'X',    '\x89', 'G',    '\xB0',
    'I',    '\xA9', 'Q',    '\x24', '\xCD', '\xA6', '\x8',  '\xA4', 'H',    'c',    '\x91', 'B',    '\xB',  '\xAF', 'V',    '\xC1', 'F',    '\xB4', '\x15', '\xCF', '\x22', 'X',    '\x98', '\xB',
    'T',    'H',    '\x8A', 'd',    '\x93', '\x8D', '\xFB', 'F',    'g',    '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f',    'v',    'f',    '\xDF', '\x7C', '\xEF', '\xE7', 'g',    'F',    '\xA8',
    '\xD5', 'j',    'H',    '\x24', '\x12', '\x2A', '\x0',  '\x5',  '\xBF', 'G',    '\xD4', '\xEF', '\xF7', '\x2F', '6',    '\xEC', '\x12', '\x20', '\x1E', '\x8F', '\xD7', '\xAA', '\xD5', '\xEA',
    '\xAF', 'I',    '5',    'F',    '\xAA', 'T',    '\x5F', '\x9F', '\x22', 'A',    '\x2A', '\x95', '\xA',  '\x83', '\xE5', 'r',    '9',    'd',    '\xB3', 'Y',    '\x96', '\x99', 'L',    '\x6',
    '\xE9', 't',    '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',    '\xA7', '\xC4', 'b',    '1',    '\xB5', '\x5E', '\x0',  '\x3',  'h',    '\x9A', '\xC6', '\x16', '\x82', '\x20', 'X',    'R',
    '\x14', 'E',    '6',    'S',    '\x94', '\xCB', 'e',    'x',    '\xBD', '\x5E', '\xAA', 'U',    'T',    '\x23', 'L',    '\xC0', '\xE0', '\xE2', '\xC1', '\x8F', '\x0',  '\x9E', '\xBC', '\x9',
    'A',    '\x7C', '\x3E', '\x1F', '\x83', 'D',    '\x22', '\x11', '\xD5', 'T',    '\x40', '\x3F', '8',    '\x80', 'w',    '\xE5', '3',    '\x7',  '\xB8', '\x5C', '\x2E', 'H',    '\x92', '\x4',
    '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g',    '\x98', '\xE9', '6',    '\x1A', '\xA6', 'g',    '\x15', '\x4',  '\xE3', '\xD7', '\xC8', '\xBD', '\x15', '\xE1', 'i',    '\xB7', 'C',
    '\xAB', '\xEA', 'x',    '\x2F', 'j',    'X',    '\x92', '\xBB', '\x18', '\x20', '\x9F', '\xCF', '3',    '\xC3', '\xB8', '\xE9', 'N',    '\xA7', '\xD3', 'l',    'J',    '\x0',  'i',    '6',
    '\x7C', '\x8E', '\xE1', '\xFE', 'V',    '\x84', '\xE7', '\x3C', '\x9F', 'r',    '\x2B', '\x3A', 'B',    '\x7B', '7',    'f',    'w',    '\xAE', '\x8E', '\xE',  '\xF3', '\xBD', 'R',    '\xA9',
    'd',    '\x2',  'B',    '\xAF', '\x85', '2',    'f',    'F',    '\xBA', '\xC',  '\xD9', '\x9F', '\x1D', '\x9A', 'l',    '\x22', '\xE6', '\xC7', '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15',
    '\x90', '\x7',  '\x93', '\xA2', '\x28', '\xA0', 'S',    'j',    '\xB1', '\xB8', '\xDF', '\x29', '5',    'C',    '\xE',  '\x3F', 'X',    '\xFC', '\x98', '\xDA', 'y',    'j',    'P',    '\x40',
    '\x0',  '\x87', '\xAE', '\x1B', '\x17', 'B',    '\xB4', '\x3A', '\x3F', '\xBE', 'y',    '\xC7', '\xA',  '\x26', '\xB6', '\xEE', '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
    '\xA',  '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X',    '\x0',  '\x27', '\xEB', 'n',    'V',    'p',    '\xBC', '\xD6', '\xCB', '\xD6', 'G',    '\xAB', '\x3D', 'l',    '\x7D', '\xB8', '\xD2',
    '\xDD', '\xA0', '\x60', '\x83', '\xBA', '\xEF', '\x5F', '\xA4', '\xEA', '\xCC', '\x2',  'N',    '\xAE', '\x5E', 'p',    '\x1A', '\xEC', '\xB3', '\x40', '9',    '\xAC', '\xFE', '\xF2', '\x91',
    '\x89', 'g',    '\x91', '\x85', '\x21', '\xA8', '\x87', '\xB7', 'X',    '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N',    'N',    'b',    't',    '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
    '\xEC', '\x86', '\x2',  'H',    '\x26', '\x93', '\xD0', 'u',    '\x1D', '\x7F', '\x9',  '2',    '\x95', '\xBF', '\x1F', '\xDB', '\xD7', 'c',    '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF',
    '\x22', 'J',    '\xC3', '\x87', '\x0',  '\x3',  '\x0',  'K',    '\xBB', '\xF8', '\xD6', '\x2A', 'v',    '\x98', 'I',    '\x0',  '\x0',  '\x0',  '\x0',  'I',    'E',    'N',    'D',    '\xAE',
    'B',    '\x60', '\x82',
};
