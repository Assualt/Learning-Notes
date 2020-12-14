#include "hashutils.hpp"
#include "httpserver.h"
#include <cmdline.hpp>
#include <fstream>
#include <regex>

struct utils {
public:
    static std::string _ltrim(const std::string &src, char ch = ' ') {
        std::string           temp = src;
        std::string::iterator p    = std::find_if(temp.begin(), temp.end(), [ &ch ](char c) { return ch != c; });
        temp.erase(temp.begin(), p);
        return temp;
    }
    static std::string _rtrim(const std::string &src, char ch = ' ') {
        string                   temp = src;
        string::reverse_iterator p    = find_if(temp.rbegin(), temp.rend(), [ &ch ](char c) { return ch != c; });
        temp.erase(p.base(), temp.end());
        return temp;
    }
    static std::string trim(const std::string &src, char ch = ' ') {
        return utils::_rtrim(utils::_ltrim(src, ch), ch);
    }
    static std::vector<std::string> split(const std::string &src, char divider) {
        std::vector<std::string> result;
        std::string              temp;
        for (auto &ch : src) {
            if (ch == divider) {
                result.push_back(utils::trim(temp));
                temp.clear();
            } else {
                temp.push_back(ch);
            }
        }
        if (!temp.empty())
            result.push_back(utils::trim(temp));
        return result;
    }
    static std::string join(const vector<std::string> &con, char ch, int start = 0, int end = -1, bool skipEmpty = true) {
        std::string result;
        if (end == -1 || (end && end < start))
            end = con.size();
        for (auto i = start; i < end; ++i) {
            if (con[ i ].empty() && skipEmpty)
                continue;
            result.append(con[ i ]);
            if (i != end - 1)
                result.push_back(ch);
        }
        return result;
    }
    static bool startswith(const std::string &src, const std::string &prefix) {
        return src.find(prefix) == 0;
    }
    static bool endswith(const std::string &src, const std::string &prefix) {
        return src.rfind(prefix) == 0;
    }
};

struct Key{
public:    
    Key(const std::string &pattern, const std::string &method="GET", bool needval=false)
        :pattern(pattern), method(method),needval(false){
        if(needval){
            auto itemVector = utils::split(pattern, '/');
            int i = 0;
            for(auto &item: itemVector){
                if(item.front() == '{' && item.back() == '}'){
                    keyPoint.push_back(i++);
                    keySet.push_back(item.substr(1, item.size() - 2));
                }
            }
        }
    }
    bool MatchFilter(const std::string &reqPath, std::map<std::string, std::string>&valMap){
        if(!needval) return reqPath == pattern;
        else{
            auto itemList = utils::split(reqPath, '/');
            if(itemList.size() != keyPoint.size())
                return false;            
            for(auto i = 0; i < keyPoint.size(); i++){
                valMap.insert(std::pair<std::string, std::string>(keySet[i],itemList[i]));
            }
            return true;
        }
    }
public:
    std::string pattern;
    std::string method;
    bool needval;
    std::vector<std::string> keySet;
    std::vector<int> keyPoint;
};
template<class T=Key>
struct keyCompare{    
    int operator()(const T &x,const T &y) const{
        if(x.bregex && std::regex_match(y.pattern, std::regex(x.pattern)) && x.method == y.method)
            return 0;
        else if(x.pattern == y.pattern)
            return 0;
        return 1;
    }    
};

class RequestMapper{
public:
    void insert(const Key &key, http::Func &&F){
        m_RequestMapper.push_back(std::pair<Key, http::Func>(key, F));
    }
    http::Func find(const std::string &RequestPath, std::map<std::string, std::string> &resultMap){
        for(auto& iter: m_RequestMapper){
            if(iter.first.MatchFilter(RequestPath, resultMap)){
                return iter.second;
            }
        }
        return nullptr;
    }
protected:
    std::vector<std::pair<Key, http::Func>> m_RequestMapper;
};

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
        
        // server.ExecForever();
    }

 
    testItem();
    return 0;
}
