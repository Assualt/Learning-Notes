set(src_cpp 
    mycontroller.cpp
)

add_library(mycontroller SHARED
    ${src_cpp}
)

target_link_directories(mycontroller
    PUBLIC
        ${PROJECT_SOURCE_DIR}/
)
