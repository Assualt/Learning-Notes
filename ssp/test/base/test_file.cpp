//
// Created by 侯鑫 on 2024/1/1.
//

#include <gtest/gtest.h>
#include <unistd.h>
#include "base/file.h"

using namespace ssp::base;
TEST(File, open_no_exist_file)
{
    auto file = File::New("test.txt", FileOpType::READ, 0666);
    EXPECT_EQ(file.GetErr(), "No such file or directory");
}

TEST(File, open_and_write_file)
{
    auto file = File::New("test.txt", FileOpType::WRITE, 0666);
    EXPECT_EQ(file.GetErr(), "No such file or directory");
}

TEST(File, open_read_write_file)
{
    system("rm -f test1.txt");
    auto str = "Hello World";
    {
        auto file = File::New("test1.txt", FileOpType::CREATE | FileOpType::TRUNC | FileOpType::RDWR, 0660);
        auto size = file.Write((void *)str, strlen(str));
        EXPECT_EQ(size, strlen(str));
    }

    {
        auto file2 = File::New("test1.txt", FileOpType::READ, 0666);
        std::string line;
        auto rSize = file2.ReadLine(line);
        EXPECT_EQ(line, std::string(str));
        EXPECT_EQ(rSize, strlen(str));
        file2.Flush();
    }
}

TEST(File, temp_newFile)
{
    auto tmpFile = File::Temporary();
    auto attr = tmpFile.GetFileAttr();
    std::cout << attr << std::endl;
    sleep(2);
    auto str = "Hello World";
    auto size = tmpFile.Write((void*)str, strlen(str));
    attr = tmpFile.GetFileAttr();
    EXPECT_TRUE(attr.size == size);
    std::cout << attr << std::endl;
}