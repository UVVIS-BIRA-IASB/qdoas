add_executable(doas_cl
  ../qdoas/CQdoasConfigHandler.h
  ../qdoas/CQdoasConfigHandler.cpp
  ../qdoas/CProjectConfigSubHandlers.h
  ../qdoas/CProjectConfigSubHandlers.cpp
  ../qdoas/CProjectConfigAnalysisWindowSubHandlers.h
  ../qdoas/CProjectConfigAnalysisWindowSubHandlers.cpp
  ../qdoas/CProjectConfigTreeNode.h
  ../qdoas/CProjectConfigTreeNode.cpp
  ../qdoas/CProjectConfigItem.h
  ../qdoas/CProjectConfigItem.cpp
  ../convolution/CConvConfigHandler.h
  ../convolution/CConvConfigHandler.cpp
  ../ring/CRingConfigHandler.h
  ../ring/CRingConfigHandler.cpp
  ../usamp/CUsampConfigHandler.h
  ../usamp/CUsampConfigHandler.cpp
  ../common/CConfigHandler.cpp
  ../common/CConfigHandler.h
  ../common/CConfigSubHandlerUtils.cpp
  ../common/CConfigSubHandlerUtils.h
  ../common/CPathSubHandler.cpp
  ../common/CPathSubHandler.h
  ../common/CEngineController.h
  ../common/CEngineError.h
  ../common/CEngineResponse.cpp
  ../common/CEngineResponse.h
  ../common/CWorkSpace.cpp
  ../common/CWorkSpace.h
  ../common/CPlotDataSet.cpp
  ../common/CPlotDataSet.h
  CBatchEngineController.cpp
  CBatchEngineController.h
  cmdline.cpp
  ./glob_match/glob_match.hpp
  ./glob_match/glob_match.cpp
  convxml.cpp
  convxml.h
  qdoasxml.cpp
  qdoasxml.h
)

target_compile_features(doas_cl PUBLIC cxx_std_11)
target_link_libraries(doas_cl
  engine mediate
  ${LIBXMLXX_LIBRARIES})

target_include_directories(doas_cl PRIVATE
  ./glob_match
  ../qdoas ../convolution ../usamp ../ring
  ${LIBXMLXX_INCLUDE_DIRS})

install(TARGETS doas_cl DESTINATION ${BIN_PREFIX})
