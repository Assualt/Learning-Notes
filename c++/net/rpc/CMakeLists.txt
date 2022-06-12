add_executable(
    main_server
    main_server.cpp
    RpcRegisterManager.cpp
    RpcServer.cpp
)

target_link_libraries(main_server muduo_base muduo_net pthread)