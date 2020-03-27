#include <gtest/gtest.h>
#include "conf/xconfmgrbase.h"
using namespace xmt;
TEST(TConfigureManager__Test, test) {
    xmt::TInIConfigureManager cpnf(
            "/home/xhou/work/GitRepository/mytools/funny/fastpro/mxt/resource/"
            "conf");
    cpnf.init();
    std::cout << "strString:" << cpnf.getString("", "test", "mysql/host") << std::endl;
    std::cout << "strInt:" << cpnf.getInt(0, "test", "mysql/port") << std::endl;
    std::cout << "strBool:" << cpnf.getBool(false, "test", "mysql/useDefault") << std::endl;
    std::cout << "strFloat:" << cpnf.getFloat(0.0, "test", "mysql/float") << std::endl;
    std::cout << "strDouble:" << cpnf.getDouble(0.0, "test", "mysql/double") << std::endl;
    std::cout << "----------------Default--------------" << std::endl;
    std::cout << "strString:" << cpnf.getString("hostdefault", "test", "mysql/host1") << std::endl;
    std::cout << "strInt:" << cpnf.getInt(120, "test", "mysql/port1") << std::endl;
    std::cout << "strBool:" << cpnf.getBool(true, "test", "mysql/useDefault1") << std::endl;
    std::cout << "strFloat:" << cpnf.getFloat(12312.11, "test", "mysql/float1") << std::endl;
    std::cout << "strDouble:" << cpnf.getDouble(-12312.33, "test", "mysql/double1") << std::endl;

    EXPECT_FALSE(false);
}
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
