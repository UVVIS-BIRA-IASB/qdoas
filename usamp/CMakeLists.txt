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
  Qt::Widgets Qt::PrintSupport Qt::Xml
  ${QWT_LIBRARY}
  )
target_include_directories(usamp PRIVATE ${QWT_INCLUDE_DIR})

if(APPLE)
  install(TARGETS usamp DESTINATION qdoas.app/Contents/MacOS)
else()
  install(TARGETS usamp DESTINATION ${BIN_PREFIX})
endif()
