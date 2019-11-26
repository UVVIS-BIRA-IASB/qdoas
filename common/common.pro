TEMPLATE = lib

include( ../config.pri )

CONFIG += qt thread $$CODE_GENERATION
QT += core gui xml widgets printsupport

SOURCES       = CConfigHandler.cpp \
		CConfigSubHandlerUtils.cpp \
		CEngineResponse.cpp \
		CHelpSystem.cpp \
		ConfigWriterUtils.cpp \
		CPathMgr.cpp \
		CPathSubHandler.cpp \
		CPlotDataSet.cpp \
		CPlotPageData.cpp \
		CPlotProperties.cpp \
		CScaleControl.cpp \
		CTablePageData.cpp \
		CValidator.cpp \
		CWAboutDialog.cpp \
		CWFilteringEditor.cpp \
		CWPlotArea.cpp \
		CWPlotPage.cpp \
		CWPlotPropertiesConfig.cpp \
		CWPlotPropertiesDialog.cpp \
                CWSlitEditors.cpp \
                CWorkSpace.cpp

HEADERS += *.h

unix {
  DEFINES += QDOAS_HELP_PATH=\"\\\"$${INSTALL_PREFIX}/doc/qdoas/Help\"\\\"
}

windows {
  TARGET = ../common
}

caro {
  contains( QWT_LINKAGE, qwtstatic ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB
  }
  contains( QWT_LINKAGE, qwtdll ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB$$QWT_LIB_VERSION
    DEFINES     += QWT_DLL
  }

  LIBS         += -L$$GSL_LIB_PATH -lgsl -L$$CODA_LIB_PATH -lcoda -L$$HDF_LIB_PATH -lhdf -L$$MFHDF_LIB_PATH -lmfhdf -L$$HDF5_LIB_PATH -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lhdf5_tools  -L$$HDFEOS_LIB_PATH -lhdfeos -L$$HDFEOS5_LIB_PATH -lhe5_hdfeos -lm

  CONFIG      += windows
}
