//
// Created by 侯鑫 on 2024/1/3.
//

#include <gtest/gtest.h>
#include "net/uri.h"

using namespace ssp::net;

TEST(URI, test)
{
    auto uriStr = "https://root:123@www.baidu.com:443/test/a/b/c?query=c++&test=123#target";

    Uri uri(uriStr);

    EXPECT_EQ(uri.Scheme(), "https");
    EXPECT_EQ(uri.Username(), "root");
    EXPECT_EQ(uri.Password(), "123");
    EXPECT_EQ(uri.Host(), "www.baidu.com");
    EXPECT_EQ(uri.Port(), 443);
    EXPECT_EQ(uri.Path(), "/test/a/b/c");
    auto params = uri.GetQueryParams();
    EXPECT_EQ(params.size(), 2);
    EXPECT_EQ(params[0].first, "query");
    EXPECT_EQ(params[0].second, "c++");

    EXPECT_EQ(params[1].first, "test");
    EXPECT_EQ(params[1].second, "123");

    EXPECT_EQ(uri.Fragment(), "target");
    EXPECT_EQ(uri.Authority(), "root:123@www.baidu.com:443");
    EXPECT_EQ(uri.Hostname(), "www.baidu.com");

    uri.Dump(std::cout);
}