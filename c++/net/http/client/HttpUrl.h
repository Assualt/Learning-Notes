#include "third_party/url/include/url.h"
#include <sstream>
#include <string>

struct HttpUrl {
public:
    std::string scheme;
    std::string username;
    std::string password;
    std::string host;
    int         port;
    std::string path;
    std::string query;
    std::string fragment;
    std::string fullUrl;
    std::string netloc;

public:
    HttpUrl(const std::string url)
        : fullUrl(url)
        , scheme("http")
        , port(80) {
        parse();
    }
    HttpUrl() = default;

    void resetUrl(const std::string url) {
        fullUrl = url;
        parse();
    }

    std::string getHostUrl() const {
        std::stringstream ss;
        ss << scheme << "://";
        if (!username.empty())
            ss << username;
        if (!username.empty() && !password.empty())
            ss << ":" << password << "@";
        ss << host << path;
        return ss.str();
    }

private:
    void parse() {
        if (fullUrl.find("://") == std::string::npos)
            fullUrl = "http://" + fullUrl;
#ifdef USE_GURL
        GURL url(fullurl);
        if (!url.is_valid() || !url.IsStandard())
            return;
        if (url.has_scheme())
            scheme = url.scheme();
        if (url.has_username())
            username = url.username();
        if (url.has_password())
            password = url.password();
        if (url.has_host())
            host = url.host();
        if (url.has_port())
            port = atoi(url.port().c_str());
        if (url.has_path())
            path = url.path();
        if (url.has_query())
            query = url.query();
        if (url.has_ref())
            fragment = url.ref();
#else
        Url url = Url::create(fullUrl);
        host    = url.getHost();
        if (host.empty())
            return;
        scheme   = url.getScheme();
        username = url.getUsername();
        password = url.getPassword();
        port     = url.getPort();
        path     = url.getPath();
        query    = url.getQuery();
        if (!query.empty() && query.front() == '?')
            query = query.substr(1);
        fragment = url.getFragment();
#endif
        netloc = host + ":" + std::to_string(port);
    }
};