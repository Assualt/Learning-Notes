
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(poll_objs)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(poll_objs pollerkqueue_poller.cpp)
endif()

set(ssp_net_files
    buffer.cpp
    acceptor.cpp
    socket.cpp
    socket_ops.cpp
    socket_typedef.cpp
    net_address.cpp
    uri.cpp
    protocol/http_request.cpp
    protocol/http_response.cpp
    server/tcp_server.cpp
    server/http_server.cpp
    event_loop.cpp
    poller/poll_poller.cpp
    poller.cpp
    channel.cpp
    tcp_connection.cpp
    client/tcp_client.cpp
    client/http_client.cpp
    client/http_uri.cpp
    client/http_context.cpp
    client/http_utils.cpp
    ${poll_objs}
)

add_library(ssp_net SHARED ${ssp_net_files})
target_include_directories(ssp_net
PUBLIC
    ${OPENSSL_INCLUDE_DIRS}
)

target_link_libraries(ssp_net
PUBLIC
    ssp_base
    ${OPENSSL_LIBRARIES}
)