/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef _MEDIATE_ANALYSIS_WINDOW_H_GUARD
#define _MEDIATE_ANALYSIS_WINDOW_H_GUARD

#include "mediate_limits.h"
#include "mediate_general.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif

  /* mediate_analysis_window_t
   *
   * Contains all user-specified information about a single spectral analysis window.
   * It allows the GUI to provide information to the engine.
   */

  typedef struct mediate_analysis_window
  {
    char name[ANLYSWIN_NAME_BUFFER_LENGTH];
    int kuruczMode;
    int refSpectrumSelection;
    int refSpectrumSelectionScanMode;
    int refMaxdoasSelection;
    char refOneFile[FILENAME_BUFFER_LENGTH];
    char refTwoFile[FILENAME_BUFFER_LENGTH];
    char residualFile[FILENAME_BUFFER_LENGTH];
    int    saveResidualsFlag;
    double fitMinWavelength;
    double fitMaxWavelength;
    double resolFwhm;
    double lambda0;
    double refSzaCenter;
    double refSzaDelta;
    double refMinLongitude;
    double refMaxLongitude;
    double refMinLatitude;
    double refMaxLatitude;
    int refNs;
    double cloudFractionMin;
    double cloudFractionMax;
    int requireSpectrum;
    int requirePolynomial;
    int requireFit;
    int requireResidual;
    int requirePredefined;
    int requireRefRatio;
    /* table data ... */
    cross_section_list_t crossSectionList;
    struct anlyswin_linear linear;
    struct anlyswin_nonlinear nonlinear;
    shift_stretch_list_t shiftStretchList;
    gap_list_t gapList;
    output_list_t outputList;

  } mediate_analysis_window_t;


  /****************************************************/
  /* Helper functions */

  void initializeMediateAnalysisWindow(mediate_analysis_window_t *d);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
