#include "HttpParttern.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpUtils.h"
#include "base/Logging.h"
#include <algorithm>
#include <map>
using namespace muduo::base;

#define NOTFOUND "/404"
#define NOTFOUNDHTML "<html><head><title>404 Not Found</title></head><body>404 not found</body></html>"

RequestMapper::Key::Key(const std::string &pattern, int methods, bool needval)
    : pattern_(pattern)
    , method_(methods)
    , needval_(needval) {
    splitKeyPoint();
}

RequestMapper::Key::Key(const std::string &pattern, const std::string &method, bool needval)
    : pattern_(pattern)
    , method_(transferMethod(method))
    , needval_(needval) {
    splitKeyPoint();
}

void RequestMapper::Key::splitKeyPoint() {
    if (!needval_) {
        return;
    }

    auto itemVector = utils::split(pattern_, '/');
    int  i          = 0;
    for (auto &item : itemVector) {
        if (item.front() == '{' && item.back() == '}') {
            keyPoint_.push_back(i);
            keySet_.push_back(item.substr(1, item.size() - 2));
        }
        i++;
    }
}

bool RequestMapper::Key::MatchFilter(const std::string &reqPath, const std::string &reqType, std::map<std::string, std::string> &valMap, bool &allowed) {
    allowed = checkAllowed(reqType);
    if (!needval_)
        return reqPath == pattern_;
    else {
        std::string path = reqPath;
        if (reqPath.find("?") != std::string::npos)
            path = path.substr(0, path.find("?"));
        auto itemList    = utils::split(path, '/');
        auto patternList = utils::split(pattern_, '/');
        // reqpath: /index/
        // pattern: /index/{user}/{pass}  false
        if (itemList.size() != patternList.size())
            return false;
        // reqpath: /index/x/y
        // pattern: /indep/{user}/{pass}  false
        for (int i = 0; i < keyPoint_.front(); i++) {
            if (itemList[ i ] != patternList[ i ])
                return false;
        }
        for (auto i = 0, j = 0; i < keyPoint_.size(); i++) {
            int pos = keyPoint_[ i ];
            if (pos >= itemList.size())
                break;
            valMap.insert(std::pair<std::string, std::string>(keySet_[ j++ ], itemList[ pos ]));
        }
        return true;
    }
}

bool RequestMapper::Key::MatchFilter(const std::string &reqPath, const std::string &reqType, bool &allowed) {
    allowed = checkAllowed(reqType);
    return reqPath == pattern_;
}

void RequestMapper::addRequestMapping(const Key &key, Func &&F) {
    m_vRequestMapper.push_back(std::pair<RequestMapper::Key, Func>(key, F));
}

Func RequestMapper::find(const std::string &RequestPath, const std::string &reqType, std::map<std::string, std::string> &resultMap) {
    for (auto iter : m_vRequestMapper) {
        bool allowed;
        if (iter.first.MatchFilter(RequestPath, reqType, resultMap, allowed)) {
            logger.debug("request path:%s, handle path:%s, method:%s allow:%b", RequestPath, iter.first.pattern_, reqType, allowed);
            if (!allowed)
                return find("/405", "GET");
            return iter.second;
        }
    }
    return find(NOTFOUND, reqType, resultMap);
}

Func RequestMapper::find(const std::string &RequestPath, const std::string &reqType) {
    for (auto iter : m_vRequestMapper) {
        bool allowed;
        if (iter.first.MatchFilter(RequestPath, reqType, allowed)) {
            logger.debug("request path:%s, handle path:%s, method:%s allow:%b", RequestPath, iter.first.pattern_, reqType, allowed);
            if (!allowed)
                return find("/405", "GET");
            return iter.second;
        }
    }
    return [ = ](const HttpRequest &, HttpResponse &response, const HttpConfig &) {
        response.setStatusMessage(404, "HTTP/1.1", "not found");
        response.addHeader("Content-Type", "text/html");
        response.addHeader("Content-Length", 0);
        return true;
    };
}

int RequestMapper::Key::transferMethod(const std::string &reqType) {
    int                                    method  = 0;
    std::string                            reqtype = utils::toLower(reqType);
    static std::map<std::string, REQ_TYPE> reqMap  = {
         {"get", REQ_TYPE::TYPE_GET},
         {"post", REQ_TYPE::TYPE_POST},
         {"put", REQ_TYPE::TYPE_PUT},
    };

    auto itr = reqMap.find(reqtype);
    if (itr != reqMap.end()) {
        return static_cast<int>(itr->second);
    }
    return static_cast<int>(REQ_TYPE::TYPE_BUT);
}

bool RequestMapper::Key::checkAllowed(const std::string &reqType) {
    int reqMethod = transferMethod(reqType);
    if (reqMethod == UINT32_MAX) {
        return false;
    }
    return (reqMethod & method_) == reqMethod;
}
