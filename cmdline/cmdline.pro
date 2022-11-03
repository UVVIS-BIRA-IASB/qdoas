#----------------------------------------------
# CmdLine General Configuration
#----------------------------------------------

include( ../../config.pri )

TEMPLATE = app
PRE_TARGETDEPS += ../../Obj/$$SYSTEM/common/libcommon.a ../../Obj/$$SYSTEM/engine/libengine.a ../../Obj/$$SYSTEM/mediator/libmediator.a
LIBS += -L../../Obj/$$SYSTEM/mediator -lmediator -L../../Obj/$$SYSTEM/engine -lengine -L../../Obj/$$SYSTEM/common -lcommon

INCLUDEPATH  += ../mediator ../common ../qdoas ../convolution ../usamp ../engine ../ring
OBJECTS_DIR = ../../Obj/$$SYSTEM/cmdline
TARGET   = ../../Bin/$$SYSTEM/doas_cl

CONFIG += qt thread $$CODE_GENERATION
QT = core xml

#----------------------------------------------
# Platform dependency ... based on ../../config.pri
#----------------------------------------------

unix {
  LIBS         += -L/bira-iasb/projects/DOAS/Programmes/QDOAS/lib_2019g/lib -lcoda -lhdfeos -lGctp -lnetcdf -lmfhdf -ldf -lz -ljpeg -lhe5_hdfeos -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp
}

hpc {
  LIBS += -lGctp # must be linked after hdfeos
}

linux_package {
  TARGET = ../../linux_package/bin/doas_cl.bin
  LIBS         += -lcoda -lhdfeos -lnetcdf -lmfhdf -ldf -ljpeg -lz -lhe5_hdfeos -lhdf5_hl -lhdf5
}

mxe {
  LIBS += -lcoda -lhdfeos -lnetcdf -lmfhdf -ldf -lz -ljpeg -lhe5_hdfeos -lhdf5_hl -lhdf5
  LIBS += -lportablexdr
}

caro {
  INCLUDEPATH  += ../mediator ../common ../qdoas ../convolution ../usamp ../engine ../ring

  contains( QWT_LINKAGE, qwtstatic ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB
  }
  contains( QWT_LINKAGE, qwtdll ) {
    LIBS        += -L$$QWT_LIB_PATH -l$$QWT_LIB$$QWT_LIB_VERSION
    DEFINES     += QWT_DLL
  }

  LIBS         += -llibgsl.dll -llibgslcblas.dll -lcoda -lhdf -lmfhdf -lhdf5 -lhdf5_hl -lhdf5_cpp -lhdf5_hl_cpp -lhdf5_tools -lhdfeos -lhe5_hdfeos -lnetcdf -lm

  CONFIG      += console
}

#----------------------------------------------
# Source files
#----------------------------------------------

SOURCES += CBatchEngineController.cpp
SOURCES += ../qdoas/CQdoasConfigHandler.cpp
SOURCES += ../qdoas/CProjectConfigSubHandlers.cpp
SOURCES += ../qdoas/CProjectConfigAnalysisWindowSubHandlers.cpp
SOURCES += ../qdoas/CProjectConfigTreeNode.cpp
SOURCES += ../qdoas/CProjectConfigItem.cpp

SOURCES += ../convolution/CConvConfigHandler.cpp
SOURCES += ../ring/CRingConfigHandler.cpp
SOURCES += ../usamp/CUsampConfigHandler.cpp
SOURCES += ../mediator/mediate_convolution.c
SOURCES += ../mediator/mediate_ring.c
SOURCES += ../mediator/mediate_usamp.c
SOURCES += ../mediator/mediate_xsconv.c
SOURCES += ../mediator/mediate_xsconv_output_netcdf.cpp

SOURCES += cmdline.cpp
SOURCES += convxml.cpp
SOURCES += qdoasxml.cpp

HEADERS += CBatchEngineController.h
HEADERS += convxml.h
HEADERS += qdoasxml.h
HEADERS += ../qdoas/CEngineRequest.h
HEADERS += ../qdoas/CQdoasConfigHandler.h
HEADERS += ../qdoas/CProjectConfigSubHandlers.h
HEADERS += ../qdoas/CProjectConfigAnalysisWindowSubHandlers.h
HEADERS += ../qdoas/CProjectConfigTreeNode.h
HEADERS += ../qdoas/CProjectConfigItem.h

HEADERS += ../convolution/CConvConfigHandler.h
HEADERS += ../ring/CRingConfigHandler.h
HEADERS += ../usamp/CUsampConfigHandler.h

HEADERS += ../mediator/mediate_convolution.h
HEADERS += ../mediator/mediate_ring.h
HEADERS += ../mediator/mediate_usamp.h
HEADERS += ../mediator/mediate_xsconv.h
HEADERS += ../mediator/mediate_xsconv_output_netcdf.h

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
