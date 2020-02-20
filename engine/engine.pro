TEMPLATE = lib

include( ../../config.pri )

CONFIG += $$CODE_GENERATION

QMAKE_CXXFLAGS += -std=c++0x # for centos -std=gnu99 # for Sles -std=c++0x # -std=gnu11 # ++0x

SOURCES += *.c *.cpp
HEADERS += *.h

DEFINES += HAVE_INLINE

DEPENDPATH += ../mediator
INCLUDEPATH += ../mediator ../common

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE -= -O2
# QMAKE_CXXFLAGS_RELEASE += -O3
# QMAKE_CFLAGS_RELEASE += -O3

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

  LIBS         += -lgsl -lgslcblas -lcoda -lhdf -lmfhdf -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lhdf5_tools -lhdfeos -lhe5_hdfeos -lnetcdf -lm

  CONFIG      += windows
}
