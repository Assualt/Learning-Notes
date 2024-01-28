//
// Created by 侯鑫 on 2024/1/14.
//

#include "http_utils.h"

using namespace ssp::net;

namespace
{
unsigned char ToHex(unsigned char x)
{
    return x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
    unsigned char y = 0;
    if (x >= 'A' && x <= 'Z')
        y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z')
        y = x - 'a' + 10;
    else if (x >= '0' && x <= '9')
        y = x - '0';
    return y;
}
}

int32_t HttpUtils::GetChunkSize(const std::string &buffer)
{
    std::string temp;
    for (auto &item : buffer) {
        if (isdigit(item) || isalnum(item)) {
            temp.push_back(item);
        } else {
            break;
        }
    }

    if (temp.empty()) {
        return -1;
    }

    return static_cast<int32_t>(std::strtol(temp.c_str(), nullptr, 16));
}


std::string HttpUtils::EncodeUrl(const std::string &str)
{
    std::string strTemp = "";
    size_t      length  = str.length();
    for (size_t i = 0; i < length; i++) {
        if (isalnum((unsigned char)str[ i ]) || (str[ i ] == '-') || (str[ i ] == '_') || (str[ i ] == '.') ||
            (str[ i ] == '~'))
            strTemp.push_back(str[ i ]);
        else if (str[ i ] == ' ') {
            strTemp.append("%20");
        } else {
            strTemp.push_back('%');
            strTemp.push_back(ToHex((unsigned char)str[ i ] >> 4));
            strTemp.push_back(ToHex((unsigned char)str[ i ] % 16));
        }
    }
    return strTemp;
}

std::string HttpUtils::DecodeUrl(const std::string &str)
{
    std::string strTemp;
    size_t      length = str.length();
    for (size_t i = 0; i < length; i++) {
        if (str[ i ] == '%' && i + 2 < length) {
            if (str[ i + 1 ] == '2' && str[ i + 2 ] == '0') {
                strTemp.push_back(' ');
                i += 2;
            } else {
                unsigned char high = FromHex((unsigned char)str[ ++i ]);
                unsigned char low  = FromHex((unsigned char)str[ ++i ]);
                strTemp.push_back(high * 16 + low);
            }
        } else
            strTemp.push_back(str[ i ]);
    }
    return strTemp;
}