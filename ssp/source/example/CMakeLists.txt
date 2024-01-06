add_executable(tcp_server unit_tcp_server.cpp)
target_link_libraries(tcp_server PRIVATE ssp_base ssp_net)

add_executable(gui_window unit_gtk_window.cpp)
target_link_libraries(gui_window PUBLIC ${GTKMM_LIBRARIES})

add_subdirectory(tick_query)