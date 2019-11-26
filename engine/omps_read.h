#ifndef OMPS_READ_H
#define OMPS_READ_H

#include "doas.h"

#if defined(__cplusplus)
extern "C" {
#endif

  RC OMPS_read(ENGINE_CONTEXT *pEngineContext,int recordNo);

  RC OMPS_set(ENGINE_CONTEXT *pEngineContext);

  RC OMPS_load_analysis(ENGINE_CONTEXT *pEngineContext,void *responseHandle);

  void OMPS_ReleaseBuffers(void);

  int OMPS_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day);

#if defined(__cplusplus)
}
#endif

#endif
