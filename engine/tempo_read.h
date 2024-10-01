#ifndef TEMPO_READ_H
#define TEMPO_READ_H

#include "mediate_analysis_window.h"
#include "doas.h"

extern const size_t TEMPO_SPECTRAL_CHANNEL;
extern const size_t TEMPO_XTRACK;

int TEMPO_set(ENGINE_CONTEXT *pEngineContext);

int TEMPO_read(ENGINE_CONTEXT *pEngineContext, int record);

int TEMPO_init_irradiances(const mediate_analysis_window_t* analysis_windows, int num_windows, const ENGINE_CONTEXT* pEngineContext);

int TEMPO_get_irradiance_reference(const char* file_name, int row, double *lambda, double *spectrum, double *sigma);

void TEMPO_get_orbit_date(int *year, int *month, int *day);

void TEMPO_cleanup();

#endif
