cmake_minimum_required(VERSION 3.11)

project(JRSTriangle VERSION 1.6 LANGUAGES C CXX)

add_library(JRSTriangle jrs_triangle.cpp)

target_compile_definitions(JRSTriangle PRIVATE TRILIBRARY ANSI_DECLARATORS EXTERNAL_TEST)
