set(ssp_net_files
    socket.cpp
    socket_ops.cpp
)

add_library(ssp_net SHARED ${ssp_net_files})
target_link_libraries(ssp_net PUBLIC ssp_base_interface ssp_base)