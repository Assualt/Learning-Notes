//
// Created by 侯鑫 on 2023/12/26.
//

#include <gtest/gtest.h>
#include "base/system.h"

using namespace ssp::base;
TEST(system, suit)
{
    EXPECT_EQ(System::MakeDir("tmp", 0660), 0);
    EXPECT_EQ(System::Access("tmp", F_OK), true);

    EXPECT_EQ(System::Rename("tmp", "tmp1"), 0);
    EXPECT_EQ(System::Access("tmp1", F_OK), true);

    EXPECT_EQ(System::RemoveDir("tmp1"), 0);

    EXPECT_TRUE(!System::CurrentPwd().empty());

    System::SetThreadName("loop1");
    EXPECT_EQ(System::GetCurrentThreadName(), "loop1");

    EXPECT_EQ(system("touch 1.txt"), 0);
    EXPECT_EQ(System::Remove("1.txt"), 0);

    EXPECT_EQ(std::string(System::GetErrMsg(2)), "No such file or directory");
    EXPECT_EQ(std::string(System::GetSigName(SIGINT)), "SIGINT");

}