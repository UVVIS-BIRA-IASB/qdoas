add_executable(ring
  ../common/QdoasVersion.h
  ../common/CPreferences.cpp
  ../common/CPreferences.h
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
  common mediate engine
  Qt::Widgets Qt::PrintSupport
  ${QWT_LIBRARY}
  ${LIBXMLXX_LIBRARIES}
  )
target_include_directories(ring PRIVATE
  ${QWT_INCLUDE_DIR}
  ${LIBXMLXX_INCLUDE_DIRS})

if(APPLE)
  install(TARGETS ring DESTINATION qdoas.app/Contents/MacOS)
else()
  install(TARGETS ring DESTINATION ${BIN_PREFIX})
endif()
