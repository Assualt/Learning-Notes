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
}