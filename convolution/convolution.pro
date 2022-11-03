#----------------------------------------------
# Convolution Tool General Configuration
#----------------------------------------------

include( ../../config.pri )

TEMPLATE = app
PRE_TARGETDEPS += ../../Obj/$$SYSTEM/common/libcommon.a ../../Obj/$$SYSTEM/engine/libengine.a ../../Obj/$$SYSTEM/mediator/libmediator.a
LIBS += -L../../Obj/$$SYSTEM/mediator -lmediator -L../../Obj/$$SYSTEM/engine -lengine -L../../Obj/$$SYSTEM/common -lcommon 

# QMAKE_CXXFLAGS += -std=c++0x # for centos -std=gnu99

INCLUDEPATH  += ../mediator ../common ../engine
OBJECTS_DIR = ../../Obj/$$SYSTEM/convolution
TARGET   = ../../Bin/$$SYSTEM/convolution

CONFIG += qt thread $$CODE_GENERATION
QT = core gui xml widgets printsupport

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
  LIBS += -L/bira-iasb/projects/DOAS/Programmes/QDOAS/lib_2019g/lib -lnetcdf -lmfhdf -ldf -lz -ljpeg -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lqwt-qt5
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
SOURCES += ../mediator/mediate_xsconv.c
SOURCES += ../mediator/mediate_xsconv_output_netcdf.cpp
SOURCES += ../mediator/mediate_convolution.c
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
HEADERS += ../mediator/mediate_xsconv.h
HEADERS += ../mediator/mediate_xsconv_output_netcdf.h
HEADERS += ../mediator/mediate_convolution.h

HEADERS += CWMain.h
HEADERS += CWConvTabGeneral.h
HEADERS += CWConvTabSlit.h
HEADERS += CConvEngineController.h
HEADERS += CConvConfigHandler.h
HEADERS += CConvConfigWriter.h
HEADERS += convolution_output_netcdf.h


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