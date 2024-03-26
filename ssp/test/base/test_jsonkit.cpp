//
// Created by 侯鑫 on 2024/3/25.
//

#include <gtest/gtest.h>
#include <fstream>
#include <set>
#include "base/json_kit.h"

using namespace ssp::base;

std::string rawJson = R"({
    "key1": "this is a key string",
    "key2": true,
    "key3": 1234,
    "key4": 123.123,
    "key5" : [
        0, 1, 2, 3, 4, 5
    ],
    "key6" : {
        "hello" : 123,
        "world" : 456,
        "12312" : 341
    }
}
)";

std::string rawArrayTest = R"({
    "int" : [
        1, 2, 3, 4, 5
    ],
    "string" : [
        "1", "2", "3"
    ],
    "double" : [
        1.1, 2.2, 3.3
    ], 
    "strKey" : 123,
    "strKey1" : "hell world"
})";

TEST(JsonKit, read_file)
{
    std::ofstream fout("/tmp/1.json");
    fout << rawJson;
    fout.close();
    JsonKit kit("/tmp/1.json");
    EXPECT_EQ(kit.HasError(), false);

    std::string val1;
    bool val2;
    int32_t val3;
    double val4;
    std::vector<int32_t> val5;
    JsonKit val6;
    kit.Get("key1", val1).Get("key2", val2).Get("key3", val3)
       .Get("key4", val4).GetArray("key5", val5).Get("key6", val6);

    EXPECT_EQ(val1, "this is a key string");
    EXPECT_EQ(val2, 1);
    EXPECT_EQ(val3, 1234);
    EXPECT_EQ(val4, 123.123);
    EXPECT_EQ(val5.size(), 6);

    for (uint32_t idx = 0; idx < val5.size(); ++idx) {
        EXPECT_EQ(val5[idx], idx);
    }

    uint32_t tmpVal = 0;
    val6.Get("Hello", tmpVal, true);
    EXPECT_EQ(val6.HasError(), true);
    val6.Reset();
    val6.Get("hello", tmpVal, true);
    EXPECT_EQ(tmpVal, 123);
    val6.Reset();
    val6.Get("world", tmpVal, true);
    EXPECT_EQ(tmpVal, 456);

    std::set<std::string> myTestKeySet = { "hello", "world", "12312" };
    auto keyList = val6.Keys();
    for (auto key : keyList) {
        EXPECT_TRUE(myTestKeySet.count(key));
    }
}

TEST(JsonKit, operator_get)
{
    std::ofstream fout("/tmp/2.json");
    fout << rawArrayTest;
    fout.close();
    JsonKit kit("/tmp/2.json");

    EXPECT_FALSE(kit.HasError());
    EXPECT_EQ(kit.HasError(), false);

    JsonKit intKit;
    kit.Get("int", intKit);
    EXPECT_EQ(intKit[0], 1);
    EXPECT_EQ(intKit[1], 2);
    EXPECT_EQ(intKit[2], 3);
    EXPECT_EQ(intKit[3], 4);
    EXPECT_EQ(intKit[4], 5);
    EXPECT_THROW(intKit[6], std::out_of_range);

    JsonKit strKit;
    kit.Get("string", strKit);
    EXPECT_EQ(strKit[0], "1");
    EXPECT_EQ(strKit[1], "2");
    EXPECT_EQ(strKit[2], "3");
    EXPECT_THROW(strKit[6], std::out_of_range);

    JsonKit doubleKit;
    kit.Get("double", doubleKit); // 隐式转换优先级 ？
    // std::cout << (doubleKit[0] == 1.10000) << std::endl;
    // EXPECT_EQ(doubleKit[1], 2.2);
    // EXPECT_EQ(doubleKit[2], 3.30000);
    // EXPECT_THROW(doubleKit[6], std::out_of_range);

    EXPECT_EQ(kit["strKey"], 123);
    EXPECT_EQ(kit["strKey1"], "hell world");
}