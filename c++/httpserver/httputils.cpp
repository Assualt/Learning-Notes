#include "httputils.h"
#include <string.h>
#include <unistd.h>

std::string utils::_ltrim(const std::string &src, char ch) {
    std::string           temp = src;
    std::string::iterator p    = std::find_if(temp.begin(), temp.end(), [ &ch ](char c) { return ch != c; });
    temp.erase(temp.begin(), p);
    return temp;
}

std::string utils::_rtrim(const std::string &src, char ch) {
    std::string                   temp = src;
    std::string::reverse_iterator p    = find_if(temp.rbegin(), temp.rend(), [ &ch ](char c) { return ch != c; });
    temp.erase(p.base(), temp.end());
    return temp;
}

std::string utils::trim(const std::string &src, char ch) {
    return _rtrim(_ltrim(src, ch), ch);
}

size_t utils::chunkSize(const std::string &strChunkSize) {
    std::string temp;
    for (size_t i = 0; i < strChunkSize.size(); i++) {
        if ((strChunkSize[ i ] >= '0' && strChunkSize[ i ] <= '9') || (strChunkSize[ i ] >= 'A' && strChunkSize[ i ] <= 'F') || (strChunkSize[ i ] >= 'a' && strChunkSize[ i ] <= 'f')) {
            temp.push_back(strChunkSize[ i ]);
        } else {
            break;
        }
    }
    if (temp.empty())
        return -1;
    return std::stoi(temp, nullptr, 16);
}
std::vector<std::string> utils::split(const std::string &src, char divider) {
    std::vector<std::string> result;
    std::string              temp;
    for (auto &ch : src) {
        if (ch == divider) {
            if (!utils::trim(temp).empty())
                result.push_back(utils::trim(temp));
            temp.clear();
        } else {
            temp.push_back(ch);
        }
    }
    if (!temp.empty())
        result.push_back(utils::trim(temp));
    return result;
}

std::string utils::loadFileString(const std::string &filePath) {
    if (access(filePath.c_str(), F_OK) == -1) {
        return "";
    }
    FILE *fp = fopen(filePath.c_str(), "rb");
    if (fp == nullptr)
        return "";
    std::stringstream ss;
    char              ch;
    while ((ch = fgetc(fp)) != EOF) {
        ss << ch;
    }
    fclose(fp);
    return ss.str();
}

std::string utils::toResponseBasicDateString() {
    time_t     t    = time(nullptr);
    struct tm *info = localtime(&t);
    char       temp[ 1024 ];
    size_t     nSize = strftime(temp, 1024, "%a, %d %b %Y %H:%M:%S %Z", info);

    return std::string(temp, nSize);
}

std::string utils::requstTimeFmt() {
    time_t     t    = time(nullptr);
    struct tm *info = localtime(&t);
    char       temp[ 1024 ];
    size_t     nSize = strftime(temp, 1024, "%a, %d %b %Y %H:%M:%S %Z", info);

    return std::string(temp, nSize);
}

bool utils::FileIsBinary(const std::string &filePath) {
    char temp[ 1024 ];
    if (access(filePath.c_str(), F_OK) == -1)
        return false;
    std::ifstream fin(filePath.c_str(), std::ios::binary);
    if (!fin.is_open())
        return false;
    fin.read(temp, 1024);
    fin.close();
    magic_t magic = magic_open(MAGIC_MIME_TYPE);
    if (magic == nullptr)
        return false;
    const char *fileType = magic_buffer(magic, temp, strlen(temp));
    magic_close(magic);
    if (fileType == nullptr)
        return true;
    if (strncasecmp(fileType, "text/", 5) == 0)
        return false;
    return true;
}
