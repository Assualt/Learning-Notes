//
// Created by 侯鑫 on 2024/4/27.
//

#include "crypto/archive_core.h"
#include "gtest/gtest.h"

using namespace ssp::crypto;

TEST(Archive, test_list)
{
    ArchiveLib lib;
    if (!lib.Init(TYPE_7ZIP)) {
        return;
    }
}