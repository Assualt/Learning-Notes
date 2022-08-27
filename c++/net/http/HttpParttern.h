#pragma once
#include "HttpConfig.h"
#include "base/Mutex.h"
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <vector>

using namespace muduo::base;
class HttpRequest;
class HttpResponse;
class HttpConfig;

using Func = std::function<bool(const HttpRequest &, HttpResponse &, const HttpConfig &)>;
class RequestMapper {
public:
    struct Key {
    public:
        Key(const std::string &pattern, int methods, bool needVal = false, bool useRegex = false);

        Key(const std::string &pattern, const std::string &method = "GET", bool needVal = false);

        bool MatchFilter(const std::string &reqPath, const std::string &reqType,
                         std::map<std::string, std::string> &valMap, bool &methodAllowed);

        bool MatchFilter(const std::string &reqPath, const std::string &reqType, bool &methodAllowed);

    private:
        bool checkAllowed(const std::string &reqType);

        int transferMethod(const std::string &method);

        void splitKeyPoint();

        bool tryRegexMatch(const std::string &pattern);

    public:
        std::string              pattern_;
        int                      method_;
        bool                     needVal_{false};
        bool                     useRegex_{false};
        std::vector<std::string> keySet_;
        std::vector<int>         keyPoint_;
    };

    void addRequestObject(const Key &key, uintptr_t object);

    void removeRequestObject(const std::string &pattern);

    std::optional<uintptr_t> findHandle(const std::string &repPath, const std::string &reqType,
                                        std::map<std::string, std::string> &resultMap);

protected:
    std::vector<std::pair<Key, uintptr_t>> m_vRequestsMapper;
    MutexLock                              lock_;
};
