add_executable(
    rpc_server
    main_server.cpp
    RpcRegisterManager.cpp
    RpcRequest.cpp
    RpcResponse.cpp
    RpcServer.cpp
)

target_link_libraries(rpc_server muduo_base muduo_net pthread)