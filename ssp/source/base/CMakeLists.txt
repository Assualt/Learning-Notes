set(ssp_base_files
    object.cpp
    object_pool.cpp
    backtrace.cpp
    log.cpp
    log_impl.cpp
    timestamp.cpp
    string_utils.cpp
    file.cpp
    mutex.cpp
    dll.cpp
    dir.cpp
    config.cpp
    condition.cpp
    thread.cpp
    thread_context.cpp
    thread_pool.cpp
    json_kit.cpp
)

add_library(ssp_base SHARED ${ssp_base_files})

target_include_directories(ssp_base
PUBLIC
    ${Jsonc_INCLUDE_DIRS}
)

message(STATUS "Jsonc_LIBRARIES --> ${Jsonc_LIBRARIES}")
message(STATUS "Jsonc_INCLUDE_DIRS --> ${Jsonc_INCLUDE_DIRS}")

target_link_libraries(ssp_base PUBLIC ssp_base_interface ${Jsonc_LIBRARIES})