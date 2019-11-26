
#include "../engine/doas.h"
#include "engine_context.h"

#ifndef _ENGINE_XSCONV_
#define _ENGINE_XSCONV_

#if defined(_cplusplus) || defined(__cplusplus)
extern "C" {
#endif


typedef struct _enigneXsconvContext
 {                                                                              // GENERAL OPTIONS
  int    convolutionType;                                                       // type of convolution
  int    conversionMode;                                                        // conversion mode
  double shift;                                                                 // shift to apply to the original high resolution cross section
  char   crossFile[DOAS_MAX_PATH_LEN+1];                                              // high resolution cross section file
  char   path[DOAS_MAX_PATH_LEN+1];                                                   // output path
  char   calibrationFile[DOAS_MAX_PATH_LEN+1];                                        // calibration file
  int    noComment;                                                             // flag, 1 to save the convoluted cross section without comment

                                                                                // I0 CORRECTION
  char  kuruczFile[DOAS_MAX_PATH_LEN+1];                                             // Kurucz file used when I0 correction is applied
  double conc;                                                                  // concentration to use when applying I0 correction

                                                                                // SLIT FUNCTION
  SLIT   slitConv;                                                              // convolution slit function
  SLIT   slitDConv;                                                             // deconvolution slit function

                                                                                // FILTERING
  PRJCT_FILTER lfilter;                                                         // low filtering options
  PRJCT_FILTER hfilter;                                                         // high filtering options
  double *filterVector;
  int nFilter;

  MATRIX_OBJECT xsNew;                                                          // New cross section

                                                                                // CALIBRATION
  char   calibReference[DOAS_MAX_PATH_LEN+1];                                   // reference file

                                                                                // UNDERSAMPLING
  int    analysisMethod;                                                        // analysis method
  char  path2[DOAS_MAX_PATH_LEN+1];                                                  // output path for the second phase
  double fraction;                                                              // tunes the phase

                                                                                // RING
  double    temperature;                                                        // temperature
  int    normalizeFlag;                                                         // normalization of the raman spectru
  int saveRaman;
 }
ENGINE_XSCONV_CONTEXT;

ENGINE_XSCONV_CONTEXT *EngineXsconvCreateContext(void);
RC                     EngineXsconvDestroyContext(ENGINE_XSCONV_CONTEXT *pEngineContext);

// ================
// CONVOLUTION TOOL
// ================

RC                     XSCONV_Convolution(ENGINE_XSCONV_CONTEXT *pEngineContext,void *responseHandle);

// ================
// RING EFFECT TOOL
// ================

// Constants definitions

#define RING_SLIT_WIDTH (double)6.

#define N2_SIZE  48    // size of n2 data
#define O2_SIZE  185    // size of o2 data

// Global declarations

extern double n2pos[N2_SIZE];
extern double o2pos[O2_SIZE];

// Prototypes

void raman_n2(double temp,double *n2xsec);
void raman_o2(double temp,double *o2xsec);

#if defined(_cplusplus) || defined(__cplusplus)
}
#endif

#endif
