
add_executable(Test main.cpp)

target_link_libraries(Test PRIVATE OMG)

add_executable(CreateStats create_stats.cpp)

target_link_libraries(CreateStats PRIVATE OMG)

add_executable(LimitingTest limiting_test.cpp)

target_link_libraries(LimitingTest PRIVATE OMG)