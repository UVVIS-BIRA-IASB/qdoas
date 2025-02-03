# Common Ambient Variables:
#   CURRENT_BUILDTREES_DIR    = ${VCPKG_ROOT_DIR}/buildtrees/${PORT}
#   CURRENT_PACKAGES_DIR      = ${VCPKG_ROOT_DIR}/packages/${PORT}_${TARGET_TRIPLET}
#   CURRENT_PORT_DIR          = ${VCPKG_ROOT_DIR}/ports/${PORT}
#   CURRENT_INSTALLED_DIR     = ${VCPKG_ROOT_DIR}/installed/${TRIPLET}
#   DOWNLOADS                 = ${VCPKG_ROOT_DIR}/downloads
#   PORT                      = current port name (zlib, etc)
#   TARGET_TRIPLET            = current triplet (x86-windows, x64-windows-static, etc)
#   VCPKG_CRT_LINKAGE         = C runtime linkage type (static, dynamic)
#   VCPKG_LIBRARY_LINKAGE     = target library linkage type (static, dynamic)
#   VCPKG_ROOT_DIR            = <C:/path/to/current/vcpkg>
#   VCPKG_TARGET_ARCHITECTURE = target architecture (x64, x86, arm)
#   VCPKG_TOOLCHAIN           = ON OFF
#   TRIPLET_SYSTEM_ARCH       = arm x86 x64
#   BUILD_ARCH                = "Win32" "x64" "ARM"
#   DEBUG_CONFIG              = "Debug Static" "Debug Dll"
#   RELEASE_CONFIG            = "Release Static"" "Release DLL"
#   VCPKG_TARGET_IS_WINDOWS
#   VCPKG_TARGET_IS_UWP
#   VCPKG_TARGET_IS_LINUX
#   VCPKG_TARGET_IS_OSX
#   VCPKG_TARGET_IS_FREEBSD
#   VCPKG_TARGET_IS_ANDROID
#   VCPKG_TARGET_IS_MINGW
#   VCPKG_TARGET_EXECUTABLE_SUFFIX
#   VCPKG_TARGET_STATIC_LIBRARY_SUFFIX
#   VCPKG_TARGET_SHARED_LIBRARY_SUFFIX
#
# 	See additional helpful variables in /docs/maintainers/vcpkg_common_definitions.md

# Also consider vcpkg_from_* functions if you can; the generated code here is for any web accessable
# source archive.
#  vcpkg_from_github
#  vcpkg_from_gitlab
#  vcpkg_from_bitbucket
#  vcpkg_from_sourceforge
vcpkg_download_distfile(ARCHIVE
    URLS "https://github.com/stcorp/coda/archive/refs/tags/2.25.3.zip"
    FILENAME "2.25.3.zip"
    SHA512 8b37175ad53e18f4e5066af86aa83060eb69167224b656e4c003fbf065f4d7fe9008c00aa130d4fe8ee7698b77511eef33776418b2831100eabb4df142b156cf
)

vcpkg_extract_source_archive_ex(
    OUT_SOURCE_PATH SOURCE_PATH
    ARCHIVE "${ARCHIVE}"
    # (Optional) A friendly name to use instead of the filename of the archive (e.g.: a version number or tag).
    # REF 1.0.0
    # (Optional) Read the docs for how to generate patches at:
    # https://github.com/microsoft/vcpkg-docs/blob/main/vcpkg/examples/patching.md
    PATCHES 
      "fix-cmake-install.patch"
)

# # Check if one or more features are a part of a package installation.
# # See /docs/maintainers/vcpkg_check_features.md for more details
# vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
#   FEATURES
#     tbb   WITH_TBB
#   INVERTED_FEATURES
#     tbb   ROCKSDB_IGNORE_PACKAGE_TBB
# )

# Hack to get the path to a conda environment with flex and bison without hardcoding it in this portfile:
file(READ "${CMAKE_CURRENT_LIST_DIR}/BISON_FLEX_PREFIX.txt" BISON_FLEX_PREFIX)
if(NOT BISON_FLEX_PREFIX)
  message(SEND_ERROR "Please put the prefix path where bison and flex can be found in ${CMAKE_CURRENT_LIST_DIR}/BISON_FLEX_PREFIX.txt.  e.g. '/path/to/conda_env/Library/usr'")
endif()
message("Have bison/flex prefix " "${BISON_FLEX_PREFIX}")
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
	OPTIONS -DCMAKE_PREFIX_PATH="${BISON_FLEX_PREFIX}"
        OPTIONS -DCODA_WITH_HDF4=ON -DHDF4_INCLUDE_DIR=${VCPKG_ROOT_DIR}/packages/hdf4_${TARGET_TRIPLET}/include
)

vcpkg_cmake_install()

# # Moves all .cmake files from /debug/share/coda/ to /share/coda/
# # See /docs/maintainers/ports/vcpkg-cmake-config/vcpkg_cmake_config_fixup.md for more details
# When you uncomment "vcpkg_cmake_config_fixup()", you need to add the following to "dependencies" vcpkg.json:
#{
#    "name": "vcpkg-cmake-config",
#    "host": true
#}
# vcpkg_cmake_config_fixup()

# Uncomment the line below if necessary to install the license file for the port
# as a file named `copyright` to the directory `${CURRENT_PACKAGES_DIR}/share/${PORT}`
# vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
