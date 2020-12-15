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

std::string utils::FileMagicType(const std::string &filePath) {
    char temp[ 1024 ];
    if (access(filePath.c_str(), F_OK) == -1)
        return "";
    std::ifstream fin(filePath.c_str(), std::ios::binary);
    if (!fin.is_open())
        return "";
    fin.read(temp, 1024);
    fin.close();
    magic_t magic = magic_open(MAGIC_MIME_TYPE);
    if (magic == nullptr)
        return "";
    const char *pfileType = magic_buffer(magic, temp, strlen(temp));
    std::string fileType;
    if (pfileType != nullptr)
        fileType = pfileType;
    magic_close(magic);
    return fileType;
}

bool utils::FileIsBinary(const std::string &filePath) {
    std::string fileType = FileMagicType(filePath).c_str();
    if (fileType.empty())
        return true;
    if (strncasecmp(fileType.c_str(), "text/", 5) == 0)
        return false;
    return true;
}

bool utils::FileExists(const std::string &filePath) {
    return access(filePath.c_str(), F_OK) == 0;
}

std::string utils::toSizeString(off_t nSize) {
    std::string suffix = "B";
    double      c      = nSize;
    if (c > 1000) {
        suffix = "KB";
        c /= 1000.0;
    }

    if (c > 1000) {
        suffix = "MB";
        c /= 1000.0;
    }

    if (c > 1000) {
        suffix = "GB";
        c /= 1000.0;
    }
    return to_string(c) + suffix;
}

std::string utils::FileDirentTime(struct stat *st) {
    time_t     localTime = st->st_mtime;
    struct tm *info      = localtime(&localTime);
    char       temp[ 1024 ];
    size_t     nSize = strftime(temp, 1024, "%Y/%m/%d %H:%M:%S", info);
    return std::string(temp, nSize);
}
bool utils::ISDir(const std::string &filepath) {
    struct stat st;
    if (stat(filepath.c_str(), &st) != -1) {
        return S_ISDIR(st.st_mode & S_IFMT);
    }
    return false;
}