
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS/FRM4DOAS
//  Module purpose    :  READ NETCDF FILES FOR FRM4DOAS PROJECT
//  Name of module    :  FRM4DOAS_READ.H
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//        BIRA-IASB
//        Belgian Institute for Space Aeronomy
//        Ringlaan 3 Avenue Circulaire
//        1180     UCCLE
//        BELGIUM
//        qdoas@aeronomie.be
//
//  ----------------------------------------------------------------------------

#ifndef FRM4DOAS_READ_H
#define FRM4DOAS_READ_H

#include "doas.h"

#ifdef __cplusplus
extern "C" {
#endif

  RC   FRM4DOAS_Set(ENGINE_CONTEXT *pEngineContext);
  RC   FRM4DOAS_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay);
  void FRM4DOAS_Cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
