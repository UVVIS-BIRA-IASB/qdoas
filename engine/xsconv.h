#ifndef XSCONV_H
#define XSCONV_H

#include "constants.h"
#include "doas.h"

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif


// ===============================
// CROSS SECTIONS CONVOLUTION TOOL
// ===============================

  // ----------------
  // GLOBAL VARIABLES
  // ----------------

  extern const char *XSCONV_slitTypes[SLIT_TYPE_MAX];

  // ----------
  // PROTOTYPES
  // ----------

  RC   XSCONV_LoadCalibrationFile(MATRIX_OBJECT *pLambda,char *lambdaFile,int nextraPixels);
  RC   XSCONV_LoadSlitFunction(MATRIX_OBJECT *slitXs,SLIT *pSlit,double *pGaussWidth,int *pSlitType);
  RC   XSCONV_NewSlitFunction(SLIT *pSlitOptions,MATRIX_OBJECT *pSlit,double slitParam,SLIT *pSlit2Options,MATRIX_OBJECT *pSlit2,double slitParam2);
  RC   XSCONV_LoadCrossSectionFile(MATRIX_OBJECT *pCross,char *crossFile,double lambdaMin,double lambdaMax,double shift,int conversionMode);
  RC   XSCONV_ConvertCrossSectionFile(MATRIX_OBJECT *pCross, double lambdaMin,double lambdaMax,double shift,int conversionMode);

  // Convolution functions
  RC   XSCONV_GetFwhm(double *lambda,double *slit,double *deriv2,int nl,int slitType,double *slitParam);
  RC   XSCONV_TypeNone(MATRIX_OBJECT *pXsnew,MATRIX_OBJECT *pXshr);
  RC   XSCONV_TypeGauss(const double *lambda, const double *Spec, const double *SDeriv2,double lambdaj,double dldj,double *SpecConv,double fwhm,double n,int slitType, int ndet);
  RC   XSCONV_TypeStandard(MATRIX_OBJECT *pXsnew,INDEX indexLambdaMin,INDEX indexLambdaMax,const MATRIX_OBJECT *pXshr,const MATRIX_OBJECT *pI, double *Ic,int slitType,const MATRIX_OBJECT *slitMatrix, double *slitParam,int wveDptFlag);
  RC   XSCONV_TypeI0Correction(MATRIX_OBJECT *pXsnew,MATRIX_OBJECT *pXshr,MATRIX_OBJECT *pI0,double conc,int slitType,MATRIX_OBJECT *slitMatrix,double *slitParam,int wveDptFlag);

  // Cross section to convolute
  struct _FFT {
    double *fftIn;
    double *fftOut;
    double *invFftIn;
    double *invFftOut;
    int     fftSize;
    int     oldSize;
  };

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
