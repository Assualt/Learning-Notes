set(test_source test_fsm_state.cpp)

add_executable(test_fsm ${test_source})

add_test(NAME test_fsm COMMAND test_fsm)