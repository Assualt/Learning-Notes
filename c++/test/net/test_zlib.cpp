//
// Created by xhou on 2022/9/11.
//

#include "net/ZlibStream.h"
#include <fstream>
#include <stdio.h>

using namespace muduo::net;

void TestGzipFile(const std::string &path) {
    MyStringBuffer buffer;

    char   temp[ 1024 ];
    FILE  *fp     = fopen(path.c_str(), "rb");
    size_t nTotal = 0;
    while (true) {
        size_t nRead = fread(temp, 1, sizeof(temp), fp);
        nTotal += nRead;
        std::cout << "read:" << nRead << " total:" << nTotal << " " << buffer.size() << std::endl;
        if (nRead < 1024) {
            buffer.sputn(temp, nRead);
            break;
        }
        buffer.sputn(temp, nRead);
    }

    std::cout << "total:" << buffer.size() << std::endl;
    MyStringBuffer out;
    buffer.seekReadPos(0);
    auto ret = ZlibStream::GzipDecompress(buffer, out);

    std::cout << "ret for buffer decode is=>" << ret << " str:" << out.toString() << std::endl;

}

int main(int argc, char **argv) {
    TestGzipFile("lTime");
    return 0;
}