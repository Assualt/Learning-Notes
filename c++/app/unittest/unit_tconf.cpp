#include <gtest/gtest.h>
#include "conf/xconfmgrbase.h"
using namespace hmt;

static const tstring ConfigFilesResource = "../unittest/sources";

TEST(TConfigureManager__Test, test) {
    hmt::TInIConfigureManager cpnf(ConfigFilesResource);
    cpnf.init();

    EXPECT_EQ(cpnf.getString("", "test", "mysql/host"), "192.168.201.167");
    EXPECT_EQ(cpnf.getInt(0, "test", "mysql/port"), 3306);
    EXPECT_EQ(cpnf.getBool(false, "test", "mysql/useDefault"), true);
    EXPECT_EQ(cpnf.getInt(-123, "test", "mysql/negetive"), -123);
    // EXPECT_EQ(cpnf.getFloat(0.0, "test", "mysql/float"), 1.23);
    EXPECT_EQ(cpnf.getDouble(0.0, "test", "mysql/double"), 23123.232);
    EXPECT_EQ(cpnf.getString("hostdefault", "test", "mysql/host1"), "hostdefault");
    EXPECT_EQ(cpnf.getInt(120, "test", "mysql/port1"), 120);
    EXPECT_EQ(cpnf.getBool(true, "test", "mysql/useDefault1"), true);
    // EXPECT_EQ(cpnf.getFloat(12312.11, "test", "mysql/float1"), 12312.11);
    EXPECT_EQ(cpnf.getDouble(-12312.33, "test", "mysql/double1"), -12312.33);
    EXPECT_FALSE(false);
}
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
