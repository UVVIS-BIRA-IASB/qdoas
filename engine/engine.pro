TEMPLATE = lib

include( ../config.pri )

CONFIG += $$CODE_GENERATION

QMAKE_CXXFLAGS += -std=gnu++0x

SOURCES += *.c *.cpp
HEADERS += *.h

DEFINES += HAVE_INLINE

DEPENDPATH += ../mediator
INCLUDEPATH += ../mediator ../common

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CFLAGS_RELEASE += -O3

windows {
  TARGET   = ../engine
}

mxe {
  DEFINES += "H4_BUILT_AS_STATIC_LIB=1"
}

caro {

  contains( QWT_LINKAGE, qwtstatic ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB
  }
  contains( QWT_LINKAGE, qwtdll ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB$$QWT_LIB_VERSION
    DEFINES     += QWT_DLL
  }

  LIBS         += -L$$GSL_LIB_PATH -lgsl -L$$GSL_LIB_PATH -lgslcblas -L$$CODA_LIB_PATH -lcoda -L$$HDF_LIB_PATH -lhdf -L$$MFHDF_LIB_PATH -lmfhdf -L$$HDF5_LIB_PATH -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lhdf5_tools -L$$HDFEOS_LIB_PATH -lhdfeos -L$$HDFEOS5_LIB_PATH -lhe5_hdfeos -L$$NETCDF_LIB_PATH -lnetcdf -lm

  CONFIG      += windows
}
