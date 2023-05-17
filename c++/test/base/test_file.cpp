#include "base/File.h"
#include <gtest/gtest.h>
using namespace muduo::base;

TEST(File, test_file_open) {
    {
        File f;
        EXPECT_EQ(f.Fd(), -1);
    }
}

TEST(File, test_file_read_by_char) {

    FILE *fp = fopen("/tmp/a.txt", "w");
    fwrite("HelloWorld\nHello\n", 17, 1, fp);
    fclose(fp);

    File file("/tmp/a.txt");

    std::string str = file.ReadLineByChar('\n');
    EXPECT_EQ(str, "HelloWorld");

    str = file.ReadLineByChar('\n');
    EXPECT_EQ(str, "Hello");

    str = file.ReadLineByChar('\n');
    EXPECT_EQ(str, "");

    remove("/tmp/a.txt");
}

TEST(File, test_file_read) {

    FILE *fp = fopen("/tmp/a.txt", "w");
    fwrite("HelloWorld\nHello\n", 17, 1, fp);
    fclose(fp);

    File file("/tmp/a.txt");

    char temp[ 256 ] = {0};
    auto size        = file.ReadBytes(temp, sizeof(temp));

    EXPECT_EQ(size, 17);
    EXPECT_TRUE(strncmp(temp, "HelloWorld\nHello\n", 17) == 0);

    remove("/tmp/a.txt");
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
