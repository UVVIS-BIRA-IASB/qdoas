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
