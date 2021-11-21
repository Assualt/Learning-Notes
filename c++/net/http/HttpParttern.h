#include <functional>
#include <iostream>
#include <map>
#include <vector>

class HttpRequest;
class HttpResponse;
class HttpConfig;

using Func = std::function<bool(const HttpRequest &, HttpResponse &, HttpConfig &)>;
class RequestMapper {
public:
    struct Key {
    public:
        Key(const std::string &pattern, const std::string &method = "GET", bool needval = false);
        bool MatchFilter(const std::string &reqPath, const std::string &reqType, std::map<std::string, std::string> &valMap, bool &MethodAllowed);
        bool MatchFilter(const std::string &reqPath, const std::string &reqType, bool &MethodAllowed);

    public:
        std::string              pattern;
        std::string              method;
        bool                     needval;
        std::vector<std::string> keySet;
        std::vector<int>         keyPoint;
    };
    void addRequestMapping(const Key &key, Func &&F);
    Func find(const std::string &RequestPath, const std::string &reqType, std::map<std::string, std::string> &resultMap);
    Func find(const std::string &RequestPath, const std::string &reqType);

protected:
    std::vector<std::pair<Key, Func>> m_vRequestMapper;
};
