set(base_SRCS
    Date.cpp
    DirScanner.cpp
    File.cpp
    Format.cpp
    Logging.cpp
    Mutex.cpp
    Thread.cpp
    ThreadPool.cpp
    Timestamp.cpp
    stream/StreamBase.cpp
)

add_library(muduo_base ${base_SRCS})
target_link_libraries(muduo_base pthread rt)

install(TARGETS muduo_base DESTINATION lib)

file(GLOB HEADERS 
    "*.h"
    "stream/*.h"
)
install(FILES ${HEADERS} DESTINATION include/base)

if(BUILD_TESTS)
  add_subdirectory(test)  
endif()
