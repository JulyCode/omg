
file(GLOB SOURCES */*.cpp */*.h)

add_library(OMG ${SOURCES})

target_link_libraries(OMG PRIVATE JRSTriangle)

target_link_libraries(OMG PRIVATE Triangle::triangle-api)

find_package(OpenMesh REQUIRED)
target_link_libraries(OMG PRIVATE OpenMeshCore)
target_compile_definitions(OMG PUBLIC _USE_MATH_DEFINES OM_STATIC_BUILD)

target_link_libraries(OMG PRIVATE netcdf-cxx4)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(OMG PUBLIC OpenMP::OpenMP_CXX)
endif()

target_include_directories(OMG PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(OMG PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../external")

# enable all warnings
if(MSVC)
  target_compile_options(OMG PRIVATE /W4)
else()
  target_compile_options(OMG PRIVATE -Wall -Wextra -pedantic)
endif()
