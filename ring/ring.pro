#----------------------------------------------
# Ring Tool General Configuration
#----------------------------------------------

TEMPLATE = app
TARGET   = ../../qdoas/release/ring

INCLUDEPATH  += ../mediator ../common ../engine

include( ../config.pri )
PRE_TARGETDEPS += ../common/libcommon.a ../engine/libengine.a ../mediator/libmediator.a

CONFIG += qt thread $$CODE_GENERATION
QT = core gui svg xml widgets printsupport

DEFINES += APP_RING

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
  TARGET = ../../linux_package/bin/ring.bin
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

  LIBS         += -L$$GSL_LIB_PATH -lgsl -lgslcblas
  CONFIG      += windows
  QT += svg
}

#----------------------------------------------
# GUI Source files
#----------------------------------------------

SOURCES += ../common/CPreferences.cpp

SOURCES += CWMain.cpp
SOURCES += CWRingTabGeneral.cpp
SOURCES += CRingEngineController.cpp
SOURCES += CRingConfigHandler.cpp
SOURCES += CRingConfigWriter.cpp
SOURCES += ringtool.cpp

#----------------------------------------------
# GUI Header files
#----------------------------------------------

HEADERS += ../common/CPreferences.h

HEADERS += CWMain.h
HEADERS += CWRingTabGeneral.h
HEADERS += CRingEngineController.h
HEADERS += CRingConfigHandler.h
HEADERS += CRingConfigWriter.h

#----------------------------------------------
# Resource files
#----------------------------------------------
RESOURCES = ../resources/ring.qrc

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
