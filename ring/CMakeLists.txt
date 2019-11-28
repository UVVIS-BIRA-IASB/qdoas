include_directories(../common ../engine ../mediator ${QWT_INCLUDE_DIR})

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
  Qt5::Widgets Qt5::PrintSupport Qt5::Xml
  ${QWT_LIBRARY}
  ${GSL_LIBRARIES}
)

if(APPLE)
  install(TARGETS ring DESTINATION qdoas.app/Contents/MacOS)
else()
  install(TARGETS ring DESTINATION ${BIN_PREFIX})
endif()
