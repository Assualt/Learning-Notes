#pragma once
#include <algorithm>
#include <fstream>
#include <iostream>
#include <magic.h>
#include <sstream>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <vector>
using namespace std;
class utils {
public:
    template <class T> static std::string toString(const T &val) {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }
    static std::string _ltrim(const std::string &src, char ch = ' ');

    static std::string _rtrim(const std::string &src, char ch = ' ');

    static std::string              trim(const std::string &src, char ch = ' ');
    static size_t                   chunkSize(const std::string &strChunkSize);
    static std::vector<std::string> split(const std::string &src, char divider);

    static std::string loadFileString(const std::string &filePath);
    static std::string toResponseBasicDateString(time_t t = time(nullptr));
    static std::string requstTimeFmt();
    static bool        FileIsBinary(const std::string &filePath);
    static bool        FileExists(const std::string &filePath);
    static std::string FileMagicType(const std::string &filePath);
    static std::string toSizeString(off_t nSize);
    static std::string FileDirentTime(struct stat *st);
    static bool        ISDir(const std::string &filepath);
};