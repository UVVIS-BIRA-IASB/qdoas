#ifndef KURUCZ_H
#define KURUCZ_H

#include "doas.h"
#include "engine_context.h"
#include "fit_properties.h"

struct _KuruczFeno {
  struct fit_properties * subwindow_fits; // fit properties for calibration subwindows
  double         *Grid;
  FFT            *fft;                                  // fourier transform of high resolution kurucz spectrum
  CROSS_RESULTS **results;
  double         *chiSquare;
  double         *rms;
  double         *wve;
  int            *nIter;
  bool           have_calibration; // "true" if this struct contains results.
  double         preshift;
  int            rc;
};

struct _Kurucz {
  KURUCZ_FENO *KuruczFeno;
  MATRIX_OBJECT hrSolar;                        // high resolution kurucz spectrum for convolution
  MATRIX_OBJECT slitFunction;                   // user-defined slit function (file option)
  double *solar,                                // convolved kurucz spectrum
         *lambdaF,
         *solarF,                               // filtered solar spectrum (high pass filtering)
         *solarF2,                              // second derivatives for the previous vector
         *offset,
         *fwhmVector[MAX_KURUCZ_FWHM_PARAM],    // wavelength dependence of fwhm
         *fwhmDeriv2[MAX_KURUCZ_FWHM_PARAM],    // wavelength dependence of fwhm
         *VSig,*Pcalib,                   // polynomial coefficients computation
         *lambdaMin,*lambdaMax,
         *pixMid,*VLambda,*VShift,              // display
         **dispAbsolu,**dispSecX,               // copy of vectors to display for overlapping subwindows
         *fwhm[MAX_KURUCZ_FWHM_PARAM],          // fwhm found for each little window
         *fwhmSigma[MAX_KURUCZ_FWHM_PARAM],     // errors on fwhm
         *fwhmPolySpec[MAX_KURUCZ_FWHM_PARAM];  // polynomial coefficients for building wavelength dependence of fwhm for spectra
  int    *NIter;                                // number of iterations

  MATRIX_OBJECT crossFits;                      // cross sections fits to display

  int     Nb_Win,                               // number of little windows
          shiftDegree,                          // degree of the shift polynomial
          fwhmDegree,                           // degree of the fwhm polynomial
          solarFGap;

  INDEX   indexKurucz;                          // index of analysis window with Kurucz description

  char   displayFit;                           // display fit flag
  char   displayResidual;                      // display new calibration flag
  char   displayShift;                         // display shift in each pixel flag
  char   displaySpectra;                       // display complete spectra
  char   method;                               // analysis method (Marquadt,SVD)
};

extern KURUCZ KURUCZ_buffers[MAX_SWATHSIZE];

// -------------------
// GLOBAL DECLARATIONS
// -------------------

extern FFT *pKURUCZ_fft;
extern int KURUCZ_indexLine;

// ----------
// PROTOTYPES
// ----------

RC   KURUCZ_Spectrum(const double *oldLambda, double *newLambda, double *spectrum, const double *reference, double *instrFunction,
                   char displayFlag,const char *windowTitle,double **coeff,double **fwhmVector,double **fwhmDeriv2,int saveFlag,
                   INDEX indexFeno,void *responseHandle,INDEX indexFenoColumn);
RC   KURUCZ_ApplyCalibration(FENO *pTabFeno,double *newLambda,INDEX indexFenoColumn);
RC   KURUCZ_Reference(double *instrFunction,INDEX refFlag,int saveFlag,int gomeFlag,void *responseHandle,INDEX indexFenoColumn);
RC   KURUCZ_Alloc(const PROJECT *pProject, const double *lambda, INDEX indexKurucz, double lambdaMin, double lambdaMax,
                  INDEX indexFenoColumn, const MATRIX_OBJECT *hr_solar);
void KURUCZ_Init(int gomeFlag,INDEX indexFenoColumn);
void KURUCZ_Free(void);

#endif
