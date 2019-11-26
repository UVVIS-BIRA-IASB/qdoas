TEMPLATE = lib

include( ../config.pri )

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

  LIBS         += -L$$GSL_LIB_PATH -lgsl -L$$CODA_LIB_PATH -lcoda -L$$HDF_LIB_PATH -lhdf -L$$MFHDF_LIB_PATH -lmfhdf -L$$HDF5_LIB_PATH -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lhdf5_tools -L$$HDFEOS_LIB_PATH -lhdfeos -L$$HDFEOS5_LIB_PATH -lhe5_hdfeos -lm

  CONFIG      += windows
}

