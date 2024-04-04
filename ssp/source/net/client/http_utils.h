//
// Created by 侯鑫 on 2024/1/14.
//

#ifndef SSP_TOOLS_HTTP_UTILS_H
#define SSP_TOOLS_HTTP_UTILS_H

#include <string>
#include <cstdint>

namespace ssp::net {
class HttpUtils {
public:
    static int32_t GetChunkSize(const std::string &buffer);

    static std::string EncodeUrl(const std::string &str);

    static std::string DecodeUrl(const std::string &str);
};
}

#endif //SSP_TOOLS_HTTP_UTILS_H
