#include "HttpUtils.h"
#include <iomanip>
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
    int               ch;
    while ((ch = fgetc(fp)) != EOF) {
        ss << (char)ch;
    }
    fclose(fp);
    return ss.str();
}

int utils::loadBinaryStream(const std::string &filePath, MyStringBuffer &buf) {
    if (access(filePath.c_str(), F_OK) == -1) {
        return 0;
    }
    FILE *fp = fopen(filePath.c_str(), "rb");
    if (fp == nullptr)
        return 0;
    int n = 0;
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        buf.sputc((char)ch);
        n++;
    }
    fclose(fp);
    return n;
}

std::string utils::toResponseBasicDateString(time_t t) {
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
    if (access(filePath.c_str(), F_OK) == -1)
        return "";
    magic_t magic = magic_open(MAGIC_MIME_TYPE);
    if (magic == nullptr)
        return "";
    magic_load(magic, nullptr);
    const char *pfileType = magic_file(magic, filePath.c_str());
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
    std::string suffix = " B";
    double      c      = nSize;
    if (c > 1000) {
        suffix = " KB";
        c /= 1000.0;
    }

    if (c > 1000) {
        suffix = " MB";
        c /= 1000.0;
    }

    if (c > 1000) {
        suffix = " GB";
        c /= 1000.0;
    }
    std::stringstream ss;
    ss << setprecision(5) << c;
    ss << suffix;
    return ss.str();
}

std::string utils::FileDirentTime(struct stat *st) {
    time_t     localTime = st->st_mtime;
    struct tm *info      = localtime(&localTime);
    char       temp[ 1024 ];
    size_t     nSize = strftime(temp, 1024, "%Y/%m/%d %H:%M:%S", info);
    return std::string(temp, nSize);
}
bool utils::IsDir(const std::string &filepath) {
    struct stat st;
    if (stat(filepath.c_str(), &st) != -1) {
        return S_ISDIR(st.st_mode & S_IFMT);
    }
    return false;
}
std::string utils::toHexString(ssize_t nSize) {
    std::stringstream ss;
    ss << std::hex << nSize;
    return ss.str();
}

unsigned char UrlUtils::ToHex(unsigned char x) {
    return x > 9 ? x + 55 : x + 48;
}

unsigned char UrlUtils::FromHex(unsigned char x) {
    unsigned char y = 0;
    if (x >= 'A' && x <= 'Z')
        y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z')
        y = x - 'a' + 10;
    else if (x >= '0' && x <= '9')
        y = x - '0';
    return y;
}

std::string UrlUtils::UrlEncode(const std::string &str) {
    std::string strTemp = "";
    size_t      length  = str.length();
    for (size_t i = 0; i < length; i++) {
        if (isalnum((unsigned char)str[ i ]) || (str[ i ] == '-') || (str[ i ] == '_') || (str[ i ] == '.') || (str[ i ] == '~'))
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

std::string UrlUtils::UrlDecode(const std::string &str) {
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