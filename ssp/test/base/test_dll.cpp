//
// Created by 侯鑫 on 2024/1/1.
//

#include <gtest/gtest.h>
#include "base/dll.h"

using namespace ssp::base;
TEST(Dll, open_so)
{
    Dll helper;
    auto ret = helper.Open("libz.1.dylib");
    if (ret == -2) {
        std::cout << Dll::ErrMsg() << std::endl;
        return;
    }
}