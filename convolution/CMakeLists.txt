include_directories(../common ../engine ../mediator ${QWT_INCLUDE_DIR})

add_executable(convolution
  ../common/QdoasVersion.h
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
  common mediate engine
  Qt5::Widgets Qt5::PrintSupport Qt5::Xml
  ${QWT_LIBRARY}
  ${CODA_LIBRARY}
  ${GSL_LIBRARIES}
  ${HDF5_LIBRARIES}
  ${HDFEOS2_LIBRARY}
  ${HDFEOS5_LIBRARY}
  ${netCDF_LIBRARIES}
)

if(APPLE)
  install(TARGETS convolution DESTINATION qdoas.app/Contents/MacOS)
else()
  install(TARGETS convolution DESTINATION ${BIN_PREFIX})
endif()
