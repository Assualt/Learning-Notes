#include <algorithm>
#include <fstream>
#include <iostream>
#include <magic.h>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <sstream>
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
    static std::string toResponseBasicDateString();
    static std::string requstTimeFmt();
    static bool        FileIsBinary(const std::string &filePath);
};