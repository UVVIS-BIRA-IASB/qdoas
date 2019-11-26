#----------------------------------------------
# CmdLine General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = ../../qdoas/release/doas_cl

include( ../config.pri )
PRE_TARGETDEPS += ../common/libcommon.a ../engine/libengine.a ../mediator/libmediator.a

CONFIG += qt thread $$CODE_GENERATION
QT = core xml

INCLUDEPATH  += ../mediator ../common ../qdoas ../convolution ../usamp ../engine ../ring

#----------------------------------------------
# Platform dependency ... based on ../config.pri
#----------------------------------------------

unix {
  LIBS         += -lcoda -lhdfeos -lnetcdf -lmfhdf -ldf -lz -ljpeg -lhe5_hdfeos -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp
}

hpc {
  LIBS += -lGctp # must be linked after hdfeos
}

linux_package {
  TARGET = ../../linux_package/bin/doas_cl.bin
  LIBS         += -lcoda -lhdfeos -lnetcdf -lmfhdf -ldf -ljpeg -lz -lhe5_hdfeos -lhdf5_hl -lhdf5
}

mxe {
  LIBS += -lcoda -lhdfeos -lnetcdf -lmfhdf -ldf -lz -ljpeg -lhe5_hdfeos -lhdf5_hl -lhdf5
  LIBS += -lportablexdr
}

caro {
  INCLUDEPATH  += ../mediator ../common ../qdoas ../convolution ../usamp ../engine ../ring

  contains( QWT_LINKAGE, qwtstatic ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB
  }
  contains( QWT_LINKAGE, qwtdll ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB$$QWT_LIB_VERSION
    DEFINES     += QWT_DLL
  }

  LIBS         += -L$$GSL_LIB_PATH -lgsl -lgslcblas -L$$CODA_LIB_PATH -lcoda -L$$HDF_LIB_PATH -lhdf -L$$MFHDF_LIB_PATH -lmfhdf  -L$$HDFEOS_LIB_PATH -lhdfeos -L$$HDFEOS5_LIB_PATH -lhe5_hdfeos -L$$NETCDF_LIB_PATH -L$$HDF5_LIB_PATH -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lhdf5_tools -lnetcdf -lm

  CONFIG      += console
}

#----------------------------------------------
# Source files
#----------------------------------------------

SOURCES += CBatchEngineController.cpp
SOURCES += ../qdoas/CQdoasConfigHandler.cpp
SOURCES += ../qdoas/CProjectConfigSubHandlers.cpp
SOURCES += ../qdoas/CProjectConfigAnalysisWindowSubHandlers.cpp
SOURCES += ../qdoas/CProjectConfigTreeNode.cpp
SOURCES += ../qdoas/CProjectConfigItem.cpp

SOURCES += ../convolution/CConvConfigHandler.cpp
SOURCES += ../ring/CRingConfigHandler.cpp
SOURCES += ../usamp/CUsampConfigHandler.cpp

SOURCES += cmdline.cpp
SOURCES += convxml.cpp
SOURCES += qdoasxml.cpp

HEADERS += CBatchEngineController.h
HEADERS += convxml.h
HEADERS += qdoasxml.h
HEADERS += ../qdoas/CEngineRequest.h
HEADERS += ../qdoas/CQdoasConfigHandler.h
HEADERS += ../qdoas/CProjectConfigSubHandlers.h
HEADERS += ../qdoas/CProjectConfigAnalysisWindowSubHandlers.h
HEADERS += ../qdoas/CProjectConfigTreeNode.h
HEADERS += ../qdoas/CProjectConfigItem.h

HEADERS += ../convolution/CConvConfigHandler.h
HEADERS += ../ring/CRingConfigHandler.h
HEADERS += ../usamp/CUsampConfigHandler.h

#----------------------------------------------
# Install
#----------------------------------------------

target.path = $${INSTALL_PREFIX}/bin
compute {
  target.path = $${INSTALL_PREFIX}/bin_$${QDOAS_VERSION}
}

hpc {
  target.path = $${INSTALL_PREFIX}/bin/QDOAS_$${QDOAS_VERSION}
}

INSTALLS += target
