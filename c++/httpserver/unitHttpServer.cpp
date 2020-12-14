#include "hashutils.hpp"
#include "httpserver.h"
#include <cmdline.hpp>
#include <fstream>
#include <regex>
using namespace http;
void testItem(){
    RequestMapper test;
    test.insert({"/index/{user}/{name}", "GET", true}, std::move(
        [](http::HttpRequest &request, http::HttpResponse &response){
            std::cout << "Request /index" << std::endl;
        }
    ));
    
    test.insert({"/favorite", "GET"}, std::move(
        [](http::HttpRequest &request, http::HttpResponse &response){
            std::cout << "Request /favorite" << std::endl;
        }
    ));

    test.insert({"/login", "GET"}, std::move(
        [](http::HttpRequest &request, http::HttpResponse &response){
            std::cout << "Request /login" << std::endl;
        }
    ));
    http::HttpRequest request; 
    http::HttpResponse response;
    std::map<std::string, std::string> valMap;
    auto iter = test.find("/index/1234/5678", valMap);
    if(iter != nullptr){
        iter(request, response);
        for(auto item: valMap){
            std::cout << "Get " << item.first << " val:" << item.second << std::endl;
        }
    }else{
        std::cout << "not Found" << std::endl;
    }

}

void IndexPatter(http::HttpRequest &request, http::HttpResponse &response) {
    std::ifstream fin("./html/index.html");
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
        server.addRequestMapping("/", std::move(static_cast<http::Func>(IndexPatter)));
        auto &mapper = server.getMapper();
        mapper.insert({"/index/{user}/{name}", "GET", true}, std::move(IndexPatter));
        server.ExecForever();
    }

 
    testItem();
    return 0;
}
