#include "httpserver.h"
#include <cmdline.hpp>
#include <dirent.h>
#include <fstream>
#include <regex>
#include <sys/stat.h>
using namespace http;

bool IndexPatter(http::HttpRequest &request, http::HttpResponse &response) {
    std::string resultString = utils::loadFileString("./html/index.html");
    response.setStatusMessage(200, request.getRequestType(), "OK");
    response.setHeader(ContentLength, resultString.size() + 4);
    response.setHeader(ContentType, "text/html");
    response.setBodyString(resultString);
    return true;
}

int main(int argc, char **argv) {
    logger.BasicConfig("%(thread)s %(levelname)s %(ctime)s [%(filename)s-%(lineno)s-%(funcName)s] %(message)s", "%Y-%m-%d %H:%M:%S,%s", "", "a");

    cmdline::parser CommandParse;

    CommandParse.add<std::string>("server_name", 0, "The http server name", false, "HttpServer");
    CommandParse.add<std::string>("server_ip", 0, "The http server ip.", false, "127.0.0.1");
    CommandParse.add<std::string>("server_description", 0, "The http server's description.", false, "A simple Http Server");
    CommandParse.add<int>("server_port", 0, "The http server's port", false, 8080, cmdline::range<int>(1, 65535));
    CommandParse.add<std::string>("server_root", 0, "The http server's root path", true);
    CommandParse.add<int>("threads_count", 'n', "The http server's threads count", false, 3, cmdline::range<int>(1, 10));
    CommandParse.add<std::string>("config_path", 0, "The http server's config path.", false, "httpd.conf");
    CommandParse.add<int>("logLevel", 0, "The http server's logs level.", false, 1, cmdline::range<int>(0, 3));
    bool ok = CommandParse.parse(argc, argv);

    if (!ok) {
        std::cout << CommandParse.error() << std::endl;
        std::cout << CommandParse.usage() << std::endl;
    } else {
        std::string strServerName        = CommandParse.get<std::string>("server_name");
        std::string strServerIP          = CommandParse.get<std::string>("server_ip");
        std::string strServerDescription = CommandParse.get<std::string>("server_description");
        std::string strServerRoot        = CommandParse.get<std::string>("server_root");
        int         nPort                = CommandParse.get<int>("server_port");
        std::string strConfigPath        = CommandParse.get<std::string>("config_path");

        auto nLevel = CommandParse.get<int>("logLevel");
        logger.setLevel(static_cast<tlog::detail::Level>(nLevel));

        http::HttpServer server(strServerName, strServerIP, strServerDescription, nPort);
        server.setServerRoot(strServerRoot);
        logger.info("setting server root:%s", server.getServerRoot());
        server.loadHttpConfig(strConfigPath);
        server.getHttpConfig().loadMimeType("html/mime.types");
        auto &mapper = server.getMapper();
        mapper.addRequestMapping({"/index"}, std::move(IndexPatter));
        server.getHttpConfig().loadDirentTmplateHtml("./html/dirHtml.tmpl");
        mapper.addRequestMapping({"/404"}, std::move([ &server ](http::HttpRequest &request, http::HttpResponse &response) {
                                     //  logger.info("do 404 function");
                                     size_t nWrite = response.loadFileString("html/40x.html");
                                     response.setStatusMessage(404, request.getHttpVersion(), "not found");
                                     response.setHeader(ContentLength, nWrite);
                                     response.setHeader(ContentType, "text/html");
                                     return true;
                                 }));
        mapper.addRequestMapping({"/#/"}, std::move([ &server ](http::HttpRequest &request, http::HttpResponse &response) {
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
                                     response.setHeader(ContentType, server.getHttpConfig().getMimeType(strRequestPath));
                                     struct stat st;
                                     if (stat(strRequestPath.c_str(), &st) != -1)
                                         response.setHeader("Last-Modified", utils::toResponseBasicDateString(st.st_mtime));
                                     response.setHeader("Connection", "close");
                                     return true;
                                 }));
        mapper.addRequestMapping({"/#//"}, std::move([ &server ](http::HttpRequest &request, http::HttpResponse &response) {
                                     auto tmplateHtml = server.getHttpConfig().getDirentTmplateHtml();
                                     auto currentDir  = server.getServerRoot();
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
                                     response.setHeader(ContentLength, tmplateHtml.size() + 4);
                                     response.setHeader("Date", utils::toResponseBasicDateString());
                                     response.setHeader("Connection", "close");
                                     return true;
                                 }));
        server.StartThreads(CommandParse.get<int>("threads_count"));
        server.ExecForever();
    }
    return 0;
}
