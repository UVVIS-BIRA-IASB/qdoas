cmake_minimum_required(VERSION 3.8.0...3.26)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(qdoas VERSION 3.7.8)

set(QDOAS_DATE "21 October 2025")

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

option(BIN_VERSION "Append version to BIN_PREFIX" OFF)

set(BIN_PREFIX bin)
set(LIB_PREFIX lib)
set(PKGDATA_PREFIX share/qdoas)
if(WIN32 AND DEFINED ENV{CONDA_PREFIX})
  # all non-python components need to be installed in a Library sub-directory for Conda on Windows
  set(BIN_PREFIX Library/bin)
  set(LIB_PREFIX Library/lib)
  set(PKGDATA_PREFIX Library/share/qdoas)
elseif(BIN_VERSION)
  set(BIN_PREFIX "bin_${CMAKE_PROJECT_VERSION}")
endif()

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LIB_PREFIX}" CACHE STRING "")

option(PEDANTIC "Enable pedantic compiler warnings." OFF)

if(MSVC)
  if(PEDANTIC)
    add_compile_options(/W4)
  endif()
  # Suppress warnings from VC on deprecated POSIX names and unsafe functions
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS)
  # Add dirent.h implementation for windows
  include_directories(dirent)
else()
  if(PEDANTIC)
    add_compile_options(-Wall -Wextra -Wpedantic)
  endif()
endif()

# We can build with Qt6 or Qt5:
find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport QUIET)
if (NOT Qt6_FOUND)
  find_package(Qt5 COMPONENTS Core Gui Widgets PrintSupport)
endif()

# first try to find the qwt target provided the vcpkg port
find_package(unofficial-qwt QUIET)
if (unofficial-qwt_FOUND)
  # the vcpkg qwt port places the headers in a qwt subdirectory:
  find_path(QWT_INCLUDE_DIR NAMES qwt.h PATH_SUFFIXES qwt)
else() # Not using vcpkg, try our own FindQwt macro
  find_package(Qwt)
endif()

find_package(Boost REQUIRED)
find_package(CODA REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(HDF4 QUIET)
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
# We support different ABI versions of libxml++.
pkg_check_modules(LIBXMLXX IMPORTED_TARGET libxml++-5.0)
if(LIBXMLXX_FOUND)
  add_definitions(-DLIBXMLXX_5)  # set a define to handle different API
endif()
if(NOT LIBXMLXX_FOUND)
  pkg_check_modules(LIBXMLXX IMPORTED_TARGET libxml++-4.0)
endif()
if(NOT LIBXMLXX_FOUND)
  pkg_check_modules(LIBXMLXX REQUIRED IMPORTED_TARGET libxml++-2.6)
endif()

add_definitions(-DQWT_DLL)

add_subdirectory(config)
add_subdirectory(engine)
add_subdirectory(mediator)
add_subdirectory(cmdline)

if((TARGET Qt::Widgets) AND (unofficial-qwt_FOUND OR QWT_FOUND))
  add_subdirectory(qdoas)
  add_subdirectory(convolution)
  add_subdirectory(common)
  add_subdirectory(ring)
  add_subdirectory(usamp)
else()
  message("Qt and/or Qwt libraries were not found.  Only the command line tool doas_cl will be built.")
endif()
