#----------------------------------------------
# Convolution Tool General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = ../../qdoas/release/convolution

include( ../config.pri )
PRE_TARGETDEPS += ../common/libcommon.a ../engine/libengine.a ../mediator/libmediator.a

INCLUDEPATH  += ../mediator ../common ../engine

CONFIG += qt thread $$CODE_GENERATION
QT = core gui svg xml widgets printsupport

DEFINES += APP_CONV

# Help system to use...
contains ( HELP_SYSTEM, assistant ) {
    CONFIG  += assistant
    DEFINES += HELP_QT_ASSISTANT
}

#----------------------------------------------
# Platform dependency ...
#----------------------------------------------

INCLUDEPATH  += $$QWT_INC_PATH
unix {
  LIBS += -lqwt
}

linux_package {
  TARGET = ../../linux_package/bin/convolution.bin
}

mxe {
  LIBS += -Wl,-Bstatic -lqwt -lQtSvg
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

  LIBS         += -L$$GSL_LIB_PATH -lgsl -lgslcblas -L$$CODA_LIB_PATH -lcoda -L$$HDF_LIB_PATH -lhdf -L$$MFHDF_LIB_PATH -lmfhdf -L$$HDF5_LIB_PATH -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lhdf5_tools  -L$$HDFEOS_LIB_PATH -lhdfeos -L$$HDFEOS5_LIB_PATH -lhe5_hdfeos -L$$NETCDF_LIB_PATH -lnetcdf -lm

  CONFIG      += windows
  QT += svg
}

#----------------------------------------------
# GUI Source files
#----------------------------------------------

SOURCES += ../common/CPreferences.cpp

SOURCES += CWMain.cpp
SOURCES += CWConvTabGeneral.cpp
SOURCES += CWConvTabSlit.cpp
SOURCES += CConvEngineController.cpp
SOURCES += CConvConfigHandler.cpp
SOURCES += CConvConfigWriter.cpp
SOURCES += convolution.cpp

#----------------------------------------------
# GUI Header files
#----------------------------------------------

HEADERS += ../common/CPreferences.h

HEADERS += CWMain.h
HEADERS += CWConvTabGeneral.h
HEADERS += CWConvTabSlit.h
HEADERS += CConvEngineController.h
HEADERS += CConvConfigHandler.h
HEADERS += CConvConfigWriter.h


#----------------------------------------------
# Resource files
#----------------------------------------------
RESOURCES = ../resources/convolution.qrc

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
