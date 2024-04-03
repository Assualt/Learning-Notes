add_executable(tcp_server unit_tcp_server.cpp)
target_link_libraries(tcp_server PRIVATE ssp_base ssp_net)

add_executable(gui_window unit_gtk_window.cpp)
target_link_libraries(gui_window PRIVATE ssp-gui-interface)

add_subdirectory(tick_query)

add_executable(tcp_client unit_tcp_client.cpp)
target_link_libraries(tcp_client PUBLIC ssp_base ssp_net)

add_executable(http_client unit_http_client.cpp)
target_include_directories(http_client
PUBLIC
    ${PROJECT_SOURCE_DIR}/source
)
target_link_libraries(http_client PUBLIC ssp_base ssp_net)

add_executable(http_server unit_http_server.cpp)
target_link_libraries(http_server PRIVATE ssp_base ssp_net)

add_subdirectory(tldextract)