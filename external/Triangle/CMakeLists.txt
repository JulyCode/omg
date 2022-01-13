cmake_minimum_required(VERSION 3.14)

include(FetchContent)

FetchContent_Declare(
  triangle
  URL http://www.netlib.org/voronoi/triangle.zip
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/triangle"
)

FetchContent_MakeAvailable(triangle)

project(JRSTriangle VERSION 1.6 LANGUAGES C CXX)

add_library(JRSTriangle STATIC jrs_triangle.cpp)

target_compile_definitions(JRSTriangle PRIVATE TRILIBRARY ANSI_DECLARATORS EXTERNAL_TEST NO_TIMER)
