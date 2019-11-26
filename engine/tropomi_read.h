/* Copyright (C) 2017 Royal Belgian Institute for Space Aeronomy
 * (BIRA-IASB)
 *
 * BIRA-IASB
 * Ringlaan 3 Avenue Circulaire
 * 1180 Uccle
 * Belgium
 * qdoas@aeronomie.be
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TROPOMI_READ_H
#define TROPOMI_READ_H

#include "doas.h"
#include "tropomi.h"

#ifdef __cplusplus
extern "C" {
#endif

  // load reference spectra, set NDET[] and use_row[] arrays.
  int tropomi_init(const char *ref_filename, ENGINE_CONTEXT *pEngineContext, int *n_wavel_temp);

  int tropomi_prepare_automatic_reference(ENGINE_CONTEXT *pEngineContext, void *responseHandle);

  int tropomi_read(ENGINE_CONTEXT *pEngineContext,int recordNo);

  int tropomi_set(ENGINE_CONTEXT *pEngineContext);

  int tropomi_get_reference(const char* filename, int pixel, double *lambda, double *spectrum, double *sigma,  int n_wavel, const int radAsRef);

  int tropomi_get_orbit_date(int *orbit_year, int *orbit_month, int *orbit_day);

  void tropomi_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif
