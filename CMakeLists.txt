cmake_minimum_required(VERSION 3.14)

project(OceanMeshGeneration LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(OMG_BUILD_REQUIRE_NETCDF "Require the netCDF library to read .nc files" ON)

option(OMG_BUILD_APPS "Build applications" ON)

if (OMG_BUILD_APPS)
  if (NOT OMG_BUILD_REQUIRE_NETCDF)
    message( SEND_ERROR "NetCDF is required to build the applications" )
  endif ()

  add_subdirectory(apps)
endif ()

add_subdirectory(src)
add_subdirectory(external)
