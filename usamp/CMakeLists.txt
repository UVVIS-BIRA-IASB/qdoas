include_directories(../common ../engine ../mediator ${QWT_INCLUDE_DIR})

add_executable(usamp
  ../common/QdoasVersion.h
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
  common mediate engine
  Qt5::Widgets Qt5::PrintSupport Qt5::Xml
  ${QWT_LIBRARY}
  ${GSL_LIBRARIES}
)

if(APPLE)
  install(TARGETS usamp DESTINATION qdoas.app/Contents/MacOS)
else()
  install(TARGETS usamp DESTINATION ${BIN_PREFIX})
endif()
