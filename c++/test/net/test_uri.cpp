#include "net/Uri.h"
#include <gtest/gtest.h>
using namespace testing;
using namespace muduo::net;

TEST(uri, test_basic_url_scheme) {
    Uri u("https://root:123456@www.baidu.com:8080/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u.scheme(), "https");

    Uri u1("http://root:123456@www.baidu.com:8080/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u1.scheme(), "http");

    Uri u2("ftp://root:123456@www.baidu.com:8080/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u2.scheme(), "ftp");

    Uri u3("sftp://root:123456@www.baidu.com:8080/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u3.scheme(), "sftp");
}

TEST(uri, test_basic_url_user_pass) {
    Uri u("https://root@www.baidu.com/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u.username(), "root");

    Uri u1("https://root1@www.baidu.com/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u1.username(), "root1");

    Uri u2("https://@www.baidu.com/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u2.username(), "");
    EXPECT_EQ(u2.password(), "");

    Uri u3("https://:123@www.baidu.com/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u3.username(), "");
    EXPECT_EQ(u3.password(), "123");
}

TEST(uri, test_basic_url_port) {
    Uri u("https://www.baidu.com/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u.port(), 443);

    Uri u1("https://www.baidu.com:80/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u1.port(), 80);

    Uri u2("http://www.baidu.com/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u2.port(), 80);

    Uri u3("http://www.baidu.com:123/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u3.port(), 123);

    Uri u4("ftp://www.baidu.com/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u4.port(), 21);

    Uri u5("sftp://www.baidu.com/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u5.port(), 22);
}

TEST(uri, test_basic_url_path) {
    Uri u("https://root@www.baidu.com/query/a/b/c?test=123&arg1=arg2#test");
    EXPECT_EQ(u.path(), "/query/a/b/c");

    Uri u1("https://root1@www.baidu.com?test=123&arg1=arg2#test");
    EXPECT_EQ(u1.path(), "");
}

TEST(uri, test_basic_url_query) {
    Uri u("https://root@www.baidu.com?test=123&arg1=arg2#test");
    EXPECT_EQ(u.query(), "test=123&arg1=arg2");

    Uri u1("https://root1@www.baidu.com?#test");
    EXPECT_EQ(u1.query(), "");
}

TEST(uri, test_basic_url_fragment) {
    Uri u("https://root@www.baidu.com?test=123&arg1=arg2#test");
    EXPECT_EQ(u.fragment(), "test");

    Uri u1("https://root1@www.baidu.com?");
    EXPECT_EQ(u1.fragment(), "");
}

int main(int argc, char **argv) {
    ::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}