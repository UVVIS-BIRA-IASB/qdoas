include_directories(../common ../engine ../mediator ../qdoas ../convolution ../ring ../usamp)

add_executable(doas_cl
  ../qdoas/CQdoasConfigHandler.cpp
  ../qdoas/CProjectConfigSubHandlers.cpp
  ../qdoas/CProjectConfigAnalysisWindowSubHandlers.cpp
  ../qdoas/CProjectConfigTreeNode.cpp
  ../qdoas/CProjectConfigItem.cpp
  ../convolution/CConvConfigHandler.cpp
  ../ring/CRingConfigHandler.cpp
  ../usamp/CUsampConfigHandler.cpp
  CBatchEngineController.cpp
  CBatchEngineController.h
  cmdline.cpp
  cmdline.pro
  convxml.cpp
  convxml.h
  qdoasxml.cpp
  qdoasxml.h
)

target_compile_features(doas_cl PUBLIC cxx_std_11)
target_link_libraries(doas_cl
  common mediate engine
  Qt5::Core Qt5::Xml
  ${CODA_LIBRARY}
  ${GSL_LIBRARIES}
  ${HDF5_LIBRARIES}
  ${HDFEOS2_LIBRARY}
  ${HDFEOS5_LIBRARY}
  ${netCDF_LIBRARIES}
)

install(TARGETS doas_cl DESTINATION ${BIN_PREFIX})
