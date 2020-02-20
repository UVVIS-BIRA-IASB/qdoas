TEMPLATE = lib

include( ../../config.pri )

CONFIG += $$CODE_GENERATION

SOURCES += *.c *.cpp
HEADERS += *.h

INCLUDEPATH  += ../engine ../common

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

