
option(OMG_BUILD_TESTS "Build test programs" ON)

if (OMG_BUILD_TESTS)
    message(STATUS "Building OMG tests...")
    add_subdirectory(tests)
endif ()
