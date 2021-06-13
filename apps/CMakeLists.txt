
add_executable(Test main.cpp)

target_link_libraries(Test PRIVATE OMG)

add_subdirectory(reference)
