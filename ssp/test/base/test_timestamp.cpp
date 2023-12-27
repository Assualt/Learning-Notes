//
// Created by 侯鑫 on 2023/12/26.
//

#include "base/Timestamp.h"
#include <gtest/gtest.h>

using namespace ssp::base;
TEST(timestamp, show) {
    TimeStamp t = TimeStamp::Now();
    std::cout << t.ToString() << std::endl;
    std::cout << t.ToFmtString() << std::endl;
    std::cout << t.ToFmtString("%Y-%m-%d %H:%M:%S.%k") << std::endl;

    std::string str = "2023-12-28 10:10:10";
    std::string str1 = "2023-12-29 10:10:10";
    std::string str2 = "2023-12-30 10:10:10";
    auto t1 = TimeStamp::FromTimeStr(str);
    EXPECT_EQ(t1.ToFmtString("%Y-%m-%d %H:%M:%S"), str);

    auto t2 = TimeStamp::FromTimeStr(str1);
    EXPECT_GT(t2, t1);

    auto t3 = TimeStamp::FromTimeStr(str2);
    EXPECT_LE(t2, t3);

    auto t4 = t3 - t2;
    EXPECT_EQ(t4.Seconds(), 60 * 60 * 24);

    auto t5 = t3 - t1;
    EXPECT_EQ(t5.Seconds(), 2 * 60 * 60 * 24);
}