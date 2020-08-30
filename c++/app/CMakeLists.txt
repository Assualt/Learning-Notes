cmake_minimum_required(VERSION 3.17)
project(app)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(include/base)
include_directories(include/conf)
include_directories(include/net)
include_directories(include/system)
include_directories(src)

add_executable(app
        include/base/xexception.h
        include/base/xref.h
        include/base/xstring.h
        include/base/xstringbuilder.h
        include/base/xtoken.h
        include/base/xvariant.h
        include/conf/xconfmgrbase.h
        include/conf/xxmlconfmgr.h
        include/net/xtcpsocket.h
        include/system/xfile.h
        include/system/xlock.h
        include/system/xlog.h
        include/system/xrunable.h
        include/system/xthread.h
        include/xmtdef.h
        src/base/xref.cpp
        src/base/xstring.cpp
        src/base/xtoken.cpp
        src/base/xvariant.cpp
        src/conf/xconfmgrbase.cpp
        src/conf/xxmlconfmgr.cpp
        src/net/xtcpsocket.cpp
        src/system/xfile.cpp
        src/system/xrunable.cpp
        src/system/xthread.cpp
        src/system/xlog.cpp
        main.cpp)
#        unittest/unit_tconf.cpp
#        unittest/unit_tstring.cpp)
