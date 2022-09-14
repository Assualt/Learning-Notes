//
// Created by xhou on 2022/9/11.
//

#include "base/stream/ZlibStream.h"
#include <fstream>
#include <gtest/gtest.h>

using namespace muduo::base;
using namespace testing;

TEST(test_zlib, gzip) {
    std::string    expStr = "Hello World";
    MyStringBuffer inBuf;
    MyStringBuffer outBuf;
    auto           encodeLen = ZlibStream::GzipCompress(expStr.data(), expStr.size(), inBuf);
    EXPECT_TRUE(encodeLen != ULONG_MAX);
    auto decodeLen = ZlibStream::GzipDecompress(inBuf, outBuf);
    EXPECT_EQ(decodeLen, expStr.size());
    EXPECT_EQ(outBuf.toString(), expStr);
}

TEST(test_zlib, deflate) {
    std::string    expStr = "Hello World";
    MyStringBuffer inBuf;
    MyStringBuffer outBuf;
    auto           encodeLen = ZlibStream::DeflateCompress(expStr.data(), expStr.size(), inBuf);
    EXPECT_TRUE(encodeLen != ULONG_MAX);
    auto decodeLen = ZlibStream::DeflateDecompress(inBuf, outBuf);
    EXPECT_EQ(decodeLen, expStr.size());
    EXPECT_EQ(outBuf.toString(), expStr);
}

TEST(test_zlib, zlib) {
    std::string    expStr = "Hello World";
    MyStringBuffer inBuf;
    MyStringBuffer outBuf;
    auto           encodeLen = ZlibStream::ZlibCompress(expStr.data(), expStr.size(), inBuf);
    EXPECT_TRUE(encodeLen != ULONG_MAX);
    auto decodeLen = ZlibStream::ZlibDeCompress(inBuf, outBuf);
    EXPECT_EQ(decodeLen, expStr.size());
    EXPECT_EQ(outBuf.toString(), expStr);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}