add_executable(ldb_test
    leveldbclient.cpp
)

target_link_libraries(ldb_test
    leveldb
    pthread
    muduo_base
)