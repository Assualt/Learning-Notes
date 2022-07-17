#include "base/checksum/checksum.h"
#include <gtest/gtest.h>
#include <stdio.h>
#include <vector>

using namespace muduo::base;

std::string Trim(const std::string &result, char ch) {
    auto p = result.find(ch);
    return p == std::string::npos ? result : result.substr(0, p);
}

std::string ToHexString(uint32_t num) {
    std::stringstream ss;
    ss << std::hex << num;
    return ss.str();
}

std::string GetSystemResult(const std::string &cmd) {
    auto fp = popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        return "";
    }

    constexpr uint32_t maxSize = 256;
    std::vector<char>  result(256);

    (void)fread(result.data(), maxSize, 1, fp);
    fclose(fp);

    return std::string(result.data());
}

TEST(SimpleCheckSum, test_crc32) {
    CRC32 crc32;
    crc32.write("HelloWorld", 10);
    crc32.flush();
    auto result = Trim(GetSystemResult("echo -n HelloWorld | gzip -1 | tail -c 8 | head -c 4 | hexdump -e '1/4 \"%08x\" \"\n\"'"), '\n');
    EXPECT_EQ(result, ToHexString(crc32.getChecksum()));
}

TEST(SimpleCheckSum, test_md5) {
    MD5 md5;
    md5.write("HelloWorld", 10);
    md5.flush();

    auto result = Trim(GetSystemResult("echo -n 'HelloWorld' | md5sum | cut -d ' ' -f1"), '\n');
    EXPECT_EQ(result, md5.getMD5String());
    EXPECT_EQ(strlen(md5.getMD5String()), 32);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}