cmake_minimum_required(VERSION 3.8.0)

project(qdoas)

set(VERSION 3.2)
set(QDOAS_VERSION "FRM4DOAS")
set(QDOAS_DATE "14 June 2018")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_MACOSX_RPATH ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(WIN32 AND DEFINED ENV{CONDA_PREFIX})
  # all non-python components need to be installed in a Library sub-directory for Conda on Windows
  set(BIN_PREFIX Library/bin)
  set(LIB_PREFIX Library/lib)
  set(PKGDATA_PREFIX Library/share/qdoas)
else()
  set(BIN_PREFIX bin)
  set(LIB_PREFIX lib)
  set(PKGDATA_PREFIX share/qdoas)
endif()

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LIB_PREFIX}")

if(MSVC)
  # Suppress warnings from VC on deprecated POSIX names and unsafe functions
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

find_package(Qt5 COMPONENTS Core Gui Widgets Xml PrintSupport REQUIRED)
find_package(Qwt REQUIRED)

find_package(CODA REQUIRED)
find_package(GSL REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS CXX)
find_package(netCDF REQUIRED)

find_library(HDFEOS2_LIBRARY NAMES hdfeos libhdfeos)
find_library(HDFEOS5_LIBRARY NAMES he5_hdfeos libhe5_hdfeos)

add_definitions(-DQDOAS_VERSION="${QDOAS_VERSION}" -DQDOAS_DATE="${QDOAS_DATE}")

add_subdirectory(common)
add_subdirectory(engine)
add_subdirectory(mediator)
add_subdirectory(qdoas)
add_subdirectory(convolution)
add_subdirectory(ring)
add_subdirectory(usamp)
add_subdirectory(cmdline)
