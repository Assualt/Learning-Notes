//
// Created by 侯鑫 on 2024/1/1.
//

#include <gtest/gtest.h>
#include "base/config.h"
#include "base/format.h"
#include <fstream>

using namespace ssp::base;

TEST(Config, val)
{
    auto program = R"""(
[server]
test=1
test2=1.242
str="Hello World"
change=123456


[server/port]
port=1234
)""";

    auto server = R"""(
[server]
test=1
test2=1.242
testString="Hello World"
Long=123456788912

[server/port]
port=1234
isEnable=true
isDisabled=false
float=123.4123
)""";

    {
        std::ofstream out("test.cf");
        out << program;

        std::ofstream out1("server.cf");
        out1 << server;
    }

    ConfigureManager mgr(".");
    mgr.Init();

    auto val = mgr.GetString("test", "/test/server/str");
    EXPECT_EQ(val, "Hello World");

    val = mgr.GetString("test", "/test/server/str1");
    EXPECT_EQ(val, "test");

    auto val1 = mgr.GetInt(10, "/test/server/test");
    EXPECT_EQ(val1, 1);

    val1 = mgr.GetInt(10, "/test/server/test1");
    EXPECT_EQ(val1, 10);

    auto val2 = mgr.GetDouble(1.23, "/test/server/test2");
    EXPECT_EQ(val2, 1.242);

    val2 = mgr.GetDouble(1.23, "/test/server/test21");
    EXPECT_EQ(val2, 1.23);

    auto val3 = mgr.GetBool(false, "/server/server/port/isEnable");
    EXPECT_EQ(val3, true);

    val3 = mgr.GetBool(true, "/server/server/port/isDisabled");
    EXPECT_EQ(val3, false);

    mgr.ChangeAccessPath("/test/server");
    auto val4 = mgr.GetInt(123, "change");
    EXPECT_EQ(val4, 123456);

    mgr.ChangeAccessPath("/server/server/port");
    auto val5 = mgr.GetBool(false, "isEnable");
    EXPECT_EQ(val5, true);

    auto val6 = mgr.GetFloat(0.0, "float");
    auto diff = val6 - 123.4123;
    EXPECT_LT(diff, 1e-05);

    mgr.ChangeAccessPath("/");
    auto val7 = mgr.GetSection("test/server/");
    EXPECT_EQ(val7.size(), 4);

    auto val8 = mgr.GetLong(123, "server/server/Long");
    EXPECT_EQ(val8, 123456788912);

    system("rm -rf *.cfg");

    mgr.Dump(std::cout);
}