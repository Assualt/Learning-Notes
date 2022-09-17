set(mail_objects
    mail_env.cpp
    mailserver.cpp
    mail_process.cpp
)

add_library(mail_runtime SHARED ${mail_objects})
target_link_libraries(mail_runtime pthread dl)

add_executable(mailserver unitMailServer.cpp)
target_link_libraries(mailserver
    mail_runtime
    pthread
    dl
    muduo_base
    muduo_net
    uuid
)