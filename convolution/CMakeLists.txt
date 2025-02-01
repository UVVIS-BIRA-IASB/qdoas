add_executable(convolution
  ../common/CPreferences.cpp
  ../common/CPreferences.h
  ../resources/convolution.qrc
  ../resources/qdoas.icns
  CConvConfigHandler.cpp
  CConvConfigHandler.h
  CConvConfigWriter.cpp
  CConvConfigWriter.h
  CConvEngineController.cpp
  CConvEngineController.h
  CMakeLists.txt
  CWConvTabGeneral.cpp
  CWConvTabGeneral.h
  CWConvTabSlit.cpp
  CWConvTabSlit.h
  CWMain.cpp
  CWMain.h
  convolution.cpp
)

add_definitions(-DAPP_CONV)

target_compile_features(convolution PUBLIC cxx_std_11)
target_link_libraries(convolution
  engine mediate common
  Qt::Widgets Qt::PrintSupport
  PkgConfig::LIBXMLXX
  )
target_include_directories(convolution PRIVATE
  ${QWT_INCLUDE_DIR}
)

if(unofficial-qwt_FOUND)
  target_link_libraries(convolution unofficial::qwt::qwt)
else()
  target_link_libraries(convolution ${QWT_LIBRARY})
endif()

if(DEFINED VCPKG_TARGET_TRIPLET)
  add_custom_command(TARGET convolution POST_BUILD
    COMMAND Qt6::windeployqt
    ARGS $<TARGET_FILE:convolution>
  )
endif()

if(APPLE)
  install(TARGETS convolution DESTINATION qdoas.app/Contents/MacOS)
else()
  install(TARGETS convolution DESTINATION ${BIN_PREFIX})
endif()
