#include "HttpParttern.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpUtils.h"
#include "base/Logging.h"
#include <map>
using namespace muduo::base;

#define NOTFOUND "/404"
#define NOTFOUNDHTML "<html><head><title>404 Not Found</title></head><body>404 not found</body></html>"

RequestMapper::Key::Key(const std::string &pattern, const std::string &method, bool needval)
    : pattern(pattern)
    , method(method)
    , needval(needval) {
    auto itemVector = utils::split(pattern, '/');
    if (needval) {
        int i = 0;
        for (auto &item : itemVector) {
            if (item.front() == '{' && item.back() == '}') {
                keyPoint.push_back(i);
                keySet.push_back(item.substr(1, item.size() - 2));
            }
            i++;
        }
    }
}
bool RequestMapper::Key::MatchFilter(const std::string &reqPath, const std::string &reqType, std::map<std::string, std::string> &valMap, bool &MethodAllowed) {
    if (strcasecmp(reqType.c_str(), method.c_str()) == 0)
        MethodAllowed = true;
    else
        MethodAllowed = false;
    if (!needval)
        return reqPath == pattern;
    else {
        std::string path = reqPath;
        if (reqPath.find("?") != std::string::npos)
            path = path.substr(0, path.find("?"));
        auto itemList    = utils::split(path, '/');
        auto patternList = utils::split(pattern, '/');
        // reqpath: /index/
        // pattern: /index/{user}/{pass}  false
        if (itemList.size() != patternList.size())
            return false;
        // reqpath: /index/x/y
        // pattern: /indep/{user}/{pass}  false
        for (int i = 0; i < keyPoint.front(); i++) {
            if (itemList[ i ] != patternList[ i ])
                return false;
        }
        for (auto i = 0, j = 0; i < keyPoint.size(); i++) {
            int pos = keyPoint[ i ];
            if (pos >= itemList.size())
                break;
            valMap.insert(std::pair<std::string, std::string>(keySet[ j++ ], itemList[ pos ]));
        }
        return true;
    }
}

bool RequestMapper::Key::MatchFilter(const std::string &reqPath, const std::string &reqType, bool &MethodAllowed) {
    if (strcasecmp(reqType.c_str(), method.c_str()) == 0)
        MethodAllowed = true;
    return reqPath == pattern;
}

void RequestMapper::addRequestMapping(const Key &key, Func &&F) {
    m_vRequestMapper.push_back(std::pair<RequestMapper::Key, Func>(key, F));
}

Func RequestMapper::find(const std::string &RequestPath, const std::string &reqType, std::map<std::string, std::string> &resultMap) {
    for (auto iter : m_vRequestMapper) {
        bool MethodAllowed;
        if (iter.first.MatchFilter(RequestPath, reqType, resultMap, MethodAllowed)) {
            logger.debug("request path:%s, handle path:%s, method:%s allow:%d", RequestPath, iter.first.pattern, reqType, MethodAllowed);
            if (!MethodAllowed)
                return find("/405", "GET");
            return iter.second;
        }
    }
    return find(NOTFOUND, reqType, resultMap);
}
Func RequestMapper::find(const std::string &RequestPath, const std::string &reqType) {
    for (auto iter : m_vRequestMapper) {
        bool MethodAllowed;
        if (iter.first.MatchFilter(RequestPath, reqType, MethodAllowed)) {
            logger.debug("request path:%s, handle path:%s, method:%s allow:%d", RequestPath, iter.first.pattern, reqType, MethodAllowed);
            if (!MethodAllowed)
                return find("/405", "GET");
            return iter.second;
        }
    }
    return [ = ](const HttpRequest &request, HttpResponse &response, HttpConfig &config) {
        response.setStatusMessage(404, "HTTP/1.1", "not found");
        response.addHeader("Content-Type", "text/html");
        response.addHeader("Content-Length", 0);
        return true;
    };
}