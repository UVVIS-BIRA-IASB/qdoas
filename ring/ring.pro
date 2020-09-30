#----------------------------------------------
# Ring Tool General Configuration
#----------------------------------------------

include( ../../config.pri )

TEMPLATE = app
PRE_TARGETDEPS += ../../Obj/$$SYSTEM/common/libcommon.a ../../Obj/$$SYSTEM/engine/libengine.a ../../Obj/$$SYSTEM/mediator/libmediator.a
LIBS += -L../../Obj/$$SYSTEM/mediator -lmediator -L../../Obj/$$SYSTEM/engine -lengine -L../../Obj/$$SYSTEM/common -lcommon

INCLUDEPATH  += ../mediator ../common ../engine
OBJECTS_DIR = ../../Obj/$$SYSTEM/ring
TARGET   = ../../Bin/$$SYSTEM/ring

CONFIG += qt thread $$CODE_GENERATION
QT = core gui xml widgets printsupport # svg

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
  LIBS += -L/bira-iasb/projects/DOAS/Programmes/QDOAS/lib_2019g/lib -lcoda -lhdfeos -lGctp -lnetcdf -lmfhdf -ldf -lz -ljpeg -lhe5_hdfeos -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp 
}

compute {
  LIBS += -lqwt-qt5
}

hpc {
  LIBS += -lqwt # must be linked after hdfeos
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

  LIBS         += -Lc:/My_Applications/QDOAS/winlibs/lib -llibgsl.dll -llibgslcblas.dll
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
