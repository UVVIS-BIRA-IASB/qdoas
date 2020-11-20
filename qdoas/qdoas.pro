#----------------------------------------------
# Qdoas General Configuration
#----------------------------------------------

include( ../../config.pri )

TEMPLATE = app
PRE_TARGETDEPS += ../../Obj/$$SYSTEM/common/libcommon.a ../../Obj/$$SYSTEM/engine/libengine.a ../../Obj/$$SYSTEM/mediator/libmediator.a
LIBS += -L../../Obj/$$SYSTEM/mediator -lmediator -L../../Obj/$$SYSTEM/engine -lengine -L../../Obj/$$SYSTEM/common -lcommon

CONFIG += qt thread $$CODE_GENERATION
QT = core gui xml widgets printsupport # svg

INCLUDEPATH  += ../mediator ../common ../engine
OBJECTS_DIR = ../../Obj/$$SYSTEM/qdoas
TARGET   = ../../Bin/$$SYSTEM/qdoas

DEFINES += APP_QDOAS

#----------------------------------------------
# Platform dependency ... based on ../../config.pri
#----------------------------------------------

unix {
  LIBS += -L/bira-iasb/projects/DOAS/Programmes/QDOAS/lib_2019g/lib -lcoda -lhdfeos -lGctp -lnetcdf -lmfhdf -ldf -lz -ljpeg -lhe5_hdfeos -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp
}

compute {
  LIBS += -L/bira-iasb/projects/DOAS/Programmes/QDOAS/lib_2019g/lib -lqwt-qt5
}

hpc {
  LIBS += -lGctp -L/bira-iasb/projects/DOAS/Programmes/QDOAS/lib_2019g/lib -lqwt # must be linked after hdfeos
}

linux_package {
  TARGET = ../../linux_package/bin/qdoas.bin
  LIBS += -lcoda -lhdfeos -lnetcdf -lmfhdf -ldf -lz -lhe5_hdfeos -lhdf5_hl -lhdf5 -L/bira-iasb/projects/DOAS/Programmes/QDOAS/lib_2019g/lib -lqwt-qt5
}

mxe {
  LIBS += -Wl,-Bstatic -lcoda -lhdfeos -lnetcdf -lmfhdf -ldf -lz -ljpeg -lhe5_hdfeos -lhdf5_hl -lhdf5
  LIBS += -lportablexdr
  LIBS += -lqwt -lQtSvg

  CONFIG += windows
}

caro {

  contains( QWT_LINKAGE, qwtstatic ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB
  }
  contains( QWT_LINKAGE, qwtdll ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB$$QWT_LIB_VERSION
    DEFINES     += QWT_DLL
  }

  LIBS         += -Lc:/My_Applications/QDOAS/winlibs/lib -lhdf -lmfhdf -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lhdf5_tools -lhdfeos -lhe5_hdfeos -lnetcdf -llibgsl.dll -llibgslcblas.dll -lcoda -lm

  CONFIG      += windows

  QT += svg
}

#----------------------------------------------
# GUI Source files
#----------------------------------------------

HEADERS += ../common/QdoasVersion.h
SOURCES += ../common/CPreferences.cpp

SOURCES += CQdoasEngineController.cpp
SOURCES += CEngineRequest.cpp
SOURCES += CEngineThread.cpp
SOURCES += CNavigationPanel.cpp
SOURCES += CMultiPageTableModel.cpp
SOURCES += CSession.cpp
SOURCES += CQdoasConfigHandler.cpp
SOURCES += CProjectConfigSubHandlers.cpp
SOURCES += CProjectConfigAnalysisWindowSubHandlers.cpp
SOURCES += CProjectConfigTreeNode.cpp
SOURCES += CProjectConfigItem.cpp
SOURCES += CQdoasConfigWriter.cpp
SOURCES += CWActiveContext.cpp
SOURCES += CDoasTable.cpp
SOURCES += PolynomialTab.cpp
SOURCES += CWAnalysisWindowDoasTables.cpp
SOURCES += CWMain.cpp
SOURCES += CWPlotRegion.cpp
SOURCES += CWTableRegion.cpp
SOURCES += CWSiteListCombo.cpp
SOURCES += CWProjectAnalysisWindowNameEditor.cpp
SOURCES += CWProjectDirectoryEditor.cpp
SOURCES += CWProjectExportEditor.cpp
SOURCES += CWProjectFolderNameEditor.cpp
SOURCES += CWProjectNameEditor.cpp
SOURCES += CWProjectPropertyEditor.cpp
SOURCES += CWProjectTabDisplay.cpp
SOURCES += CWProjectTabSelection.cpp
SOURCES += CWProjectTabAnalysis.cpp
SOURCES += CWProjectTabCalibration.cpp
SOURCES += CWProjectTabUndersampling.cpp
SOURCES += CWProjectTabInstrumental.cpp
SOURCES += CWProjectTabSlit.cpp
SOURCES += CWProjectTabOutput.cpp
SOURCES += CWOutputSelector.cpp
SOURCES += CWProjectTree.cpp
SOURCES += CProjectTreeClipboard.cpp
SOURCES += CWAnalysisWindowPropertyEditor.cpp
SOURCES += CWSiteEditor.cpp
SOURCES += CWSiteTree.cpp
SOURCES += CWPlotPropertiesEditor.cpp
SOURCES += CWSplitter.cpp
SOURCES += CWSymbolEditor.cpp
SOURCES += CWUserSymbolTree.cpp
SOURCES += CConfigStateMonitor.cpp
SOURCES += qdoas.cpp

#----------------------------------------------
# GUI Header files
#----------------------------------------------

HEADERS += ../common/CPreferences.h

HEADERS += CQdoasEngineController.h
HEADERS += CEngineRequest.h
HEADERS += CEngineThread.h
HEADERS += CNavigationPanel.h
HEADERS += CMultiPageTableModel.h
HEADERS += CSession.h
HEADERS += CViewCrossSectionData.h
HEADERS += CQdoasConfigHandler.h
HEADERS += CProjectConfigSubHandlers.h
HEADERS += CProjectConfigAnalysisWindowSubHandlers.h
HEADERS += CProjectConfigTreeNode.h
HEADERS += CProjectConfigItem.h
HEADERS += CQdoasConfigWriter.h
HEADERS += CWActiveContext.h
HEADERS += CDoasTable.h
HEADERS += PolynomialTab.h
HEADERS += CWAnalysisWindowDoasTables.h
HEADERS += CWEditor.h
HEADERS += CWMain.h
HEADERS += CWPlotRegion.h
HEADERS += CWTableRegion.h
HEADERS += CWSiteListCombo.h
HEADERS += CWProjectAnalysisWindowNameEditor.h
HEADERS += CWProjectDirectoryEditor.h
HEADERS += CWProjectExportEditor.h
HEADERS += CWProjectFolderNameEditor.h
HEADERS += CWProjectNameEditor.h
HEADERS += CWProjectPropertyEditor.h
HEADERS += CWProjectTabAnalysis.h
HEADERS += CWProjectTabDisplay.h
HEADERS += CWProjectTabSelection.h
HEADERS += CWProjectTabCalibration.h
HEADERS += CWProjectTabUndersampling.h
HEADERS += CWProjectTabInstrumental.h
HEADERS += CWProjectTabSlit.h
HEADERS += CWProjectTabOutput.h
HEADERS += CWOutputSelector.h
HEADERS += CWProjectTree.h
HEADERS += CProjectTreeClipboard.h
HEADERS += CWAnalysisWindowPropertyEditor.h
HEADERS += CWSiteEditor.h
HEADERS += CWSiteTree.h
HEADERS += CWPlotPropertiesEditor.h
HEADERS += CWSplitter.h
HEADERS += CWSymbolEditor.h
HEADERS += CWUserSymbolTree.h
HEADERS += CConfigStateMonitor.h

#----------------------------------------------
# Reasource files
#----------------------------------------------
RESOURCES = ../resources/qdoas.qrc


#----------------------------------------------
# Engine Source files
#----------------------------------------------

#----------------------------------------------
# Install
#----------------------------------------------

target.path = $${INSTALL_PREFIX}/bin
doc.path    = $${INSTALL_PREFIX}/doc/qdoas
linux_package {
  doc.path = $${INSTALL_PREFIX}/doc
}

compute {
  target.path = $${INSTALL_PREFIX}/bin_$${QDOAS_VERSION}
  doc.path = $${INSTALL_PREFIX}/doc_$${QDOAS_VERSION}
}

hpc {
  target.path = $${INSTALL_PREFIX}/bin/QDOAS_$${QDOAS_VERSION}
  doc.path = $${INSTALL_PREFIX}/doc/QDOAS_$${QDOAS_VERSION}
}

doc.files = ../../Doc/QDOAS_ReleaseNotes.doc ../../LICENSE ./Help

mxe {
  doc.files -= ./Help
}

INSTALLS += target doc
