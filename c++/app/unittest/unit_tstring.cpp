#include <gtest/gtest.h>
#include "base/xstring.h"
using namespace hmt;
TEST(TFmtString___Test, format) {
    tstring tmp1 = TFmtString("temp:%,from:%").arg("123").arg('c').str();
    EXPECT_EQ(tmp1, "temp:123,from:c");

    tstring tmp2 = TFmtString("1:%,2:%,3:%,4:%").arg('c').arg("string").arg(1).arg(1.23).str();
    EXPECT_EQ(tmp2, "1:c,2:string,3:1,4:1.23");
}

TEST(TStringHelper___Test, Split) {
    std::list<tstring> src;
    std::vector<tstring> expect = {"1", "2", "3", "4", "5"};
    TStringHelper::split(src, "1,2,3,4,5");
    EXPECT_EQ(src.size(), expect.size());
    int i = 0;
    for (tstring it : src) {
        EXPECT_EQ(it, expect[i++]);
    }
    TStringHelper::split(src, "1;2;3;4;5", ';');
    EXPECT_EQ(src.size(), expect.size());
    i = 0;
    for (tstring it : src) {
        EXPECT_EQ(it, expect[i++]);
    }
}

TEST(TStringHelper__Test, Trim) {
    tstring case1 = " abcdefg h ";
    EXPECT_EQ("abcdefg h ", TStringHelper::ltrim(case1));
    EXPECT_EQ(" abcdefg h", TStringHelper::rtrim(case1));
    EXPECT_EQ("abcdefg h", TStringHelper::trim(case1));
    EXPECT_EQ("abcdefg", TStringHelper::trim("abcdefg"));
}

TEST(TStringHelper__Test, UpperAndLower) {
    for (char c = 'A'; c <= 'Z'; c++) {
        char temp = c;
        EXPECT_EQ(temp + ('a' - 'A'), TStringHelper::tolower(temp));
    }
    for (char c = 'a'; c <= 'z'; c++) {
        char temp = c;
        EXPECT_EQ(temp + ('A' - 'a'), TStringHelper::toupper(temp));
    }
    EXPECT_EQ("abcdef", TStringHelper::tolower("AbCdEf"));
    EXPECT_EQ("ABCDEF", TStringHelper::toupper("AbCdEf"));
}

TEST(TStringHelper__Test, Replace) {
    EXPECT_EQ(TStringHelper::replaceAll("123123", '1', 'A'), "A23A23");
    EXPECT_EQ(TStringHelper::replaceAll("123123", "12", "AB"), "AB3AB3");
}

TEST(TStringHelper__Test, StartEndWith) {
    EXPECT_TRUE(TStringHelper::startWith("abcdef", "ab"));
    EXPECT_FALSE(TStringHelper::startWith("abcdef", "cb"));
    EXPECT_TRUE(TStringHelper::endWith("abcdef", "ef"));
    EXPECT_FALSE(TStringHelper::endWith("abcdef", "efc"));
}

TEST(TStringHelper__Test, toBytes) {
    EXPECT_EQ("01100001", TStringHelper::toBytes('a'));
    EXPECT_EQ("01100010", TStringHelper::toBytes('b'));

    EXPECT_EQ("0110000101100010", TStringHelper::toBytes("ab"));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
