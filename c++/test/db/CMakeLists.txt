add_executable(ldb_test test_leveldb.cpp)

target_link_libraries(ldb_test db_client_common)

add_executable(sqlite_test test_sqlite.cpp)
target_link_libraries(sqlite_test db_client_common)

add_executable(mysql_test test_mysql.cpp)
target_link_libraries(mysql_test db_client_common)

add_test(NAME mysql_test COMMAND mysql_test)