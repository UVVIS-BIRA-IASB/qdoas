/* Copyright (C) 2017 Royal Belgian Institute for Space Aeronomy
 * (BIRA-IASB)
 *
 * BIRA-IASB
 * Ringlaan 3 Avenue Circulaire
 * 1180 Uccle
 * Belgium
 * qdoas@aeronomie.be
 *
 */

#ifndef TROPOMI_READ_H
#define TROPOMI_READ_H

#include "doas.h"
#include "tropomi.h"

#include "mediate_analysis_window.h"

#ifdef __cplusplus
extern "C" {
#endif

  // load reference spectra, set NDET[] and use_row[] arrays.
  int tropomi_init_irradiances(const mediate_analysis_window_t *analysis_windows, int num_windows, enum tropomiSpectralBand spectralBand, int* n_wavel);

  int tropomi_init_radref(const mediate_analysis_window_t *analysis_windows, int num_windows, int *n_wavel);

  int tropomi_prepare_automatic_reference(ENGINE_CONTEXT *pEngineContext, void *responseHandle);

  int tropomi_read(ENGINE_CONTEXT *pEngineContext,int recordNo);

  int tropomi_set(ENGINE_CONTEXT *pEngineContext);

  int tropomi_get_reference_rad(const char* filename, int pixel, double *lambda, double *spectrum, int n_wavel, int *use_row);

  int tropomi_get_reference_irrad(const char* filename, int pixel, double *lambda, double *spectrum, double *sigma, int n_wavel);

  int tropomi_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day);

  void tropomi_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
