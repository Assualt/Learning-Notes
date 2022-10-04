add_library(mycontroller SHARED mycontroller.cpp)
target_link_directories(mycontroller PUBLIC ${PROJECT_SOURCE_DIR}/)

add_library(mycontroller1 SHARED mycontroller1.cpp)
target_link_directories(mycontroller1 PUBLIC ${PROJECT_SOURCE_DIR}/)

add_library(uploadctl SHARED upload_ctl.cpp)
target_link_libraries(uploadctl PUBLIC ${PROJECT_SOURCE_DIR}/)