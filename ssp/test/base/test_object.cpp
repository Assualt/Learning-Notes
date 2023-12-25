//
// Created by 侯鑫 on 2023/12/26.
//

#include "base/object.h"
#include <gtest/gtest.h>

using namespace ssp::base;
TEST(object, show_it)
{
    Object a;
    Object b;
    std::cout << a.Hashcode() << std::endl;
    std::cout << a.ToString() << std::endl;
}