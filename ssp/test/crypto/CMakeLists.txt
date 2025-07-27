set(test_crypto_objects
    test_crypto.cpp
)
add_executable(test_ssp_crypto ${test_crypto_objects})

target_link_libraries(test_ssp_crypto
    PRIVATE
    gtest gtest_main gmock gmock_main
    ssp_base ssp_net ssp_crypto
)

add_test(NAME test_ssp_crypto COMMAND test_ssp_crypto)