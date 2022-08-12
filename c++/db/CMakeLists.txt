set(db_client_objects
    leveldb_client.cpp
    sqlite_client.cpp
    mysql_client.cpp
)

add_library(db_client_common SHARED ${db_client_objects})
target_link_libraries(db_client_common
    muduo_base
    pthread dl
    leveldb
    sqlite3
    mysqlclient
)
