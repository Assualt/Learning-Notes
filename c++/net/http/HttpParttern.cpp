#include "HttpParttern.h"
#include "HttpRequest.h"
#include "HttpUtils.h"
#include "base/Logging.h"
#include <regex>

using namespace muduo::base;

RequestMapper::Key::Key(const std::string &pattern, int methods, bool needVal, bool useRegex)
    : pattern_(pattern)
    , method_(methods)
    , needVal_(needVal)
    , useRegex_(useRegex) {
    splitKeyPoint();
}

RequestMapper::Key::Key(const std::string &pattern, const std::string &method, bool needVal)
    : pattern_(pattern)
    , method_(transferMethod(method))
    , needVal_(needVal) {
    splitKeyPoint();
}

void RequestMapper::Key::splitKeyPoint() {
    if (!needVal_) {
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

bool RequestMapper::Key::tryRegexMatch(const std::string &pattern) {
    if (!useRegex_) {
        return false;
    }
    std::regex  regPattern(pattern_, regex::icase);
    std::smatch ret;
    if (std::regex_match(pattern, ret, regPattern)) { // 匹配OK
        logger.info("try Regex match for pattern %s success", pattern);
        return true;
    }
    logger.info("try Regex match for pattern %s failed", pattern);
    return false;
}

bool RequestMapper::Key::MatchFilter(const std::string &reqPath, const std::string &reqType,
                                     std::map<std::string, std::string> &valMap, bool &allowed) {
    allowed = checkAllowed(reqType);
    if (!needVal_) {
        return (reqPath == pattern_) || (tryRegexMatch(reqPath));
    }

    std::string path = reqPath;
    if (reqPath.find("?") != std::string::npos)
        path = path.substr(0, path.find("?"));
    auto itemList    = utils::split(path, '/');
    auto patternList = utils::split(pattern_, '/');
    // reqPath: /index/
    // pattern: /index/{user}/{pass}  false
    if (itemList.size() != patternList.size())
        return false;
    // reqPath: /index/x/y
    // pattern: /index/{user}/{pass}  false
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

bool RequestMapper::Key::MatchFilter(const std::string &reqPath, const std::string &reqType, bool &allowed) {
    allowed = checkAllowed(reqType);
    return reqPath == pattern_;
}

void RequestMapper::addRequestObject(const Key &key, uintptr_t object) {
    AutoLock lock(lock_);
    m_vRequestsMapper.push_back({key, object});
    logger.debug("success insert key:%s object:%d useRegex:%b", key.pattern_, object, key.useRegex_);
}

void RequestMapper::removeRequestObject(const std::string &pattern) {
    auto iter = std::find_if(m_vRequestsMapper.begin(), m_vRequestsMapper.end(),
                             [ &pattern ](auto &item) { return item.first.pattern_ == pattern; });
    if (iter != m_vRequestsMapper.end()) {
        AutoLock lock(lock_);
        logger.debug("success delete key:%s object:%d", pattern, iter->second);
        m_vRequestsMapper.erase(iter);
    }
}

std::optional<uintptr_t> RequestMapper::findHandle(const std::string &reqPath, const std::string &reqType,
                                                   std::map<std::string, std::string> &resultMap) {
    for (auto &iter : m_vRequestsMapper) {
        bool allowed = true;
        if (iter.first.MatchFilter(reqPath, reqType, resultMap, allowed)) {
            logger.info("success request path:%s, handle path:%s, method:%s allow:%b", reqPath, iter.first.pattern_,
                        reqType, allowed);
            if (!allowed) {
                return findHandle(PATTERN_METHOD_NOT_ALLOWED, "GET", resultMap);
            }
            return iter.second;
        }
        logger.debug("failure request path:%s, handle path:%s, method:%s allow:%b", reqPath, iter.first.pattern_,
                     reqType, allowed);
    }
    return std::nullopt;
}

int RequestMapper::Key::transferMethod(const std::string &method) {
    std::string                            reqType = utils::toLower(method);
    static std::map<std::string, REQ_TYPE> reqMap  = {
         {"get", REQ_TYPE::TYPE_GET},
         {"post", REQ_TYPE::TYPE_POST},
         {"put", REQ_TYPE::TYPE_PUT},
    };

    auto itr = reqMap.find(reqType);
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
