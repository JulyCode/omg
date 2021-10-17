
option(OMG_BUILD_TESTS "Build test programs" ON)
option(OMG_BUILD_CMD "Build cmd tool" ON)

if (OMG_BUILD_TESTS)
  message(STATUS "Building OMG tests...")
  add_subdirectory(tests)
endif ()


if (OMG_BUILD_CMD)
  message(STATUS "Building OMG cmd tool...")
    
  include(FetchContent)

  FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent.git
    GIT_TAG bec5bd2d1c308178e5a0887c20b23a409942fa29
  )

  FetchContent_MakeAvailable(json)

  add_executable(omg_cmd omg_cmd.cpp)

  target_link_libraries(omg_cmd PRIVATE OMG)
  target_link_libraries(omg_cmd PRIVATE nlohmann_json::nlohmann_json)
endif ()
