add_executable(usamp
  ../common/CPreferences.cpp
  ../common/CPreferences.h
  ../resources/usamp.qrc
  ../resources/qdoas.icns
  CUsampConfigHandler.cpp
  CUsampConfigHandler.h
  CUsampConfigWriter.cpp
  CUsampConfigWriter.h
  CUsampEngineController.cpp
  CUsampEngineController.h
  CWMain.cpp
  CWMain.h
  CWUsampTabGeneral.cpp
  CWUsampTabGeneral.h
  usamptool.cpp
)

add_definitions(-DAPP_USAMP)

target_compile_features(usamp PUBLIC cxx_std_11)
target_link_libraries(usamp
  engine mediate common
  Qt::Widgets Qt::PrintSupport
  PkgConfig::LIBXMLXX
  )
target_include_directories(usamp PRIVATE
  ${QWT_INCLUDE_DIR}
)

if(unofficial-qwt_FOUND)
  target_link_libraries(usamp unofficial::qwt::qwt)
else()
  target_link_libraries(usamp ${QWT_LIBRARY})
endif()

if(DEFINED VCPKG_TARGET_TRIPLET)
  add_custom_command(TARGET usamp POST_BUILD
    COMMAND Qt6::windeployqt
    ARGS $<TARGET_FILE:usamp>
  )
endif()

if(APPLE)
  install(TARGETS usamp DESTINATION qdoas.app/Contents/MacOS)
else()
  install(TARGETS usamp DESTINATION ${BIN_PREFIX})
endif()
