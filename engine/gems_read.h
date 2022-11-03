#ifndef GEMS_H
#define GEMS_H

#include <stdint.h>
#include <stdio.h>

#include "doas.h"

// initial length of allocated buffers
#define GEMS_INIT_LENGTH 2048

#ifdef __cplusplus
extern "C" {
#endif

  void GEMS_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day);
  int GEMS_Set(ENGINE_CONTEXT *pEngineContext);

  int GEMS_Read(ENGINE_CONTEXT *pEngineContext, int record);

  RC GEMS_LoadCalib(ENGINE_CONTEXT *pEngineContext,int indexFenoColumn,void *responseHandle);
  int GEMS_Init(ENGINE_CONTEXT *pEngineContext,char *ref_filename,int* n_wavel_temp);
  
  void gems_clean(void);
  RC GEMS_LoadReference(char *filename,int indexFenoColumn,double *lambda,double *spectrum,int *nwve);
  RC GEMS_LoadAnalysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle) ;
  void GEMS_CloseReferences(void);

#ifdef __cplusplus
}
#endif

#endif
