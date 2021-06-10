
file(GLOB SOURCES */*.cpp */*.h)

add_library(OMG ${SOURCES})

target_link_libraries(OMG PRIVATE Triangle)

target_include_directories(OMG PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(OMG PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../external")
