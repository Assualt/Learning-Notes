cmake_minimum_required(VERSION 3.10)
project(app)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(include/base)
include_directories(include/conf)
include_directories(include/net)
include_directories(include/system)
include_directories(src)

add_definitions("-Wall -g")
add_executable(app
        include/base/texception.h
        include/base/tref.h
        include/base/tstring.h
        include/base/tstringbuilder.h
        include/base/ttoken.h
        include/base/tvariant.h
        include/conf/tconfmgrbase.h
        include/conf/txmlconfmgr.h
        include/net/ttcpsocket.h
        include/system/tfile.h
        include/system/tlock.h
        include/system/tlog.h
        include/system/trunable.h
        include/system/tthread.h
        include/xmtdef.h
        src/base/tref.cpp
        src/base/tstring.cpp
        src/base/ttoken.cpp
        src/base/tvariant.cpp
        src/conf/tconfmgrbase.cpp
        src/conf/txmlconfmgr.cpp
        src/net/ttcpsocket.cpp
        src/system/tfile.cpp
        src/system/trunable.cpp
        src/system/tthread.cpp
        src/system/tlog.cpp
        main.cpp)
#        unittest/unit_tconf.cpp
#        unittest/unit_tstring.cpp)
