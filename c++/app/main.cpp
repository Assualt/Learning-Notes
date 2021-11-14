//
// Created by Sunshine on 2020/8/30.
//
#include <iostream>
#include "base/tstring.h"
#include "system/tlog.h"
#include "base/tvariant.h"
#include <thread>
using namespace std;
using namespace hmt;

void func(const std::string& name, int cnt) {
    while (cnt--) {
        logger.info("threadName %s this is %d decrease ", name, cnt);
        usleep(1000);
    }
}

int main(int argc, char** argv) {
    //    Logger stdOutLogger("main");
    //    stdOutLogger.setLevel(detail::INFO);
    //    stdOutLogger.BasicConfig(
    //            "T:(%(thread)s) %(asctime)s [%(threadname)s-%(levelname)s] [%(filename)s:%(funcname)s:%(lineno)d] "
    //            "%(message)s",
    //            "",
    //            "%04Y-%02m-%02d");
    //    logImpl::AppendLogger("main", &stdOutLogger);
    //    //    std::thread t1(func, "thread1", 100), t2(func, "thread2", 100),t3(func, "thread3", 100),t4(func,
    //    "thread4",
    //    //    100); t1.join();t2.join();t3.join();t4.join();
    //    logger.debug("Hello World %s", 'a');
    //    logger.info("Hello World %04d ", 123);
    //    logger.warning("Hello World %4.3d ", 123);
    //    logger.error("Hello World %04.2f ", 123.31);
    //    logger.alert("Hello World %04.2f ", 123.3131);
    //    logger.emergency("Hello World %04.2f ", 123.0);
    //
    //    tstring temp = "1;2;3;4";
    //
    //    auto vec = TStringHelper::split(temp, ';');
    //    for (auto it : vec) {
    //        std::cout << it << std::endl;
    //    }
    //    auto vec1 = TStringHelper::split(temp, ';', 1);
    //    for (auto it : vec1) {
    //        std::cout << it << std::endl;
    //    }
    //
    //    std::cout <<":" << TStringHelper::tolower("CONST") << std::endl;
    //    std::cout << TStringHelper::trim("abc") << std::endl;
    //    std::cout << TStringHelper::trim("   abc   ") << std::endl;
    //    std::cout << TStringHelper::ltrim("   abc   ") << std::endl;
    //    std::cout << TStringHelper::rtrim("   abc   ") << std::endl;
    //
    //
    //    std::cout << TStringHelper::capitalize("aBD") << std::endl;
    //    std::cout << TStringHelper::title("this iS a Famous booK") << std::endl;

    std::cout << "=================" << std::endl;
    TVariant t(true);
    std::cout << t << std::endl;

    TVariant t1('a');
    std::cout << t1 << std::endl;

    TVariant t2(123);
    std::cout << t2 << std::endl;

    long f = 1231231231;
    TVariant t3(f);
    std::cout << t3 << std::endl;

    TVariant t4(1.412);
    std::cout << t4 << std::endl;

    TVariant t5("cstring", 10);
    std::cout << t5 << std::endl;

    tstring temp = "tstring";
    TVariant t6(temp);
    std::cout << t6 << std::endl;

    std::cout << "======ARRAY========" << std::endl;
    TVariant t7(TVariant::TYPE_ARRAY);
    t7.append(true);
    std::cout << t7 << std::endl;
    t7.append('a');
    std::cout << t7 << std::endl;
    t7.append(12);
    std::cout << t7 << std::endl;
    t7.append(12.123);
    std::cout << t7 << std::endl;
    t7.append(f);
    std::cout << t7 << std::endl;
    tstring t12 = "temp";
    t7.append(t12);
    std::cout << t7 << std::endl;
    t7.append("123412321");
    std::cout << t7 << std::endl;

    TVariant tempArray(TVariant::TYPE_ARRAY);
    tempArray.append(123);
    t7.append(t1);
    t7.append(tempArray);
    std::cout << t7 << std::endl;

    std::cout << "======STRUCT========" << std::endl;
    TVariant t8(TVariant::TYPE_STRUCT);
    t8.set("key1", true);
    std::cout << t8 << std::endl;
    t8.set("key2", 'a');
    std::cout << t8 << std::endl;
    t8.set("key3", 123);
    std::cout << t8 << std::endl;
    t8.set("key4", 123.123);
    std::cout << t8 << std::endl;
    t8.set("key5", f);
    std::cout << t8 << std::endl;
    t8.set("key6", tstring("tstring"));
    std::cout << t8 << std::endl;
    t8.set("key7", "cstring", -1);
    std::cout << t8 << std::endl;

    std::cout << "Append Array" << std::endl;

    //    t8.set("key9", t7);
    //    std::cout << t8 << std::endl;

    std::cout << "Array Append Array" << std::endl;

    t7.append(t8);
    std::cout << t7 << std::endl;

    std::cout << "--- iterator ----" << std::endl;
    for (size_t i = 0; i < t7.size(); ++i) {
        std::cout << " index " << i << " -->" << t7.at(i) << std::endl;
    }

    std::cout << "--- struct get ----" << std::endl;
    auto keys = t8.keys();
    for(auto key : keys){
        std::cout << key <<" in t8 is " <<t8[key] << std::endl;
    }
    std::cout <<"t8 size " <<t8.size() << std::endl;

    TVariant s("temp",-1);
    std::cout << s.asString() << std::endl;
    return 0;
}