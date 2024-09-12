add_executable(qdoas MACOSX_BUNDLE
  ../common/QdoasVersion.h
  ../common/CPreferences.cpp
  ../common/CPreferences.h
  ../resources/qdoas.icns
  ../resources/qdoas.qrc
  CConfigStateMonitor.cpp
  CConfigStateMonitor.h
  CDoasTable.cpp
  CDoasTable.h
  CEngineRequest.cpp
  CEngineRequest.h
  CEngineThread.cpp
  CEngineThread.h
  CMultiPageTableModel.cpp
  CMultiPageTableModel.h
  CNavigationPanel.cpp
  CNavigationPanel.h
  CProjectConfigAnalysisWindowSubHandlers.cpp
  CProjectConfigAnalysisWindowSubHandlers.h
  CProjectConfigItem.cpp
  CProjectConfigItem.h
  CProjectConfigSubHandlers.cpp
  CProjectConfigSubHandlers.h
  CProjectConfigTreeNode.cpp
  CProjectConfigTreeNode.h
  CProjectTreeClipboard.cpp
  CProjectTreeClipboard.h
  CQdoasConfigHandler.cpp
  CQdoasConfigHandler.h
  CQdoasConfigWriter.cpp
  CQdoasConfigWriter.h
  CQdoasEngineController.cpp
  CQdoasEngineController.h
  CSession.cpp
  CSession.h
  CViewCrossSectionData.h
  CWActiveContext.cpp
  CWActiveContext.h
  CWAnalysisWindowDoasTables.cpp
  CWAnalysisWindowDoasTables.h
  CWAnalysisWindowPropertyEditor.cpp
  CWAnalysisWindowPropertyEditor.h
  CWEditor.h
  CWMain.cpp
  CWMain.h
  CWOutputSelector.cpp
  CWOutputSelector.h
  CWPlotPropertiesEditor.cpp
  CWPlotPropertiesEditor.h
  CWPlotRegion.cpp
  CWPlotRegion.h
  CWProjectAnalysisWindowNameEditor.cpp
  CWProjectAnalysisWindowNameEditor.h
  CWProjectDirectoryEditor.cpp
  CWProjectDirectoryEditor.h
  CWProjectExportEditor.cpp
  CWProjectExportEditor.h
  CWProjectFolderNameEditor.cpp
  CWProjectFolderNameEditor.h
  CWProjectNameEditor.cpp
  CWProjectNameEditor.h
  CWProjectPropertyEditor.cpp
  CWProjectPropertyEditor.h
  CWProjectTabAnalysis.cpp
  CWProjectTabAnalysis.h
  CWProjectTabCalibration.cpp
  CWProjectTabCalibration.h
  CWProjectTabDisplay.cpp
  CWProjectTabDisplay.h
  CWProjectTabInstrumental.cpp
  CWProjectTabInstrumental.h
  CWProjectTabOutput.cpp
  CWProjectTabOutput.h
  CWProjectTabSelection.cpp
  CWProjectTabSelection.h
  CWProjectTabSlit.cpp
  CWProjectTabSlit.h
  CWProjectTabUndersampling.cpp
  CWProjectTabUndersampling.h
  CWProjectTree.cpp
  CWProjectTree.h
  CWSiteEditor.cpp
  CWSiteEditor.h
  CWSiteListCombo.cpp
  CWSiteListCombo.h
  CWSiteTree.cpp
  CWSiteTree.h
  CWSplitter.cpp
  CWSplitter.h
  CWSymbolEditor.cpp
  CWSymbolEditor.h
  CWTableRegion.cpp
  CWTableRegion.h
  CWUserSymbolTree.cpp
  CWUserSymbolTree.h
  PolynomialTab.cpp
  PolynomialTab.h
  qdoas.cpp
)

add_definitions(-DAPP_QDOAS)

target_compile_features(qdoas PUBLIC cxx_std_11)
target_link_libraries(qdoas
  common engine mediate
  Qt::Widgets Qt::PrintSupport
  ${QWT_LIBRARY}
)
target_include_directories(qdoas PRIVATE ${QWT_INCLUDE_DIR})

target_link_libraries(qdoas ${LIBXMLXX_LIBRARIES})
target_include_directories(qdoas PRIVATE ${LIBXMLXX_INCLUDE_DIRS})

set_target_properties(qdoas PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/qdoas.plist)
set_source_files_properties(../resources/qdoas.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

if(APPLE)
  install(TARGETS qdoas DESTINATION .)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Help DESTINATION qdoas.app/Contents/Resources)
else()
  install(TARGETS qdoas DESTINATION ${BIN_PREFIX})
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Help DESTINATION ${PKGDATA_PREFIX})
endif()
