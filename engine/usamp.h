#ifndef USAMP_H
#define USAMP_H

#include "matrix.h"
#include "doas.h"

typedef struct _usamp
 {
  MATRIX_OBJECT hrSolar;
  int     *lambdaRange[4];                       // for each analysis window, give the lambda range
  double **kuruczInterpolated,                   // high resolution and convoluted kurucz on analysis windows calibrations
         **kuruczInterpolated2;                  // second derivatives of previous vectors
 }
USAMP;

RC USAMP_BuildCrossSections(double *phase1,                                     // OUTPUT : phase 1 calculation
                            double *phase2,                                     // OUTPUT : phase 2 calculation
                            double *gomeLambda,                                 // GOME calibration
                            double *gomeLambda2,                                // shifted GOME calibration
                            double *kuruczInterpolated,                         // preconvoluted Kurucz spectrum interpolated on gome calibration
                            double *kuruczInterpolatedDeriv2,                   // interpolated Kurucz spectrum second derivatives
                            int     nGome,                                      // size of GOME calibration
                            double *kuruczLambda,                               // Kurucz high resolution wavelength scale
                            double *kuruczConvolved,                            // preconvoluted Kurucz spectrum on high resolution wavelength scale
                            double *kuruczConvolvedDeriv2,                      // preconvoluted Kurucz second derivatives
                            int     nKurucz,                                    // size of Kurucz vectors
                            int     analysisMethod);                            // analysis method

RC USAMP_Build(double *phase1,                                                  // OUTPUT : phase 1 calculation
               double *phase2,                                                  // OUTPUT : phase 2 calculation
               double *gomeLambda,                                              // GOME calibration
               int     nGome,                                                   // size of GOME calibration
               MATRIX_OBJECT *pKuruczMatrix,                                    // Kurucz matrix
               SLIT   *pSlit,                                                   // slit function
               double  fraction,                                                // tunes the phase
               int     analysisMethod);                                         // analysis method

#endif
