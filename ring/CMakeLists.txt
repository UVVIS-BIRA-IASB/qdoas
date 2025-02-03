add_executable(ring
  ../common/CPreferences.cpp
  ../common/CPreferences.h
  ../common/CHelpSystem.cpp
  ../common/CHelpSystem.h
  ../resources/ring.qrc
  ../resources/qdoas.icns
  CRingConfigHandler.cpp
  CRingConfigHandler.h
  CRingConfigWriter.cpp
  CRingConfigWriter.h
  CRingEngineController.cpp
  CRingEngineController.h
  CWMain.cpp
  CWMain.h
  CWRingTabGeneral.cpp
  CWRingTabGeneral.h
  ringtool.cpp
)

add_definitions(-DAPP_RING)

target_compile_features(ring PUBLIC cxx_std_11)
target_link_libraries(ring
  engine mediate common
  Qt::Widgets Qt::PrintSupport
  PkgConfig::LIBXMLXX
  )
target_include_directories(ring PRIVATE
  ${QWT_INCLUDE_DIR}
)

if(unofficial-qwt_FOUND)
  target_link_libraries(ring unofficial::qwt::qwt)
else()
  target_link_libraries(ring ${QWT_LIBRARY})
endif()

if((DEFINED VCPKG_TARGET_TRIPLET) AND MSVC)
  add_custom_command(TARGET ring POST_BUILD
    COMMAND Qt6::windeployqt
    ARGS $<TARGET_FILE:ring>
  )
endif()

if(APPLE)
  install(TARGETS ring DESTINATION qdoas.app/Contents/MacOS)
else()
  install(TARGETS ring DESTINATION ${BIN_PREFIX})
endif()
