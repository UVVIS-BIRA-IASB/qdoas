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

#ifndef FIT_PROPERTIES_H
#define FIT_PROPERTIES_H

#include "doas.h"
#include "spectral_range.h"
#include "linear_system.h"

struct fit_properties {
  struct linear_system *linfit;
  double LFenetre[MAX_FEN][2]; // gaps and analysis window limits in wavelength units (nm)
  doas_spectrum *specrange;     // gaps and analysis window limits in pixels units
  double **A; // matrix of linear system in optical density fitting mode, cross sections in intensity fitting
  double **P; // matrix of linear system in  intensity fitting mode
  double **covar, *SigmaSqr;
  int DimL, // number of rows in A,P => number of data points in the fit
    DimC, // number of columns in A
    DimP, // number of columns in P
    Z, // number of rows in LFenetre => number of intervals in the analysis window
    NF,
    NP,
    nFit;
};

void FIT_PROPERTIES_free(const char *callingFunctionShort, struct fit_properties *fitprops);
RC FIT_PROPERTIES_alloc(const char *callingFunctionShort,struct fit_properties *fitprops);

#endif
