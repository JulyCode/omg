
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
  GIT_REPOSITORY https://github.com/Unidata/netcdf-cxx4.git
  GIT_TAG 162d78a8c4dc33379df746dfe232d0aaebc705db
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/netcdf-cxx4"
)
OPTION(NCXX_ENABLE_TESTS "Enable tests. Run with 'make test'." OFF)

FetchContent_MakeAvailable(jigsaw wo80_Triangle)

find_package(OpenMesh)
if (OpenMesh_FOUND)
  message("OpenMesh found")
else ()
  message("Fetching OpenMesh")
  FetchContent_MakeAvailable(OpenMesh)
endif ()

#find_package(netCDFCxx)
if (netCDFCxx_FOUND)
  message("netCDFCxx found")
else ()
  message("Fetching netCDFCxx")
  FetchContent_MakeAvailable(netcdf-cxx4)
endif ()

add_subdirectory(wo80_Triangle/src)

add_subdirectory(Triangle)
