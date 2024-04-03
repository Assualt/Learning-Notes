
add_executable(tickQuery
    query_widget.cpp
    example_window.cpp
    main.cpp
)

target_link_libraries(tickQuery PRIVATE ssp-gui-interface)

add_test(NAME tickQuery COMMAND tickQuery)