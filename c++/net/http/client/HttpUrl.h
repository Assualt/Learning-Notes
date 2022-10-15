#pragma once
#include "net/Uri.h"
#include <sstream>
#include <string>

using namespace muduo::net;
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
        Uri url(StringPiece(fullUrl.c_str(), fullUrl.size()));
        host = url.host();
        if (host.empty())
            return;
        scheme   = url.scheme();
        username = url.username();
        password = url.password();
        port     = url.port();
        if (port == 0 && scheme == "https") {
            port = 443;
        } else if (port == 0 && scheme == "http") {
            port = 80;
        }
        path  = url.path();
        query = url.query();
        if (!query.empty() && query.front() == '?')
            query = query.substr(1);
        fragment = url.fragment();
#endif
        netloc = host + ":" + std::to_string(port);
    }
};