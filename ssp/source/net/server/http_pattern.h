//
// Created by 侯鑫 on 2024/4/4.
//

#ifndef SSP_TOOLS_HTTP_PATTERN_H
#define SSP_TOOLS_HTTP_PATTERN_H

#include <string>

namespace ssp::net {

class HttpPattern {
public:
    HttpPattern(const std::string &path)
        : path_(path)
    {
    }

private:
    std::string path_{};
};

} // namespace ssp::net

#endif // SSP_TOOLS_HTTP_PATTERN_H
