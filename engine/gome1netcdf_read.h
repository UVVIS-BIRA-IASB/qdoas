
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  READ GOME ERS FILES IN NETCDF FORMAT
//  Name of module    :  GOME1NETCDF_READ.H
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

#ifndef GOME1NETCDF_READ_H
#define GOME1NETCDF_READ_H

#include "doas.h"

#ifdef __cplusplus
extern "C" {
#endif

  RC   GOME1NETCDF_Set(ENGINE_CONTEXT *pEngineContext);
  RC   GOME1NETCDF_Read(ENGINE_CONTEXT *pEngineContext,int recordNo,INDEX fileIndex);
  void GOME1NETCDF_Cleanup(void);
  int  GOME1NETCDF_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day);
  int GOME1NETCDF_InitRef(const char *reference_filename, int *n_wavel_temp, ENGINE_CONTEXT *pEngineContext);
  RC   GOME1NETCDF_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

#ifdef __cplusplus
}
#endif

#endif
