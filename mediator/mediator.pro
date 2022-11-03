TEMPLATE = lib

include( ../../config.pri )

CONFIG += $$CODE_GENERATION

SOURCES += mediate_analysis_window.c
SOURCES += mediate.c
SOURCES += mediate_common.c
SOURCES += mediate_project.c
SOURCES += mediate_response.cpp

HEADERS += mediate_analysis_window.h
HEADERS += mediate_common.h
HEADERS += mediate_general.h
HEADERS += mediate.h
HEADERS += mediate_limits.h
HEADERS += mediate_project.h
HEADERS += mediate_request.h
HEADERS += mediate_response.h

INCLUDEPATH  += ../engine ../common
OBJECTS_DIR = ../../Obj/$$SYSTEM/mediator
TARGET = $$OBJECTS_DIR/mediator

windows {
  TARGET   = ../mediator
}

caro {

  contains( QWT_LINKAGE, qwtstatic ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB
  }
  contains( QWT_LINKAGE, qwtdll ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB$$QWT_LIB_VERSION
    DEFINES     += QWT_DLL
  }

  LIBS         += -lgsl -lgslcblas -lcoda -lhdf -lmfhdf -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lhdf5_tools -lhdfeos -lhe5_hdfeos -lnetcdf -lm

  CONFIG      += windows
}

