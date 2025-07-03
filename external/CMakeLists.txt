
include(FetchContent)

FetchContent_Declare(
  jigsaw
  GIT_REPOSITORY https://github.com/dengwirda/jigsaw.git
  GIT_TAG c1ae47455f6f906a2dc28ea26b78909e2a5dc0e4
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/jigsaw"
)
set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT TRUE)

FetchContent_Declare(
  wo80_Triangle
  GIT_REPOSITORY https://github.com/wo80/Triangle.git
  GIT_TAG d3d0ccc94789e7e760f71de568d9d605127bb954
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/wo80_Triangle"
)
option(BUILD_EXAMPLES "Enable examples." OFF)
option(BUILD_TESTING "Enable unit tests." OFF)

FetchContent_Declare(
  OpenMesh
  GIT_REPOSITORY https://www.graphics.rwth-aachen.de:9000/OpenMesh/OpenMesh.git
  GIT_TAG db8c0608830ae7324b2b2e1d91e4db565b2d3eae
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/OpenMesh"
)
set(OPENMESH_BUILD_SHARED true)
set(BUILD_APPS false)
set(OPENMESH_DOCS false)

FetchContent_Declare(
  netcdf-cxx4
  GIT_REPOSITORY https://github.com/truenicfel/netcdf-cxx4.git
  GIT_TAG a71484cb88872b7e4b1c5e2a85ef92849b71dce2
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/netcdf-cxx4"
)
OPTION(NCXX_ENABLE_TESTS "Enable tests. Run with 'make test'." OFF)
option(NCXX_BUILD_TESTS "Build tests?" OFF)
option(NCXX_BUILD_EXAMPLES "Build examples?" OFF)

message("Fetching wo80_Triangle")
FetchContent_MakeAvailable(jigsaw wo80_Triangle)

find_package(OpenMesh)
if (OpenMesh_FOUND)
  message("OpenMesh found")
else ()
  message("Fetching OpenMesh")
  FetchContent_MakeAvailable(OpenMesh)
endif ()

if (OMG_BUILD_REQUIRE_NETCDF)
  message("Fetching netCDFCxx")
  FetchContent_MakeAvailable(netcdf-cxx4)
endif ()

add_subdirectory(wo80_Triangle/src)

add_subdirectory(Triangle)
