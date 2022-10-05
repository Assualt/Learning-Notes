set(db_client_objects
    leveldb_cli.cpp
    sqlite_cli.cpp
    mysql_cli.cpp
    redis_cli.cpp
)

add_library(db_client_common SHARED ${db_client_objects})
target_link_libraries(db_client_common
    muduo_base
    muduo_net
    pthread dl
    leveldb
    sqlite3
    mysqlclient

)
