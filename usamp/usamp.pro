#----------------------------------------------
# Usamp Tool General Configuration
#----------------------------------------------

include( ../../config.pri )

TEMPLATE = app
PRE_TARGETDEPS += ../../Obj/$$SYSTEM/common/libcommon.a ../../Obj/$$SYSTEM/engine/libengine.a ../../Obj/$$SYSTEM/mediator/libmediator.a
LIBS += -L../../Obj/$$SYSTEM/mediator -lmediator -L../../Obj/$$SYSTEM/engine -lengine -L../../Obj/$$SYSTEM/common -lcommon

INCLUDEPATH  += ../mediator ../common ../engine
OBJECTS_DIR = ../../Obj/$$SYSTEM/usamp
TARGET   = ../../Bin/$$SYSTEM/usamp

CONFIG += qt thread $$CODE_GENERATION
QT = core gui xml widgets printsupport

DEFINES += APP_USAMP

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
  LIBS += -L/bira-iasb/projects/DOAS/Programmes/QDOAS/lib_2019g/lib -lnetcdf -lmfhdf -ldf -lz -ljpeg -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lqwt-qt5
}

linux_package {
  TARGET = ../../linux_package/bin/usamp.bin
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

  CONFIG      += windows
  LIBS         += -Lc:/My_Applications/QDOAS/winlibs/lib -llibgsl.dll -llibgslcblas.dll
  QT += svg
}

#----------------------------------------------
# GUI Source files
#----------------------------------------------

SOURCES += ../common/CPreferences.cpp
SOURCES += ../mediator/mediate_xsconv.c
SOURCES += ../mediator/mediate_xsconv_output_netcdf.cpp
SOURCES += ../mediator/mediate_usamp.c
SOURCES += CWMain.cpp
SOURCES += CWUsampTabGeneral.cpp
SOURCES += CUsampEngineController.cpp
SOURCES += CUsampConfigHandler.cpp
SOURCES += CUsampConfigWriter.cpp
SOURCES += usamptool.cpp

#----------------------------------------------
# GUI Header files
#----------------------------------------------

HEADERS += ../common/CPreferences.h
HEADERS += ../mediator/mediate_xsconv_output_netcdf.h
HEADERS += CWMain.h
HEADERS += CWUsampTabGeneral.h
HEADERS += CUsampEngineController.h
HEADERS += CUsampConfigHandler.h
HEADERS += CUsampConfigWriter.h

#----------------------------------------------
# Resource files
#----------------------------------------------
RESOURCES = ../resources/usamp.qrc

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
