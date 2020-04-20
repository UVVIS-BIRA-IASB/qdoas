#----------------------------------------------
# Convolution Tool General Configuration
#----------------------------------------------

include( ../../config.pri )

TEMPLATE = app
PRE_TARGETDEPS += ../../Obj/$$SYSTEM/common/libcommon.a ../../Obj/$$SYSTEM/engine/libengine.a ../../Obj/$$SYSTEM/mediator/libmediator.a
LIBS += -L../../Obj/$$SYSTEM/mediator -lmediator -L../../Obj/$$SYSTEM/engine -lengine -L../../Obj/$$SYSTEM/common -lcommon

INCLUDEPATH  += ../mediator ../common ../engine
OBJECTS_DIR = ../../Obj/$$SYSTEM/convolution
TARGET   = ../../Bin/$$SYSTEM/convolution

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
  LIBS += -lqwt-qt5
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

  LIBS         += -Lc:/My_Applications/QDOAS/winlibs/lib -llibgsl.dll -llibgslcblas.dll

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
