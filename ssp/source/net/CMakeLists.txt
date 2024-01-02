set(ssp_net_files
    socket.cpp
    socket_ops.cpp
    socket_typedef.cpp
    net_address.cpp
)

add_library(ssp_net SHARED ${ssp_net_files})
target_include_directories(ssp_net
PUBLIC
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(ssp_net
PUBLIC
    ssp_base_interface ssp_base
    ${OPENSSL_LIBRARIES}
)