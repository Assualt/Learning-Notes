
add_executable(tickQuery
    query_widget.cpp
    examplewindow.cpp
    main.cpp
)

target_link_libraries(tickQuery ${GTKMM_LIBRARIES})

add_test(NAME tickQuery COMMAND tickQuery)