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

#include <string.h>

#include "mediate_project.h"
#include "constants.h"

// ==========================================================
// INITIALIZATIONS OF NON-ZERO VALUES AND float/DOUBLE FIELDS
// ==========================================================

void initializeMediateProjectDisplay(mediate_project_display_t *d)
{
  memset(d, 0, sizeof(mediate_project_display_t));

  // any non-zero defaults...

  d->requireSpectra = 1;
  d->requireData = 1;
  d->requireCalib = 1;
  d->requireFits = 1;
}

void initializeMediateProjectSelection(mediate_project_selection_t *d)
{
  memset(d, 0, sizeof(mediate_project_selection_t));

  // any non-zero defaults...

  d->szaMinimum = 0.0;
  d->szaMaximum = 0.0;
  d->szaDelta   = 0.0;

  d->refAngle   = 90.;
  d->refTolerance = 10.;


  d->elevationMinimum = 0.0;
  d->elevationMaximum = 0.0;
  d->elevationTolerance = 0.0;

  d->cloudFractionMinimum=0.0;
  d->cloudFractionMaximum=1.0;
}

void initializeMediateProjectAnalysis(mediate_project_analysis_t *d)
{
  memset(d, 0, sizeof(mediate_project_analysis_t));

  // any non-zero defaults...

  d->interpolationType = PRJCT_ANLYS_INTERPOL_SPLINE;
  d->interpolationSecurityGap = 10;
  d->maxIterations = 0;
  d->convergenceCriterion = 1.0e-4;
  d->spike_tolerance = 999.9;
}

void initializeMediateProjectFiltering(mediate_filter_t *d)
{
  memset(d, 0, sizeof(mediate_filter_t));

  // any non-zero defaults...
}

void initializeMediateProjectCalibration(mediate_project_calibration_t *d)
{
  int i;
  memset(d, 0, sizeof(mediate_project_calibration_t));

  // any non-zero defaults...

  d->subWindows=0;
  d->wavelengthMin=(double)330.;
  d->wavelengthMax=(double)600.;
  d->windowSize=(double)10.;
  d->requireSpectra=1;
  d->requireFits=1;
  d->requireResidual=1;
  d->requireShiftSfp=1;
  d->preshiftMin=(double)-3.;
  d->preshiftMax=(double)3.;

  for (i=0;i<NSFP;i++)
   {
    d->sfp[i].fitFlag=1;
    d->sfp[i].initialValue=(double)0.5;
    d->sfp[i].deltaValue=(double)1.e-3;
   }

  for (i=0;i<MAX_CALIB_WINDOWS;i++)
   d->customLambdaMin[i]=d->customLambdaMax[i]=(double)0.;
}

void initializeMediateProjectUndersampling(mediate_project_undersampling_t *d)
{
  memset(d, 0, sizeof(mediate_project_undersampling_t));

  // any non-zero defaults...
}

void initializeMediateProjectInstrumental(mediate_project_instrumental_t *d)
{
  memset(d, 0, sizeof(mediate_project_instrumental_t));

  // any non-zero defaults...

  d->saozvis.spectralRegion=PRJCT_INSTR_SAOZ_REGION_VIS;

  d->ascii.lambdaMin=d->ascii.lambdaMax=(double)0.;
  d->saozefm.lambdaMin=d->saozefm.lambdaMax=(double)0.;
  d->mfc.lambdaMin=d->mfc.lambdaMax=(double)0.;
  d->mfcstd.lambdaMin=d->mfcstd.lambdaMax=(double)0.;
  d->mfcbira.lambdaMin=d->mfcbira.lambdaMax=(double)0.;
  d->rasas.lambdaMin=d->rasas.lambdaMax=(double)0.;
  d->pdasieasoe.lambdaMin=d->pdasieasoe.lambdaMax=(double)0.;
  d->ccdeev.lambdaMin=d->ccdeev.lambdaMax=(double)0.;
  d->uoft.lambdaMin=d->uoft.lambdaMax=(double)0.;
  d->noaa.lambdaMin=d->noaa.lambdaMax=(double)0.;
  d->mkzy.lambdaMin=d->mkzy.lambdaMax=(double)0.;
  d->biramobile.lambdaMin=d->biramobile.lambdaMax=(double)0.;
  d->biraairborne.lambdaMin=d->biraairborne.lambdaMax=(double)0.;
  d->oceanoptics.lambdaMin=d->oceanoptics.lambdaMax=(double)0.;

  d->omi.pixelQFMaxGaps=5;
}

void initializeMediateProjectSlit(mediate_project_slit_t *d)
{
  memset(d, 0, sizeof(mediate_project_slit_t));

  // any non-zero defaults...
}

void initializeMediateProjectOutput(mediate_project_output_t *d)
{
  memset(d, 0, sizeof(mediate_project_output_t));

  // non-zero defaults:
  strcpy(d->swath_name, OUTPUT_HDF5_DEFAULT_GROUP);

  d->successFlag=1;
  d->bandWidth=1.;
}

void initializeMediateProjectExport(mediate_project_export_t *d)
{
  memset(d, 0, sizeof(mediate_project_export_t));

  // any non-zero defaults...

  d->titlesFlag = 1;
  d->directoryFlag = 1;
}

void initializeMediateProject(mediate_project_t *d, const char *config_file, const char *project_name)
{
  strncpy(d->project_name, project_name, PROJECT_NAME_BUFFER_LENGTH -1);

  /* delegate to sub component initialization functions */
  initializeMediateProjectDisplay(&(d->display));
  initializeMediateProjectSelection(&(d->selection));
  initializeMediateProjectAnalysis(&(d->analysis));
  initializeMediateProjectFiltering(&(d->lowpass));
  initializeMediateProjectFiltering(&(d->highpass));
  initializeMediateProjectCalibration(&(d->calibration));
  initializeMediateProjectUndersampling(&(d->undersampling));
  initializeMediateProjectInstrumental(&(d->instrumental));
  initializeMediateProjectSlit(&(d->slit));
  initializeMediateProjectOutput(&(d->output));
  initializeMediateProjectExport(&(d->export_spectra));
}
