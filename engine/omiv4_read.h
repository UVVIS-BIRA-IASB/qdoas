#ifndef OMIV4_READ_H
#define OMIV4_READ_H

#include "mediate_analysis_window.h"
#include "doas.h"

void OMIV4_cleanup();

int OMIV4_read(ENGINE_CONTEXT *pEngineContext,int recordNo);

int OMIV4_set(ENGINE_CONTEXT *pEngineContext);

int OMIV4_init_irradiances(const mediate_analysis_window_t* analysis_windows, int num_windows, const ENGINE_CONTEXT *pEngineContext);

int OMIV4_get_irradiance_reference(const char* filename, int pixel, double *lambda, double *spectrum, double *sigma);

int OMIV4_prepare_automatic_reference(ENGINE_CONTEXT *pEngineContext, void *responseHandle);

int OMIV4_get_orbit_date(int *year, int *month, int *day);

#endif
