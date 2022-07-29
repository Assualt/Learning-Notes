#include "base/stream/CompressStream.h"
#include "gtest/gtest.h"

using namespace muduo::base;
TEST(SuiteCompress, Compress_string) {
    std::string  str  = "Hello World";
    std::string  str1 = "Hello World";
    Base64Stream test;
    auto size = test.writeBytes(str.c_str(), str.size());
    auto out = std::make_unique<char[]>(100);
    test.seekg(0, StreamBase::seek_begin);
    size = test.readBlock(out.get(), 1);
    EXPECT_EQ(std::string(out.get()), str);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}