//
// Created by 侯鑫 on 2024/1/1.
//

#include <gtest/gtest.h>
#include "base/dir.h"

using namespace ssp::base;
TEST(DirScanner, test)
{
    DirScanner scanner(".");
    FileAttr attr;

    while (scanner.Fetch(attr)) {
        std::cout << attr << std::endl;
    }
}