set(ssp_base_files
    object.cpp
)

add_library(ssp_base SHARED ${ssp_base_files})
target_link_libraries(ssp_base PUBLIC ssp_base_interface)