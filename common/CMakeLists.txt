add_library(common STATIC
  CConfigHandler.cpp
  CConfigHandler.h
  CConfigSubHandlerUtils.cpp
  CConfigSubHandlerUtils.h
  CEngineController.h
  CEngineError.h
  CEngineResponse.cpp
  CEngineResponse.h
  CHelpSystem.cpp
  CHelpSystem.h
  ConfigWriterUtils.cpp
  ConfigWriterUtils.h
  CPathMgr.cpp
  CPathMgr.h
  CPathSubHandler.cpp
  CPathSubHandler.h
  CPlotDataSet.cpp
  CPlotDataSet.h
  CPlotImage.h
  CPlotPageData.cpp
  CPlotPageData.h
  CPlotProperties.cpp
  CPlotProperties.h
  CScaleControl.cpp
  CScaleControl.h
  CTablePageData.cpp
  CTablePageData.h
  CValidator.cpp
  CValidator.h
  CWAboutDialog.cpp
  CWAboutDialog.h
  CWFilteringEditor.cpp
  CWFilteringEditor.h
  CWPlotArea.cpp
  CWPlotArea.h
  CWPlotPage.cpp
  CWPlotPage.h
  CWPlotPropertiesConfig.cpp
  CWPlotPropertiesConfig.h
  CWPlotPropertiesDialog.cpp
  CWPlotPropertiesDialog.h
  CWSlitEditors.cpp
  CWSlitEditors.h
  CWorkSpace.cpp
  CWorkSpace.h
  debugutil.h
  visual_c_compat.h
)

target_compile_features(common PUBLIC cxx_std_17)
target_link_libraries(common config Qt::Widgets Qt::PrintSupport PkgConfig::LIBXMLXX)
if (unofficial-qwt_FOUND)
  target_link_libraries(common unofficial::qwt::qwt)
else()
  target_link_libraries(common ${QWT_LIBRARY})
endif()
target_include_directories(common PRIVATE ${QWT_INCLUDE_DIR})
target_include_directories(common PUBLIC ../common ../engine ../mediator)

if(APPLE)
  add_definitions(-DQDOAS_HELP_PATH="../Resources/Help")
else()
  add_definitions(-DQDOAS_HELP_PATH="../share/qdoas/Help")
endif()
