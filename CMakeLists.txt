cmake_minimum_required(VERSION 3.8.0...3.26)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(qdoas)

set(VERSION 3.6.11)
set(QDOAS_VERSION "3.6.11" CACHE STRING "")
set(QDOAS_DATE "3 October 2024" CACHE STRING "")

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(isMultiConfig)
    if(NOT "Asan" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Asan)
    endif()
else()
    set(allowedBuildTypes Asan Debug Release RelWithDebInfo MinSizeRel)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")

    if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
    endif()
endif()

set(CMAKE_C_FLAGS_ASAN
    "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer" CACHE STRING
    "Flags used by the C compiler for Asan build type or configuration." FORCE)

set(CMAKE_CXX_FLAGS_ASAN
    "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer" CACHE STRING
    "Flags used by the C++ compiler for Asan build type or configuration." FORCE)

set(CMAKE_EXE_LINKER_FLAGS_ASAN
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address" CACHE STRING
    "Linker flags to be used to create executables for Asan build type." FORCE)

set(CMAKE_SHARED_LINKER_FLAGS_ASAN
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address" CACHE STRING
    "Linker lags to be used to create shared libraries for Asan build type." FORCE)


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
  # Add dirent.h implementation for windows
  include_directories(dirent)
endif(MSVC)

find_package(Qt5 COMPONENTS Core Gui Widgets PrintSupport)
find_package(Qwt)

find_package(Boost REQUIRED)
find_package(CODA REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS CXX)
find_package(netCDF QUIET)
if(netCDF_FOUND EQUAL 0)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(netCDF netcdf REQUIRED)
  if(NOT netCDF_LINK_LIBRARIES)
    list_transform_prepend(${netCDF_LIBRARIES} "${netCDF_LIBRARY_DIRS}")
    string(REGEX REPLACE "([^;]+)" "${netCDF_LIBRARY_DIRS}/lib\\1.so" netCDF_LINK_LIBRARIES "${netCDF_LIBRARIES}")
  endif()
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(netCDF netCDF_INCLUDE_DIR netCDF_LINK_LIBRARIES netCDF_VERSION)
  set(netCDF_LIBRARIES ${netCDF_LINK_LIBRARIES})
endif()
set(netCDF_INCLUDE_DIR  "${netCDF_INCLUDE_DIRS}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBXMLXX IMPORTED_TARGET libxml++-4.0)
if(NOT LIBXMLXX_FOUND)
  pkg_check_modules(LIBXMLXX REQUIRED IMPORTED_TARGET libxml++-2.6)
endif()


add_definitions(-DQDOAS_VERSION="${QDOAS_VERSION}" -DQDOAS_DATE="${QDOAS_DATE}")

add_definitions(-DQWT_DLL)

add_subdirectory(common)
add_subdirectory(engine)
add_subdirectory(nelmin)
add_subdirectory(mediator)
add_subdirectory(cmdline)

if(Qt5Widgets_FOUND AND QWT_FOUND)
  add_subdirectory(qdoas)
  add_subdirectory(convolution)
  add_subdirectory(ring)
  add_subdirectory(usamp)
else()
  message("Qt and/or Qwt libraries were not found.  Only the command line tool doas_dl will be built.")
endif()
