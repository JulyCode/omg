
file(GLOB SOURCES *.h */*.cpp */*.h)

add_library(OMG ${SOURCES})

target_link_libraries(OMG PRIVATE JRSTriangle)

target_link_libraries(OMG PRIVATE Triangle::triangle-api)
target_link_libraries(OMG PRIVATE jigsaw-lib)

target_link_libraries(OMG PRIVATE ${OPENMESH_LIBRARIES})
target_compile_definitions(OMG PUBLIC _USE_MATH_DEFINES OM_STATIC_BUILD)
target_include_directories(OMG PRIVATE ${OPENMESH_INCLUDE_DIR})

target_link_libraries(OMG PRIVATE netcdf-cxx4) # TODO: HDF is missing

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
