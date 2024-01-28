//
// Created by 侯鑫 on 2024/1/10.
//

#ifndef SSP_TOOLS_HTTP_URI_H
#define SSP_TOOLS_HTTP_URI_H

#include "net/uri.h"
#include <sstream>
#include <string>

namespace ssp::net {

class HttpUrl {
public:
    std::string scheme_;
    std::string username_;
    std::string password_;
    std::string host_;
    int         port_{80};
    std::string path_;
    std::string query_;
    std::string fragment_;
    std::string fullUrl_;
    std::string netloc_;

public:
    [[maybe_unused]] explicit HttpUrl(std::string url);

    HttpUrl() = default;

    [[maybe_unused]] void ResetUrl(const std::string &url);

    [[maybe_unused]] [[nodiscard]] std::string GetHostUrl() const;

    bool IsValid() const;

private:
    void Parse();
};
}
#endif //SSP_TOOLS_HTTP_URI_H
