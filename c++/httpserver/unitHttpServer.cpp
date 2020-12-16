#include "httpserver.h"
#include <cmdline.hpp>
#include <fstream>
#include <regex>
#include <dirent.h>
#include <sys/stat.h>
using namespace http;

bool IndexPatter(http::HttpRequest &request, http::HttpResponse &response) {
    std::string resultString = utils::loadFileString("./html/index.html");
    response.setStatusMessage(200, "HTTP/1.1", "OK");
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

    bool ok = CommandParse.parse(argc, argv);

    if (!ok) {
        std::cout << CommandParse.error() << std::endl;
        std::cout << CommandParse.usage() << std::endl;
    } else {
        std::string strServerName = CommandParse.get<std::string>("server_name");
        std::string strServerIP = CommandParse.get<std::string>("server_ip");
        std::string strServerDescription = CommandParse.get<std::string>("server_description");
        std::string strServerRoot = CommandParse.get<std::string>("server_root");
        int nPort = CommandParse.get<int>("server_port");

        http::HttpServer server(strServerName, strServerIP, strServerDescription, nPort);
        server.setServerRoot(strServerRoot);

        logger.info("setting server root:%s", server.getServerRoot());
        server.loadHttpConfig();
        auto &mapper = server.getMapper();
        mapper.addRequestMapping({"/index"}, std::move(IndexPatter));
        server.getHttpConfig().loadDirentTmplateHtml("./html/dirHtml.tmpl");
        mapper.addRequestMapping({"/404"}, std::move([&server](http::HttpRequest &request, http::HttpResponse &response) {
                                     logger.info("do 404 function");
                                     size_t nWrite = response.loadFileString(server.getServerRoot() + "/40x.html");
                                     response.setStatusMessage(404, "HTTP/1.1", "not found");
                                     response.setHeader(ContentLength, nWrite);
                                     response.setHeader(ContentType, "text/html");
                                     return true;
                                 }));
        mapper.addRequestMapping({"/#/"}, std::move([&server](http::HttpRequest &request, http::HttpResponse &response) {
                                     // handle image
                                     long nSize = 0;
                                     std::string result;
                                     std::string strRequestPath = server.getServerRoot() + "/" + request.getRequestPath();
                                     if (utils::FileIsBinary(strRequestPath)) {
                                         nSize = response.loadBinaryFile(strRequestPath);
                                     } else {
                                         nSize = response.loadFileString(strRequestPath);
                                     }
                                     logger.info("successful read bytes:%d", nSize);
                                     if (nSize <= 0) {
                                         response.setStatusMessage(404, "HTTP/1.1", "not found");
                                         response.setHeader(ContentLength, 0);
                                     } else {
                                         response.setStatusMessage(200, "HTTP/1.1", "OK");
                                         response.setHeader(ContentLength, nSize);
                                     }
                                     response.setHeader(ContentType, utils::FileMagicType(strRequestPath));
                                     response.setHeader("Connection", "close");
                                     return true;
                                 }));
        mapper.addRequestMapping({"/#//"}, std::move([&server](http::HttpRequest &request, http::HttpResponse &response) {
                                     auto tmplateHtml = server.getHttpConfig().getDirentTmplateHtml();
                                     auto currentDir = server.getServerRoot() + "/" + request.getRequestPath();
                                     logger.info("begin to list current dir:%s", currentDir);
                                     tmplateHtml.append("<script>start(\"");
                                     tmplateHtml.append(currentDir);
                                     tmplateHtml.append("\");</script><script>onHasParentDirectory();</script>");
                                     DIR *dir = opendir(currentDir.c_str());
                                     struct dirent *dr;
                                     while ((dr = readdir(dir)) != nullptr) {
                                         std::string statFilePath = currentDir + "/";
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
                                             tmpString.append(utils::toSizeString(st.st_size));
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
    std::cout << utils::FileIsBinary("/bubblesort.gif?123") << std::endl;
    std::cout << utils::ISDir("./html") << std::endl;
    // http::HttpResponse response;
    // std::cout << response.loadBinaryFile("./html//bubblesort.gif") << std::endl;
    // testItem();
    return 0;
}
