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
  convxml.cpp
  convxml.h
  qdoasxml.cpp
  qdoasxml.h
)

target_compile_features(doas_cl PUBLIC cxx_std_11)
target_link_libraries(doas_cl
  common engine mediate 
  Qt::Core
  )
target_include_directories(doas_cl PRIVATE ../qdoas ../convolution ../usamp ../ring)

target_link_libraries(doas_cl ${LIBXMLXX_LIBRARIES})
target_include_directories(doas_cl PRIVATE ${LIBXMLXX_INCLUDE_DIRS})

install(TARGETS doas_cl DESTINATION ${BIN_PREFIX})
