#include <gtest/gtest.h>
#include "base/any.h"

using namespace ssp::base;
// 测试套件
TEST(SuiteName, TestName)
{
    // 测试代码
    Any<int> any(10);
    ASSERT_EQ(any.Get(), 10);

    Any<std::string> any2("hello");
    ASSERT_EQ(any2.Get(), "hello");
}
