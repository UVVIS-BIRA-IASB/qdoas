/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <string.h>

#include "mediate_analysis_window.h"
#include "constants.h"

void initializeMediateAnalysisWindow(mediate_analysis_window_t *d)
 {
  // Declarations

  int i;

  // Initializations

  memset(d, 0, sizeof(mediate_analysis_window_t));

  d->requireSpectrum=
  d->requirePolynomial=
  d->requireFit=
  d->requireResidual=
  d->requirePredefined=
  d->requireRefRatio=1;

  // Initialize non integer fields

  d->refSzaCenter=
  d->refSzaDelta=(double)0.;

  d->cloudFractionMin=(double)0.;
  d->cloudFractionMax=(double)1.;

  d->resolFwhm=(double)0.5;
  d->fitMinWavelength=
  d->fitMaxWavelength=
  d->lambda0=(double)0.;

  d->refNs=1;
  d->refSpectrumSelectionScanMode=ANLYS_MAXDOAS_REF_SCAN_AFTER;

  // Cross sections

  for (i=0;i<MAX_AW_CROSS_SECTION;i++)
   {
     d->crossSectionList.crossSection[i].crossType=ANLYS_CROSS_ACTION_INTERPOLATE;
     d->crossSectionList.crossSection[i].requireFit=1;
     d->crossSectionList.crossSection[i].requireCcFit=1;
     d->crossSectionList.crossSection[i].initialCc=(double)0.;
     d->crossSectionList.crossSection[i].deltaCc=(double)1.e-3;
   }

  // Non-linear parameters

  d->nonlinear.solInitial=
  d->nonlinear.off0Initial=
  d->nonlinear.off1Initial=
  d->nonlinear.off2Initial=
  d->nonlinear.comInitial=
  d->nonlinear.usamp1Initial=
  d->nonlinear.usamp2Initial=
  d->nonlinear.resolInitial=(double)0.;

  d->nonlinear.solDelta=
  d->nonlinear.off0Delta=
  d->nonlinear.off1Delta=
  d->nonlinear.off2Delta=
  d->nonlinear.comDelta=
  d->nonlinear.usamp1Delta=
  d->nonlinear.usamp2Delta=
  d->nonlinear.resolDelta=(double)1.e-3;

  // Shift and stretch

  for (i=0;i<MAX_AW_SHIFT_STRETCH;i++)
   {
       d->shiftStretchList.shiftStretch[i].shFit=ANLYS_SHIFT_TYPE_NONLINEAR;

    d->shiftStretchList.shiftStretch[i].shInit=
    d->shiftStretchList.shiftStretch[i].stInit=
    d->shiftStretchList.shiftStretch[i].stInit2=(double)0.;

    d->shiftStretchList.shiftStretch[i].shDelta=
    d->shiftStretchList.shiftStretch[i].stDelta=(double)1.e-3;

    d->shiftStretchList.shiftStretch[i].shMin=
    d->shiftStretchList.shiftStretch[i].shMax=(double)0.;
   }

  // Gaps

  for (i=0;i<MAX_AW_GAP;i++)
   d->gapList.gap[i].minimum=d->gapList.gap[i].maximum=(double)0.;

  // Output

  for (i=0;i<MAX_AW_CROSS_SECTION;i++)
   {
    d->outputList.output[i].slantFactor=(double)1.;
    d->outputList.output[i].slantCol=
    d->outputList.output[i].slantErr=1;
   }
 }
