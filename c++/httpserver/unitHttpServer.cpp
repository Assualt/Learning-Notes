#include "httpserver.h"
#include <cmdline.hpp>
#include <dirent.h>
#include <fstream>
#include <regex>
#include <sys/stat.h>
using namespace http;

bool IndexPatter(http::HttpRequest &request, http::HttpResponse &response, HttpConfig &config) {
    std::string resultString = utils::loadFileString("./html/index.html");
    response.setStatusMessage(200, request.getRequestType(), "OK");
    response.setHeader(ContentLength, resultString.size() + 4);
    response.setHeader(ContentType, "text/html; charset=utf-8");
    response.setBodyString(resultString);
    return true;
}

bool NotFoundIndexPatter(http::HttpRequest &request, http::HttpResponse &response, HttpConfig &config) {
    //  logger.info("do 404 function");
    size_t nRead = response.loadFileString("html/40x.html");
    response.setStatusMessage(404, request.getHttpVersion(), "not found");
    response.setHeader(ContentLength, nRead);
    response.setHeader(ContentType, "text/html; charset=utf8");
    return true;
}

bool BadRequestIndexPattern(http::HttpRequest &request, http::HttpResponse &response, HttpConfig &config) {
    response.setStatusMessage(400, request.getHttpVersion(), "Bad Request");
    response.setHeader(ContentLength, strlen(BADREQUEST));
    response.setHeader(ContentType, "text/html; charset=utf8");
    response.setBodyString(BADREQUEST);
    return true;
}

bool DefaultIndexPattern(http::HttpRequest &request, http::HttpResponse &response, HttpConfig &config) {
    // handle File
    long        nSize = 0;
    std::string result;
    std::string strRequestPath = request.getRequestFilePath();
    if (utils::FileIsBinary(strRequestPath)) {
        nSize = response.loadBinaryFile(strRequestPath);
    } else {
        nSize = response.loadFileString(strRequestPath);
    }
    logger.debug("load %s file bytes:%d", strRequestPath, nSize);
    if (nSize <= 0) {
        response.setStatusMessage(404, request.getHttpVersion(), "not found");
        response.setHeader(ContentLength, 0);
    } else {
        response.setStatusMessage(200, request.getHttpVersion(), "OK", request.get(AcceptEncoding));
    }
    response.setHeader(ContentType, config.getMimeType(strRequestPath));
    struct stat st;
    if (stat(strRequestPath.c_str(), &st) != -1)
        response.setHeader("Last-Modified", utils::toResponseBasicDateString(st.st_mtime));
    response.setHeader("Connection", "close");
    return true;
}

bool ListDirIndexPatter(http::HttpRequest &request, http::HttpResponse &response, HttpConfig &config) {
    auto tmplateHtml = config.getDirentTmplateHtml();
    auto currentDir  = config.getServerRoot();
    if (currentDir.back() != '/')
        currentDir += "/";
    if (request.getRequestPath().front() == '/')
        currentDir += request.getRequestPath().substr(1);
    else
        currentDir += request.getRequestPath().substr(0);
    tmplateHtml.append("<script>start(\"");
    tmplateHtml.append(currentDir);
    tmplateHtml.append("\");</script><script>onHasParentDirectory();</script>");
    DIR *          dir = opendir(currentDir.c_str());
    struct dirent *dr;
    while ((dr = readdir(dir)) != nullptr) {
        std::string statFilePath = currentDir;
        if (statFilePath.back() != '/')
            statFilePath.append("/");
        statFilePath.append(dr->d_name);
        struct stat st;
        if (stat(statFilePath.c_str(), &st) != -1) {
            std::string tmpString;
            tmpString.append("<script>");
            tmpString.append("addRow(\"");
            tmpString.append(dr->d_name);
            tmpString.append("\",\"");
            tmpString.append(dr->d_name);
            tmpString.append("\",");
            if (S_ISDIR(st.st_mode & S_IFMT))
                tmpString.append("1,");
            else
                tmpString.append("0,");
            // size
            tmpString.append(to_string(st.st_size));
            // string_size
            tmpString.append(",\"");
            if (!S_ISDIR(st.st_mode & S_IFMT))
                tmpString.append(utils::toSizeString(st.st_size));
            else
                tmpString.append("4096 B");
            tmpString.append("\",");
            tmpString.append(to_string(st.st_mtime));
            tmpString.append(",\"");
            tmpString.append(utils::FileDirentTime(&st));
            tmpString.append("\"");
            tmpString.append(");</script>\r\n");
            tmplateHtml.append(tmpString);
        }
    }
    closedir(dir);

    response.setStatusMessage(200, "HTTP/1.1", "OK");
    response.setBodyString(tmplateHtml);
    response.setHeader(ContentType, "text/html");
    response.setHeader(ContentLength, tmplateHtml.size());
    response.setHeader("Date", utils::toResponseBasicDateString());
    response.setHeader("Connection", "close");
    return true;
}

bool AuthRequiredIndexPattern(http::HttpRequest &request, http::HttpResponse &response, HttpConfig &config) {
    response.setStatusMessage(401, "HTTP/1.1", "Unauthorized");
    response.setHeader(ContentType, "text/html");
    response.setHeader(ContentLength, strlen(AUTHREQUIRED));
    response.setHeader("WWW-Authenticate", config.getAuthName());
    response.setBodyString(AUTHREQUIRED);
    return true;
}

int main(int argc, char **argv) {
    logger.BasicConfig("%(thread)s %(levelname)s %(ctime)s [%(filename)s-%(lineno)s-%(funcName)s] %(message)s", "%Y-%m-%d %H:%M:%S,%s", "", "a");

    cmdline::parser CommandParse;
    CommandParse.add("version", 'v', "show this HttpServer Version and exit");
    CommandParse.add<int>("threads_count", 'n', "The http server's threads count", false, 3, cmdline::range<int>(1, 10));
    CommandParse.add<std::string>("config_path", 'c', "The http server's config path.", true);
    CommandParse.add<int>("logLevel", 'l', "The http server's logs level.", false, 1, cmdline::range<int>(0, 3));
    bool ok = CommandParse.parse(argc, argv);

    if (!ok) {
        std::cout << CommandParse.error() << std::endl;
        std::cout << CommandParse.usage() << std::endl;
    } else {
        std::string strConfigPath = CommandParse.get<std::string>("config_path");

        auto nLevel = CommandParse.get<int>("logLevel");
        logger.setLevel(static_cast<tlog::detail::Level>(nLevel));

        http::HttpServer server;
        server.loadHttpConfig(strConfigPath);
        logger.info("Run in server Root:%s", server.getServerRoot());
        // server.getHttpConfig().loadMimeType("html/mime.types");
        // server.getHttpConfig().loadDirentTmplateHtml("./html/dirHtml.tmpl");
        auto &mapper = server.getMapper();
        mapper.addRequestMapping({"/index"}, std::move(IndexPatter));
        mapper.addRequestMapping({"/404"}, std::move(NotFoundIndexPatter));
        mapper.addRequestMapping({"/#/"}, std::move(DefaultIndexPattern));
        mapper.addRequestMapping({"/#//"}, std::move(ListDirIndexPatter));
        mapper.addRequestMapping({"/401"}, std::move(AuthRequiredIndexPattern));
        mapper.addRequestMapping({"/400"}, std::move(BadRequestIndexPattern));
        server.StartThreads(CommandParse.get<int>("threads_count"));
        server.ExecForever();
    }
    return 0;
}
