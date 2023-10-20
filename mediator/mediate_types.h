/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/


#ifndef _MEDIATE_TYPES_H_GUARD
#define _MEDIATE_TYPES_H_GUARD

enum eEngineErrorType {
  NoEngineError,
  InformationEngineError,
  WarningEngineError,
  FatalEngineError
};

enum eCurveStyleType {
  Line,
  DashLine,
  Point
};

enum ePlotScaleType {
  Spectrum,
  SpecMax,
  Residual
};

// Plot scaling control

enum _spectraAutoscale
 {
     allowFixedScale,                                                              // allow the scaling if checked in the plot properties
     forceAutoScale                                                                // do not account for the scaling
 };

// Indexes for the plot pages

#define plotPageCalib           0                                               // kurucz
#define plotPageRef             1
#define plotPageSpectrum        2                                               // spectra to browse or analyze
#define plotPageDarkCurrent     3                                               // dark currents
#define plotPageOffset          4
#define plotPageSpecMax         5                                               // specmax (variation of the signal with the scans)
#define plotPageIrrad           6                                               // irradiance spectra
#define plotPageErrors          7                                               // errors on measurements

#define plotPageCross          10                                               // cross sections
#define plotPageAnalysis       20                                               // analysis
#define plotPageImage          30                                               // image

#endif

