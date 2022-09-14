#include "base/crypto/base64.h"
#include "gtest/gtest.h"
#include <sstream>

TEST(SuiteBase64, test_b64) {
    std::string     str = "Hello World";
    base64::encoder encoder;
    {
        std::stringstream is;
        is << str;
        std::ostringstream out;
        encoder.encode(is, out);
        EXPECT_EQ("SGVsbG8gV29ybGQ=", out.str());
    }
    {
        auto outPtr = std::make_unique<char[]>(40);
        auto size   = encoder.encode_str(str.c_str(), str.size(), outPtr.get());
        EXPECT_EQ("SGVsbG8gV29ybGQ=", std::string(outPtr.get()));
    }
}

TEST(SuiteBase64, test_b64_decode) {
    auto            decodeStr = "SGVsbG8gV29ybGQ=";
    base64::decoder decoder;
    {
        std::stringstream is;
        is << decodeStr;
        std::ostringstream out;
        decoder.decode(is, out);
        EXPECT_EQ("Hello World", out.str());
    }
    {
        auto decodePtr = std::make_unique<char[]>(40);
        decoder.decode(decodeStr, strlen(decodeStr), decodePtr.get());
        EXPECT_EQ("Hello World", std::string(decodePtr.get()));
    }
}

TEST(SuiteBase64, test_b64_decode1) {
    auto            decodeStr = "KipAKiouY29t";
    base64::decoder decoder;

    auto decodePtr = std::make_unique<char[]>(40);
    decoder.decode_str(decodeStr, strlen(decodeStr), decodePtr.get());
    EXPECT_EQ("**@**.com", std::string(decodePtr.get()));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}