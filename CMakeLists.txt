cmake_minimum_required(VERSION 3.8.0)

project(qdoas)

set(VERSION 3.4.6)
set(QDOAS_VERSION "3.4.6" CACHE STRING "")
set(QDOAS_DATE "18 May 2022" CACHE STRING "")



function(list_transform_prepend var prefix)
  set(temp "")
  foreach(f ${${var}})
    list(APPEND temp "${prefix}${f}")
  endforeach()
  set(${var} "${temp}" PARENT_SCOPE)
endfunction()


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
  set(BIN_PREFIX bin CACHE STRING "") 
  set(LIB_PREFIX lib)
  set(PKGDATA_PREFIX share/qdoas)
endif()

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LIB_PREFIX}" CACHE STRING "") 

if(MSVC)
  # Suppress warnings from VC on deprecated POSIX names and unsafe functions
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

find_package(Qt5 COMPONENTS Core Gui Widgets Xml PrintSupport REQUIRED)
find_package(Qwt REQUIRED)

find_package(CODA REQUIRED)
find_package(GSL REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS CXX)
find_package(netCDF QUIET)
if(netCDF_FOUND EQUAL 0)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(netCDF netcdf-cxx4 REQUIRED)
  if(NOT netCDF_LINK_LIBRARIES)
    list_transform_prepend(${netCDF_LIBRARIES} "${netCDF_LIBRARY_DIRS}")
    string(REGEX REPLACE "([^;]+)" "${netCDF_LIBRARY_DIRS}/lib\\1.so" netCDF_LINK_LIBRARIES "${netCDF_LIBRARIES}")
  endif()
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(netCDF netCDF_INCLUDE_DIR netCDF_LINK_LIBRARIES netCDF_VERSION)
  set(netCDF_LIBRARIES ${netCDF_LINK_LIBRARIES})
endif()
set(netCDF_INCLUDE_DIR  "${netCDF_INCLUDE_DIRS}")
set(HDFEOS_LIB CACHE STRING "")
find_library(HDFEOS2_LIBRARY NAMES hdfeos libhdfeos PATHS ${HDFEOS_LIB})
find_library(HDFEOS5_LIBRARY NAMES he5_hdfeos libhe5_hdfeos PATHS ${HDFEOS_LIB} )
find_library(GCTP_LIBRARY NAMES libGctp Gctp PATHS ${HDFEOS_LIB} )
list(APPEND HDFEOS_LIBRARIES ${HDFEOS2_LIBRARY} ${HDFEOS5_LIBRARY} ${GCTP_LIBRARY})


add_definitions(-DQDOAS_VERSION="${QDOAS_VERSION}" -DQDOAS_DATE="${QDOAS_DATE}")

add_subdirectory(common)
add_subdirectory(engine)
add_subdirectory(mediator)
add_subdirectory(qdoas)
add_subdirectory(convolution)
add_subdirectory(ring)
add_subdirectory(usamp)
add_subdirectory(cmdline)
