//
// Created by 侯鑫 on 2024/1/10.
//

#include "http_uri.h"
#include <functional>
#include <utility>

using namespace ssp::net;

[[maybe_unused]] HttpUrl::HttpUrl(std::string url)
    : scheme_("http")
    , port_(80)
    , fullUrl_(std::move(url))
{
    Parse();
}

[[maybe_unused]] void HttpUrl::ResetUrl(const std::string &url)
{
    fullUrl_ = url;
    Parse();
}

[[maybe_unused]] std::string HttpUrl::GetHostUrl() const
{
    std::stringstream ss;
    ss << scheme_ << "://";
    if (!username_.empty()) {
        ss << username_;
    }
    if (!username_.empty() && !password_.empty()) {
        ss << ":" << password_ << "@";
    }
    ss << host_ << path_;
    return ss.str();
}

void HttpUrl::Parse()
{
    if (fullUrl_.find("://") == std::string::npos) {
        fullUrl_ = "https://" + fullUrl_;
    }
#ifdef USE_GURL
    GURL url(fullurl_);
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
    Uri url(fullUrl_);
    if (url.Host().empty()) {
        return;
    }
    host_ = url.Host();
    scheme_   = url.Scheme();
    username_ = url.Username();
    password_ = url.Password();
    port_     = url.Port();
    if (port_ == 0 && scheme_ == "https") {
        port_ = 443;
    } else if (port_ == 0 && scheme_ == "http") {
        port_ = 80;
    }
    path_  = url.Path();
    if (path_.empty()) {
        path_ = "/";
    }

    query_ = url.Query();
    if (!query_.empty() && query_.front() == '?')
        query_ = query_.substr(1);
    fragment_ = url.Fragment();
#endif
    netloc_ = host_ + ":" + std::to_string(port_);
}

bool HttpUrl::IsValid() const
{
    std::vector<std::function<bool()>> validChecks = {
        [this]() {
            return !host_.empty();
        },
        [this]() {
            return !host_.empty();
        }
    };

    return std::any_of(validChecks.begin(), validChecks.end(), [](auto func) {
        return func();
    });
}