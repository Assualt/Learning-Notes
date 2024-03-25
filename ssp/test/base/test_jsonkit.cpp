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

TEST(JsonKit, read_file)
{
    std::ofstream fout("/tmp/1.json");
    fout << rawJson;
    fout.close();
    JsonKit kit("/tmp/1.json");
    EXPECT_EQ(kit.IsError(), false);

    std::string val1;
    bool val2;
    int32_t val3;
    double val4;
    std::vector<int32_t> val5;
    JsonKit val6;
    kit.Get("key1", val1)
       .Get("key2", val2)
       .Get("key3", val3)
       .Get("key4", val4)
       .GetArray("key5", val5)
       .Get("key6", val6)
       ;

    EXPECT_EQ(val1, "this is a key string");
    EXPECT_EQ(val2, 1);
    EXPECT_EQ(val3, 1234);
    EXPECT_EQ(val4, 123.123);
    EXPECT_EQ(val5.size(), 6);
    for (uint32_t idx = 0; idx < val5.size(); ++idx) {
        EXPECT_EQ(val5[idx], idx);
    }

    int32_t tmpVal = 0;
    val6.Get("hello", tmpVal);
    EXPECT_EQ(tmpVal, 123);
    tmpVal = 0;
    val6.Get("Hello", tmpVal);
    EXPECT_EQ(tmpVal, 0);
    std::set<std::string> myTestKeySet = { "hello", "world", "12312" };
    auto keyList = val6.Keys();
    for (auto key : keyList) {
        EXPECT_TRUE(myTestKeySet.count(key));
    }
}