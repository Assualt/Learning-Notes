#include "gtest/gtest.h"
#include <regex>
TEST(SuiteName, TestName1) {
    EXPECT_EQ(1, 1);
}

TEST(SuitName, TestRegex) {
    std::regex  strReg("/my*\\d+/ok");
    std::string in = "/my555/ok";
    std::smatch ret;
    if (std::regex_match(in, ret, strReg)) {
        std::cout << "match" << std::endl;
    } else {
        std::cout << "match failed" << std::endl;
    }
}
