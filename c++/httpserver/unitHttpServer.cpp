#include "httpserver.h"
#include <cmdline.hpp>
#include <fstream>
#include <regex>
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
    CommandParse.add<std::string>(
            "server_description", 0, "The http server's description.", false, "A simple Http Server");
    CommandParse.add<int>("server_port", 0, "The http server's port", false, 8080, cmdline::range<int>(1, 65535));

    bool ok = CommandParse.parse(argc, argv);

    if (!ok) {
        std::cout << CommandParse.error() << std::endl;
        std::cout << CommandParse.usage() << std::endl;
    } else {
        std::string strServerName = CommandParse.get<std::string>("server_name");
        std::string strServerIP = CommandParse.get<std::string>("server_ip");
        std::string strServerDescription = CommandParse.get<std::string>("server_description");
        int nPort = CommandParse.get<int>("server_port");

        http::HttpServer server(strServerName, strServerIP, strServerDescription, nPort);
        server.loadHttpConfig();
        auto &mapper = server.getMapper();
        mapper.addRequestMapping({"/index"}, std::move(IndexPatter));
        mapper.addRequestMapping({"/404"}, std::move([&server](http::HttpRequest &request, http::HttpResponse &response){
            logger.info("do 404 function");
            size_t nWrite = response.loadFileString(server.getServerRoot() +"/40x.html");
            response.setStatusMessage(404, "HTTP/1.1", "not found");
            response.setHeader(ContentLength, nWrite);
            response.setHeader(ContentType, "text/html");
            return true;
        }));
        mapper.addRequestMapping({"/#/"}, std::move([&server](http::HttpRequest &request, http::HttpResponse &response){
            // handle image
            long nSize = 0;
            std::string result;
            if(utils::FileIsBinary(request.getRequestPath())){
                nSize = response.loadBinaryFile(server.getServerRoot() + "/" + request.getRequestPath());
            }else{
                nSize = response.loadFileString(server.getServerRoot() + "/" + request.getRequestPath());
            }
            logger.info("successful read bytes:%d", nSize);
            if(nSize <= 0){
                response.setStatusMessage(404, "HTTP/1.1", "not found");
                response.setHeader(ContentLength, 0);
            }else{
                response.setStatusMessage(200, "HTTP/1.1", "OK");
                response.setHeader(ContentLength, nSize);
            }
            response.setHeader(ContentType, "bytes");
            return true;
        }));
        server.ExecForever();
    }
    std::cout << utils::FileIsBinary("/bubblesort.gif?123") << std::endl;
    // http::HttpResponse response;
    // std::cout << response.loadBinaryFile("./html//bubblesort.gif") << std::endl;
    // testItem();
    return 0;
}
