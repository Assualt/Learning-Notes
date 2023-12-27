//
// Created by 侯鑫 on 2023/12/27.
//

#include <gtest/gtest.h>
#include "base/string_utils.h"

using namespace ssp::base;
using namespace ssp::base::util;

TEST(util, case1)
{
    EXPECT_TRUE(EqualsIgnoreCase("Hello World", "hello world"));
    EXPECT_TRUE(EndsWith("Hello World", "World"));
    EXPECT_FALSE(EndsWith("Hello World", "world"));
    EXPECT_TRUE(EndsWithIgnoreCase("Hello World", "world"));

    EXPECT_TRUE(StartsWithIgnoreCase("Hello World", "hello"));
    EXPECT_TRUE(StartsWith("Hello World", "Hello"));
    EXPECT_FALSE(StartsWith("Hello World", "hello"));

    EXPECT_TRUE(StrContains("Hello World H", "World"));
    EXPECT_FALSE(StrContains("Hello World H", "world"));
    EXPECT_TRUE(StrContains("Hello World", 'H'));

    std::string val = " Hello ";
    std::string key = " ";
    EXPECT_EQ(TrimLeft(val, key), "Hello ");
    EXPECT_EQ(TrimRight(val, key), " Hello");
    EXPECT_EQ(Trim(val, key), "Hello");

    EXPECT_EQ(trimRight(val, ' '), " Hello");
    EXPECT_EQ(trimLeft(val, ' '), "Hello ");
    EXPECT_EQ(trim(val, ' '), "Hello");

    EXPECT_EQ(Count("HelloH", 'H'), 2);
    EXPECT_EQ(Count("HelloH", 'h'), 0);

    std::vector<uint32_t> tmp = { 1, 2, 3};
    EXPECT_EQ(SplitToVector("1,2,3",',').size(), tmp.size());
    auto vec = SplitToVector("1,2,3,,4", ',');
    EXPECT_EQ(vec[0], "1");
    EXPECT_EQ(vec[1], "2");
    EXPECT_EQ(vec[2], "3");
    EXPECT_EQ(vec[3], "4");
}

TEST(lexical1, case1)
{
    EXPECT_EQ(lexical_cast<uint32_t>("123"), 123);
    EXPECT_EQ(lexical_cast<uint32_t>("456"), 456);
}