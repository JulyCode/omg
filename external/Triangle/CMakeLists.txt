cmake_minimum_required(VERSION 3.11)

project(Triangle VERSION 1.6 LANGUAGES C)

add_library(Triangle triangle.c triunsuitable_external.c)

target_compile_definitions(Triangle PUBLIC TRILIBRARY ANSI_DECLARATORS EXTERNAL_TEST)
