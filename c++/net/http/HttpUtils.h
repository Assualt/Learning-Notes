#pragma once
#include "net/ZlibStream.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <magic.h>
#include <memory>
#include <sstream>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <vector>
using namespace std;
using muduo::net::MyStringBuffer;
class utils {
public:
    template <class T> static std::string toString(const T &val) {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }
    static std::string _ltrim(const std::string &src, char ch = ' ');

    static std::string _rtrim(const std::string &src, char ch = ' ');

    static std::string trim(const std::string &src, char ch = ' ');

    static size_t chunkSize(const std::string &strChunkSize);

    static std::vector<std::string> split(const std::string &src, char divider);

    static std::string loadFileString(const std::string &filePath);

    static int loadBinaryStream(const std::string &filePath, MyStringBuffer &buf);

    static std::string toResponseBasicDateString(time_t t = time(nullptr));

    static std::string requestTimeFmt();

    static bool FileIsBinary(const std::string &filePath);

    static bool FileExists(const std::string &filePath);

    static std::string FileMagicType(const std::string &filePath);

    static std::string toSizeString(off_t nSize);

    static std::string FileDirentTime(struct stat *st);

    static bool IsDir(const std::string &filepath);

    static std::string toHexString(ssize_t nSize);

    static std::string toLower(const std::string &);

    static bool endWith(const std::string &src, const char *backFix);
};

class UrlUtils {
public:
    static unsigned char ToHex(unsigned char x);

    static unsigned char FromHex(unsigned char x);

    static std::string UrlEncode(const std::string &str);

    static std::string UrlDecode(const std::string &str);
};