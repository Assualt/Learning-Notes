#include "base/Range.h"
#include "gtest/gtest.h"
#include <vector>
using namespace muduo::base;

template <class T> struct IsSomeString : std::false_type {};

template <typename Alloc> struct IsSomeString<std::basic_string<char, std::char_traits<char>, Alloc>> : std::true_type {};

TEST(Range, Test1) {
    std::string test = "Hello World";
    StringPiece c(test.data(), 4);

    EXPECT_TRUE(c.startsWith("H"));
    EXPECT_TRUE(c.startsWith('H'));

    EXPECT_EQ(c.front(), 'H');
    EXPECT_FALSE(c.front() == 'h');

    EXPECT_EQ(c.size(), 4);
    EXPECT_EQ(c.back(), 'l');
    EXPECT_EQ(c.walk_size(), 4);
    EXPECT_FALSE(c.empty());

    EXPECT_THROW(c.at(5), std::out_of_range);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
