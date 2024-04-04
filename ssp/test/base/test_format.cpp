//
// Created by 侯鑫 on 2024/1/1.
//

#include <gtest/gtest.h>
#include "base/format.h"

using namespace ssp::base;

TEST(Format, case1)
{
    auto str = FmtString("%/%").arg(123).arg(456).str();
    EXPECT_EQ(str, "123/456");

    str = FmtString("%/%%/%").arg(123).arg(456).str();
    EXPECT_EQ(str, "123/%/456");
}
