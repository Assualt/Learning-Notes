#include "hashutils.hpp"
#include "httpserver.h"
#include <cmdline.hpp>
#include <fstream>
void IndexPatter(http::HttpRequest &request, http::HttpResponse &response) {
    std::ifstream fin("unitHttpServer.cpp");
    std::string   resultString;
    if (fin.is_open()) {
        fin.seekg(0, std::ios::end);
        off_t length = fin.tellg();
        fin.seekg(0, std::ios::beg);
        char *temp = new char[ length + 1 ];
        fin.read(temp, length);
        resultString.append(temp, length);
        delete[] temp;
        fin.close();
    }
    response.setStatusMessage(200, "HTTP/1.1", "OK");
    response.setHeader(ContentLength, resultString.size() + 4);
    response.setHeader(ContentType, "text/html");
    response.setBodyString(resultString);
}

int main(int argc, char **argv) {

    cmdline::parser CommandParse;

    CommandParse.add<std::string>("server_name", 0, "The http server name", false, "HttpServer");
    CommandParse.add<std::string>("server_ip", 0, "The http server ip.", false, "127.0.0.1");
    CommandParse.add<std::string>("server_description", 0, "The http server's description.", false, "A simple Http Server");
    CommandParse.add<int>("server_port", 0, "The http server's port", false, 8080, cmdline::range<int>(1, 65535));

    bool ok = CommandParse.parse(argc, argv);

    if (!ok) {
        std::cout << CommandParse.error() << std::endl;
        std::cout << CommandParse.usage() << std::endl;
    } else {
        std::string strServerName        = CommandParse.get<std::string>("server_name");
        std::string strServerIP          = CommandParse.get<std::string>("server_ip");
        std::string strServerDescription = CommandParse.get<std::string>("server_description");
        int         nPort                = CommandParse.get<int>("server_port");

        http::HttpServer server(strServerName, strServerIP, strServerDescription, nPort);

        server.addRequestMapping("/", (http::Func)IndexPatter);
        server.ExecForever();
    }
    return 0;
}
