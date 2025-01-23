//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  ANALYSIS PROCESSING
//  Name of module    :  ANALYSIS.C
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//  The QT version of the program has been developed jointly by the Belgian
//  Institute for Space Aeronomy (BIRA-IASB) and the Science and Technology
//  company (S[&]T) - Copyright (C) 2007
//
//      BIRA-IASB                                   S[&]T
//      Belgian Institute for Space Aeronomy        Science [&] Technology
//      Avenue Circulaire, 3                        Postbus 608
//      1180     UCCLE                              2600 AP Delft
//      BELGIUM                                     THE NETHERLANDS
//      caroline.fayt@aeronomie.be                  info@stcorp.nl
//
//  ----------------------------------------------------------------------------
//  FUNCTIONS
//
//  =================
//  UTILITY FUNCTIONS
//  =================
//
//  FNPixel - get a pixel from a wavelength;
//  ShiftVector - apply shift and stretch on vector; convolve when fitting SFP during Kurucz
//  Norm - vector norm computation
//  OrthogonalizeVector - orthogonalize a column in A matrix to a set of other columns of A;
//  Orthogonalization - orthogonalization of matrix A processing;
//
//  AnalyseLoadVector - load a vector from file;
//  VECTOR_NormalizeVector - vector normalization;
//  ANALYSE_LinFit - use svd facilities for linear regressions;
//
//  ===============================
//  SVD WORKSPACE MEMORY MANAGEMENT
//  ===============================
//
//  AnalyseSvdGlobalAlloc - global allocations;
//
//  ===============
//  ANALYSIS METHOD
//  ===============
//
//  ANALYSE_SvdInit - all parameters initialization for best Shift and Stretch determination and concentrations computation;
//  Function - cross sections and spectrum alignment using spline fitting functions and new Yfit computation;
//  NumDeriv - derivatives computation;
//  DerivFunc - set derivatives for non linear parameters;
//  ANALYSE_CurFitMethod - make a least-square fit to a non linear function;
//
//  ANALYSE_AlignReference - align reference spectrum on etalon;
//  ANALYSE_Spectrum - spectrum record analysis;
//
//  ===============
//  DATA PROCESSING
//  ===============
//
//  ANALYSE_ResetData - release and reset all data used for a project analysis;
//  ANALYSE_LoadSlit - load slit function for fwhm correction;
//
//  AnalyseLoadCross - load cross sections data from cross sections type tab page;
//  AnalyseLoadContinuous - load continuous functions;
//  AnalyseLoadShiftStretch - load shift and stretch for cross sections implied in SVD decomposition;
//  AnalyseLoadPredefined - load predefined parameters;
//  AnalyseLoadGaps - load gaps defined in an analysis windows;
//  AnalyseLoadRef - load reference spectra;
//
//  ANALYSE_LoadData - load data from a project;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  ANALYSE_Alloc - all analysis buffers allocation and initialization;
//  ANALYSE_Free - release buffers used for analysis;
//
//  =============
//  UNDERSAMPLING
//  =============
//
//  ANALYSE_UsampGlobalAlloc - allocate buffers (not depending on analysis windows) for the calculation of the undersampling XS
//  ANALYSE_UsampLocalAlloc - allocate buffers (depending on analysis windows) for the calculation of the undersampling XS
//  ANALYSE_UsampLocalFree - release the buffers previously allocated by the ANALYSE_UsampLocalAlloc function
//  ANALYSE_UsampGlobalFree - release the buffers previously allocated by the ANALYSE_UsampGlobalAlloc function
//  ANALYSE_UsampBuild - build undersampling cross sections during analysis process;
//  ----------------------------------------------------------------------------

//
//  HISTORY
//
//  Version 2.03 (october 2000)
//  ---------------------------
//
//  15/01/2001 - ANALYSE_CurFitMethod : fix the concentration of a molecule to the
//                                      value found in a previous window;
//
//  17/01/2001 - Function : bug with the calculation of the offset from the
//                          initial values when the offset is not fitted;
//
//  18/01/2001 - ANALYSE_XsConvolution : division by 0 with I0 real-time convolution
//                                       using the wavelength dependent slit function
//                                       determined by Kurucz;
//

/*! \file analyse.c Main doas analysis code.*/

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

#ifndef WIN32
// for "is_same_file" implementation on unix:
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "analyse.h"

#include "engine_context.h"
#include "fit_properties.h"
#include "mediate.h"
#include "winfiles.h"
#include "winsymb.h"
#include "engine.h"
#include "kurucz.h"
#include "spline.h"
#include "xsconv.h"
#include "filter.h"
#include "usamp.h"
#include "spectral_range.h"
#include "tropomi_read.h"
#include "gome2_read.h"
#include "scia-read.h"
#include "output.h"
#include "stdfunc.h"
#include "winthrd.h"
#include "curfit.h"
#include "vector.h"
#include "zenithal.h"
#include "radiance_ref.h"
#include "omi_read.h"
#include "omiv4_read.h"
#include "apex_read.h"
#include "gdp_bin_read.h"
#include "raman.h"
#include "gems_read.h"
#include "output_netcdf.h"
#include "matrix_netcdf_read.h"
#include "visual_c_compat.h"

// ===================
// Global DECLARATIONS
// ===================

#define ANALYSE_LONGPATH 0                                                      // 0 normal mode, 1 for Anoop specific needs
#define MAX_REPEAT_CURFIT 3

PROJECT *PRJCT_itemList;

doas_spectrum *global_doas_spectrum; // Should better remove this global variable.

int    ANALYSE_plotKurucz,ANALYSE_plotRef,ANALYSE_indexLine;

int NFeno;                             // number of analysis windows

int SvdPDeb,SvdPFin,                   // analysis window limits
  LimMin,LimMax,LimN;

WRK_SYMBOL *WorkSpace;                 // list of symbols in a project
int NWorkSpace;
FENO **TabFeno,*Feno;                  // list of analysis windows in a project

int ANALYSE_swathSize=0;

double *x,*Lambda,*LambdaSpec,
  *ANALYSE_pixels,
  *ANALYSE_splineX,              // abscissa used for spectra, in the units selected by user
  *ANALYSE_absolu,               // residual spectrum
  *ANALYSE_secX,                 // residual spectrum + the contribution of a cross section for fit display
  *ANALYSE_t,                    // residual transmission in Marquardt-Levenberg not linear method
  *ANALYSE_tc,                   // residual transmission in Marquardt-Levenberg not linear method
  *ANALYSE_xsTrav,               // temporary buffer for processing on cross sections
  *ANALYSE_xsTrav2,              // cross sections second derivatives
  *ANALYSE_shift,
  *ANALYSE_zeros,
  *ANALYSE_ones;
double   ANALYSE_nFree;                // number of free degrees

MATRIX_OBJECT ANALYSIS_slitMatrix[NSFP],ANALYSIS_slitK;
double ANALYSIS_slitParam[NSFP];
USAMP  ANALYSE_usampBuffers;

// ===================
// STATIC DECLARATIONS
// ===================

int NOrtho,
  *OrthoSet,
  hFilterSpecLog,hFilterRefLog;

int NDET[MAX_SWATHSIZE];
// description of an analysis windows

PRJCT_FILTER *ANALYSE_plFilter,*ANALYSE_phFilter;
PRJCT_ANLYS  *pAnalysisOptions;        // analysis options
PRJCT_SLIT   *pSlitOptions;            // slit function options
PRJCT_KURUCZ *pKuruczOptions;          // Kurucz options
PRJCT_USAMP  *pUsamp;                  // undersampling options

double *Fitp,
  *FitDeltap,
  *FitMinp,
  *FitMaxp,
  *a,
  *b,
  *Sigma,
  *SplineSpec,
  *SplineRef,
  StretchFact1,StretchFact2,
  Square,
  ZM,TDET;

// Internal variables

int KuruczUseRef=0;   // 0 if spectrum is shifted, 1 if reference is shifted
int debugResetFlag=0;
int analyseDebugMask=0;
int analyseDebugVar=0;
INDEX analyseIndexRecord;

// =================
// UTILITY FUNCTIONS
// =================

/*! Plot a set of curves, leaving blank space for gaps.
 *
 * \param page
 * \param curve_data (num_curves x 2) array of pointers to the x and y data of the curves.
 * \param num_curves The number of different curves to plot.
 * \param type
 * \param forceAutoScaling
 * \param title
 * \param responseHandle
 * \param specrange The doas_spectrum describing the gaps in the spectrum
 */
void plot_curves( int page,
                  double *(*curve_data)[2],
                  int num_curves,
                  enum ePlotScaleType type,
                  int forceAutoScaling,
                  const char *title,
                  void *responseHandle,
                  doas_spectrum* specrange) {
  int num_segments = num_curves * spectrum_num_windows(specrange);
  struct curve_data *plot_curves = malloc(num_segments * sizeof(*plot_curves));
  doas_iterator my_iterator;
  size_t segment = 0;
  for (doas_interval *interval = iterator_start_interval(&my_iterator, specrange);
       interval != NULL;
       interval = iterator_next_interval(&my_iterator)) {
    int start = interval_start(interval);
    int end = interval_end(interval);
    for (int j = 0; j!=num_curves; ++j,++segment) {
      plot_curves[segment] = CURVE(.x = (curve_data[j][0])+start, .y = (curve_data[j][1])+start, .length = end-start+1, .number=j);
    }
  }
  mediateResponsePlotData(page, plot_curves, num_segments, type, forceAutoScaling, title, "Wavelength (nm)", "", responseHandle);
  free(plot_curves);
 }

static inline double sum_of_squares(const double *array, const doas_spectrum *ranges) {
  double sum = 0.;
  doas_iterator my_iterator;
  for( int i = iterator_start(&my_iterator, ranges); i != ITERATOR_FINISHED; i=iterator_next(&my_iterator))
   sum += array[i]*array[i];
  return sum;
}

static inline double root_mean_square(const double * array, const doas_spectrum *ranges) {
  return sqrt(sum_of_squares(array, ranges) / spectrum_length(ranges));
}

// Return the wavelength of the center pixel of the analysis window
// (SvdPDeb + SvdPFin)/2.  If this is in the middle of 2 pixels,
// return the average wavelength of those pixels.
inline double center_pixel_wavelength(int first, int last) {
  int center = (first + last)/2;
  if ( (first + last) %2) {
    return 0.5*(ANALYSE_splineX[center] + ANALYSE_splineX[1+center]);
  } else {
    return ANALYSE_splineX[center];
  }
}

/*! Remove pixels with residuals above a certain threshold.
 *
 * If the array of residuals contains values > (max_residual), exclude
 * these from the given set of spectral windows.
 *
 * \param residuals The array containing the residuals.
 *
 * \param max_residual The threshold.
 *
 * \param pspecrange The current set of valid point of the spectrum.
 *
 * \param spike_arr An array of boolean values to keep track of which
 * pixels have been removed.  Updated.
 *
 * \return True if pixels were removed.
 */
bool remove_spikes(double *residuals,
     double max_residual,
     doas_spectrum *pspecrange, // updated to exclude pixels with spikes
     bool * spike_arr) // array to store value of residiuals for pixels with spikes
 {
  bool spikes = 0;

  doas_spectrum *temprange = spectrum_copy(pspecrange);
  doas_iterator my_iterator;
  for( int pixel = iterator_start(&my_iterator,temprange); pixel != ITERATOR_FINISHED; pixel=iterator_next(&my_iterator))
   {
    if (fabs(residuals[pixel]) > max_residual)
     {
      spikes = 1;
      spectrum_remove_pixel(pspecrange, pixel);
      spike_arr[pixel] = 1;
     }
   }
  spectrum_destroy(temprange);

  return spikes;
 }

int reinit_analysis(FENO *pFeno, const int n_wavel) {
  pFeno->fit_properties.DimL = spectrum_length(pFeno->fit_properties.specrange);
  pFeno->Decomp = 1;

  memcpy(ANALYSE_absolu, ANALYSE_zeros, sizeof(double) * n_wavel);

  return ANALYSE_SvdInit(pFeno,&pFeno->fit_properties, n_wavel, Lambda);
 }

void AnalyseGetFenoLim(FENO *pFeno,INDEX *pLimMin,INDEX *pLimMax, const int n_wavel)
{
  int deb,fin,Dim;

  // Debugging

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("AnalyseGetFenoLim",DEBUG_FCTTYPE_UTIL);
#endif

  deb=Dim=0;
  fin=n_wavel-1;

  if (!pFeno->hidden)
   {

    deb = spectrum_start(pFeno->fit_properties.specrange);
    fin = spectrum_end(pFeno->fit_properties.specrange);

    Dim=0;

    if (!pFeno->hidden && (ANALYSE_plFilter->type!=PRJCT_FILTER_TYPE_NONE) && (ANALYSE_plFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
     Dim+=(int)(ANALYSE_plFilter->filterSize*sqrt(ANALYSE_plFilter->filterNTimes));
    if (((!pFeno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((pFeno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
        (ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_NONE) && (ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
     Dim+=(int)(ANALYSE_phFilter->filterSize*sqrt(ANALYSE_phFilter->filterNTimes));

    Dim=max(Dim,pAnalysisOptions->securityGap);

   }

  *pLimMin=max(deb-Dim,0);
  *pLimMax=min(fin+Dim,n_wavel-1);

  // Debugging

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("AnalyseGetFenoLim",0);
#endif
}

void ANALYSE_InitResults(void)
 {
     // Declarations

     INDEX indexFenoColumn,indexFeno;
     FENO *pTabFeno;

  for (indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize);indexFenoColumn++)
   {
    for (indexFeno=0;(indexFeno<NFeno);indexFeno++)
     {
      // Pointers initialization

      pTabFeno=&TabFeno[indexFenoColumn][indexFeno];
      pTabFeno->RMS=QDOAS_FILL_DOUBLE;

      if (!pTabFeno->hidden)
       OUTPUT_InitResults(pTabFeno);
     }
   }
 }

// ---------------------------------------
// FNPixel : Get a pixel from a wavelength
// ---------------------------------------

RC FNPixel(double *lambdaVector,double lambdaValue,int npts,int pixelSelection)
{
  // Declarations

  INDEX klo,khi,rc;

  // Initialization

  rc=0;

  if (lambdaValue<=lambdaVector[0])
   rc=0;
  else if (lambdaValue>=lambdaVector[npts-1])
   rc=npts-1;
  else
   {
    rc=(npts-1)>>1;

    for (klo=0,khi=npts-1;khi-klo>1;)
     {
      rc=(khi+klo)>>1;

      if (fabs(lambdaVector[rc]-lambdaValue)<EPSILON)
       break;

      if (lambdaVector[rc]>lambdaValue)
       khi=rc;
      else
       klo=rc;
     }

    if (fabs(lambdaVector[rc]-lambdaValue)>EPSILON)
     {
      switch(pixelSelection)
       {
        // --------------------------------------------------------------------------
       case PIXEL_BEFORE :
         if ((rc>0) && (lambdaVector[rc]>lambdaValue))
          rc--;
         break;
         // --------------------------------------------------------------------------
       case PIXEL_AFTER :
         if ((rc<npts-1) && (lambdaVector[rc]<lambdaValue))
          rc++;
         break;
         // --------------------------------------------------------------------------
       case PIXEL_CLOSEST :

         if ((rc>0) && (lambdaVector[rc]>lambdaValue) &&
             (lambdaVector[rc]-lambdaValue>lambdaValue-lambdaVector[rc-1]))
          rc--;
         else if ((rc<npts-1) && (lambdaVector[rc]<lambdaValue) &&
                  (lambdaValue-lambdaVector[rc]>lambdaVector[rc+1]-lambdaValue))
          rc++;

         break;
         // --------------------------------------------------------------------------
       default :
         break;
         // --------------------------------------------------------------------------
       }
     }
   }

  // Return

  return rc;
}

// Check if two files are equal.
//
// windows: compare paths
//
// unix: compare paths, if paths are different, check if they point to
// the same file by comparing inodes
RC is_same_file(const char *file1, const char *file2, bool *result) {
  RC rc = ERROR_ID_NO;

  #if defined WIN32

  *result=(!strcasecmp(file1,file2))?true:false;

  #else

  if(!strcmp(file1,file2)) {
   *result = true;
   return rc;
  }
  *result = false;

  int fid1 = open(file1, O_RDONLY);
  if (fid1 == -1) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_NOT_FOUND, file1);
  }

  // we have a valid fid1:
  struct stat filestat;
  if(fstat(fid1, &filestat) == -1) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_STAT, file1);
  }
  close(fid1);
  if (rc != ERROR_ID_NO) {
    return rc;
  }
  // fstat result was valid
  ino_t ino1 = filestat.st_ino;

  int fid2 = open(file2, O_RDONLY);
  if (fid2 == -1) {
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_NOT_FOUND, file2);
  }

  // we have a valid fid2
  if(fstat(fid2, &filestat) == -1) {
    rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_STAT, file2);
  }
  close(fid2);

  if (rc == ERROR_ID_NO) { // fstat result for fid2 was valid
    *result = (ino1 == filestat.st_ino);
  }

  #endif
  return rc;
}

// ======
// PUKITE
// ======

INDEX AnalyseGetPukiteIndex(CROSS_REFERENCE *pTabCross,int nTabCross,const char *xsName,int pukiteOrder)
 {
  // Declarations

  int indexPukite;
  char symbolName[MAX_STR_LEN];

  // Initialize the symbol to search for

  if (pukiteOrder==1)
   sprintf(symbolName,"Slope(%s)",xsName);
  else
   sprintf(symbolName,"Pukite(%s)",xsName);

  for (indexPukite=0;indexPukite<nTabCross;indexPukite++)
   if (!strcasecmp(symbolName,WorkSpace[pTabCross[indexPukite].Comp].symbolName))
    break;

  // Return

  return((indexPukite<nTabCross)?indexPukite:ITEM_NONE);
 }

RC AnalysePukiteConvoluteI0(const FENO *pTabFeno,double conc,double lambda0,
                               const MATRIX_OBJECT *pXs,
                               const MATRIX_OBJECT *slitMatrix, const double *slitParam, int slitType,
                               const double *newlambda,
                               double *outputPukite1,double *lambdaEff,
                               double *outputPukite2,double *sigmaEff,
                               INDEX indexlambdaMin, INDEX indexlambdaMax, const int n_wavel,
                               INDEX indexFenoColumn, int wveDptFlag)
 {
  // Declarations

  MATRIX_OBJECT xsNew0,xsNewI0,xsNew1,xsNew2,hrSolar,xshr,xshr0,xshr1,xshr2;
  double xs0;
  INDEX i,j;
  int icolumn;
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  memset(&xsNew0,0,sizeof(MATRIX_OBJECT));
  memset(&xsNew1,0,sizeof(MATRIX_OBJECT));
  memset(&xsNew2,0,sizeof(MATRIX_OBJECT));
  memset(&xsNewI0,0,sizeof(MATRIX_OBJECT));

  memset(&xshr,0,sizeof(MATRIX_OBJECT));                                        // cross section interpolated on the grid of the solar spectrum
  memset(&xshr0,0,sizeof(MATRIX_OBJECT));
  memset(&xshr1,0,sizeof(MATRIX_OBJECT));
  memset(&xshr2,0,sizeof(MATRIX_OBJECT));

  icolumn=(pXs->nc==2) ? 1 : 1+indexFenoColumn;

  rc=ERROR_ID_NO;

  if (outputPukite1!=NULL)  // if outputPukite1==NULL, nothing to do
   {
    memcpy(outputPukite1,ANALYSE_zeros,sizeof(double)*n_wavel);

    // Get high resolution Solar spectrum

    if (pKuruczOptions->fwhmFit && (!pTabFeno->hidden && (pTabFeno->useKurucz!=ANLYS_KURUCZ_NONE)))
     memcpy(&hrSolar,&KURUCZ_buffers[indexFenoColumn].hrSolar,sizeof(MATRIX_OBJECT));
    else
     memcpy(&hrSolar,&ANALYSIS_slitK,sizeof(MATRIX_OBJECT));

    if (!(rc=MATRIX_Allocate(&xshr,hrSolar.nl,2,0,0,1,"AnalysePukiteConvoluteI0 (xshr)")) &&
        !(rc=MATRIX_Allocate(&xshr0,hrSolar.nl,2,0,0,1,"AnalysePukiteConvoluteI0 (xshr0)")) &&
        !(rc=MATRIX_Allocate(&xshr1,hrSolar.nl,2,0,0,1,"AnalysePukiteConvoluteI0 (xshr1)")) &&
        !(rc=MATRIX_Allocate(&xsNew0,n_wavel,2,0,0,1,"AnalysePukiteConvoluteI0 (xsNew)")) &&
        !(rc=MATRIX_Allocate(&xsNewI0,n_wavel,2,0,0,1,"AnalysePukiteConvoluteI0 (xsNewI0)")) &&
        !(rc=MATRIX_Allocate(&xsNew1,n_wavel,2,0,0,1,"AnalysePukiteConvoluteI0 (xsNew1)")) &&

        !(rc=SPLINE_Vector(pXs->matrix[0],pXs->matrix[icolumn],pXs->deriv2[icolumn],pXs->nl,           // interpolation of XS on the grid of the high resolution solar spectrum
                           hrSolar.matrix[0],xshr.matrix[1],xshr.nl,pAnalysisOptions->interpol)) &&
        !(rc=SPLINE_Vector(pXs->matrix[0],pXs->matrix[icolumn],pXs->deriv2[icolumn],pXs->nl,           // interpolation of XS on lambda0
                           &lambda0,&xs0,1,pAnalysisOptions->interpol))
                           )
     {
      // copy of the solar wavelength calibration

      memcpy(xshr.matrix[0],hrSolar.matrix[0],xshr.nl*sizeof(double));
      memcpy(xshr0.matrix[0],hrSolar.matrix[0],xshr.nl*sizeof(double));
      memcpy(xshr1.matrix[0],hrSolar.matrix[0],xshr.nl*sizeof(double));

      // Copy of the new wavelength calibration

      memcpy(xsNew0.matrix[0],newlambda,sizeof(double)*n_wavel);
      memcpy(xsNewI0.matrix[0],newlambda,sizeof(double)*n_wavel);
      memcpy(xsNew1.matrix[0],newlambda,sizeof(double)*n_wavel);

      // Initialize output of the convolution

      memcpy(xsNew0.matrix[1],ANALYSE_zeros,sizeof(double)*n_wavel);
      memcpy(xsNewI0.matrix[1],ANALYSE_zeros,sizeof(double)*n_wavel);
      memcpy(xsNew1.matrix[1],ANALYSE_zeros,sizeof(double)*n_wavel);

      // Calculate all the synthetic cross sections to convolve

      for (i=0;(i<xshr.nl) && !rc;i++)
       {
        xshr0.matrix[1][i]=(double)hrSolar.matrix[1][i]*exp(-xshr.matrix[1][i]*conc);          // creation of a synthetic spectrum : I0 exp(-xs*conc)
        xshr1.matrix[1][i]=(double)xshr0.matrix[1][i]*hrSolar.matrix[0][i]*xshr.matrix[1][i];  // Pukite first term : I0 exp(-xs*conc) * lambda * xs
       }

      // Calculate the second derivatives

      if (!(rc=SPLINE_Deriv2(xshr.matrix[0],xshr.matrix[1],xshr.deriv2[1],xshr.nl,__func__)) &&
          !(rc=SPLINE_Deriv2(xshr0.matrix[0],xshr0.matrix[1],xshr0.deriv2[1],xshr1.nl,__func__)) &&
          !(rc=SPLINE_Deriv2(xshr1.matrix[0],xshr1.matrix[1],xshr1.deriv2[1],xshr1.nl,__func__)) &&

      // Covolve the Pukite terms separately

          !(rc=XSCONV_TypeI0Correction((MATRIX_OBJECT *)&xsNewI0,(MATRIX_OBJECT *)&xshr,(MATRIX_OBJECT *)&hrSolar,conc,slitType,(MATRIX_OBJECT *)slitMatrix,(double *)slitParam,wveDptFlag)) &&
          !(rc=XSCONV_TypeStandard(&xsNew0,indexlambdaMin,indexlambdaMax,&xshr0,&xshr0,NULL,slitType,slitMatrix,(double *)slitParam,wveDptFlag)) &&
          !(rc=XSCONV_TypeStandard(&xsNew1,indexlambdaMin,indexlambdaMax,&xshr1,&xshr1,NULL,slitType,slitMatrix,(double *)slitParam,wveDptFlag)))
       {
        for (j=indexlambdaMin;(j<indexlambdaMax) && !rc;j++)
         {
          // int j0=FNPixel((double *)newlambda,lambda0,n_wavel,PIXEL_CLOSEST);
          // ???int j0=FNPixel((double *)pXs->matrix[0],lambda0,pXs->nl,PIXEL_CLOSEST);

          if (fabs(xsNew0.matrix[1][j])<(double)1.e-260)
           {
            rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"I0-Convolution of Pukite terms");
            break;
           }

          outputPukite1[j]=xsNew1.matrix[1][j]/xsNew0.matrix[1][j]-lambda0*xsNewI0.matrix[1][j];

          // lambdaEff[j]=xsNew1.matrix[1][j]/(xsNew0.matrix[1][j]*xsNewI0.matrix[1][j]);
          // sigmaEff[j]=xsNew2.matrix[1][j]/(xsNew0.matrix[1][j]*xsNewI0.matrix[1][j]);
         }

        // Order 2

        if ((outputPukite2!=NULL) && !rc)
         {
          memcpy(outputPukite2,ANALYSE_zeros,sizeof(double)*n_wavel);

          if (!(rc=MATRIX_Allocate(&xshr2,hrSolar.nl,2,0,0,1,"AnalysePukiteConvoluteI0 (xshr2)")) &&
              !(rc=MATRIX_Allocate(&xsNew2,n_wavel,2,0,0,1,"AnalysePukiteConvoluteI0 (xsNew2)")))
           {
            memcpy(xshr2.matrix[0],hrSolar.matrix[0],xshr.nl*sizeof(double));
            memcpy(xsNew2.matrix[0],newlambda,sizeof(double)*n_wavel);
            memcpy(xsNew2.matrix[1],ANALYSE_zeros,sizeof(double)*n_wavel);

            for (i=0;(i<xshr.nl) && !rc;i++)
             xshr2.matrix[1][i]=(double)xshr0.matrix[1][i]*xshr.matrix[1][i]*xshr.matrix[1][i];     // Pukite second term : I0 exp(-xs*conc) * xs ^ 2

            if (!(rc=SPLINE_Deriv2(xshr2.matrix[0],xshr2.matrix[1],xshr2.deriv2[1],xshr2.nl,__func__)) &&
                !(rc=XSCONV_TypeStandard(&xsNew2,indexlambdaMin,indexlambdaMax,&xshr2,&xshr2,NULL,slitType,slitMatrix,(double *)slitParam,wveDptFlag)))
             {
              for (j=indexlambdaMin;(j<indexlambdaMax) && !rc;j++)
               {
                // to MVR : check if we need to use central pixel
                // int j0=FNPixel((double *)newlambda,lambda0,n_wavel,PIXEL_CLOSEST);
                // int j0=FNPixel((double *)pXs->matrix[0],lambda0,pXs->nl,PIXEL_CLOSEST);

                if (fabs(xsNew0.matrix[1][j])<(double)1.e-260)
                 {
                  rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"I0-Convolution of Pukite terms");
                  break;
                 }

                outputPukite2[j]=xsNew2.matrix[1][j]/xsNew0.matrix[1][j]-xs0*xsNewI0.matrix[1][j];

                // lambdaEff[j]=xsNew1.matrix[1][j]/(xsNew0.matrix[1][j]*xsNewI0.matrix[1][j]);
                // sigmaEff[j]=xsNew2.matrix[1][j]/(xsNew0.matrix[1][j]*xsNewI0.matrix[1][j]);
               }
             }
           }
         }
       }
     }

    // Release high resolution buffers

    MATRIX_Free(&xshr,__func__);
    MATRIX_Free(&xshr0,__func__);
    MATRIX_Free(&xshr1,__func__);
    MATRIX_Free(&xshr2,__func__);

    // Release buffers for the convolution results

    MATRIX_Free(&xsNew0,__func__);
    MATRIX_Free(&xsNewI0,__func__);
    MATRIX_Free(&xsNew1,__func__);
    MATRIX_Free(&xsNew2,__func__);

    #if defined(__DEBUG_) && __DEBUG_
    DEBUG_FunctionStop(__func__,rc);
    #endif
   }

  // Return

  return rc;
 }

RC AnalyseSimplePukiteTerms(double *lambda,double *xs,
                               double *pukite1Vector,double *pukite1Deriv2,
                               double *pukite2Vector,double *pukite2Deriv2,
                               int n,double lambda0)
 {
  // Declarations

  int i0;
  RC rc;

  // Initializations

  i0=FNPixel(lambda,lambda0,n,PIXEL_CLOSEST);
  rc=ERROR_ID_NO;

  if (pukite1Vector!=NULL)
   {
    for (int i=0;i<n;i++)
     {
      pukite1Vector[i]=xs[i]*(lambda[i]-lambda0);
      if (pukite2Vector!=NULL)
       pukite2Vector[i]=(xs[i]-xs[i0])*xs[i];
     }

    if (!(rc=SPLINE_Deriv2(lambda,pukite1Vector,pukite1Deriv2,n,__func__)) && (pukite2Vector!=NULL))
     rc=SPLINE_Deriv2(lambda,pukite2Vector,pukite2Deriv2,n,__func__);
   }

  // Return

  return rc;
 }

RC AnalyseAddPukiteTerm(ENGINE_CONTEXT *pEngineContext, CROSS_REFERENCE *pCross,INDEX indexFenoColumn,int pukiteOrder,INDEX *pIndexPukite)
 {
  // Declarations

  WRK_SYMBOL *pWrkSymbol,*pWrkCross;                                            // pointer to a general description of a symbol
  char symbolName[SYMBOL_NAME_BUFFER_LENGTH];
  CROSS_REFERENCE *pPukiteCross;                                                // pointer of the current cross section in the engine list
  FENO *pTabFeno;                                                    // pointer to the current analysis window
  INDEX indexSymbol,i,indexSvd;
  const int n_wavel = NDET[indexFenoColumn];
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  pWrkCross=&WorkSpace[pCross->Comp];
  pTabFeno=&TabFeno[indexFenoColumn][NFeno];
  rc=ERROR_ID_NO;
  *pIndexPukite=ITEM_NONE;
  if (pukiteOrder==1)
   sprintf(symbolName,"Slope(%s)",pWrkCross->symbolName);
  else
   sprintf(symbolName,"Pukite(%s)",pWrkCross->symbolName);

  // Search for symbols in list

  for (indexSymbol=0;indexSymbol<NWorkSpace;indexSymbol++) {
    pWrkSymbol=&WorkSpace[indexSymbol];

    bool same_file = false;

    if ((pWrkSymbol->type==WRK_SYMBOL_CROSS) &&
        !strcasecmp(pWrkSymbol->symbolName,symbolName) &&
        ( (rc=is_same_file(pWrkSymbol->crossFileName,pWrkCross->crossFileName, &same_file) != ERROR_ID_NO)
          || same_file ) // stop loop (with error) if file comparison returns error, or (without error) if files are same
        )

      break;
  }
  // Add a new cross section
  if (rc==ERROR_ID_NO && (indexSymbol==NWorkSpace) && (NWorkSpace<MAX_SYMB)) {

    // Allocate a new symbol

    pWrkSymbol=&WorkSpace[indexSymbol];

    pWrkSymbol->type=WRK_SYMBOL_CROSS;
    strcpy(pWrkSymbol->symbolName,symbolName);
    strcpy(pWrkSymbol->crossFileName,pWrkCross->crossFileName);
    strcpy(pWrkSymbol->amfFileName,"");

    // Just copy original cross section (this cross section will be modified later)
    if (!(rc=MATRIX_Copy(&pWrkSymbol->xs,&pWrkCross->xs,__func__)))
     NWorkSpace++;

      // Recalculate second derivatives ???
  }

  if ((rc==ERROR_ID_NO) && (indexSymbol<NWorkSpace) && (pTabFeno->NTabCross<MAX_FIT)) {

     pPukiteCross=&pTabFeno->TabCross[pTabFeno->NTabCross];

    // Allocate vectors for cross section and its second derivative for analysis processing

    if (((pPukiteCross->vector=(double *)MEMORY_AllocDVector(__func__,"vector",0,n_wavel-1))==NULL) ||
        ((pPukiteCross->Deriv2=(double *)MEMORY_AllocDVector(__func__,"Deriv2",0,n_wavel-1))==NULL))

     rc=ERROR_ID_ALLOC;

    else
     {
      memcpy(pPukiteCross->vector, ANALYSE_zeros, sizeof(double) * n_wavel);
      memcpy(pPukiteCross->Deriv2, ANALYSE_zeros, sizeof(double) * n_wavel);            
      
      pPukiteCross->crossAction=pCross->crossAction;
      pPukiteCross->crossCorrection=ANLYS_CORRECTION_TYPE_NONE;
      pPukiteCross->amfType=ANLYS_AMF_TYPE_NONE;
      pPukiteCross->filterFlag=pCross->filterFlag;
      pPukiteCross->isPukite=1;

      if (rc==ERROR_ID_NO)
       {
        pPukiteCross->Comp=indexSymbol;
        pPukiteCross->IndSvdA=++pTabFeno->fit_properties.DimC;
        pTabFeno->xsToConvolute+=((pPukiteCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE) ||
                                  (pPukiteCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0) ||
                                  (pPukiteCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_RING))?1:0;

        pTabFeno->xsToConvoluteI0+=(pPukiteCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?1:0;
        pPukiteCross->IndSubtract=ITEM_NONE;
        pPukiteCross->display=pCross->display;                    // fit display
        pPukiteCross->InitConc=pCross->InitConc;                    // initial concentration
        pPukiteCross->FitConc=pCross->FitConc;                  // modify concentration
        pPukiteCross->FitFromPrevious=pCross->FitFromPrevious;

        pPukiteCross->DeltaConc=(pPukiteCross->FitConc)?pCross->DeltaConc:(double)0.;   // delta on concentration
        pPukiteCross->I0Conc=(pPukiteCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?pCross->I0Conc:(double)0.;
        pPukiteCross->MinConc=pCross->MinConc;
        pPukiteCross->MaxConc=pCross->MaxConc;

        // Swap columns of original matrix A in order to have in the end of the matrix, cross sections with fixed concentrations

        if (pPukiteCross->FitConc!=0)   // the difference between SVD and Marquardt+SVD hasn't to be done yet but later
         {
          for (i=pTabFeno->NTabCross-1;i>=0;i--)
           if (((indexSvd=pTabFeno->TabCross[i].IndSvdA)!=0) && !pTabFeno->TabCross[i].FitConc)
            {
             pTabFeno->TabCross[i].IndSvdA=pPukiteCross->IndSvdA;
             pPukiteCross->IndSvdA=indexSvd;
            }

          if (pTabFeno->analysisMethod!=OPTICAL_DENSITY_FIT)     // In the intensity fitting method, FitConc is an index
           pPukiteCross->FitConc=pTabFeno->fit_properties.NF++;                   // in the non linear parameters vectors
          pTabFeno->fit_properties.nFit++;
         }
        else if (pTabFeno->analysisMethod!=OPTICAL_DENSITY_FIT)
         pPukiteCross->FitConc=ITEM_NONE;                              // so if the parameter hasn't to be fitted, index is ITEM_NONE

       *pIndexPukite=pTabFeno->NTabCross;
        pTabFeno->NTabCross++;
       }
     }
   }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
  #endif

 // Return

 return rc;
}

// ==============
// MOLECULAR RING
// ==============

RC Analyse_Molecular_Ring_Init(FENO *pFeno,double *lambda,int n_wavel)
 {
  // Declarations

  CROSS_REFERENCE *pTabCross;
  INDEX indexTabCross;
  
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;  

  for (indexTabCross=0;indexTabCross<pFeno->NTabCross;indexTabCross++)
   {
    pTabCross=&pFeno->TabCross[indexTabCross];

    if (((pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING) || (pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE)) &&
         (pTabCross->vectorBackup!=NULL) &&
         (pTabCross->Deriv2Backup!=NULL) &&
         (pTabCross->molecularCrossIndex!=ITEM_NONE))
     {
      memcpy(pTabCross->vectorBackup,pTabCross->vector,sizeof(double)*n_wavel);
      memcpy(pTabCross->Deriv2Backup,pTabCross->Deriv2,sizeof(double)*n_wavel);
      
      if (!(rc=SPLINE_Deriv2(lambda,pTabCross->vector,pTabCross->Deriv2,n_wavel,__func__)) &&
           (pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE) && (pTabCross->indexPukite1!=ITEM_NONE))

       rc=AnalyseSimplePukiteTerms(lambda,pTabCross->vector,
                                   pFeno->TabCross[pTabCross->indexPukite1].vector,pFeno->TabCross[pTabCross->indexPukite1].Deriv2,
                                   NULL,NULL,n_wavel,pFeno->lambda0);      
     }
   }
   
   // Return
   
   return rc;
 }

RC Analyse_Molecular_Ring_Calculate(FENO *pFeno,double *lambda,int n_wavel,double ar)
 {
  // Declarations

  CROSS_REFERENCE *pTabCross,*pTabCrossMolec;
  INDEX indexTabCross;
  double ns,ns0,lambda0;
  // MVR : to see later double *lambdaEff,*sigmaEff;
  int i,j0;
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  lambda0=pFeno->lambda0;
  j0=FNPixel((double *)lambda,lambda0,n_wavel,PIXEL_CLOSEST);

  for (indexTabCross=0;(indexTabCross<pFeno->NTabCross) && !rc;indexTabCross++)
   {
    pTabCross=&pFeno->TabCross[indexTabCross];

    if (((pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING) || (pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE)) &&
         (pTabCross->vectorBackup!=NULL) &&
         (pTabCross->Deriv2Backup!=NULL) &&
         (pTabCross->molecularCrossIndex!=ITEM_NONE))
     {
      ns0=pFeno->TabCrossResults[pTabCross->molecularCrossIndex].SlntCol;
      pTabCrossMolec=&pFeno->TabCross[pTabCross->molecularCrossIndex];

      for (i=0;i<n_wavel;i++)
       {
        ns=ns0;
        if ((pTabCrossMolec->crossCorrection==ANLYS_CORRECTION_TYPE_SLOPE) ||
            (pTabCrossMolec->crossCorrection==ANLYS_CORRECTION_TYPE_PUKITE))
         {
          ns+=pFeno->TabCrossResults[pTabCrossMolec->indexPukite1].SlntCol*(lambda[i]-lambda0);                     // lambda[i]
          if (pTabCrossMolec->crossCorrection==ANLYS_CORRECTION_TYPE_PUKITE)
           ns+=pFeno->TabCrossResults[pTabCrossMolec->indexPukite2].SlntCol*(pTabCrossMolec->vector[i]-pTabCrossMolec->vector[j0]);   // pTabCrossMolec->vector[i]
         }

        pTabCross->vector[i]=pTabCross->vectorBackup[i]/exp(-ar*ns*pTabCrossMolec->molecularCrossSection[i])-1.;
       }

      if (!(rc=SPLINE_Deriv2(lambda,pTabCross->vector,pTabCross->Deriv2,n_wavel,__func__)) &&
           (pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE) && (pTabCross->indexPukite1!=ITEM_NONE))

       rc=AnalyseSimplePukiteTerms(lambda,pTabCross->vector,
                                   pFeno->TabCross[pTabCross->indexPukite1].vector,pFeno->TabCross[pTabCross->indexPukite1].Deriv2,
                                   NULL,NULL,n_wavel,lambda0);
     }
   }

  // Return

  return rc;
 }

void Analyse_Molecular_Ring_End(FENO *pFeno,int n_wavel)
 {
  for (int indexTabCross=0;indexTabCross<pFeno->NTabCross;indexTabCross++)
   {
    CROSS_REFERENCE *pTabCross=&pFeno->TabCross[indexTabCross];

    if (((pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING) || (pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE))&&
         (pTabCross->vectorBackup!=NULL) &&
         (pTabCross->Deriv2Backup!=NULL) &&
         (pTabCross->molecularCrossIndex!=ITEM_NONE))
     {
      memcpy(pTabCross->vector,pTabCross->vectorBackup,sizeof(double)*n_wavel);
      memcpy(pTabCross->Deriv2,pTabCross->Deriv2Backup,sizeof(double)*n_wavel);
     }
   }
 }

// ---------------------------------------------------------------------------------------
// OrthogonalizeVector : Orthogonalize a column in A matrix to a set of other columns of A
// ---------------------------------------------------------------------------------------
//
// OrthoSet[0..NOrthoSet-1]: column indices in A of the orthogonal base
// NormSet: norms of the orthogonal base
// NOrthoSet: number of orthogonal base vectors.
// indexColumn: column index in A of the vector we want to orghogonalize
// A[..][1..DimL]: vectors
// DimL: vector dimension
void OrthogonalizeVector(const int *OrthoSet, const double *NormSet,int NOrthoSet,INDEX indexColumn,double **A,int DimL) {
  for (int j=0;j<NOrthoSet;j++) {

    if (NormSet[j]!=0.) {
      const int indexSvd=Feno->TabCross[OrthoSet[j]].IndSvdA;

      double dot = 0.;
      for (int i=1;i<=DimL;i++)
        dot+=A[indexSvd][i]*A[indexColumn][i];

      dot /=NormSet[j];

      for (int i=1;i<=DimL;i++)
        A[indexColumn][i]-=dot*A[indexSvd][i];
    }
  }
}

static void OrthogonalizeToCross(int indexCross,double *NormSet,int currentNOrtho, double **A, int DimL) {
  // Declarations

  CROSS_REFERENCE *pTabCross;
  INDEX indexTabCross;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_UTIL);
#endif

  for (indexTabCross=0;indexTabCross<Feno->NTabCross;indexTabCross++) {
    pTabCross=&Feno->TabCross[indexTabCross];

    if (pTabCross->IndOrthog==indexCross) {
      OrthoSet[currentNOrtho]=indexCross;
      NormSet[currentNOrtho]=VECTOR_Norm(A[Feno->TabCross[indexCross].IndSvdA],DimL);

      if ((ANALYSE_phFilter->filterFunction==NULL) ||
          (!Feno->hidden && !ANALYSE_phFilter->hpFilterAnalysis) ||
          ((Feno->hidden==1) && !ANALYSE_phFilter->hpFilterCalib))
        OrthogonalizeVector(OrthoSet,NormSet,currentNOrtho+1,pTabCross->IndSvdA,A,DimL);
      else
        OrthogonalizeVector(&OrthoSet[NOrtho],&NormSet[NOrtho],currentNOrtho-NOrtho+1,pTabCross->IndSvdA,A,DimL);

      OrthogonalizeToCross(indexTabCross,NormSet,currentNOrtho+1,A,DimL);
    }
  }

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,0);
#endif
}

// --------------------------------------
// Differences between two cross sections
// --------------------------------------

static void XsDifferences(double **A,int DimL)
 {
  // Declarations

  CROSS_REFERENCE *pTabCross;

  // Subtraction

  for (int j=0;j<Feno->NTabCross;j++)   // Index in the WorkSpace symbols list -> use IndSvdA to have indexes in the matrix
   {
    pTabCross=&Feno->TabCross[j];

    if (pTabCross->IndSvdA && (pTabCross->IndSubtract!=ITEM_NONE))
     for (int i=1;i<=DimL;i++)
      A[pTabCross->IndSvdA][i]=A[Feno->TabCross[pTabCross->IndSubtract].IndSvdA][i]-A[pTabCross->IndSvdA][i];
   }
 }

// ------------------------------------------------------------
// Orthogonalization : Orthogonalization of matrix A processing
// ------------------------------------------------------------

static void Orthogonalization(double **A, int DimL) {
  if (NOrtho) { // if no orthogonal base, cross sections can not be orthogonalized to another cross section

    double *NormSet = malloc(Feno->NTabCross * sizeof(*NormSet));

    NormSet[0]=VECTOR_Norm(A[Feno->TabCross[OrthoSet[0]].IndSvdA],DimL);
    for (int i=1;i<NOrtho;++i) {
      // orthogonalize vector i w.r.t. vectors 0..i-1:
      OrthogonalizeVector(OrthoSet,NormSet,i,Feno->TabCross[OrthoSet[i]].IndSvdA,A,DimL);
      // calculate norm of orthogonalized vector i:
      NormSet[i]=VECTOR_Norm(A[Feno->TabCross[OrthoSet[i]].IndSvdA],DimL);
    }

    // Orthogonalization to base only

    if ((ANALYSE_phFilter->filterFunction==NULL) ||
        (!Feno->hidden && !ANALYSE_phFilter->hpFilterAnalysis) ||
        ((Feno->hidden==1) && !ANALYSE_phFilter->hpFilterCalib))

      for (int j=0;j<Feno->NTabCross;j++) {
        CROSS_REFERENCE *pTabCross=&Feno->TabCross[j];

        if (pTabCross->IndSvdA && (pTabCross->IndOrthog==ORTHOGONAL_BASE))
          OrthogonalizeVector(OrthoSet,NormSet,NOrtho,pTabCross->IndSvdA,A,DimL);
      }

    // Orthogonalization to base plus another vector

    for (int j=0;j<Feno->NTabCross;j++) {
      CROSS_REFERENCE *pTabCross=&Feno->TabCross[j];

      if (pTabCross->IndSvdA && (pTabCross->IndOrthog==ORTHOGONAL_BASE))
       OrthogonalizeToCross(j,NormSet,NOrtho,A,DimL);
    }
    free(NormSet);
  }
}

// =======================================
// Real time interpolation and convolution
// =======================================

// Correction of a cross section by the effective temperature
// Test 24/01/2002

RC TemperatureCorrection(double *xs,double *A,double *B,double *C,double *newXs,double T, const int n_wavel)
{
#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_UTIL);
#endif

  memcpy(newXs,ANALYSE_zeros,sizeof(double)*n_wavel);

  for (int j=LimMin;j<=LimMax;j++)
    newXs[j]=xs[j]+(T-241)*A[j]+(T-241)*(T-241)*B[j]+(T-241)*(T-241)*(T-241)*C[j];

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,0);
#endif

  return 0;
}

// -----------------------------------------------
// ShiftVector : Apply shift and stretch on vector; convolve reference when fitting SFP in Kurucz
// -----------------------------------------------

RC ShiftVector(const double *lambda, double *source, const double *deriv, double *target, const int n_wavel,
               double DSH,double DST,double DST2,                           // first shift and stretch
               double DSH_,double DST_,double DST2_,                        // second shift and stretch
               const double *Param,int fwhmDir,int kuruczFlag,int slitFlag,INDEX indexFenoColumn)
{
#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  double lambda0 = (!Feno->hidden)?Feno->lambda0: center_pixel_wavelength(SvdPDeb, SvdPFin);
  memcpy(ANALYSE_shift,ANALYSE_zeros,sizeof(double)*n_wavel);
  int fwhmFlag=((Feno->analysisType==ANALYSIS_TYPE_FWHM_NLFIT) && (fwhmDir!=0) && (Param!=NULL))?1:0;
  CROSS_REFERENCE *TabCross=Feno->TabCross;

  RC rc=ERROR_ID_NO;

  // Buffer allocation for second derivative

  for (int j=LimMin;j<=LimMax;j++) {             // !! p'=p-(DSH+DST*(p-p0)+DST2*(p-p0)^2
    // Second shift and stretch              //    p''=p'-(DSH'+DST'*(p'-p0')+DST2'*(p'-p0')^2
    // with   p=ANALYSE_splineX (Lambda if unit is nm;pixels if unit is pixels)
    double x0=(ANALYSE_splineX[j]-lambda0);        //        p0'=p0-DSH
    double y=ANALYSE_splineX[j]-(DSH_+DST_*x0+DST2_*x0*x0);

    // First shift and stretch

    x0=(y-lambda0+DSH_);
    ANALYSE_shift[j]=y-(DSH+DST*x0*StretchFact1+DST2*x0*x0*StretchFact2);

    // Fit difference of resolution between spectrum and reference

    if (fwhmFlag) {
      double fwhm= 0.;

      const double deltaX=(ANALYSE_splineX[j]-lambda0);

      if (Feno->indexFwhmConst!=ITEM_NONE)
       fwhm+=(TabCross[Feno->indexFwhmConst].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmConst].FitParam]:(double)TabCross[Feno->indexFwhmConst].InitParam;
      if (Feno->indexFwhmOrder1!=ITEM_NONE)
       fwhm+=((TabCross[Feno->indexFwhmOrder1].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmOrder1].FitParam]:(double)TabCross[Feno->indexFwhmOrder1].InitParam)*deltaX*TabCross[Feno->indexFwhmOrder1].Fact;
      if (Feno->indexFwhmOrder2!=ITEM_NONE)
       fwhm+=((TabCross[Feno->indexFwhmOrder2].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmOrder2].FitParam]:(double)TabCross[Feno->indexFwhmOrder2].InitParam)*deltaX*deltaX*TabCross[Feno->indexFwhmOrder2].Fact;

      // Apply shift and stretch

      if ( fwhm!=(double)0. &&
           ( (fwhmDir>0 && fwhm>(double)0.) || (fwhmDir<0 && fwhm<(double)0.) ) ) {
        rc = XSCONV_TypeGauss(lambda,source,deriv,ANALYSE_shift[j],(ANALYSE_splineX[j+1]-ANALYSE_splineX[j]),
                              &target[j],fabs(fwhm),(double)0.,SLIT_TYPE_GAUSS, n_wavel);
      } else {
        rc = SPLINE_Vector(lambda,source,deriv,n_wavel,&ANALYSE_shift[j],&target[j],1,pAnalysisOptions->interpol);
      }
      if (rc != ERROR_ID_NO)
        break;
     }
  }

  if (kuruczFlag) {
    // Declarations

    double slitParam,slitParam2,slitParam3,fwhmStretch1,fwhmStretch2;
    double slitParamVector[NSFP];

    // Initializations

    slitParam=(Feno->indexFwhmParam[0]!=ITEM_NONE)?((TabCross[Feno->indexFwhmParam[0]].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmParam[0]].FitParam]:(double)TabCross[Feno->indexFwhmParam[0]].InitParam):(double)0.;

    if (pKuruczOptions->fwhmType==SLIT_TYPE_INVPOLY)
     slitParam2=(double)pKuruczOptions->invPolyDegree;
    else if (((pKuruczOptions->fwhmType==SLIT_TYPE_ERF) || (pKuruczOptions->fwhmType==SLIT_TYPE_AGAUSS) || (pKuruczOptions->fwhmType==SLIT_TYPE_SUPERGAUSS) || (pKuruczOptions->fwhmType==SLIT_TYPE_VOIGT)) && (Feno->indexFwhmParam[1]!=ITEM_NONE))
     slitParam2=(TabCross[Feno->indexFwhmParam[1]].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmParam[1]].FitParam]:(double)TabCross[Feno->indexFwhmParam[1]].InitParam;
    else
     slitParam2=(double)0.;

    if (pKuruczOptions->fwhmType==SLIT_TYPE_SUPERGAUSS)
     slitParam3=(TabCross[Feno->indexFwhmParam[2]].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmParam[2]].FitParam]:(double)TabCross[Feno->indexFwhmParam[2]].InitParam;
    else
     slitParam3=(double)0.;

    // Interpolation
    if ( pKuruczOptions->fwhmType!=SLIT_TYPE_FILE &&
         ( slitParam==(double)0. ||
           ( ( pKuruczOptions->fwhmType==SLIT_TYPE_ERF || pKuruczOptions->fwhmType==SLIT_TYPE_INVPOLY) &&
             slitParam2==(double)0. )))
     {

      rc=SPLINE_Vector(KURUCZ_buffers[indexFenoColumn].hrSolar.matrix[0],KURUCZ_buffers[indexFenoColumn].hrSolar.matrix[1],
                       KURUCZ_buffers[indexFenoColumn].hrSolar.deriv2[1],KURUCZ_buffers[indexFenoColumn].hrSolar.nl,
                       ANALYSE_shift,source,n_wavel,pAnalysisOptions->interpol);

     }
    else
     {    // Convolution
      if ((pKuruczOptions->fwhmType==SLIT_TYPE_GAUSS) || (pKuruczOptions->fwhmType==SLIT_TYPE_ERF))
       {
        double F,G,w,a,sigma,delta,step;
        INDEX i;
        int ndemi;

        ndemi=pKURUCZ_fft->fftSize>>1;
        step=(pKURUCZ_fft->fftIn[pKURUCZ_fft->oldSize]-pKURUCZ_fft->fftIn[1])/(pKURUCZ_fft->oldSize-1.);

        sigma=slitParam*0.5;
        a=sigma/sqrt(log(2.));
        delta=slitParam2*0.5;

        w=(double)DOAS_PI/step;
        F=exp(-a*a*w*w*0.25);
        G=(pKuruczOptions->fwhmType==SLIT_TYPE_GAUSS)?(double)1.:sin(w*delta)/(w*delta);

        pKURUCZ_fft->invFftIn[1]=pKURUCZ_fft->fftOut[1];
        pKURUCZ_fft->invFftIn[2]=pKURUCZ_fft->fftOut[2]*F*G;

        for (i=2;i<=ndemi;i++)
         {
           w=(double)DOAS_PI*(i-1.)/(ndemi*step);

           F=(double)exp(-a*a*w*w*0.25);
           G=(double)(pKuruczOptions->fwhmType==SLIT_TYPE_GAUSS)?(double)1.:(double)sin(w*delta)/(w*delta);

           pKURUCZ_fft->invFftIn[(i<<1) /* i*2 */-1]=pKURUCZ_fft->fftOut[(i<<1) /* i*2 */-1]*F*G;      // Real part
           pKURUCZ_fft->invFftIn[(i<<1) /* i*2 */]=pKURUCZ_fft->fftOut[(i<<1) /* i*2 */]*F*G;          // Imaginary part
         }

        realft(pKURUCZ_fft->invFftIn,pKURUCZ_fft->invFftOut,pKURUCZ_fft->fftSize,-1);

        for (i=1;i<=pKURUCZ_fft->fftSize;i++)
          pKURUCZ_fft->invFftOut[i]/=step;

        SPLINE_Deriv2(pKURUCZ_fft->fftIn+1,pKURUCZ_fft->invFftOut+1,pKURUCZ_fft->invFftIn+1,pKURUCZ_fft->oldSize,__func__);

        memcpy(&source[LimMin],&ANALYSE_shift[LimMin],sizeof(double)*LimN);

        SPLINE_Vector(pKURUCZ_fft->fftIn+1,pKURUCZ_fft->invFftOut+1,pKURUCZ_fft->invFftIn+1,pKURUCZ_fft->oldSize,
                      &ANALYSE_shift[LimMin],&target[LimMin],LimN,pAnalysisOptions->interpol);
      } else {
           MATRIX_OBJECT xsNew;
           MATRIX_OBJECT slitMatrix[NSFP];
        SLIT slitOptions;
        int slitType;
        int shiftIndex;

        memset(slitMatrix,0,sizeof(MATRIX_OBJECT)*NSFP);

        memset(&xsNew,0,sizeof(MATRIX_OBJECT));

        for (int i=0;i<NSFP;i++)
         slitParamVector[i]=(double)0.;

        if (MATRIX_Allocate(&xsNew,n_wavel,2,0,0,0,__func__))
         {
          rc=ERROR_ID_ALLOC;
         }
        else
         {
          memcpy(xsNew.matrix[0],ANALYSE_shift,sizeof(double)*n_wavel);
          memcpy(xsNew.matrix[1],target,sizeof(double)*n_wavel);

          slitType=pKuruczOptions->fwhmType;

          if (slitType==SLIT_TYPE_FILE)
           {
            int nc=KURUCZ_buffers[indexFenoColumn].slitFunction.nc;
            int nl=KURUCZ_buffers[indexFenoColumn].slitFunction.nl;

            shiftIndex=(nc==2)?0:1;

            fwhmStretch1=(Feno->indexFwhmParam[0]!=ITEM_NONE)?
              ((TabCross[Feno->indexFwhmParam[0]].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmParam[0]].FitParam]:(double)TabCross[Feno->indexFwhmParam[0]].InitParam)
              :(double)1.;

            fwhmStretch2=(Feno->indexFwhmParam[1]!=ITEM_NONE)?
              ((TabCross[Feno->indexFwhmParam[1]].FitParam!=ITEM_NONE)?(double)Param[TabCross[Feno->indexFwhmParam[1]].FitParam]:(double)TabCross[Feno->indexFwhmParam[1]].InitParam)
              :(double)1.;

           if (MATRIX_Allocate(&slitMatrix[0],nl,nc,0,0,1,"ShiftVector (slitMatrix)"))
            {
              rc=ERROR_ID_ALLOC;
             }
            else
             {
              // make a backup of the matrix

              for (int i=0;i<nc;i++)
                memcpy(slitMatrix[0].matrix[i],KURUCZ_buffers[indexFenoColumn].slitFunction.matrix[i],sizeof(double)*nl);

              // determine slit center wavelength, defined as wavelength
              // corresponding to the maximum value
              //
              // TODO: for low-sampled slit functions, lambda of maximum
              // might not be a good measure of the center => perform an
              // interpolation here and in KuruczConvolveSolarSpectrum.
              double lambda_center = 0.;
              double slit_max = 0.;
              for (int i=shiftIndex; i<slitMatrix[0].nl; ++i) {
                if (slitMatrix[0].matrix[1][i] > slit_max) {
                  slit_max = slitMatrix[0].matrix[1][i];
                  lambda_center = slitMatrix[0].matrix[0][i];
                }
              }

                 // Apply the stretch on the slit wavelength calibration
              for (int i=shiftIndex;i<slitMatrix[0].nl;i++) {
                // stretch wavelength grid around the center wavelength,
                // using fwhmStretch1 on the left, and fwhmStretch2 on the
                // right:
                double delta_lambda = slitMatrix[0].matrix[0][i] - lambda_center;
                delta_lambda *= (delta_lambda < 0.) ? fwhmStretch1 : fwhmStretch2;
                slitMatrix[0].matrix[0][i]= lambda_center + delta_lambda;
              }

                 // Recalculate second derivatives and the FWHM
                 for (int i=1;i<slitMatrix[0].nc;i++)
                rc=SPLINE_Deriv2(slitMatrix[0].matrix[0]+shiftIndex,slitMatrix[0].matrix[i]+shiftIndex,slitMatrix[0].deriv2[i]+shiftIndex,slitMatrix[0].nl-shiftIndex,__func__);
             }
           }
          else if ((slitType==SLIT_TYPE_VOIGT) || (slitType==SLIT_TYPE_AGAUSS) || (pKuruczOptions->fwhmType==SLIT_TYPE_SUPERGAUSS) || (slitType==SLIT_TYPE_INVPOLY))
           {
            memset(&slitOptions,0,sizeof(SLIT));

            slitOptions.slitType=slitType;
            slitOptions.slitFile[0]=0;

            slitOptions.slitParam=slitParam;
            slitOptions.slitParam2=slitParam2;
            slitOptions.slitParam3=slitParam3;

            memcpy(&source[LimMin],&ANALYSE_shift[LimMin],sizeof(double)*LimN);
            rc=XSCONV_LoadSlitFunction(slitMatrix,&slitOptions,NULL,&slitType);
           }

          if (!rc &&
               !(rc=XSCONV_TypeStandard(&xsNew,LimMin,LimMax+1,&KURUCZ_buffers[indexFenoColumn].hrSolar,
                                        &KURUCZ_buffers[indexFenoColumn].hrSolar,NULL,slitType,slitMatrix,slitParamVector,0)))

            memcpy(target,xsNew.matrix[1],sizeof(double)*n_wavel);
         }

        for (int i=0;i<NSFP;i++)
          MATRIX_Free(&slitMatrix[i],__func__);

        MATRIX_Free(&xsNew,__func__);
       }

    }

    if (hFilterRefLog && !(rc=SPLINE_Vector(KURUCZ_buffers[indexFenoColumn].lambdaF,KURUCZ_buffers[indexFenoColumn].solarF,KURUCZ_buffers[indexFenoColumn].solarF2,n_wavel+2*KURUCZ_buffers[indexFenoColumn].solarFGap,ANALYSE_shift+LimMin,source+LimMin,LimN,pAnalysisOptions->interpol))) {
      int i;
      for (i=LimMin;(i<=LimMax) && (source[i]>(double)0.) && (target[i]>(double)0.);i++)
        target[i]=log(target[i]/source[i]);

      if (i<=LimMax)
        rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_LOG,analyseIndexRecord);
    }
  }
  else if (!fwhmFlag && slitFlag && (Feno->analysisType==ANALYSIS_TYPE_FWHM_SLIT) && (KURUCZ_buffers[indexFenoColumn].hrSolarGridded.matrix!=NULL) &&  
           !(rc=SPLINE_Vector(KURUCZ_buffers[indexFenoColumn].hrSolarGridded.matrix[0],
                              KURUCZ_buffers[indexFenoColumn].hrSolarGridded.matrix[1],
                              KURUCZ_buffers[indexFenoColumn].hrSolarGridded.deriv2[1],
                              KURUCZ_buffers[indexFenoColumn].hrSolarGridded.nl,&ANALYSE_shift[LimMin],&target[LimMin],LimN,pAnalysisOptions->interpol))) 
   {
//     if (hFilterRefLog && !(rc=SPLINE_Vector(KURUCZ_buffers[indexFenoColumn].lambdaF,KURUCZ_buffers[indexFenoColumn].solarF,KURUCZ_buffers[indexFenoColumn].solarF2,n_wavel+2*KURUCZ_buffers[indexFenoColumn].solarFGap,ANALYSE_shift+LimMin,source+LimMin,LimN,pAnalysisOptions->interpol))) {
//       int i;
//       for (i=LimMin;(i<=LimMax) && (source[i]>(double)0.) && (target[i]>(double)0.);i++)
//         target[i]=log(target[i]/source[i]);
//       
//       if (i<=LimMax)
//        rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_LOG,analyseIndexRecord);
//     }
      
   }
  else if (!fwhmFlag && !slitFlag) 
   rc=SPLINE_Vector(lambda,source,deriv,n_wavel,&ANALYSE_shift[LimMin],&target[LimMin],LimN,pAnalysisOptions->interpol); 
    
  // Return

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
}

// -----------------------------------------------------------------------------------
// ANALYSE_XsInterpolation : Interpolation of all cross sections in an analysis window
// ----------------------------------------------------------------------------------

RC ANALYSE_XsInterpolation(FENO *pTabFeno, const double *newLambda,INDEX indexFenoColumn)
{
  // Declarations

  CROSS_REFERENCE *pTabCross;
  double *filtCross,*filtDeriv2;
  INDEX indexTabCross,icolumn;
  MATRIX_OBJECT *pXs;
  int oldNl;
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_XsInterpolation",DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  filtCross=filtDeriv2=NULL;
  rc=ERROR_ID_NO;
  oldNl=0;

  if (newLambda[0] == 0.0) { // newLambda should now contain a wavelength grid from a calibration file or reference spectrum
    return ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_MISSING_INITIAL_CALIB);
  }

  // Browse cross sections

  for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
   {
    pTabCross=&pTabFeno->TabCross[indexTabCross];

    if ( (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS &&   // take only cross sections into account
          (pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING ||
           pTabCross->crossAction==ANLYS_CROSS_ACTION_INTERPOLATE) ) ||
         (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_PREDEFINED &&   // take only cross sections into account
          pTabCross->crossAction==ANLYS_CROSS_ACTION_INTERPOLATE) ) {
      pXs=&WorkSpace[pTabCross->Comp].xs;
      icolumn= (pXs->nc==2) ? 1 : 1+indexFenoColumn;

      // Buffer allocation

      if (oldNl!=pTabFeno->NDET)
       {
        if (filtCross!=NULL)
         MEMORY_ReleaseDVector("ANALYSE_XsInterpolation ","filtCross",filtCross,0);
        if (filtDeriv2!=NULL)
         MEMORY_ReleaseDVector("ANALYSE_XsInterpolation ","filtDeriv2",filtDeriv2,0);

        if (((filtCross=MEMORY_AllocDVector("ANALYSE_XsInterpolation ","filtCross",0,pTabFeno->NDET-1))==NULL) ||
            ((filtDeriv2=MEMORY_AllocDVector("ANALYSE_XsInterpolation ","filtDeriv2",0,pTabFeno->NDET-1))==NULL))
         {
          rc=ERROR_ID_ALLOC;
          break;
         }

        oldNl=pTabFeno->NDET;
       }

      if (((pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_SLOPE) ||
           (pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_PUKITE)) &&
           (pTabCross->indexPukite1!=ITEM_NONE) &&
           (pTabFeno->TabCross[pTabCross->indexPukite1].isPukite==1) &&
         ((rc=AnalyseSimplePukiteTerms(pXs->matrix[0],pXs->matrix[icolumn],
                                      (pTabCross->indexPukite1!=ITEM_NONE)?WorkSpace[pTabFeno->TabCross[pTabCross->indexPukite1].Comp].xs.matrix[icolumn]:NULL,(pTabCross->indexPukite1!=ITEM_NONE)?WorkSpace[pTabFeno->TabCross[pTabCross->indexPukite1].Comp].xs.deriv2[1]:NULL,
                                      (pTabCross->indexPukite2!=ITEM_NONE)?WorkSpace[pTabFeno->TabCross[pTabCross->indexPukite2].Comp].xs.matrix[icolumn]:NULL,(pTabCross->indexPukite2!=ITEM_NONE)?WorkSpace[pTabFeno->TabCross[pTabCross->indexPukite2].Comp].xs.deriv2[1]:NULL,
                                       pXs->nl,pTabFeno->lambda0))!=ERROR_ID_NO))
       break;

      // Interpolate cross section

      if ((pTabCross->crossAction==ANLYS_CROSS_ACTION_NOTHING) || ((pXs->nl==pTabFeno->NDET) && VECTOR_Equal(pXs->matrix[0],newLambda,pTabFeno->NDET,(double)1.e-7)))          // wavelength scale is the same as new one
       memcpy(filtCross,pXs->matrix[icolumn],sizeof(double)*pTabFeno->NDET);
      else
       if ((rc=SPLINE_Vector(pXs->matrix[0],pXs->matrix[icolumn],pXs->deriv2[icolumn],pXs->nl,newLambda,filtCross,pTabFeno->NDET,pAnalysisOptions->interpol))!=0)           // interpolation processing
        break;

      //
      // FILTERING :
      //
      // Filter after interpolation (because cross sections can be high resoluted and real-time convoluted
      // The filtering before interpolation can be better to avoid interpolation error (because filtered
      // cross sections are smoother than non filtered ones) but for that it's important that cross sections are
      // pre-interpolated on the same grid as the reference one (but it's not the generality in WinDOAS).
      //
      // So ->>>>> FIRST INTERPOLATE, THEN FILTER
      //

      // Low-pass filtering of the original cross section

      if (!pTabFeno->hidden && pTabCross->filterFlag && (ANALYSE_plFilter->filterFunction!=NULL) &&
          ((rc=FILTER_Vector(ANALYSE_plFilter,filtCross,filtCross,NULL,pTabFeno->NDET,PRJCT_FILTER_OUTPUT_LOW))!=0))

       break;

      // High-pass filtering of the original cross section

      if ((pTabCross->IndOrthog!=ITEM_NONE) && (ANALYSE_phFilter->filterFunction!=NULL) &&
          ((!pTabFeno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((pTabFeno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
          ((rc=FILTER_Vector(ANALYSE_phFilter,filtCross,filtCross,NULL,pTabFeno->NDET,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=0))

       break;

      // Reuse original cross section

      memcpy(pTabCross->vector,filtCross,sizeof(double)*pTabFeno->NDET);

      // Second derivatives computation

      if ((rc=SPLINE_Deriv2(newLambda,pTabCross->vector,pTabCross->Deriv2,pTabFeno->NDET,"ANALYSE_XsInterpolation "))!=0)
       break;
     }
   }

  // Molecular ring (commented because, done in convolution)

// !!!  if (!rc && pTabFeno->molecularCorrection && !pTabFeno->xsToConvolute)
// !!!   {
// !!!    for (indexTabCross=0;(indexTabCross<pTabFeno->NTabCross) && !rc;indexTabCross++)
// !!!     {
// !!!      pTabCross=&pTabFeno->TabCross[indexTabCross];
// !!!
// !!!      if ((WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) &&   // take only cross sections into account
// !!!          (pTabCross->molecularCrossSection!=NULL) && !pTabCross->isPukite)
// !!!       {
// !!!        memcpy(pTabCross->molecularCrossSection,ANALYSE_zeros,sizeof(double)*pTabFeno->NDET);
// !!!
// !!!        if (!(rc=raman_convolution(newLambda,
// !!!                                   pTabCross->vector,
// !!!                                   pTabCross->Deriv2,
// !!!                                   pTabCross->molecularCrossSection,pTabFeno->NDET,(double)250.,1)))
// !!!
// !!!         for (int i=0;i<pTabFeno->NDET;i++)
// !!!          pTabCross->molecularCrossSection[i]=pTabCross->vector[i]-pTabCross->molecularCrossSection[i];
// !!!       }
// !!!     }
// !!!   }

  // Return

  if (filtCross!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_XsInterpolation ","filtCross",filtCross,0);
  if (filtDeriv2!=NULL)
   MEMORY_ReleaseDVector("ANALYSE_XsInterpolation ","filtDeriv2",filtDeriv2,0);

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_XsInterpolation",rc);
#endif

 return rc;
}

RC ANALYSE_ConvoluteXs(const FENO *pTabFeno,int action,double conc,
                       const MATRIX_OBJECT *pXs,
                       const MATRIX_OBJECT *slitMatrix, const double *slitParam, int slitType,
                       const double *newlambda, double *output, INDEX indexlambdaMin, INDEX indexlambdaMax, const int n_wavel,
                       INDEX indexFenoColumn, int wveDptFlag)
{
  // Declarations

  MATRIX_OBJECT xsNew,xsI0,hrSolar,xshr;
  double *IcVector;
  int icolumn;
  INDEX i,j;
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  memset(&xsNew,0,sizeof(MATRIX_OBJECT));
  memset(&xsI0,0,sizeof(MATRIX_OBJECT));
  memset(&xshr,0,sizeof(MATRIX_OBJECT));

  IcVector=NULL;
  icolumn= (pXs->nc==2) ? 1 : 1+indexFenoColumn;

  rc=ERROR_ID_NO;

  memcpy(output,ANALYSE_zeros,sizeof(double)*n_wavel);

  if (action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)
   {
    if ((IcVector=MEMORY_AllocDVector(__func__,"IcVector",0,n_wavel-1))==NULL)
     rc=ERROR_ID_ALLOC;
    else
     {
      // Get high resolution Solar spectrum

      if (pKuruczOptions->fwhmFit && (!pTabFeno->hidden && (pTabFeno->useKurucz!=ANLYS_KURUCZ_NONE)))
       memcpy(&hrSolar,&KURUCZ_buffers[indexFenoColumn].hrSolar,sizeof(MATRIX_OBJECT));
      else
       memcpy(&hrSolar,&ANALYSIS_slitK,sizeof(MATRIX_OBJECT));

      if (!(rc=MATRIX_Allocate(&xsI0,hrSolar.nl,2,0,0,1,"ANALYSE_ConvoluteXs (xsI0)")) &&
          !(rc=MATRIX_Allocate(&xshr,hrSolar.nl,2,0,0,1,"ANALYSE_ConvoluteXs (xshr)")) &&
          !(rc=SPLINE_Vector(pXs->matrix[0],pXs->matrix[icolumn],pXs->deriv2[icolumn],pXs->nl,           // interpolation of XS on the grid of the high resolution solar spectrum
                             hrSolar.matrix[0],xshr.matrix[1],xshr.nl,pAnalysisOptions->interpol)))
       {
        memcpy(xsI0.matrix[0],hrSolar.matrix[0],xshr.nl*sizeof(double));               // copy of the solar wavelength calibration
        memcpy(xshr.matrix[0],hrSolar.matrix[0],xshr.nl*sizeof(double));               // copy of the solar wavelength calibration

        for (i=0;(i<xshr.nl) && !rc;i++)
         {
          xsI0.matrix[1][i]=(double)hrSolar.matrix[1][i]*exp(-xshr.matrix[1][i]*conc); // creation of a synthetic spectrum : I0 exp(-xs*conc)
          xshr.matrix[1][i]=hrSolar.matrix[1][i];                                      // solar spectrum
         }

        if (!(rc=SPLINE_Deriv2(xshr.matrix[0],xshr.matrix[1],xshr.deriv2[1],xshr.nl,__func__)))
          rc=SPLINE_Deriv2(xsI0.matrix[0],xsI0.matrix[1],xsI0.deriv2[1],xsI0.nl,__func__);
       }
     }
   }
  else
   memcpy(&xshr,pXs,sizeof(MATRIX_OBJECT));

  if (!rc && !(rc=MATRIX_Allocate(&xsNew,n_wavel,2,0,0,1,__func__)))
   {
        memcpy(xsNew.matrix[0],newlambda,sizeof(double)*n_wavel);
    memcpy(xsNew.matrix[1],ANALYSE_zeros,sizeof(double)*n_wavel);

    if (action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)
     memcpy(IcVector,ANALYSE_zeros,sizeof(double)*n_wavel);

    if (!(rc=XSCONV_TypeStandard(&xsNew,indexlambdaMin,indexlambdaMax,&xshr,(action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?&xsI0:&xshr,IcVector,slitType,slitMatrix,(double *)slitParam,wveDptFlag)))
     {
      for (j=indexlambdaMin;(j<indexlambdaMax) && !rc;j++)
       output[j]=xsNew.matrix[1][j];
     }

    // For I0 convolution, the synthetic spectrum (I0 exp(-xs*conc)) and the solar spectrum are convolved separately
    // Then we calculate the logarithm of the ratio
    // Even if the XS is negative, the I0 convolution should work.

    if (action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)
     for (j=indexlambdaMin;(j<indexlambdaMax) && !rc;j++)
      {
       if ((IcVector[j]==(double)0.) || (conc==(double)0.))
        rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"I0-Convolution with undefined concentration");
       else if ((double)output[j]/IcVector[j]<=(double)0.)
        rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_LOG,-1);
       else
        output[j]=(double)log(output[j]/IcVector[j])/conc;
      }
   }

  // Return

  if (action==ANLYS_CROSS_ACTION_CONVOLUTE_I0)
   {
    MATRIX_Free(&xsI0,__func__);
    MATRIX_Free(&xshr,__func__);
   }

  MATRIX_Free(&xsNew,__func__);
  if (IcVector!=NULL)
   MEMORY_ReleaseDVector(__func__,"IcVector",IcVector,0);

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
}

// -------------------------------------------------------------------------------
// ANALYSE_XsConvolution : Real time convolution of high resolution cross sections
// -------------------------------------------------------------------------------

RC ANALYSE_XsConvolution(FENO *pTabFeno,double *newlambda,
                         MATRIX_OBJECT *slitMatrix,double *slitParam,int slitType,
                         INDEX indexFenoColumn,int wveDptFlag)
{
  // Declarations

  MATRIX_OBJECT matrix,*pXs;
  CROSS_REFERENCE *pTabCross;
  INDEX indexTabCross,j,indexlambdaMin,indexlambdaMax;
  RC rc;

  // Initializations

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  const int n_wavel = NDET[indexFenoColumn];
  memset(&matrix,0,sizeof(MATRIX_OBJECT));
  rc=ERROR_ID_NO;

  AnalyseGetFenoLim(pTabFeno,&indexlambdaMin,&indexlambdaMax, n_wavel);
  indexlambdaMax++;

  // indexlambdaMin=0;
  // indexlambdaMax=n_wavel;

  if (pTabFeno->xsToConvolute)
   {
    for (indexTabCross=0;(indexTabCross<pTabFeno->NTabCross) && !rc;indexTabCross++)
     {
      pTabCross=&pTabFeno->TabCross[indexTabCross];
      pXs=&WorkSpace[pTabCross->Comp].xs;

      if ((WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) &&   // take only cross sections into account
          !pTabCross->isPukite &&
         ((pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE) ||
          (pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0) ||
          (pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_RING)))
       {
//         for (int i=0;i<n_wavel;i++)
//          pTabCross->vector[i]=pTabCross->Deriv2[i]=(double)0.;
        memcpy(pTabCross->vector,ANALYSE_zeros,sizeof(double)*n_wavel);
        memcpy(pTabCross->Deriv2,ANALYSE_zeros,sizeof(double)*n_wavel);

        // Convolution standard or I0

        if ((pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE) || (pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0))
         {
          // Do not convolve pukite cross sections

          if (!pTabCross->isPukite &&
              !(rc=ANALYSE_ConvoluteXs(pTabFeno,pTabCross->crossAction,pTabCross->I0Conc,pXs,slitMatrix,slitParam,slitType,
                                       newlambda,pTabCross->vector,indexlambdaMin,indexlambdaMax,n_wavel,indexFenoColumn,wveDptFlag)) &&
              ((pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_SLOPE) ||
               (pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_PUKITE)) &&
               (pTabCross->indexPukite1!=ITEM_NONE) && (pTabFeno->TabCross[pTabCross->indexPukite1].isPukite==1))
           {
            // Pukite

            if (pTabCross->crossAction!=ANLYS_CROSS_ACTION_CONVOLUTE_I0)
             rc=AnalyseSimplePukiteTerms(newlambda,pTabCross->vector,
                                        (pTabCross->indexPukite1!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite1].vector:NULL,(pTabCross->indexPukite1!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite1].Deriv2:NULL,
                                        (pTabCross->indexPukite2!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite2].vector:NULL,(pTabCross->indexPukite2!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite2].Deriv2:NULL,
                                         n_wavel,pTabFeno->lambda0);
           else
            rc=AnalysePukiteConvoluteI0(pTabFeno,pTabCross->I0Conc,pTabFeno->lambda0,pXs,slitMatrix,slitParam,slitType,
                                         newlambda,
                                        (pTabCross->indexPukite1!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite1].vector:NULL,(pTabCross->indexPukite1!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite1].molecularCrossSection:NULL,
                                        (pTabCross->indexPukite2!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite2].vector:NULL,(pTabCross->indexPukite2!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite2].molecularCrossSection:NULL,
                                         indexlambdaMin,indexlambdaMax,n_wavel,indexFenoColumn,wveDptFlag);
           }

         }

        // Convolution Ring

        else if ((pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_RING) &&
                 !(rc=MATRIX_Allocate(&matrix,pXs->nl,2,pXs->basel,pXs->basec,1,__func__)))
         {

          // Temporary buffers allocation
          double *raman=MEMORY_AllocDVector(__func__,"raman",0,n_wavel-1);
          double *solar=MEMORY_AllocDVector(__func__,"solar",0,n_wavel-1);

          if (raman == NULL || solar == NULL)
           rc=ERROR_ID_ALLOC;
          else
           {
            // Raman spectrum

            memcpy(matrix.matrix[0],pXs->matrix[0],sizeof(double)*pXs->nl);   // lambda
            memcpy(matrix.matrix[1],pXs->matrix[2],sizeof(double)*pXs->nl);   // Raman spectrum
            memcpy(matrix.deriv2[1],pXs->deriv2[2],sizeof(double)*pXs->nl);     // Second derivative of the Ramanspectrum

            if ((rc=ANALYSE_ConvoluteXs(pTabFeno,ANLYS_CROSS_ACTION_CONVOLUTE,(double)0.,&matrix,slitMatrix,slitParam,slitType,
                                        newlambda,raman,indexlambdaMin,indexlambdaMax,n_wavel,indexFenoColumn,wveDptFlag))!=ERROR_ID_NO)
             break;

            // Solar spectrum

            memcpy(matrix.matrix[1],pXs->matrix[3],sizeof(double)*pXs->nl);   // Raman spectrum
            memcpy(matrix.deriv2[1],pXs->deriv2[3],sizeof(double)*pXs->nl);     // Second derivative of the Ramanspectrum

            if ((rc=ANALYSE_ConvoluteXs(pTabFeno,ANLYS_CROSS_ACTION_CONVOLUTE,(double)0.,&matrix,slitMatrix,slitParam,slitType,
                                        newlambda,solar,indexlambdaMin,indexlambdaMax,n_wavel,indexFenoColumn,wveDptFlag))!=ERROR_ID_NO)
             break;

            // Calculate Raman/Solar

            memcpy(pTabCross->vector,ANALYSE_zeros,sizeof(double)*n_wavel);

            for (j=indexlambdaMin;j<indexlambdaMax;j++)
             pTabCross->vector[j]=(solar[j]!=(double)0.)?(double)raman[j]/solar[j]:(double)0.;   // log added on 2011 October 7 - test for GOME2 (ISA) : not concluding

            // Release allocated buffers
            MEMORY_ReleaseDVector(__func__,"raman",raman,0);
            MEMORY_ReleaseDVector(__func__,"solar",solar,0);
            
            if ((pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_SLOPE) &&
                (pTabCross->indexPukite1!=ITEM_NONE) && (pTabFeno->TabCross[pTabCross->indexPukite1].isPukite==1))

             rc=AnalyseSimplePukiteTerms(newlambda,pTabCross->vector,
                                        (pTabCross->indexPukite1!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite1].vector:NULL,(pTabCross->indexPukite1!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite1].Deriv2:NULL,
                                        (pTabCross->indexPukite2!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite2].vector:NULL,(pTabCross->indexPukite2!=ITEM_NONE)?pTabFeno->TabCross[pTabCross->indexPukite2].Deriv2:NULL,
                                         n_wavel,pTabFeno->lambda0);            
           }
         }
       }

      if ((WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) &&   // take only cross sections into account
         ((pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE) ||
          (pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0) ||
          (pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_RING)))
       {
        // Low-pass filtering

        if  (rc ||
             (!pTabFeno->hidden && pTabCross->filterFlag && (ANALYSE_plFilter->filterFunction!=NULL) &&
              ((rc=FILTER_Vector(ANALYSE_plFilter,pTabCross->vector,pTabCross->vector,NULL,n_wavel,PRJCT_FILTER_OUTPUT_LOW))!=ERROR_ID_NO)) ||

             // High-pass filtering

             ((pTabCross->IndOrthog!=ITEM_NONE) && (ANALYSE_phFilter->filterFunction!=NULL) &&
              ((!pTabFeno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((pTabFeno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
              ((rc=FILTER_Vector(ANALYSE_phFilter,pTabCross->vector,pTabCross->vector,NULL,n_wavel,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=ERROR_ID_NO)) ||

             // Interpolation

             ((rc=SPLINE_Deriv2(newlambda,pTabCross->vector,pTabCross->Deriv2,n_wavel,__func__))!=ERROR_ID_NO))

         break;
       }
     }
   }

  // Molecular ring

  if (!rc && pTabFeno->molecularCorrection)
   {
    for (indexTabCross=0;(indexTabCross<pTabFeno->NTabCross) && !rc;indexTabCross++)
     {
      pTabCross=&pTabFeno->TabCross[indexTabCross];

      if ((WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) &&   // take only cross sections into account
   //      ((pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE) ||
   //       (pTabCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0)) &&
          !pTabCross->isPukite &&
          (pTabCross->molecularCrossSection!=NULL))
       {
        memcpy(pTabCross->molecularCrossSection,ANALYSE_zeros,sizeof(double)*n_wavel);

        if (!(rc=raman_convolution(newlambda,
                                   pTabCross->vector,
                                   pTabCross->Deriv2,
                                   pTabCross->molecularCrossSection,pTabFeno->NDET,(double)250.,1)))

         for (int i=0;i<pTabFeno->NDET;i++)
          pTabCross->molecularCrossSection[i]=pTabCross->vector[i]-pTabCross->molecularCrossSection[i];
        
// Making the convolution on the whole vector gives similar results for interpolation only
//
//        if (!(rc=raman_convolution(&newlambda[indexlambdaMin],
//                                   &pTabCross->vector[indexlambdaMin],
//                                   &pTabCross->Deriv2[indexlambdaMin],
//                                   &pTabCross->molecularCrossSection[indexlambdaMin],indexlambdaMax-indexlambdaMin,(double)250.,1)))
//
//         for (int i=indexlambdaMin;i<indexlambdaMax;i++)
//          pTabCross->molecularCrossSection[i]=pTabCross->vector[i]-pTabCross->molecularCrossSection[i];
       }
     }
   }
  MATRIX_Free(&matrix,__func__);

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  // Return

  return rc;
 }

// --------------------------------------------
// AnalyseLoadVector : Load a (2-column) vector from a file
// --------------------------------------------

RC AnalyseLoadVector(const char *function, const char *fileName, double *lambda, double *vector, const int n_wavel, const int n_col) {
  // Declarations

  FILE *fp;
  char string[MAX_ITEM_TEXT_LEN],fullFileName[MAX_ITEM_TEXT_LEN],*str;
  int day,month,year,hour,min,sec;
  struct time refTime;
  INDEX i;
  RC rc;
  MATRIX_OBJECT fileMatrix;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_FILE|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  memset(&fileMatrix,0,sizeof(MATRIX_OBJECT));
  memset(string,0,MAX_ITEM_TEXT_LEN);
  rc=ERROR_ID_NO;

  if (strlen(FILES_RebuildFileName(fullFileName,fileName,1)) && (vector!=NULL)) {
    if ((fp=fopen(fullFileName,"rt"))==NULL)
      rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,fullFileName);
    else {
         int n_scan=0;
      MATRIX_PassCommentLines(fp);
      n_scan=MATRIX_GetColumnsNumbers(fp,NULL);
      rewind(fp);
      if ((n_scan == 2) && (!n_col)){
        for (i=0;  i<n_wavel && fgets(string,MAX_ITEM_TEXT_LEN,fp) && !rc; ) {
          if (strchr(string,';')==NULL && strchr(string,'*')==NULL && strchr(string,'#')==NULL) {
            int n_scan = sscanf(string,"%lf %lf",&lambda[i],&vector[i]);
            if (n_scan != 2) {
              rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_BAD_LENGTH, fullFileName);
            }
            i++;
          }
          else {
            if ((str=strstr(string,"Zm"))!=NULL)
              sscanf(str,"Zm : %lf",&TabFeno[0][NFeno].Zm);
            else if ((str=strstr(string,"SZA"))!=NULL)
              sscanf(str,"SZA : %lf",&TabFeno[0][NFeno].Zm);
            else if ((str=strstr(string,"TDet"))!=NULL)
              sscanf(str,"TDet : %lf",&TabFeno[0][NFeno].TDet);
            else if (((str=strchr(string,'/'))!=NULL) && (*(str+3)=='/') && (*(str+11)==':') && (*(str+14)==':')) {
              sscanf(str-2,"%02d/%02d/%d %02d:%02d:%02d",&day,&month,&year,&hour,&min,&sec);

              refTime.ti_hour=(unsigned char)hour;
              refTime.ti_min=(unsigned char)min;
              refTime.ti_sec=(unsigned char)sec;

              TabFeno[0][NFeno].refDate.da_day=(char)day;
              TabFeno[0][NFeno].refDate.da_mon=(char)month;
              TabFeno[0][NFeno].refDate.da_year= year;

              TabFeno[0][NFeno].Tm=(double)ZEN_NbSec(&TabFeno[0][NFeno].refDate,&refTime,0);
              TabFeno[0][NFeno].TimeDec=(double)hour+min/60.;
            }
          }
        }
      } else if (n_scan > n_col+1) { // 2) {
          rc = MATRIX_Load(fullFileName,&fileMatrix,0,0,0.,0.,0,0,__func__);
          for (int i = 0; i < n_wavel && fgets(string,MAX_ITEM_TEXT_LEN,fp) && !rc; ++i){
              lambda[i]=fileMatrix.matrix[0][i*(fileMatrix.nl-1)/(n_wavel-1)];
              vector[i]=fileMatrix.matrix[n_col+1][i*(fileMatrix.nl-1)/(n_wavel-1)];
          }
      } else {
          rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_FILE_BAD_LENGTH, fullFileName);
      }
      fclose(fp);
    }
  }

#if defined(__DEBUG_) && __DEBUG_
    DEBUG_FunctionStop(__func__,rc);
#endif

   MATRIX_Free(&fileMatrix,__func__);

    // Return

  return rc;
}

// ===========================
// FWHM ADJUSTMENT AND FITTING
// ===========================

// ----------------------------------------------------------------------------------------------------------
// AnalyseFwhmCorrectionK : resolution adjustment between spectrum and reference using fwhms fitted by Kurucz
// ----------------------------------------------------------------------------------------------------------

RC AnalyseFwhmCorrectionK(const double *Spectre, const double *Sref,double *SpecTrav,double *RefTrav, const int n_wavel, INDEX indexFenoColumn)
{
  // Declarations

  MATRIX_OBJECT slitMatrix[NSFP];
  double specFwhm,refFwhm,*xsTrav;
  INDEX j;
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL);
#endif

  // Initializations

  memset(slitMatrix,0,sizeof(MATRIX_OBJECT)*NSFP);
  xsTrav=NULL;

  // Function specified for fitting fwhm in Kurucz should be supported

  if ((pKuruczOptions->fwhmType!=SLIT_TYPE_GAUSS) && (pKuruczOptions->fwhmType!=SLIT_TYPE_ERF))
   rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_SLIT);

  // Buffer allocation

  else if ((xsTrav=(double *)MEMORY_AllocDVector(__func__,"xsTrav",0,n_wavel-1))==NULL)
   rc=ERROR_ID_ALLOC;

  // Second derivatives computation for spectrum and reference

  else if (!(rc=SPLINE_Deriv2(LambdaSpec,Spectre,SplineSpec,n_wavel,"AnalyseFwhmCorrectionK (Lambda) ")) &&  // !!! Lambda -> LambdaSpec
           !(rc=SPLINE_Deriv2(Lambda,Sref,SplineRef,n_wavel,"AnalyseFwhmCorrectionK (Lambda) ")))
   {
    memcpy(xsTrav,ANALYSE_zeros,sizeof(double)*n_wavel);

    // Fwhm ajustment between spectrum and reference

    for (j=LimMin;(j<=LimMax) && !rc;j++)
     {
      // Retrieve fwhm from fwhm vectors build by Kurucz procedure

      specFwhm=KURUCZ_buffers[indexFenoColumn].fwhmVector[0][j];
      refFwhm=Feno->fwhmVector[0][j];

      if ((specFwhm<=(double)0.) || (refFwhm<=(double)0.))
       rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_SQRT_ARG);

      // Case 1 : reference has highest resolution => degrade reference

      else if (specFwhm>refFwhm)
       {
        xsTrav[j]=specFwhm;
        specFwhm=sqrt(specFwhm*specFwhm-refFwhm*refFwhm);
        rc=XSCONV_TypeGauss(Lambda,Sref,SplineRef,Lambda[j],(Lambda[j+1]-Lambda[j]),&RefTrav[j],specFwhm,
                            (Feno->fwhmVector[1]!=NULL)?Feno->fwhmVector[1][j]:(double)0.,pKuruczOptions->fwhmType, n_wavel);
       }

      // Case 2 : spectrum has highest resolution => degrade spectrum

      else if (specFwhm<refFwhm)
       {
        xsTrav[j]=refFwhm;
        specFwhm=sqrt(refFwhm*refFwhm-specFwhm*specFwhm);
        rc=XSCONV_TypeGauss(LambdaSpec,Spectre,SplineSpec,Lambda[j],(Lambda[j+1]-Lambda[j]),&SpecTrav[j],specFwhm,
                            (Feno->fwhmVector[1]!=NULL)?Feno->fwhmVector[1][j]:(double)0.,pKuruczOptions->fwhmType, n_wavel);
       }

      // Case 3 : spectrum and reference have the same resolution

      else
       xsTrav[j]=specFwhm;
     }
   }

  // Real time convolution for high resolution cross sections

  // SuperGauss : to check later if (!rc && Feno->xsToConvolute)
  // SuperGauss : to check later  {
  // SuperGauss : to check later   rc=ANALYSE_XsConvolution(Feno,Lambda,NULL,NULL,pKuruczOptions->fwhmType,xsTrav,KURUCZ_buffers[0].fwhmVector[1],indexFenoColumn,(pKuruczOptions->fwhmType==SLIT_TYPE_FILE)?1:0);
  // SuperGauss : to check later  }

  // Return

  if (xsTrav!=NULL)
    MEMORY_ReleaseDVector(__func__,"xsTrav",xsTrav,0);

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
}

// ===============================
// SVD WORKSPACE MEMORY MANAGEMENT
// ===============================

// ------------------------------------------
// AnalyseSvdGlobalAlloc : Global allocations
// ------------------------------------------

RC AnalyseSvdGlobalAlloc(void)
{
#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__, DEBUG_FCTTYPE_MEM);
#endif

  // Initialization

  RC rc=ERROR_ID_NO;

  // take maximum detector size to make sure that buffers allocated
  // here are big enough
  int max_ndet = 0;
  for (int i=0; i<ANALYSE_swathSize; ++i) {
    if (NDET[i] > max_ndet)
      max_ndet = NDET[i];
  }

  // Allocation

  if (((Fitp=(double *)MEMORY_AllocDVector(__func__,"Fitp",0,MAX_FIT*4))==NULL)  ||
      ((FitDeltap=(double *)MEMORY_AllocDVector(__func__,"FitDeltap",0,MAX_FIT*4))==NULL) ||
      ((FitMinp=(double *)MEMORY_AllocDVector(__func__,"FitMinp",0,MAX_FIT*4))==NULL) ||
      ((FitMaxp=(double *)MEMORY_AllocDVector(__func__,"FitMaxp",0,MAX_FIT*4))==NULL) ||
      ((a=(double *)MEMORY_AllocDVector(__func__,"a",1,max_ndet))==NULL) ||
      ((b=(double *)MEMORY_AllocDVector(__func__,"b",1,max_ndet))==NULL) ||
      ((x=(double *)MEMORY_AllocDVector(__func__,"x",0,MAX_FIT))==NULL) ||
      ((Sigma=(double *)MEMORY_AllocDVector(__func__,"Sigma",0,MAX_FIT))==NULL) ||
      ((ANALYSE_shift=(double *)MEMORY_AllocDVector(__func__,"ANALYSE_shift",0,max_ndet-1))==NULL) ||
      ((ANALYSE_pixels=(double *)MEMORY_AllocDVector(__func__,"ANALYSE_pixels",0,max_ndet-1))==NULL) ||
      ((ANALYSE_splineX=(double *)MEMORY_AllocDVector(__func__,"ANALYSE_splineX",0,max_ndet-1))==NULL) ||
      ((ANALYSE_absolu=(double *)MEMORY_AllocDVector(__func__,"ANALYSE_absolu",0,max_ndet))==NULL) ||
      ((ANALYSE_t=(double *)MEMORY_AllocDVector(__func__,"ANALYSE_t",0,max_ndet))==NULL) ||
      ((ANALYSE_tc=(double *)MEMORY_AllocDVector(__func__,"ANALYSE_tc",0,max_ndet))==NULL) ||
      ((ANALYSE_xsTrav=(double *)MEMORY_AllocDVector(__func__,"ANALYSE_xsTrav",0,max_ndet-1))==NULL) ||
      ((ANALYSE_xsTrav2=(double *)MEMORY_AllocDVector(__func__,"ANALYSE_xsTrav2",0,max_ndet-1))==NULL) ||
      ((ANALYSE_secX=(double *)MEMORY_AllocDVector(__func__,"ANALYSE_secX",0,max_ndet))==NULL) ||
      ((SplineSpec=(double *)MEMORY_AllocDVector(__func__,"SplineSpec",0,max_ndet-1))==NULL) ||
      ((SplineRef=(double *)MEMORY_AllocDVector(__func__,"SplineRef",0,max_ndet-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else {
    // Initializations
    Sigma[0]=x[0]=(double)0.;

    for (int i=0;i<max_ndet;i++)
      ANALYSE_pixels[i]= i+1;
  }

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
}

// ===============
// ANALYSIS METHOD
// ===============

// --------------------------------------------------------------------------
// ANALYSE_SvdInit : All parameters initialization for best Shift and Stretch
//                   determination and concentrations computation
// --------------------------------------------------------------------------

RC ANALYSE_SvdInit(FENO* pFeno, struct fit_properties *fit, const int n_wavel, const double *lambda)
{
  // Declarations

  CROSS_REFERENCE *pTabCross;
  double deltaX,norm,norm1,norm2,swap,temp;
  INDEX i,j;
  double lambda0;
  RC rc=ERROR_ID_NO;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL);
#endif

  // Initializations

  memcpy(ANALYSE_splineX,lambda,sizeof(*lambda)*n_wavel);

  OrthoSet=pFeno->OrthoSet;
  NOrtho=pFeno->NOrtho;

  temp=(double)0.;

  if (!pFeno->hidden && (ANALYSE_plFilter->type!=PRJCT_FILTER_TYPE_NONE))
    temp+=ANALYSE_plFilter->filterEffWidth;

  //    for high-pass filters, don't account for the filter width in the calculation of the number of freedom
  //    if ((ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_NONE) && (ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
  //     temp+=ANALYSE_phFilter->filterEffWidth;

  ANALYSE_nFree=floor(fit->DimL/((temp>(double)1.e-6)?temp:(double)1.)+0.5)-fit->nFit;

  if (ANALYSE_nFree<=(double)0.)
    rc=ERROR_SetLast("SvdInit",ERROR_TYPE_FATAL,ERROR_ID_NFREE);
  else {
    global_doas_spectrum = fit->specrange;

    SvdPDeb=spectrum_start(fit->specrange);
    SvdPFin=spectrum_end(fit->specrange);

    lambda0 = (!pFeno->hidden)? pFeno->lambda0:center_pixel_wavelength(SvdPDeb, SvdPFin);

    int Dim=0;

    if (!pFeno->hidden && (ANALYSE_plFilter->type!=PRJCT_FILTER_TYPE_NONE) && (ANALYSE_plFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
      Dim+=(int)((ANALYSE_plFilter->filterSize)*sqrt(ANALYSE_plFilter->filterNTimes));
    if (((!pFeno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((pFeno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
        (ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_NONE) && (ANALYSE_phFilter->type!=PRJCT_FILTER_TYPE_ODDEVEN))
      Dim+=(int)((ANALYSE_phFilter->filterSize)*sqrt(ANALYSE_phFilter->filterNTimes));

    Dim=max(Dim,pAnalysisOptions->securityGap);

    LimMin=max(SvdPDeb-Dim,0);
    LimMax=min(SvdPFin+Dim,n_wavel-1);

    LimN=LimMax-LimMin+1;

    // Set non linear normalization factors

    norm1=norm2=(double)0.;

    doas_iterator my_iterator;
    for( int i = iterator_start(&my_iterator, fit->specrange); i != ITERATOR_FINISHED; i=iterator_next(&my_iterator)) {
      deltaX=(double)(ANALYSE_splineX[i]-lambda0)*(ANALYSE_splineX[i]-lambda0);

      norm1+=deltaX;
      norm2+=deltaX*deltaX;
    }

    for (j=LimMin,StretchFact1=StretchFact2=(double)0.;j<=LimMax;j++) {
      deltaX=(ANALYSE_splineX[j]-lambda0);

      deltaX=ANALYSE_splineX[j]-lambda0-pFeno->Stretch*deltaX-pFeno->Stretch2*deltaX*deltaX;
      deltaX*=deltaX;

      StretchFact1+=deltaX;
      StretchFact2+=deltaX*deltaX;
    }

    if ((norm1<=(double)0.) || (norm2<=(double)0.) ||
        (StretchFact1<=(double)0.) || (StretchFact2<=(double)0.)) {

      rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_SQRT_ARG);
    } else {
      StretchFact1=(double)1./sqrt(StretchFact1);
      StretchFact2=(double)1./sqrt(StretchFact2);

      for (i=0,norm1=sqrt(norm1),norm2=sqrt(norm2);i<pFeno->NTabCross;i++) {
        pTabCross=&pFeno->TabCross[i];

        // Normalization of non linear parameters

        if ((i==pFeno->indexOffsetOrder1) || (i==pFeno->indexFwhmOrder1))
          pTabCross->Fact=norm1;
        else if ((i==pFeno->indexOffsetOrder2) || (i==pFeno->indexFwhmOrder2))
          pTabCross->Fact=norm2;

        pTabCross->InitStretch/=StretchFact1;
        pTabCross->InitStretch2/=StretchFact2;

        // Fill, 'Fit' vectors with data on parameters to fit

        // ---------------------------------------------------------------------------
        if ((pFeno->analysisMethod!=OPTICAL_DENSITY_FIT) && (pTabCross->FitConc!=ITEM_NONE)) {
          //
          // The best would be to use Fact (calculated in ANALYSE_Function when Decomp=1) but this implies that
          // FitMinp and FitMaxp are in parameters of ANALYSE_Function (called by curfit)
          //

          FitDeltap[pTabCross->FitConc]=pTabCross->DeltaConc;

          if ((fabs(pTabCross->InitConc)>EPSILON) || (fabs(pTabCross->MinConc)>EPSILON) || (fabs(pTabCross->MaxConc)>EPSILON)) {
            norm=(double)0.;

            for( int i = iterator_start(&my_iterator, fit->specrange); i != ITERATOR_FINISHED; i=iterator_next(&my_iterator))
              norm+=pTabCross->vector[i]*pTabCross->vector[i];

            if (norm<=0.)
              rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_SQRT_ARG);
            else {
              norm=sqrt(norm);

              Fitp[pTabCross->FitConc]=(fabs(pTabCross->InitConc)>EPSILON)?pTabCross->InitConc*norm:(double)0.;
              FitMinp[pTabCross->FitConc]=(fabs(pTabCross->MinConc)>EPSILON)?(double)pTabCross->MinConc*norm:(double)0.;
              FitMaxp[pTabCross->FitConc]=(fabs(pTabCross->MaxConc)>EPSILON)?(double)pTabCross->MaxConc*norm:(double)0.;
            }
          } else {
            Fitp[pTabCross->FitConc]=FitMinp[pTabCross->FitConc]=FitMaxp[pTabCross->FitConc]=(double)0.;
          }
        }
        // ---------------------------------------------------------------------------
        if ((pTabCross->FitParam!=ITEM_NONE) && !pTabCross->IndSvdP) {
          Fitp[pTabCross->FitParam]=pTabCross->InitParam;
          FitDeltap[pTabCross->FitParam]=pTabCross->DeltaParam;
          FitMinp[pTabCross->FitParam]=pTabCross->MinParam;
          FitMaxp[pTabCross->FitParam]=pTabCross->MaxParam;
        }
        // ---------------------------------------------------------------------------
        if (pTabCross->FitShift!=ITEM_NONE) {
          Fitp[pTabCross->FitShift]=pTabCross->InitShift;
          FitDeltap[pTabCross->FitShift]=pTabCross->DeltaShift;
          FitMinp[pTabCross->FitShift]=pTabCross->MinShift;
          FitMaxp[pTabCross->FitShift]=pTabCross->MaxShift;
        }
        // ---------------------------------------------------------------------------
        if (pTabCross->FitStretch!=ITEM_NONE) {
          Fitp[pTabCross->FitStretch]=pTabCross->InitStretch;
          FitDeltap[pTabCross->FitStretch]=pTabCross->DeltaStretch;
          FitMinp[pTabCross->FitStretch]=(double)0.;
          FitMaxp[pTabCross->FitStretch]=(double)0.;
        }
        // ---------------------------------------------------------------------------
        if (pTabCross->FitStretch2!=ITEM_NONE) {
          Fitp[pTabCross->FitStretch2]=pTabCross->InitStretch2;
          FitDeltap[pTabCross->FitStretch2]=pTabCross->DeltaStretch2;
          FitMinp[pTabCross->FitStretch2]=(double)0.;
          FitMaxp[pTabCross->FitStretch2]=(double)0.;
        }
        // ---------------------------------------------------------------------------
      }

      for (i=0;i<fit->NF;i++) {
        if ((FitMinp[i]!=(double)0.) && (FitMinp[i]==FitMaxp[i]))
          FitMinp[i]=-FitMaxp[i];
        if (FitMinp[i]>FitMaxp[i]) {
          swap=FitMinp[i];
          FitMinp[i]=FitMaxp[i];
          FitMaxp[i]=swap;
        }
      }
    }
  }

  // Return

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
}

// Fit shift and stretch between 2 spectra, using analysis settings
// from TabFeno[indexFenoColumn][indexFeno].
// Because we use the existing analysis settings, no shift/stretch is
// fit if the analysis window is not configured to use shift and
// stretch.
RC ANALYSE_fit_shift_stretch(int indexFeno, int indexFenoColumn, const double *spec1, const double *spec2,
                     double *shift, double *stretch, double *stretch2,
                     double *sigma_shift, double *sigma_stretch, double *sigma_stretch2) {
  FENO copy = TabFeno[indexFenoColumn][indexFeno]; // local working copy
  Feno=&copy;
  Feno->fit_properties.linfit = NULL;
  Feno->Shift=Feno->Stretch=Feno->Stretch2=0.;
  NDET[indexFenoColumn]=Feno->NDET;
  
  //int molecularRingFlag=Feno->molecularCorrection;
  
  //Feno->molecularCorrection=0;

  CROSS_REFERENCE *pTabCross;

//   for (int indexTabCross=0;indexTabCross<Feno->NTabCross;indexTabCross++)
//    {
//     pTabCross=&Feno->TabCross[indexTabCross];
// 
//     if ((pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_SLOPE) ||
//         (pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_PUKITE) ||
//         (pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE))
//      {
//       if (pTabCross->indexPukite1!=ITEM_NONE)
//        Feno->TabCross[pTabCross->indexPukite1].FitConc=0;
//       if (pTabCross->indexPukite2!=ITEM_NONE)
//        Feno->TabCross[pTabCross->indexPukite2].FitConc=0;
//       
//       pTabCross->crossCorrection=ANLYS_CORRECTION_TYPE_NONE;   // Do not use Pukite or molecular ring for the alignment of reference spectra
//      }
//    } 

  memcpy(Feno->Lambda,Feno->LambdaK,sizeof(*Feno->Lambda)*Feno->NDET); // CHECK: why this copy?
  LambdaSpec=Feno->Lambda; // now pointer LambdaSpec== pointer Feno->Lambda, and buffer content is Feno->LambdaK

  Feno->Decomp=1;
  Feno->amfFlag=0;
  Feno->indexReference=ITEM_NONE;
  
  RC rc;
  
  if (!(rc=ANALYSE_SvdInit(Feno, &Feno->fit_properties, Feno->NDET,Feno->Lambda)) && 
     (!Feno->molecularCorrection || !(rc=Analyse_Molecular_Ring_Init(Feno,Feno->LambdaK,Feno->NDET))))  
  
  // TODO: when we call curfitmethod here, absorber constraints between analysis windows will not work. is this ok?
       rc=ANALYSE_CurFitMethod(indexFenoColumn,
                               spec1,                       // etalon reference spectrum
                               NULL,                        // error on raw spectrum
                               spec2,                       // reference spectrum
                               Feno->NDET,
                               NULL,
                               &Square,                     // returned stretch order 2
                               NULL,                        // number of iterations in Curfit
                               1.,
                               1.,
                               &Feno->fit_properties);
  
  if (Feno->molecularCorrection)
      Analyse_Molecular_Ring_End(Feno,Feno->NDET);    
  
  LINEAR_free(Feno->fit_properties.linfit);
  
  //Feno->molecularCorrection=molecularRingFlag;

  if(rc>=THREAD_EVENT_STOP) {
    return ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_REF_ALIGNMENT,Feno->windowName);
  }

  const CROSS_RESULTS *pResults=&Feno->TabCrossResults[Feno->indexSpectrum];
  *shift=pResults->Shift;
  *stretch=pResults->Stretch;
  *stretch2=pResults->Stretch2;
  *sigma_shift=pResults->SigmaShift;
  *sigma_stretch=pResults->SigmaStretch;
  *sigma_stretch2=pResults->SigmaStretch2;

  return ERROR_ID_NO;
}

// ----------------------------------------------------------
// ANALYSE_AlignReference : Align reference spectrum on etalon
// ----------------------------------------------------------
//  refFlag==0 : GB, file mode selection or satellite, file mode selection, radasref as ref1
//  refFlag==1 : GB, automatic mode selection or satellite
//  refFlag==2 : Satellites, automatic mode  , file mode selection, radasref && kurucz on irradiance
RC ANALYSE_AlignReference(ENGINE_CONTEXT *pEngineContext,int refFlag,void *responseHandle,INDEX indexFenoColumn) {
  RC rc = ERROR_ID_NO;
  
  for (int WrkFeno=0; WrkFeno<NFeno && !rc; WrkFeno++) {

    const FENO *pFeno = &TabFeno[indexFenoColumn][WrkFeno];

    if (((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME1_NETCDF) ||
         (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_TROPOMI) ||
         (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_APEX) ||
         (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GEMS) ||
         (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_TROPOMI))
     &&
        !pFeno->useRefRow) continue;
        
    if (!pFeno->hidden
        && (pFeno->useKurucz!=ANLYS_KURUCZ_NONE) && (pFeno->useKurucz!=ANLYS_KURUCZ_SPEC) && (pFeno->useKurucz!=ANLYS_KURUCZ_REF_AND_SPEC)
        && (pFeno->newrefFlag || pEngineContext->satelliteFlag)
        && ( (!refFlag && pFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE && pFeno->gomeRefFlag) ||
             ((refFlag==1) && (pFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC)) ||
             ((refFlag==2) && ((pFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) || !pFeno->gomeRefFlag)) )
        && pFeno->useEtalon
        && !VECTOR_Equal(pFeno->SrefEtalon,pFeno->Sref,pFeno->NDET,0.) ) {
     double shift, stretch, stretch2, sigma_shift, sigma_stretch, sigma_stretch2;

      rc=ANALYSE_fit_shift_stretch(WrkFeno, indexFenoColumn, pFeno->SrefEtalon, pFeno->Sref, &shift, &stretch, &stretch2, &sigma_shift, &sigma_stretch, &sigma_stretch2);
      
      double *lambda=pFeno->Lambda; // CHECK: this used to be the global pointer 'Lambda'. changed it to a local pointer
      const double lambda0=pFeno->lambda0;
      // CHECK: changes to lambda here also change original pFeno->Lambda, because it's a pointer to the same buffer... is this ok?
      for (int j=0; j<pFeno->NDET; j++) { // This is used only for spectra display
        double x0=lambda[j]-lambda0;
        lambda[j]=lambda[j]-(shift+(stretch+stretch2*x0)*x0);
      }

      TabFeno[indexFenoColumn][WrkFeno].Shift=shift;
      TabFeno[indexFenoColumn][WrkFeno].Stretch=stretch;
      TabFeno[indexFenoColumn][WrkFeno].Stretch2=stretch2;

      // Display fit
      if (pFeno->displayRefEtalon && pEngineContext->project.spectra.displaySpectraFlag) {
        memcpy(ANALYSE_secX,ANALYSE_zeros,sizeof(*ANALYSE_secX)*pFeno->NDET);

        for (int i=SvdPDeb;i<=SvdPFin;i++)
          ANALYSE_secX[i]=exp(log(pFeno->SrefEtalon[i])+ANALYSE_absolu[i]);

        char refTitle[256];
        if (ANALYSE_swathSize==1)
         sprintf(refTitle,"Ref1/Ref2 in %s",pFeno->windowName);
        else
         sprintf(refTitle,"Ref1/Ref2 in %s (%d)",pFeno->windowName,indexFenoColumn+1);

        const int indexPage=plotPageRef; // (pEngineContext->satelliteFlag || (ANALYSE_swathSize>1))?plotPageRef:WrkFeno+plotPageAnalysis-1;
        MEDIATE_PLOT_CURVES(indexPage, Spectrum, forceAutoScale, refTitle, "Wavelength (nm)", "Intensity", responseHandle,
                            CURVE(.name="Measured", .x=&lambda[SvdPDeb], .y=&pFeno->SrefEtalon[SvdPDeb], .length=SvdPFin-SvdPDeb+1),
                            CURVE(.name="Calculated", .x=&lambda[SvdPDeb], .y=&ANALYSE_secX[SvdPDeb], .length=SvdPFin-SvdPDeb+1));
        mediateResponseLabelPage(indexPage,pEngineContext->fileInfo.fileName, "Reference", responseHandle);

        ANALYSE_plotRef=1;
        int indexLine=ANALYSE_indexLine;
        const int indexColumn = 2;

        if (ANALYSE_swathSize==1)
         mediateResponseCellInfo(plotPageRef,indexLine++,indexColumn,responseHandle,"ALIGNMENT REF1/REF2 IN","%s",pFeno->windowName);
        else
         mediateResponseCellInfo(plotPageRef,indexLine++,indexColumn,responseHandle,"ALIGNMENT REF1/REF2 IN","%s (row %d)",pFeno->windowName,indexFenoColumn+1);

        mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Shift Ref1/Ref2","%#10.3e +/- %#10.3e",shift,sigma_shift);
        mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Stretch Ref1/Ref2","%#10.3e +/-%#10.3e",stretch,sigma_stretch);
        mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"Stretch2 Ref1/Ref2","%#10.3e +/- %#10.3e",stretch2,sigma_stretch2);

        ANALYSE_indexLine=indexLine+1;
      }
      TabFeno[indexFenoColumn][WrkFeno].Decomp=1; // CHECK: why? seems Decomp is already == 1 here anyway
    }
  }

  return rc;
}

RC AnalyseSaveResiduals(char *fileName,ENGINE_CONTEXT *pEngineContext, const int n_wavel)
{
  RC rc;
  char *fileNamePtr,*ptr,resFile[MAX_ITEM_TEXT_LEN],ext[MAX_ITEM_TEXT_LEN];
  FILE *fp;

  rc=ERROR_ID_NO;

  FILES_RebuildFileName(resFile,fileName,1);

  if ((fileNamePtr=strrchr(resFile,PATH_SEP))==NULL)                 // extract output file name without path
   fileNamePtr=resFile;
  else
   fileNamePtr++;

  if (!strlen(fileNamePtr) && ((ptr=strrchr(pEngineContext->fileInfo.fileName,PATH_SEP))!=NULL))
   {
    strcpy(fileNamePtr,ptr+1);
    if ((ptr=strrchr(fileNamePtr,'.'))!=NULL)
     *ptr=0;

    sprintf(ext,"_%s.%s",Feno->windowName,FILES_types[FILE_TYPE_RES].fileExt);
    strcat(fileNamePtr,ext);
   }

  if ((fp=fopen(resFile,"a+t"))==NULL)
   rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,resFile);
  else
   {
    if (!STD_FileLength(fp))
     {
      fprintf(fp,"0 0 0 ");

      for(int i=0; i<n_wavel; ++i) {
       fprintf(fp,"%.14le ",Feno->Lambda[i]);
      }
      fprintf(fp,"\n");
     }

    fprintf(fp,"%-5d %.3lf %-8.4lf ",pEngineContext->indexRecord,pEngineContext->recordInfo.Zm,
            (double)ZEN_FNCaljda(&pEngineContext->recordInfo.Tm)+ZEN_FNCaldti(&pEngineContext->recordInfo.Tm)/24.);

    int curPixel = 0;
    doas_iterator my_iterator;
    doas_interval *nextinterval = iterator_start_interval(&my_iterator, Feno->fit_properties.specrange);
    while (curPixel < n_wavel) {
     int stop = (nextinterval != NULL)
       ? interval_start(nextinterval)
       : n_wavel;

     // print NaN for every pixel outside the current range.
     while(curPixel < stop) {
      fprintf(fp,"%.14le ", NAN);
      ++curPixel;
     }
     if (nextinterval != NULL) {
      int end = interval_end(nextinterval);

      // print residual for every pixel inside the current range.
      while(curPixel <= end) {
       fprintf(fp,"%.14le ",ANALYSE_absolu[curPixel]);
       ++curPixel;
      }
      nextinterval = iterator_next_interval(&my_iterator);
     }
    }

    fprintf(fp,"\n");
    fclose(fp);
   }

  return rc;
}

// --------------------------------------------------------------------------------------------------------
// Function : Cross sections and spectrum alignment using spline fitting functions and new Yfit computation
// --------------------------------------------------------------------------------------------------------

RC ANALYSE_Function(double *spectrum_orig, double *reference, const double *SigmaY, double *Yfit, int Npts,
                     double *fitParamsC, double *fitParamsF,INDEX indexFenoColumn, struct fit_properties *fitprops)
{
  // Declarations

  double *XTrav,*YTrav,*newXsTrav,*spec_nolog,*spectrum_interpolated,*reference_shifted, deltaX;
  CROSS_REFERENCE *TabCross,*pTabCross;
  int NewDimC,offsetOrder;
  INDEX indexSvdA,indexSvdP,polyOrder,polyFlag;
  double lambda0,slitParam[NSFP];
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations
  const int n_wavel = NDET[indexFenoColumn];
  TabCross=Feno->TabCross;
  XTrav=YTrav=newXsTrav=spectrum_interpolated=reference_shifted=spec_nolog=NULL;

  for (int i=0;i<NSFP;i++)
   if (Feno->indexFwhmParam[i]!=ITEM_NONE)
    slitParam[i]=(TabCross[Feno->indexFwhmParam[i]].FitParam!=ITEM_NONE)?fitParamsF[TabCross[Feno->indexFwhmParam[i]].FitParam]:TabCross[Feno->indexFwhmParam[i]].InitParam;

  polyFlag=0;
  NewDimC=fitprops->DimC;

  lambda0 = (!Feno->hidden)?Feno->lambda0:center_pixel_wavelength(SvdPDeb, SvdPFin);

  rc=ERROR_ID_NO;

  // Real time convolution for Kurucz

  if ((Feno->hidden==1) && Feno->xsToConvolute && pKuruczOptions->fwhmFit) {

    rc=ANALYSE_XsConvolution(Feno,Lambda,NULL,slitParam,pKuruczOptions->fwhmType,indexFenoColumn,(pKuruczOptions->fwhmType==SLIT_TYPE_FILE)?1:0);
    if(rc) {
      goto EndFunction;
    }
  }

  // Don't take fixed concentrations into account for singular value decomposition

  for (int i=0;i<Feno->NTabCross && (NewDimC==fitprops->DimC);i++)
   if ((Feno->analysisMethod==OPTICAL_DENSITY_FIT) && (TabCross[i].FitConc==0) &&
       (TabCross[i].DeltaConc==(double)0.) && TabCross[i].IndSvdA && (TabCross[i].IndSvdA<=NewDimC))

    NewDimC=TabCross[i].IndSvdA-1;

  // Buffers allocation

  if (((XTrav=MEMORY_AllocDVector(__func__,"XTrav",0,Npts-1))==NULL) ||                  // raw spectrum
      ((YTrav=MEMORY_AllocDVector(__func__,"YTrav",0,Npts-1))==NULL) ||                  // reference spectrum
      ((spec_nolog=MEMORY_AllocDVector(__func__,"spec_nolog",0,Npts-1))==NULL) ||
      ((newXsTrav=MEMORY_AllocDVector(__func__,"newXsTrav",0,n_wavel-1))==NULL) ||
      ((spectrum_interpolated=MEMORY_AllocDVector(__func__,"spectrum_interpolated",0,n_wavel-1))==NULL) || // spectrum interpolated on reference wavelength grid
      ((reference_shifted=MEMORY_AllocDVector(__func__,"reference_shifted",0,n_wavel-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else {
    memcpy(newXsTrav,ANALYSE_zeros,sizeof(double)*n_wavel);
    memcpy(spectrum_interpolated,ANALYSE_zeros,sizeof(double)*n_wavel);

   // ========
   // SPECTRUM
   // ========

   // ---------------------------------
   // Wavelength alignment (shift option in Shift and stretch page) for spectrum
   // ---------------------------------

    double shift_rad, stretch_rad, stretch2_rad;
    if(Feno->indexSpectrum!=ITEM_NONE) {
      shift_rad = (TabCross[Feno->indexSpectrum].FitShift!=ITEM_NONE)
        ? fitParamsF[TabCross[Feno->indexSpectrum].FitShift]
        : TabCross[Feno->indexSpectrum].InitShift;
      stretch_rad = (TabCross[Feno->indexSpectrum].FitStretch!=ITEM_NONE)
        ? fitParamsF[TabCross[Feno->indexSpectrum].FitStretch]
        : TabCross[Feno->indexSpectrum].InitStretch;
      stretch2_rad = (TabCross[Feno->indexSpectrum].FitStretch2!=ITEM_NONE)
        ? fitParamsF[TabCross[Feno->indexSpectrum].FitStretch2]
        : TabCross[Feno->indexSpectrum].InitStretch2;
    } else {
      shift_rad = stretch_rad = stretch2_rad = 0.;
    }

    if ( (rc=ShiftVector(LambdaSpec, spectrum_orig, SplineSpec, spectrum_interpolated, n_wavel,
                         shift_rad, stretch_rad, stretch2_rad, //
                         0., 0., 0., fitParamsF, -1, 0, 0, indexFenoColumn))!=ERROR_ID_NO ||
         (Feno->useUsamp && pUsamp->method==PRJCT_USAMP_AUTOMATIC && (rc=ANALYSE_UsampBuild(2,ITEM_NONE,indexFenoColumn))!=ERROR_ID_NO) )

      goto EndFunction;

   // ------------------------------
   // Low pass filtering on spectrum
   // ------------------------------

   // Filter real time only when fitting difference of resolution between spectrum and reference

   if ((Feno->analysisType==ANALYSIS_TYPE_FWHM_NLFIT) && (ANALYSE_plFilter->filterFunction!=NULL) &&
       ((rc=FILTER_Vector(ANALYSE_plFilter,&spectrum_interpolated[LimMin],&spectrum_interpolated[LimMin],NULL,LimN,PRJCT_FILTER_OUTPUT_LOW))!=0)) {
     rc=ERROR_SetLast("EndFunction",ERROR_TYPE_WARNING,ERROR_ID_ANALYSIS,analyseIndexRecord,"Filter");
     goto EndFunction;
    }

   //-------------------
   // Calculate the mean
   //-------------------
   doas_iterator my_iterator;
   Feno->xmean=(double)0.;
   for(int i = iterator_start(&my_iterator, global_doas_spectrum); i != ITERATOR_FINISHED; i=iterator_next(&my_iterator))
    Feno->xmean+=(double)spectrum_interpolated[i];

   Feno->xmean/=Npts;

   // -------------------------------
   // Spectrum correction with offset
   // -------------------------------

   if (Feno->analysisMethod!=INTENSITY_FIT) {
     offsetOrder=-1;

     if ((Feno->indexOffsetConst!=ITEM_NONE) && ((TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE) || (TabCross[Feno->indexOffsetConst].InitParam!=(double)0.)))
      offsetOrder=0;
     if ((Feno->indexOffsetOrder1!=ITEM_NONE) && ((TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE) || (TabCross[Feno->indexOffsetOrder1].InitParam!=(double)0.)))
      offsetOrder=1;
     if ((Feno->indexOffsetOrder2!=ITEM_NONE) && ((TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE) || (TabCross[Feno->indexOffsetOrder2].InitParam!=(double)0.)))
      offsetOrder=2;

     if (offsetOrder>=0) {
       for (int i=LimMin;i<=LimMax;i++) {
         deltaX=(double)(ANALYSE_splineX[i]-lambda0);

         double offset=(TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE)
           ? fitParamsF[TabCross[Feno->indexOffsetConst].FitParam]
           : TabCross[Feno->indexOffsetConst].InitParam;

         if (offsetOrder>=1) {
           const double val = (TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE)
             ? fitParamsF[TabCross[Feno->indexOffsetOrder1].FitParam]/TabCross[Feno->indexOffsetOrder1].Fact
             : TabCross[Feno->indexOffsetOrder1].InitParam;
           offset+=val*deltaX;
         }
         if (offsetOrder>=2) {
           const double val = (TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE)
             ? fitParamsF[TabCross[Feno->indexOffsetOrder2].FitParam]/TabCross[Feno->indexOffsetOrder2].Fact
             : TabCross[Feno->indexOffsetOrder2].InitParam;
           offset+=val*deltaX*deltaX;
         }
         spectrum_interpolated[i] -= offset*Feno->xmean;
       }
     }
   }

   // ------------------------------------------
   // Backup of spectrum before taking logarithm
   // ------------------------------------------
   for( int k=0,l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator))
     spec_nolog[k]=spectrum_interpolated[l];

   // -------------------------------
   // High-pass filtering on spectrum
   // -------------------------------

   if ((Feno->analysisMethod==OPTICAL_DENSITY_FIT) && !hFilterSpecLog &&  // logarithms are not calculated and filtered before entering this function
       (((rc=VECTOR_Log(&spectrum_interpolated[LimMin],&spectrum_interpolated[LimMin],LimN,"ANALYSE_Function (Spec) "))!=0) ||
        ((ANALYSE_phFilter->filterFunction!=NULL) &&
         ((!Feno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((Feno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
         ((rc=FILTER_Vector(ANALYSE_phFilter,&spectrum_interpolated[LimMin],&spectrum_interpolated[LimMin],NULL,LimN,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=0))))
    goto EndFunction;

   // ----------------------------
   // Transfer to working variable
   // ----------------------------

   for( int k=0,l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator)) {
     XTrav[k]=spectrum_interpolated[l];
   }

   // ==============
   // CROSS SECTIONS
   // ==============

   // ----------------
   // Build svd matrix
   // ----------------
   
   if (Feno->Decomp) {
     for (int i=0;i<Feno->NTabCross;i++)

       if ((indexSvdA=TabCross[i].IndSvdA)>0) {
         pTabCross=&TabCross[i];
         pTabCross->Fact=1.;

         // Fill SVD matrix with predefined components

         int numpixels = spectrum_length(global_doas_spectrum);

         if (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_PREDEFINED) {
           doas_iterator my_iterator;
           if (i==Feno->indexOffsetConst)
             for (int k=1; k<=numpixels; k++)
               fitprops->A[indexSvdA][k]=1.;
           else if (i==Feno->indexOffsetOrder1) {
             for( int k=1,l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator))
               fitprops->A[indexSvdA][k]=(double)(ANALYSE_splineX[l]-lambda0);
           }
           else if (i==Feno->indexOffsetOrder2) {
             for( int k=1,l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator))
               fitprops->A[indexSvdA][k]=(double)(ANALYSE_splineX[l]-lambda0)*(ANALYSE_splineX[l]-lambda0);
           }
           else if ((i==Feno->indexCommonResidual) || (i==Feno->indexUsamp1) || (i==Feno->indexUsamp2)) {
             for( int k=1, l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator))
               fitprops->A[indexSvdA][k]=(Feno->analysisMethod==OPTICAL_DENSITY_FIT) ?
                 -pTabCross->vector[l] : pTabCross->vector[l];
           }
           else if (i==Feno->indexResol) {

             double resolDelta=0.05;                                                   // small increment to calculate the derivative
             double resolCoeff=Feno->resolFwhm/resolDelta;
             double resolX=resolDelta*sqrt((double)1.+2.*resolCoeff);

             for( int k=1, l=max(iterator_start(&my_iterator, global_doas_spectrum),1); (l != ITERATOR_FINISHED) && !rc; k++,l=iterator_next(&my_iterator))
               if (!(rc=XSCONV_TypeGauss(ANALYSE_splineX,reference,SplineRef,ANALYSE_splineX[l],ANALYSE_splineX[l]-ANALYSE_splineX[l-1],&pTabCross->vector[l],resolX,(double)0.,SLIT_TYPE_GAUSS, n_wavel)))
                 fitprops->A[indexSvdA][k]=pTabCross->vector[l]=(reference[l]!=0)?resolCoeff*(pTabCross->vector[l]/reference[l]-1):(double)0.;
           }
         }

         // Fill SVD matrix with polynomial components

         else if (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CONTINUOUS) {
           if ((strlen(WorkSpace[pTabCross->Comp].symbolName)==2) && (WorkSpace[pTabCross->Comp].symbolName[0]=='x')) {
             polyOrder=WorkSpace[pTabCross->Comp].symbolName[1]-'0';
             polyFlag=1;
           }
           else if ((strlen(WorkSpace[pTabCross->Comp].symbolName)==5) &&
                    !strncmp(WorkSpace[pTabCross->Comp].symbolName, "offl", 4)) {
             polyOrder=WorkSpace[pTabCross->Comp].symbolName[4]-'0';
             polyFlag=0;
           }
           else
             polyOrder=ITEM_NONE;

           if (polyFlag && polyOrder == 0 ) {
             for( int k=1,l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator))
               fitprops->A[indexSvdA][k]=pTabCross->vector[l];
           }
           else if (polyOrder > 0) {
             // in order to have geophysical values of the polynomial in output,
             for( int k=1,l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator))
               fitprops->A[indexSvdA][k]=pTabCross->vector[l]=fitprops->A[indexSvdA-1][k]*(ANALYSE_splineX[l]-lambda0);
           }
           else if (Feno->analysisMethod==OPTICAL_DENSITY_FIT) { // SVD method, polyOrder == 0, polyFlag == 0 -> linear offset, order 0

             switch (Feno->linear_offset_mode) {
             case LINEAR_OFFSET_RAD: // normalized w.r.t. the spectrum
               for( int k=1,l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator)) {
                 fitprops->A[indexSvdA][k]= pTabCross->vector[l] = (fabs(spec_nolog[k-1])> 1.e-14) // 1e-6
                   ? -Feno->xmean/spec_nolog[k-1]
                   : 0.;
               }
               break;
             case LINEAR_OFFSET_REF: {
               // offset normalized w.r.t. the reference.
               const double * const offset_ref = reference;
               for (int k=0;k<Feno->NDET;k++)
              for( int k=1,l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator)) {
                 fitprops->A[indexSvdA][k]=pTabCross->vector[l]= (fabs(offset_ref[l])> 1.e-14) // 1e-6
                   ? Feno->ymean/offset_ref[l]  // !!!! XMEAN -> YMEAN
                   : 0.;

               }
             }
               break;
             default:
               // we should have either linear_offset_rad or linear_offset_ref
               assert(false);
               break;
             }
           } else { // linear offset, Marquardt+SVD method -> normalized w.r.t. the reference

             for( int k=1,l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator))
               fitprops->A[indexSvdA][k]=pTabCross->vector[l]=(fabs(reference[l])>(double)1.e-6)?(double)Feno->xmean/reference[l] :(double)0.;
           }
         }

         // Fill SVD matrix with cross sections

         else if (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS)
          {
           // Use substitution vectors for cross sections because of AMF correction or Pukite terms

           memcpy(ANALYSE_xsTrav,pTabCross->vector,sizeof(double)*n_wavel);
           memcpy(ANALYSE_xsTrav2,pTabCross->Deriv2,sizeof(double)*n_wavel);

           // --------------
           // AMF correction
           // --------------

           if ((Feno->amfFlag && ((rc=OUTPUT_GetWveAmf(&Feno->TabCrossResults[i],ZM,Lambda,ANALYSE_xsTrav,n_wavel))!=0)) ||

               // ---------------------------------------
               // Wavelength alignment (AMF) for cross sections
               // ---------------------------------------

                ((rc=ShiftVector(ANALYSE_splineX,ANALYSE_xsTrav /* (0:n_wavel-1) */,ANALYSE_xsTrav2 /* (0:n_wavel-1) */,newXsTrav /* (0:n_wavel-1) */, n_wavel,
                                 (pTabCross->FitShift!=ITEM_NONE)?(double)fitParamsF[pTabCross->FitShift]:(double)pTabCross->InitShift,
                                 (pTabCross->FitStretch!=ITEM_NONE)?(double)fitParamsF[pTabCross->FitStretch]:(double)pTabCross->InitStretch,
                                 (pTabCross->FitStretch2!=ITEM_NONE)?(double)fitParamsF[pTabCross->FitStretch2]:(double)pTabCross->InitStretch2,
                                 Feno->Shift,Feno->Stretch,Feno->Stretch2,
                                 NULL,0,0,0,indexFenoColumn))!=ERROR_ID_NO))

            goto EndFunction;

           else
            {
             doas_iterator my_iterator;
             for( int k=1,l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator))
              fitprops->A[indexSvdA][k]=newXsTrav[l];
            }
          }
        }

      XsDifferences(fitprops->A,fitprops->DimL);                                // Calculate differences of cross sections before orthogonalizations
      Orthogonalization(fitprops->A,fitprops->DimL);

      // ----------------------------------------------------
      // In optical density fitting mode, allocate linear fitting environment for cross sections:
      // ----------------------------------------------------
      
      if (Feno->analysisMethod==OPTICAL_DENSITY_FIT) {
        // clean up old linear fit environment:
        LINEAR_free(fitprops->linfit);
        fitprops->linfit = LINEAR_alloc(Npts,NewDimC,DECOMP_EIGEN_QR);
      }

      // ----------------------------------------------------
      // Cross sections correction with non linear parameters
      // ----------------------------------------------------

      for (int i=0;i<Feno->NTabCross;i++) {
        pTabCross=&TabCross[i];

        if ((indexSvdA=pTabCross->IndSvdA)>0) {
          // ----------------------------------------------------
          // Cross sections correction with non linear parameters
          // ----------------------------------------------------

          if (indexSvdA <=  NewDimC) {
            switch (Feno->analysisMethod) {
            case OPTICAL_DENSITY_FIT:
              // ----------------------------------------------------
              // Copy cross sections for which we fit the concentration to linear fit system
              // ----------------------------------------------------
              LINEAR_set_column(fitprops->linfit, indexSvdA, fitprops->A[indexSvdA]);
              break;
            case INTENSITY_FIT:
              // Calculate norm
              pTabCross->Fact = sqrt(VECTOR_Norm(fitprops->A[indexSvdA],Npts));
              break;
            default:
              assert(false); // bug, analysisMethod should be either OPTICAL_DENSITY_FIT or ...SVDMARQUARDT
              break;
            }
          }
        }
      }

      // -----------------
      // SVD or QR decomposition
      // -----------------

      if (Feno->analysisMethod==OPTICAL_DENSITY_FIT) {
        LINEAR_set_weight(fitprops->linfit, SigmaY);
        rc = LINEAR_decompose(fitprops->linfit,fitprops->SigmaSqr,fitprops->covar);
        
        if (rc != ERROR_ID_NO)
          goto EndFunction;
      }

        // We only need to recalculate the svd decomposition if the matrix can be chagned by non-linear fit parameters (NP), weighting by spectrum errors (SigmaY), or linear offset:
      if (!Feno->fit_properties.NP && (SigmaY==NULL) && ( (Feno->linear_offset_mode != LINEAR_OFFSET_RAD) || (Feno->analysisMethod==INTENSITY_FIT))) {
        Feno->Decomp=0;
      }
    }

    // =========
    // REFERENCE
    // =========

    // ----------------------------------
    // Wavelength alignment (shift) for reference
    // ----------------------------------

    double shift_ref, stretch_ref, stretch2_ref;
    if (Feno->indexReference!=ITEM_NONE) {
      shift_ref = (TabCross[Feno->indexReference].FitShift!=ITEM_NONE)
        ? fitParamsF[TabCross[Feno->indexReference].FitShift]
        : TabCross[Feno->indexReference].InitShift;
      stretch_ref = (TabCross[Feno->indexReference].FitStretch!=ITEM_NONE)
        ? fitParamsF[TabCross[Feno->indexReference].FitStretch]
        : TabCross[Feno->indexReference].InitStretch;
      stretch2_ref = (TabCross[Feno->indexReference].FitStretch2!=ITEM_NONE)
        ? fitParamsF[TabCross[Feno->indexReference].FitStretch2]
        : TabCross[Feno->indexReference].InitStretch2;
    } else {
      shift_ref = stretch_ref = stretch2_ref = 0;
    }

    if ((rc=ShiftVector(ANALYSE_splineX,reference,SplineRef,reference_shifted,n_wavel,
                        shift_ref,stretch_ref,stretch2_ref,
                        (double)0.,(double)0.,(double)0., fitParamsF,1,
                        (Feno->analysisType==ANALYSIS_TYPE_FWHM_KURUCZ)?1:0,
                        (Feno->analysisType==ANALYSIS_TYPE_FWHM_SLIT) && (KURUCZ_buffers[indexFenoColumn].hrSolarGridded.matrix!=NULL)?1:0,
                        indexFenoColumn))!=ERROR_ID_NO)

     goto EndFunction;

#if defined(__DEBUG_) && __DEBUG_  && defined(__DEBUG_DOAS_SHIFT_) && __DEBUG_DOAS_SHIFT_
    if (((analyseDebugMask&DEBUG_FCTTYPE_MATH)!=0) && analyseDebugVar &&
        (Feno->indexReference!=ITEM_NONE) &&
        ((TabCross[Feno->indexReference].FitShift!=ITEM_NONE) || (TabCross[Feno->indexReference].InitShift!=(double)0.)))
     DEBUG_PrintVar("Interpolation of the reference",ANALYSE_splineX,LimMin,LimMax,Y,LimMin,LimMax,SplineRef,LimMin,LimMax,reference_shifted,LimMin,LimMax,NULL);
#endif

    // -------------------------------
    // Low pass filtering on reference
    // -------------------------------

    // Filter real time only when fitting difference of resolution between spectrum and reference

    if ((Feno->analysisType==ANALYSIS_TYPE_FWHM_NLFIT) && (ANALYSE_plFilter->filterFunction!=NULL) &&
        ((rc=FILTER_Vector(ANALYSE_plFilter,&reference_shifted[LimMin],&reference_shifted[LimMin],NULL,LimN,PRJCT_FILTER_OUTPUT_LOW))!=0))
     {
      rc=ERROR_SetLast("EndFunction",ERROR_TYPE_WARNING,ERROR_ID_ANALYSIS,analyseIndexRecord,"Filter");
      goto EndFunction;
     }

    // ----------------------------------------------
    // Reference correction with non linear parameters
    // ----------------------------------------------

    if ((Feno->analysisMethod!=INTENSITY_FIT) &&
        (Feno->indexSol!=ITEM_NONE) &&
        ((TabCross[Feno->indexSol].FitParam!=ITEM_NONE) ||
         ((TabCross[Feno->indexSol].InitParam!=(double)0.)&&(TabCross[Feno->indexSol].InitParam!=(double)1.))))

     for (int i=LimMin;i<=LimMax;i++)
      reference_shifted[i]=pow(reference_shifted[i],(TabCross[Feno->indexSol].FitParam!=ITEM_NONE)?(double)fitParamsF[TabCross[Feno->indexSol].FitParam]:(double)TabCross[Feno->indexSol].InitParam);

    // --------------------------------
    // High pass filtering on reference
    // --------------------------------

    // logarithms are not calculated and filtered before entering this function

    if ((Feno->analysisMethod==OPTICAL_DENSITY_FIT) && !hFilterRefLog &&  // logarithms are not calculated and filtered before entering this function
        (((rc=VECTOR_Log(&reference_shifted[LimMin],&reference_shifted[LimMin],LimN,"ANALYSE_Function (Ref) "))!=0) ||
         ((ANALYSE_phFilter->filterFunction!=NULL) &&
          ((!Feno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((Feno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
          ((rc=FILTER_Vector(ANALYSE_phFilter,&reference_shifted[LimMin],&reference_shifted[LimMin],NULL,LimN,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=0))))
    {
     rc=ERROR_SetLast("EndFunction",ERROR_TYPE_WARNING,ERROR_ID_ANALYSIS,analyseIndexRecord,"Error with the selected reference spectrum");
     goto EndFunction;
    }

    // ----------------------------
    // Transfer to working variable
    // ----------------------------

    for( int k=0,l=iterator_start(&my_iterator, global_doas_spectrum); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator))
     YTrav[k]=reference_shifted[l];

    //
    // OPTICAL THICKNESS FITTING (SVD)
    //

    if (Feno->analysisMethod==OPTICAL_DENSITY_FIT) {
      // ---------------------
      // SVD back substitution
      // ---------------------

      // N.B. : YTrav -> reference spectrum, shifted and stretched
      //        XTrav -> raw spectrum, shifted and stretched

      for(int k=1; k<=spectrum_length(global_doas_spectrum); k++) {
        b[k]=YTrav[k-1]-XTrav[k-1];

        for (int l=NewDimC+1;l<=fitprops->DimC;l++)
          b[k]-=fitprops->A[l][k]*fitParamsC[l];

        if (SigmaY!=NULL)
          b[k]/=SigmaY[k-1];
      }

      rc = LINEAR_solve(fitprops->linfit, b, fitParamsC);
      if (rc != ERROR_ID_NO)
        goto EndFunction;

      // ------------------------------------------------
      // Yfit computation with the solution of the system
      // ------------------------------------------------

      for (int l=0;l<Feno->NTabCross;l++) {
        int svdIndex = TabCross[l].IndSvdA;
        double weight = (svdIndex <= NewDimC)
          ? fitParamsC[svdIndex]/TabCross[l].Fact
          : fitParamsC[svdIndex];
        if (svdIndex > 0)
          for (int k=1;k<=fitprops->DimL;k++) {
            XTrav[k-1]+= fitprops->A[svdIndex][k]*weight;
         }
      }
      for (int k=1;k<=fitprops->DimL;k++) {
        Yfit[k-1]=YTrav[k-1]-XTrav[k-1]; // NB : logarithm test on YTrav has been made in the previous loop
      }

    } else if (Feno->analysisMethod==INTENSITY_FIT) {

      // ------------------------------------------------------------
      // INTENSITY FITTING
      // ------------------------------------------------------------

      for (int k=1,i=iterator_start(&my_iterator, global_doas_spectrum); i != ITERATOR_FINISHED; k++,i=iterator_next(&my_iterator)) {
        double tau = 0.;
        for (int l=0;l<Feno->NTabCross;l++) {
          pTabCross=&TabCross[l];

          if (pTabCross->IndSvdA > 0 && WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) {
            const double conc = (pTabCross->FitConc>ITEM_NONE)
              ? fitParamsF[pTabCross->FitConc] // Conc non linear fitting
              : fitParamsC[pTabCross->IndSvdA];
            tau+= conc * fitprops->A[pTabCross->IndSvdA][k] / pTabCross->Fact; // divide by norm for numerical stability
          }
        }

        if (-tau>700.) {
          rc=ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_OVERFLOW);
          goto EndFunction;
        } else if (YTrav[k-1] == 0.) {
          rc=ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_DIVISION_BY_0);
          goto EndFunction;
        }

        ANALYSE_t[i]=exp(-tau); // exp(-tau)
        ANALYSE_tc[i]=XTrav[k-1]/YTrav[k-1];  // I/I0
        b[k] = ANALYSE_tc[i];
      }

      for (int l=0;l<Feno->NTabCross;l++) {
        pTabCross=&TabCross[l];

        if (((indexSvdA=pTabCross->IndSvdA)>0) && ((indexSvdP=pTabCross->IndSvdP)>0)) {
          for( int k=1,i=iterator_start(&my_iterator, global_doas_spectrum); i != ITERATOR_FINISHED; k++,i=iterator_next(&my_iterator)) {
            if (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CONTINUOUS && WorkSpace[pTabCross->Comp].symbolName[0]!='o') {
              // Polynomial
              fitprops->P[indexSvdP][k]=ANALYSE_t[i]*fitprops->A[indexSvdA][k]; ///pTabCross->Fact;
            } else {
              // Linear offset normalized w.r.t. the reference spectrum and other parameters (Ring ...)
              fitprops->P[indexSvdP][k]=fitprops->A[indexSvdA][k]; ///pTabCross->Fact;
            }
          }
        }
      }
      LINEAR_free(fitprops->linfit);
      fitprops->linfit = LINEAR_from_matrix((const double **)fitprops->P, Npts,fitprops->DimP,DECOMP_EIGEN_QR);
      if (SigmaY != NULL) {
        LINEAR_set_weight(fitprops->linfit, SigmaY);
        for (int i=0; i<fitprops->DimP; ++i) {
          b[1+i] /= SigmaY[i];
        }
      }

      rc = LINEAR_decompose(fitprops->linfit, fitprops->SigmaSqr, fitprops->covar);
      double *xP = malloc(fitprops->DimP * sizeof(*xP));
      double *fitParamsP=xP-1; // linear fitting functions assume index starts at 1.
      if (rc == ERROR_ID_NO) rc = LINEAR_solve(fitprops->linfit, b, fitParamsP);
      if (rc != ERROR_ID_NO)
        goto EndFunction;

      // ------------------------------------------------
      // Yfit computation with the solution of the system
      // ------------------------------------------------

      for (int i=0;i<Feno->NTabCross;i++)
       if (((indexSvdA=TabCross[i].IndSvdA)>0) && ((indexSvdP=TabCross[i].IndSvdP)>0))
        fitParamsC[indexSvdA]=fitParamsP[indexSvdP];

      for (int k=1,i=iterator_start(&my_iterator, global_doas_spectrum); i != ITERATOR_FINISHED; k++,i=iterator_next(&my_iterator)) {
        double tau = 0.;
        double offset = 0.;
        for (int l=0;l<Feno->NTabCross;l++)
         if (((indexSvdA=TabCross[l].IndSvdA)>0) && ((indexSvdP=TabCross[l].IndSvdP)>0))
          {
           if ((WorkSpace[TabCross[l].Comp].type==WRK_SYMBOL_CONTINUOUS) && (WorkSpace[TabCross[l].Comp].symbolName[0]!='o'))      // Polynomial
            tau+=fitParamsP[indexSvdP]*fitprops->A[indexSvdA][k];
           else                                                                // Offset and other parameters (ring...)
            offset+=fitParamsP[indexSvdP]*fitprops->A[indexSvdA][k];
          }

        ANALYSE_tc[i]-=offset;            // I/I0 - offset/I0
        ANALYSE_t[i]*=tau;                // tau*exp(-optical depth)

        Yfit[k-1]=ANALYSE_t[i]-ANALYSE_tc[i];
      }
      free(xP);
    }
  }

  // Release allocated buffers

 EndFunction :

  if (XTrav!=NULL)
   MEMORY_ReleaseDVector(__func__,"XTrav",XTrav,0);
  if (YTrav!=NULL)
   MEMORY_ReleaseDVector(__func__,"YTrav",YTrav,0);
  if (newXsTrav!=NULL)
   MEMORY_ReleaseDVector(__func__,"newXsTrav",newXsTrav,0);
  if (spectrum_interpolated!= NULL)
   MEMORY_ReleaseDVector(__func__,"spectrum_interpolated",spectrum_interpolated,0);
  if (reference_shifted != NULL)
   MEMORY_ReleaseDVector(__func__,"reference_shifted",reference_shifted,0);
  if (spec_nolog != NULL)
   MEMORY_ReleaseDVector(__func__,"spec_nolog",spec_nolog,0);

  // Return

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
}

/*                                                                           */
/*  ANALYSE_CurFitMethod ( Spectre, Spreflog, Absolu, Square ) :             */
/*  ==========================================================               */
/*                                                                           */
/*         Make a least-square fit to a non linear function whose non-       */
/*         linear parameters are shifts and stretches and linear para-       */
/*         meters are the concentrations implied in Beer-Lambert's law       */
/*         and are computed by singular value decomposition of cross         */
/*         sections matrix.                                                  */
/*                                                                           */
RC ANALYSE_CurFitMethod(INDEX indexFenoColumn,          // for imagers as OMI, TROPOMI, GEMS
                        const double *Spectre,          // raw spectrum
                        const double *SigmaSpec,        // error on raw spectrum
                        const double *Sref,             // reference spectrum
                        const int n_wavel,
                        double *residuals,        // pointer to store residuals (NULL if not needed)
                        double *Chisqr,           // chi square
                        int *pNiter,           // number of iterations
                        double speNormFact,
                        double refNormFact,
                        struct fit_properties *fit)
{
  // Declarations

  CROSS_REFERENCE *TabCross,*pTabCross;
  CROSS_RESULTS *pResults;
  double OldChisqr,                                      // chi square a step before
    *Y0,                                              // vector to fit; deduced from measurements
    *SigmaY,
    *Yfit,                                           // vector fitted
    *Deltap,                                         // increments for parameters
    *fitParamsF,
    *fitParamsC,
    *Sigmaa,                                         // errors on parameters
    *SpecTrav,*RefTrav,                              // substitution vectors
    Lamda,                                          // scaling factor used by curfit (not related to wavelength scale)
    scalingFactor;

  //  int i,j,k,l;                                             // indexes for loops and arrays
  int useErrors;
  int niter;
  RC rc;                                                 // return code

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  TabCross=Feno->TabCross;                               // symbol cross reference
  useErrors=((Feno->analysisMethod==OPTICAL_DENSITY_FIT) && (pAnalysisOptions->fitWeighting!=PRJCT_ANLYS_FIT_WEIGHTING_NONE) && (SigmaSpec!=NULL) && (Feno->SrefSigma!=NULL))?1:0;
  
  fitParamsC=fitParamsF=Deltap=Sigmaa=Y0=SpecTrav=RefTrav=SigmaY=NULL;          // pointers
  hFilterSpecLog=0;
  hFilterRefLog=0;
  rc=ERROR_ID_NO;                                      // return code

  /*  ==================  */
  /*  Buffers allocation  */
  /*  ==================  */

  if (
      ((Yfit=(double *)MEMORY_AllocDVector(__func__,"YFit",0,fit->DimL-1))==NULL) ||
      ((fitParamsC=(double *)MEMORY_AllocDVector(__func__,"fitParamsC",0,fit->DimC))==NULL) ||
      ((fit->NF!=0) && (((fitParamsF=(double *)MEMORY_AllocDVector(__func__,"fitParamsF",0,fit->NF-1))==NULL) ||
                   ((Deltap=(double *)MEMORY_AllocDVector(__func__,"Deltap",0,fit->NF-1))==NULL) ||
                   ((Sigmaa=(double *)MEMORY_AllocDVector(__func__,"Sigmaa",0,fit->NF-1))==NULL))) ||
      ((Y0=(double *)MEMORY_AllocDVector(__func__,"Y0",0,fit->DimL-1))==NULL) ||
      (useErrors && ((SigmaY=(double *)MEMORY_AllocDVector(__func__,"SigmaY",0,fit->DimL-1))==NULL)) ||
      ((SpecTrav=(double *)MEMORY_AllocDVector(__func__,"SpecTrav",0,n_wavel-1))==NULL) ||
      ((RefTrav=(double *)MEMORY_AllocDVector(__func__,"RefTrav",0,n_wavel-1))==NULL))

   rc=ERROR_ID_ALLOC;       // NB : call filter one time for determining the best Dim (security for filtering and interpolation)

  else
   {
    // Initializations

    memcpy(SpecTrav,Spectre,sizeof(double)*n_wavel);
    if (SigmaY!=NULL)
     memcpy(SigmaY,ANALYSE_zeros,sizeof(double)*fit->DimL);
    memcpy(RefTrav,Sref,sizeof(double)*n_wavel);

    memcpy(Yfit,ANALYSE_zeros,sizeof(double)*fit->DimL);
    memcpy(Y0,ANALYSE_zeros,sizeof(double)*fit->DimL);

    if (fit->NF)
     memcpy(Sigmaa,ANALYSE_ones,sizeof(double)*(fit->NF-1));

    // ----------------------------------------------
    // Fwhm adjustment between spectrum and reference
    // ----------------------------------------------

    if (!Feno->hidden && (Feno->useKurucz!=ANLYS_KURUCZ_SPEC)) {
      // Resolution adjustment using fwhm(lambda) found by Kurucz procedure for spectrum and reference

      if (pKuruczOptions->fwhmFit && (Feno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC))
        rc=AnalyseFwhmCorrectionK(Spectre,Sref,SpecTrav,RefTrav,n_wavel,indexFenoColumn);

      if (rc)
        goto EndCurFitMethod;
    }

    // -----------------------------
    // Filter spectrum and reference
    // -----------------------------

    // Low pass filtering

    if ((ANALYSE_plFilter->filterFunction!=NULL) &&                   // low pass filtering is requested
        (Feno->analysisType!=ANALYSIS_TYPE_FWHM_NLFIT) &&     // doesn't fit the resolution (FWHM) between the reference and the spectrum as a non linear parameter
        !Feno->hidden &&                                      // low pass filtering is disabled for calibration in order not to degrade the spectrum to calibrate

        (((rc=FILTER_Vector(ANALYSE_plFilter,&SpecTrav[LimMin],&SpecTrav[LimMin],NULL,LimN,PRJCT_FILTER_OUTPUT_LOW))!=0) ||
         ((rc=FILTER_Vector(ANALYSE_plFilter,&RefTrav[LimMin],&RefTrav[LimMin],NULL,LimN,PRJCT_FILTER_OUTPUT_LOW))!=0)))
     {
      rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_ANALYSIS,analyseIndexRecord,"Filter");
      goto EndCurFitMethod;
     }

    // High pass filtering (spectrum)

    if ((ANALYSE_phFilter->filterFunction!=NULL) &&           // high pass filtering is requested
        ((!Feno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((Feno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&
        (Feno->analysisType!=ANALYSIS_TYPE_FWHM_NLFIT) &&     // doesn't fit the resolution (FWHM) between the reference and the spectrum as a non linear parameter
        (Feno->analysisMethod==OPTICAL_DENSITY_FIT) &&     // only implemented in optical density fitting

                                                              // if offset is applied on spectrum, filter spectrum and reference at each iteration in Function
                                                              // otherwise, filter logarithms

        ((Feno->indexOffsetConst==ITEM_NONE) || (TabCross[Feno->indexOffsetConst].FitParam==ITEM_NONE)) &&
        ((Feno->indexOffsetOrder1==ITEM_NONE) || (TabCross[Feno->indexOffsetOrder1].FitParam==ITEM_NONE)) &&
        ((Feno->indexOffsetOrder2==ITEM_NONE) || (TabCross[Feno->indexOffsetOrder2].FitParam==ITEM_NONE)))
     {
      hFilterSpecLog=1;

      if (((rc=VECTOR_Log(&SpecTrav[LimMin],&SpecTrav[LimMin],LimN,"ANALYSE_CurFitMethod (Ref) "))!=0) ||           // !!!
          ((rc=FILTER_Vector(ANALYSE_phFilter,&SpecTrav[LimMin],&SpecTrav[LimMin],NULL,LimN,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=0))        // !!!

       goto EndCurFitMethod;
     }

    // High pass filtering (Reference)

    if ((ANALYSE_phFilter->filterFunction!=NULL) &&                   // high pass filtering is requested
        ((!Feno->hidden && ANALYSE_phFilter->hpFilterAnalysis) || ((Feno->hidden==1) && ANALYSE_phFilter->hpFilterCalib)) &&

        (Feno->analysisType!=ANALYSIS_TYPE_FWHM_NLFIT) &&     // doesn't fit the resolution (FWHM) between the reference and the spectrum as a non linear parameter
        (Feno->analysisType!=ANALYSIS_TYPE_FWHM_KURUCZ) &&
        (Feno->analysisMethod==OPTICAL_DENSITY_FIT) &&       // only implemented in optical density fitting
        ((Feno->indexSol==ITEM_NONE) || (TabCross[Feno->indexSol].FitParam==ITEM_NONE)))
     {
      hFilterRefLog=1;

      if (((rc=VECTOR_Log(&RefTrav[LimMin],&RefTrav[LimMin],LimN,"ANALYSE_CurFitMethod (Ref) "))!=0) ||
          ((rc=FILTER_Vector(ANALYSE_phFilter,&RefTrav[LimMin],&RefTrav[LimMin],NULL,LimN,PRJCT_FILTER_OUTPUT_HIGH_SUB))!=0))

       goto EndCurFitMethod;
     }

    Feno->ymean=(double)0.;
    doas_iterator my_iterator;
    for( int i=iterator_start(&my_iterator, global_doas_spectrum); i != ITERATOR_FINISHED;i=iterator_next(&my_iterator))
     Feno->ymean+=(double)RefTrav[i];

    Feno->ymean/=fit->DimL;

    // ---------------------------------
    // Calculation of second derivatives
    // ---------------------------------

    if (((rc=SPLINE_Deriv2(LambdaSpec,SpecTrav,SplineSpec,n_wavel,"ANALYSE_CurFitMethod (LambdaSpec) "))!=0) || // !!! ANALYSE_splineX -> LambdaSpec
        ((rc=SPLINE_Deriv2(ANALYSE_splineX,RefTrav,SplineRef,n_wavel,"ANALYSE_CurFitMethod (ANALYSE_splineX) "))!=0))

//    if (((rc=SPLINE_Deriv2(&LambdaSpec[LimMin],&SpecTrav[LimMin],&SplineSpec[LimMin],LimN,"ANALYSE_CurFitMethod (LambdaSpec) "))!=0) || // !!! ANALYSE_splineX -> LambdaSpec
//        ((rc=SPLINE_Deriv2(&ANALYSE_splineX[LimMin],&RefTrav[LimMin],&SplineRef[LimMin],LimN,"ANALYSE_CurFitMethod (ANALYSE_splineX) "))!=0))     

     goto EndCurFitMethod;

    // --------------------------------
    // Initialization of concentrations
    // --------------------------------

    int indexFeno; // TODO: this search fails for calls from alignreference because we work with a different copy of Feno
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (!TabFeno[indexFenoColumn][indexFeno].hidden &&
         (Feno==&TabFeno[indexFenoColumn][indexFeno]))
      break;

    for (int i=0;i<Feno->NTabCross;i++)                        // parameters initialization
     {
      if (TabCross[i].IndSvdA)
       {
        fitParamsC[TabCross[i].IndSvdA]=TabCross[i].InitConc;

        if ((WorkSpace[TabCross[i].Comp].type==WRK_SYMBOL_CROSS) && (indexFeno<NFeno) &&
            (TabCross[i].FitFromPrevious==1) && (TabCross[i].InitConc==(double)0.) &&
            (((Feno->analysisMethod==OPTICAL_DENSITY_FIT) && (TabCross[i].FitConc==0)) ||
             ((Feno->analysisMethod==INTENSITY_FIT) && (TabCross[i].FitConc==ITEM_NONE))))
         {
          bool found_previous = false;
          for (int indexFeno2=indexFeno-1;indexFeno2>=0 && !found_previous; indexFeno2--) {
           if (!TabFeno[indexFenoColumn][indexFeno2].hidden)
            {
             for (int j=0;j<TabFeno[indexFenoColumn][indexFeno2].NTabCross
                    && !found_previous ;j++) {
              if (TabFeno[indexFenoColumn][indexFeno2].TabCross[j].Comp==TabCross[i].Comp)
               {
                fitParamsC[TabCross[i].IndSvdA]=TabFeno[indexFenoColumn][indexFeno2].TabCrossResults[j].SlntCol;
                found_previous = true;
               }
             }
            }
          }
          if (!found_previous) {
           rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NO_RESULT_PREVIOUS_WINDOW,
                              WorkSpace[TabCross[i].Comp].symbolName, TabFeno[indexFenoColumn][indexFeno].windowName);
           goto EndCurFitMethod;
          }
         }
       }
     }

    if (useErrors)
     {
      for( int k=0,i=iterator_start(&my_iterator, global_doas_spectrum); i != ITERATOR_FINISHED; k++,i=iterator_next(&my_iterator))
       if ((SpecTrav[i]==(double)0.) || (RefTrav[i]==(double)0.))
        rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"try to divide errors by a zero");
       else {
        // error on sigma_log(I/I0) = sqrt( (sigma_I/I)^2 + (sigma_I0/I0)^2)
        double Ispec = speNormFact * Spectre[i]; // spectrum intensity
        double Iref = refNormFact * Sref[i]; // reference intensity
        SigmaY[k]=(double)sqrt( (SigmaSpec[i]*SigmaSpec[i])/(Ispec*Ispec)
                                + (Feno->SrefSigma[i]*Feno->SrefSigma[i])/(Iref*Iref) );
       }
      if (rc!=0)
       goto EndCurFitMethod;
     }

    if ((fit->NF==0) && ((rc=ANALYSE_Function(SpecTrav,RefTrav,SigmaY,Yfit,fit->DimL,fitParamsC,fitParamsF,indexFenoColumn, fit))<THREAD_EVENT_STOP))
     *Chisqr=(double)Fchisq(pAnalysisOptions->fitWeighting,(int)ANALYSE_nFree,Y0,Yfit,SigmaY,fit->DimL);
    else if (fit->NF)
     {
      for (int i=0; i<fit->NF; i++ ) { fitParamsF[i] = Fitp[i]; Deltap[i] = FitDeltap[i]; }

      /*  ==============  */
      /*  Loop on Chisqr  */
      /*  ==============  */

      *Chisqr    = (double) 0.;
      Lamda     = (double) 0.001;

      niter=0;

      do {
        OldChisqr = *Chisqr;

        if ((rc=Curfit(pAnalysisOptions->fitWeighting, niter, ANALYSE_nFree,SpecTrav,RefTrav,Y0,SigmaY,fit->DimL,
                       fitParamsC,fitParamsF,Deltap,Sigmaa,FitMinp,FitMaxp,fit->NF,Yfit,&Lamda,Chisqr,indexFenoColumn,fit))>=THREAD_EVENT_STOP)
         break;

        for (int i=0; i<fit->NF; i++ ) Deltap[i] *= 0.4;
        niter++;
      } while ( *Chisqr != 0.
                && fabs(*Chisqr-OldChisqr)/(*Chisqr) > pAnalysisOptions->convergence
                && (Feno->hidden || !pAnalysisOptions->maxIterations || niter<pAnalysisOptions->maxIterations) );

      if (pNiter!=NULL)
        *pNiter=niter;
     }

    if (rc<THREAD_EVENT_STOP) {
      /*  ====================  */
      /*  Residual Computation  */
      /*  ====================  */
      for( int k=0,i=iterator_start(&my_iterator, global_doas_spectrum); i != ITERATOR_FINISHED; k++,i=iterator_next(&my_iterator)) {
        ANALYSE_absolu[i]  =  (Yfit[k]-Y0[k]);
        if (Feno->analysisMethod!=OPTICAL_DENSITY_FIT)
         ANALYSE_t[i]=(ANALYSE_tc[i]!=(double)0.)?(double)1.+ANALYSE_absolu[i]/ANALYSE_tc[i]:(double)0.;
      }

      if (residuals != NULL)
        memcpy(residuals,ANALYSE_absolu,n_wavel * sizeof(*residuals));

      scalingFactor=(pAnalysisOptions->fitWeighting==PRJCT_ANLYS_FIT_WEIGHTING_NONE)?(*Chisqr):(double)1.;

      for (int i=0;i<Feno->NTabCross;i++) {
        pResults=&Feno->TabCrossResults[i];
        pTabCross=&TabCross[i];

        // in Intensity fitting mode, fitted concentrations are scaled by the cross section normalization factor:
        if (Feno->analysisMethod==INTENSITY_FIT) {
          if (pTabCross->IndSvdA > 0
              && WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS
              && pTabCross->FitConc > ITEM_NONE) {
            fitParamsF[pTabCross->FitConc] /= pTabCross->Fact;
            Sigmaa[pTabCross->FitConc] /= pTabCross->Fact;
          }
        }

        /*  =============================================== */
        /*  Scale concentrations for display, store results */
        /*  =============================================== */

        if (pTabCross->IndSvdA) { // Cross section, polynomial, linear offset, undersampling, resol, common residual

          if ((Feno->analysisMethod==OPTICAL_DENSITY_FIT && pTabCross->FitParam==ITEM_NONE) || pTabCross->IndSvdP) {
            // Linear fitting:
            // OD mode: cross sections, polynomial, pre-defined parameters (except non-linear offset)
            // Intensity fitting mode: polynomial, pre-defined parameters Resol, Usamp, Comon residual
            pResults->SlntCol=x[pTabCross->IndSvdA] = fitParamsC[pTabCross->IndSvdA];
            // In intensity fitting mode, use SvdP index:
            int linear_index = pTabCross->IndSvdP ? pTabCross->IndSvdP : pTabCross->IndSvdA;
            pResults->SlntErr=Sigma[pTabCross->IndSvdA]= (pTabCross->FitConc!=0)
              ? sqrt(fit->SigmaSqr[linear_index]*scalingFactor)
              : 0.;

            if (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CONTINUOUS
                && !Feno->hidden) {
              // Intensity fitting but polynomial is fitted linearly

              if ((pTabCross->IndSvdP) && (fabs(refNormFact)>EPSILON)) {                    // polynomial : the output differs from the display in order
                pResults->SlntCol*= speNormFact/refNormFact;                          //  to make the values geophysical
                                                                                              // the sign is inverted because it is better to compare with the
                // SVD                                                                        // log(spe/irrad) instead of log(irrad/spe)
                // to build the polynomial :
              } else if ( fabs(speNormFact)>EPSILON && refNormFact/speNormFact>EPSILON) {    // poly=x0+x1*(w-w0)+x2*(w-w0).*(w-w0)...;
                // in comparison to a polyfit, polyval in Matlab :
                pResults->SlntCol = -pResults->SlntCol;                                       //     >> p=polyfit(spectra(:,1),log(irrad(i,2)./spe(i,2)),2);
                                                                                              //     >> newp=polyval(p,spectra(:,1));
                if (!strcasecmp(WorkSpace[pTabCross->Comp].symbolName,"x0"))                // offset, shift can influence the polynomial
                  pResults->SlntCol -= log(refNormFact/speNormFact);                   // it is also recommended for a better comparison with Matlab
              }                                                                            // to orthogonalize all cross sections (O4, BrO, HCHO...)
            }
          } else {
            // Non-linear fitting: cross sections in Intensity fitting mode or Raman in OD mode
            pResults->SlntCol=x[pTabCross->IndSvdA] = (pTabCross->FitConc!=ITEM_NONE)
              ? fitParamsF[pTabCross->FitConc]
              : fitParamsC[TabCross[i].IndSvdA];
            pResults->SlntErr=Sigma[pTabCross->IndSvdA] = (pTabCross->FitConc!=ITEM_NONE)
              ? Sigmaa[pTabCross->FitConc]
              : 0.;

            if ((WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CONTINUOUS) && (fabs(refNormFact)>EPSILON))
             pResults->SlntCol*= speNormFact/refNormFact;
           }
         }
        else if (pTabCross->FitParam!=ITEM_NONE) {
          // SFP, non-linear offset, ...
          pResults->Param=(double)fitParamsF[pTabCross->FitParam]/pTabCross->Fact;
          pResults->SigmaParam = Sigmaa[pTabCross->FitParam]/pTabCross->Fact;
        } else {
          // Spectrum, Ref (no fit param for ref/spectrum itself, but shift/stretch can be fitted)
          pResults->Param=pTabCross->InitParam;
          pResults->SigmaParam=1.;
        }

        pResults->Shift = ( pTabCross->FitShift != ITEM_NONE ) ? (double) fitParamsF[pTabCross->FitShift] : pTabCross->InitShift;
        pResults->Stretch = ( pTabCross->FitStretch != ITEM_NONE ) ? (double) fitParamsF[pTabCross->FitStretch]*StretchFact1 : pTabCross->InitStretch*StretchFact1;
        pResults->Stretch2 = ( pTabCross->FitStretch2 != ITEM_NONE ) ? (double) fitParamsF[pTabCross->FitStretch2]*StretchFact2 : pTabCross->InitStretch2*StretchFact2;

        pResults->SigmaShift = (pTabCross->FitShift != ITEM_NONE) ? Sigmaa[pTabCross->FitShift] : (double)1.;
        pResults->SigmaStretch = (pTabCross->FitStretch != ITEM_NONE) ? Sigmaa[pTabCross->FitStretch]*StretchFact1 : (double)1.;
        pResults->SigmaStretch2 = (pTabCross->FitStretch2 != ITEM_NONE) ? Sigmaa[pTabCross->FitStretch2]*StretchFact2 : (double)1.;
       }
     }
   }

  /*  ===========  */
  /*  Free Memory  */
  /*  ===========  */

 EndCurFitMethod :

  for (int i=0;i<Feno->NTabCross;i++)
   {
    pTabCross=&TabCross[i];

    pTabCross->InitParam*=pTabCross->Fact;
    pTabCross->InitStretch*=StretchFact1;
    pTabCross->InitStretch2*=StretchFact2;
   }

  if (Sigmaa!=NULL)
   MEMORY_ReleaseDVector(__func__,"Sigmaa",Sigmaa,0);
  if (fitParamsC!=NULL)
   MEMORY_ReleaseDVector(__func__,"fitParamsC",fitParamsC,0);
  if (fitParamsF!=NULL)
   MEMORY_ReleaseDVector(__func__,"fitParamsF",fitParamsF,0);
  if (Deltap!=NULL)
   MEMORY_ReleaseDVector(__func__,"Deltap",Deltap,0);
  if (Yfit!=NULL)
   MEMORY_ReleaseDVector(__func__,"Yfit",Yfit,0);
  if (Y0!=NULL)
   MEMORY_ReleaseDVector(__func__,"Y0",Y0,0);
  if (SigmaY!=NULL)
   MEMORY_ReleaseDVector(__func__,"SigmaY",SigmaY,0);
  if (SpecTrav!=NULL)
   MEMORY_ReleaseDVector(__func__,"SpecTrav",SpecTrav,0);
  if (RefTrav!=NULL)
   MEMORY_ReleaseDVector(__func__,"RefTrav",RefTrav,0);

  // Return

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
}

// -------------------------------------------
// ANALYSE_Spectrum : Spectrum record analysis
// -------------------------------------------

RC ANALYSE_Spectrum(ENGINE_CONTEXT *pEngineContext,void *responseHandle)
{
  // Declarations

  PROJECT *pProject;                                                            // pointer to the project part of the engine context
  PRJCT_INSTRUMENTAL *pInstrumental;                                            // pointer to the instrumental part of the project
  BUFFERS *pBuffers;                                                            // pointer to the buffers part of the engine context
  RECORD_INFO *pRecord;                                                         // pointer to the record part of the engine context

  CROSS_REFERENCE *TabCross;                 // list of symbols hold by a analysis window
  CROSS_RESULTS *Results;                    // corresponding results
  char windowTitle[MAX_ITEM_TEXT_LEN];    // window title for graphs
  char tabTitle[MAX_ITEM_TEXT_LEN];
  char graphTitle[MAX_ITEM_TEXT_LEN];     // graph title
  INDEX i;                               // indexes for loops and arrays
  INDEX indexFenoColumn;

  doas_spectrum *old_range = NULL;

  double lambda0;
  double molecularRing_a;

  double *Spectre,                           // raw spectrum
    *SpectreK,                          // spectrum shifted on new calibration build by Kurucz
    *LambdaK,                           // calibration found by Kurucz
    *Sref,                              // reference spectrum
    *Trend,                             // fitted trend
    *offset,                            // fitted linear offset
    maxOffset,
    newVal,
    speNormFact;                          // normalization factor

  int NbFeno,Niter,
    displayFlag,                           // number of MDI child windows used for display analysis fits
    useKurucz,                             // flag set if Kurucz should be applied on spectra
    saveFlag;

  INDEX indexPage,indexLine,indexColumn;
  RC  rc,rcOutput;                                    // return code
  int nrc,irc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  pRecord=&pEngineContext->recordInfo;
  pBuffers=&pEngineContext->buffers;
  pProject=&pEngineContext->project;
  pInstrumental=&pProject->instrumental;

  indexFenoColumn=pRecord->i_crosstrack;

  const int n_wavel = NDET[pRecord->i_crosstrack];

  memcpy(ANALYSE_t,ANALYSE_zeros,sizeof(double)*n_wavel);
  memcpy(ANALYSE_tc,ANALYSE_zeros,sizeof(double)*n_wavel);

  speNormFact=1.;
  ZM=pRecord->Zm;
  TDET=pRecord->TDet;

  saveFlag=(int)pEngineContext->project.spectra.displayDataFlag;
  SpectreK=LambdaK=Sref=Trend=offset=NULL;
  molecularRing_a=(is_satellite(pEngineContext->project.instrumental.readOutFormat) ||
                  (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_APEX))?(double)1./cos(ZM*DegToRad)/((double)1./cos(ZM*DegToRad)+(double)1./cos(pRecord->zenithViewAngle*DegToRad)):(double)1.;
  useKurucz=0;


  NbFeno=0;
  nrc=irc=0;
  rc=rcOutput=ERROR_ID_NO;

  // Buffers allocation

  if (((Spectre=(double *)MEMORY_AllocDVector(__func__,"Spectre",0,n_wavel-1))==NULL) ||
      ((Sref=(double *)MEMORY_AllocDVector(__func__,"Sref",0,n_wavel-1))==NULL) ||
      ((Trend=(double *)MEMORY_AllocDVector(__func__,"Trend",0,n_wavel-1))==NULL) ||
      ((offset=(double *)MEMORY_AllocDVector(__func__,"offset",0,n_wavel-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Spectrum normalization

    memcpy(Spectre,pBuffers->spectrum,sizeof(double)*n_wavel);

    if ( (pRecord->rc=rc=VECTOR_NormalizeVector(Spectre-1,n_wavel,&speNormFact,"ANALYSE_Spectrum (Spectrum) "))!=ERROR_ID_NO ) {
     goto EndAnalysis;
    }
    // Apply Kurucz on spectrum

    for (int WrkFeno=0;WrkFeno<NFeno;WrkFeno++)
     if (!TabFeno[indexFenoColumn][WrkFeno].hidden && !TabFeno[indexFenoColumn][WrkFeno].rc &&
         ((TabFeno[indexFenoColumn][WrkFeno].useKurucz==ANLYS_KURUCZ_REF_AND_SPEC) ||
          (TabFeno[indexFenoColumn][WrkFeno].useKurucz==ANLYS_KURUCZ_SPEC)))

      useKurucz++;

    if (useKurucz || (THRD_id==THREAD_TYPE_KURUCZ))
     {
      if (((SpectreK=(double *)MEMORY_AllocDVector(__func__,"SpectreK",0,n_wavel-1))==NULL) ||
          ((LambdaK=(double *)MEMORY_AllocDVector(__func__,"LambdaK",0,n_wavel-1))==NULL)) {

       rc=ERROR_ID_ALLOC;

      } else {
        memcpy(SpectreK,Spectre,sizeof(double)*n_wavel);

        if (((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GEMS) || !(rc=GEMS_LoadCalib(pEngineContext,indexFenoColumn,responseHandle))) &&
            !(rc=KURUCZ_Spectrum(pBuffers->lambda,LambdaK,SpectreK,KURUCZ_buffers[indexFenoColumn].solar,pBuffers->instrFunction,
                                 1,"Calibration applied on spectrum",KURUCZ_buffers[indexFenoColumn].fwhmPolySpec,KURUCZ_buffers[indexFenoColumn].fwhmVector,KURUCZ_buffers[indexFenoColumn].fwhmDeriv2,saveFlag,
                                 KURUCZ_buffers[indexFenoColumn].indexKurucz,responseHandle,indexFenoColumn))) {

          for (int WrkFeno=0;WrkFeno<NFeno;++WrkFeno) {
            FENO *pTabFeno=&TabFeno[indexFenoColumn][WrkFeno];
            if (!pTabFeno->hidden) {
              memcpy(pTabFeno->LambdaK,LambdaK,sizeof(double)*n_wavel);
              memcpy(pTabFeno->Lambda,LambdaK,sizeof(double)*n_wavel);

              if ((pTabFeno->useKurucz==ANLYS_KURUCZ_SPEC) &&
                 ((pRecord->rc=rc=KURUCZ_ApplyCalibration(pTabFeno,LambdaK,indexFenoColumn))!=ERROR_ID_NO))
                goto EndAnalysis;
            }
          }

         if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GEMS) // && save file
          rc=netcdf_save_calib(LambdaK,pBuffers->spectrum,indexFenoColumn,n_wavel);
        }

        memcpy(SpectreK,Spectre,sizeof(double)*n_wavel); // !!!
       }

      if (rc>=THREAD_EVENT_STOP)
       {
        pRecord->rc=rc;
        goto EndAnalysis;
       }
     }

    pRecord->BestShift=(double)0.;

    if (THRD_id==THREAD_TYPE_ANALYSIS) {
      // Browse analysis windows
      for (int WrkFeno=0; (WrkFeno != NFeno) && (rc<THREAD_EVENT_STOP); ++WrkFeno) {
       indexPage=WrkFeno+plotPageAnalysis;
        Feno=&TabFeno[indexFenoColumn][WrkFeno];
        if (((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GOME1_NETCDF) ||
             (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_TROPOMI) ||
             (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
             (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMIV4) ||
             (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_APEX) ||
             (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GEMS))
         &&
            !Feno->useRefRow) continue;

        // MAXDOAS measurements : Thomas Wagner request -> add the possibility to select a reference spectrum with an elevation angle different from zenith.
        // This should be improved and move to engine.c in one of the functions dedicated to the selection of the reference spectrum

        if (!Feno->hidden &&
            (VECTOR_Equal(Spectre,Feno->Sref,n_wavel, 1.e-7) ||
             (!pEngineContext->satelliteFlag && Feno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC &&
              (( Feno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SZA &&             // Additional security (in principle, if one twilight is missing,
                 ((pRecord->localTimeDec<=12. && Feno->indexRefMorning==ITEM_NONE) || // use ref of the other twilight, if both twilights are missing, exit)
                  (pRecord->localTimeDec>12. && Feno->indexRefAfternoon==ITEM_NONE))) ||

               (Feno->refMaxdoasSelectionMode==ANLYS_MAXDOAS_REF_SCAN &&
                pRecord->elevationViewAngle>=pEngineContext->project.spectra.refAngle-pEngineContext->project.spectra.refTol &&
                pRecord->elevationViewAngle<=pEngineContext->project.spectra.refAngle+pEngineContext->project.spectra.refTol))))) {
          Feno->rc = -1;
        } else {
          Feno->rc = ERROR_ID_NO;
        }
        sprintf(windowTitle,"Analysis results for %s window",Feno->windowName);

        // OMI/OMPS/TROPOMI : at this step, the irradiance is not available yet (in separate files)

        switch (pEngineContext->project.instrumental.readOutFormat) {
           case PRJCT_INSTR_FORMAT_OMI:
           case PRJCT_INSTR_FORMAT_OMIV4:
           case PRJCT_INSTR_FORMAT_OMPS:
           case PRJCT_INSTR_FORMAT_TROPOMI:
             memcpy(Feno->Lambda,pBuffers->lambda,sizeof(double)*n_wavel);
             break;
           default:
             memcpy(Feno->Lambda,Feno->LambdaK,sizeof(double)*n_wavel);
             break;
        }

        if (ANALYSE_swathSize == 1) {
          sprintf(tabTitle,"%s results (%d/%d)",Feno->windowName,pEngineContext->indexRecord,pEngineContext->recordNumber);
        } else {
          sprintf(tabTitle,"%s results (record %d/%d, measurement %d/%d, row %d/%d)",
                  Feno->windowName,pEngineContext->indexRecord,pEngineContext->recordNumber,
                  1+pEngineContext->recordInfo.i_alongtrack,pEngineContext->n_alongtrack,
                  1+pEngineContext->recordInfo.i_crosstrack,pEngineContext->n_crosstrack);
        }

        displayFlag=Feno->displaySpectrum+                            //  force display spectrum
          Feno->displayResidue+                                       //  force display residue
          Feno->displayTrend+                                         //  force display trend
          Feno->displayRefEtalon+                                     //  force display alignment of reference on etalon
          Feno->displayFits+                                          //  force display fits
          Feno->displayPredefined+                                    //  force display predefined parameters
          Feno->displayRef;

        if (displayFlag)
         mediateResponseLabelPage(indexPage,pEngineContext->fileInfo.fileName,tabTitle,responseHandle);

        if (!Feno->hidden && (Feno->rcKurucz==ERROR_ID_NO) &&
            ((Feno->useKurucz==ANLYS_KURUCZ_SPEC) || !Feno->rc)) {
          memcpy(ANALYSE_absolu,ANALYSE_zeros,sizeof(double)*n_wavel);

          if (Feno->amfFlag ||
              ((Feno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC) && Feno->xsToConvolute) ||
              ( (Feno->linear_offset_mode == LINEAR_OFFSET_RAD) && (Feno->analysisMethod==OPTICAL_DENSITY_FIT))) {
            // fit a linear offset using the inverse of the spectrum
            Feno->Decomp=1;
          }

          // Local variables initializations

          Niter=0;
          NbFeno++;
          TabCross=Feno->TabCross;
          Results=Feno->TabCrossResults;

          // Reference spectrum

          if (Feno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) {
            switch(pEngineContext->project.instrumental.readOutFormat) {
            case PRJCT_INSTR_FORMAT_OMPS:
              rc=ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_NETCDF, "Automatic reference selection not implemented for this file format");
              break;
            case PRJCT_INSTR_FORMAT_GDP_BIN:                                          // GOME1NETCDF !!!!!!
              rc = GDP_BIN_get_vza_ref(pRecord->gome.pixelType, WrkFeno, Feno);
              break;
            case PRJCT_INSTR_FORMAT_GOME2:
              rc = GOME2_get_vza_ref(pRecord->satellite.vza, WrkFeno, Feno);
              break;
            case PRJCT_INSTR_FORMAT_SCIA_PDS:
              rc = SCIA_get_vza_ref(pRecord->satellite.vza, WrkFeno, Feno);
              break;
            default:
              break;
            }
          }
          if (rc)
            goto EndAnalysis;

          memcpy(Sref,Feno->Sref,sizeof(double)*n_wavel);
          Lambda=Feno->LambdaK;
          LambdaSpec=Feno->Lambda;

          // For OMI, OMPS, Tropomi and GOME-2, interpolate earthshine          // FRM4DOAS : check with Michel what to do with ASCII spectra
          // spectrum onto the solar reference wavelength grid
          // to replace by is_satellite !!!!

          if (pInstrumental->readOutFormat == PRJCT_INSTR_FORMAT_OMI
              || pInstrumental->readOutFormat == PRJCT_INSTR_FORMAT_OMIV4
              || pInstrumental->readOutFormat == PRJCT_INSTR_FORMAT_OMPS
              || pInstrumental->readOutFormat == PRJCT_INSTR_FORMAT_TROPOMI
              || pInstrumental->readOutFormat == PRJCT_INSTR_FORMAT_GOME1_NETCDF          // ??????????????????????????????
              || pInstrumental->readOutFormat == PRJCT_INSTR_FORMAT_GEMS 
              || pInstrumental->readOutFormat == PRJCT_INSTR_FORMAT_GOME2 ) {

            double *spec_deriv2 = malloc(n_wavel * sizeof(*spec_deriv2));
            rc = SPLINE_Deriv2(pBuffers->lambda, pBuffers->spectrum, spec_deriv2, n_wavel, __func__);
            if (rc == ERROR_ID_NO)
              rc = SPLINE_Vector(pBuffers->lambda, pBuffers->spectrum, spec_deriv2, n_wavel, Feno->LambdaRef, Spectre, n_wavel, SPLINE_CUBIC);
            free(spec_deriv2);
            if (rc == ERROR_ID_NO)
              rc=VECTOR_NormalizeVector(Spectre-1,n_wavel,&speNormFact,__func__);
            if (rc != ERROR_ID_NO)
             goto EndAnalysis;

            // after putting earthshine on reference grid, assign the
            // Kurucz-corrected reference grid LambdaK to the
            // earthshine spectrum as well
            LambdaSpec = Feno->LambdaK;
          }

          // Make a backup of spectral window limits + gaps

          old_range = spectrum_copy(Feno->fit_properties.specrange);

          if ((pInstrumental->readOutFormat==PRJCT_INSTR_FORMAT_OMI) &&
              pInstrumental->omi.pixelQFRejectionFlag &&
              (pEngineContext->buffers.pixel_QF!=NULL) && (Feno->omiRejPixelsQF!=NULL)) {
            unsigned short *pixelQF=(unsigned short *)pEngineContext->buffers.pixel_QF;

            memset(Feno->omiRejPixelsQF,0,sizeof(int)*Feno->NDET);
            int start = spectrum_start(old_range);
            int end = spectrum_end(old_range);
            for (int j= start; j<= end; j++) {
              if ( ((pixelQF[j]&pInstrumental->omi.pixelQFMask)!=0) &&
                   (spectrum_num_windows(Feno->fit_properties.specrange)<=pInstrumental->omi.pixelQFMaxGaps)) {
                spectrum_remove_pixel(Feno->fit_properties.specrange,j);
                Feno->omiRejPixelsQF[j]=1;
              }
            }

            if ((spectrum_num_windows(Feno->fit_properties.specrange) > pInstrumental->omi.pixelQFMaxGaps) ||
                ((rc=reinit_analysis(Feno, n_wavel))!=ERROR_ID_NO)) {
              spectrum_destroy(Feno->fit_properties.specrange);
              Feno->fit_properties.specrange = old_range;

              rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_OMI_PIXELQF);
              goto EndAnalysis;
            }
          } else if ((rc=ANALYSE_SvdInit(Feno,&Feno->fit_properties, n_wavel, Feno->LambdaK))!=ERROR_ID_NO) {
            goto restore_specrange;
          }

          // Global variables initializations

          // TODO: check about undersampling in case of GOME/GOME-2/Scia automatic reference
          // Undersampling
//          if (!Feno->useKurucz &&
//              (((rc=KURUCZ_ApplyCalibration(Feno,Feno->LambdaK,indexFenoColumn))!=ERROR_ID_NO) ||
//               ((rc=ANALYSE_SvdInit(&Feno->svd, n_wavel))!=ERROR_ID_NO)))
//
//            goto EndAnalysis;
//
//          if (Feno->useUsamp && (THRD_id!=THREAD_TYPE_KURUCZ)) {
//            // ANALYSE_UsampLocalFree();
//
//            if (((rc=ANALYSE_UsampLocalAlloc(0))!=ERROR_ID_NO) ||
//                ((rc=ANALYSE_UsampBuild(2,ITEM_NONE))!=ERROR_ID_NO))
//              goto EndAnalysis;
//          }

          // Display spectrum in the current analysis window

          if (strlen(pRecord->Nom))
           sprintf(windowTitle,"Analysis of %s in %s window",pRecord->Nom,Feno->windowName);
          else
           sprintf(windowTitle,"Analysis of spectrum %d/%d in %s window",pEngineContext->indexRecord,pEngineContext->recordNumber,Feno->windowName);

          if (Feno->displaySpectrum)
           {
            double *spectre_plot = malloc(n_wavel * sizeof(double));
            // in case spectrum & reference have different wavelength grids (shift in pixels): interpolate Spectre on the grid of the reference
            rc = SPLINE_Vector(LambdaSpec, Spectre, NULL, n_wavel, Feno->LambdaK, spectre_plot, n_wavel, SPLINE_LINEAR);

            double *curves[2][2] = {{Feno->LambdaK, spectre_plot},
                                    {Feno->LambdaK, Sref}};
            if (!Feno->longPathFlag)
             plot_curves(indexPage, curves, 2, Spectrum, forceAutoScale, "Spectrum and reference", responseHandle, Feno->fit_properties.specrange);
            else
             plot_curves(indexPage, curves, 1, Spectrum, forceAutoScale, "Spectrum", responseHandle, Feno->fit_properties.specrange);

            free(spectre_plot);
            if (rc)
             goto restore_specrange;
           }

          // Analysis method

#if defined(__DEBUG_) && __DEBUG_
          // DEBUG_Start(ENGINE_dbgFile,"Test",(analyseDebugMask=DEBUG_FCTTYPE_MATH|DEBUG_FCTTYPE_APPL),5,(analyseDebugVar=DEBUG_DVAR_YES),0); // !debugResetFlag++);
          // DEBUG_Start(ENGINE_dbgFile,"Test",(analyseDebugMask=DEBUG_FCTTYPE_MEM),7,(analyseDebugVar=DEBUG_DVAR_YES),0); // !debugResetFlag++);
#endif

          double *residuals = malloc(n_wavel * sizeof(*residuals));
          memcpy(residuals, ANALYSE_zeros, n_wavel * sizeof(*residuals));

          for(i = 0; i<n_wavel;i++)
           Feno->spikes[i] = 0;

          double rms_residual=0,rms_residual_old=0.;
          int max_repeats_spikes = MAX_REPEAT_CURFIT;
          int max_repeats_ring = MAX_REPEAT_CURFIT;
          int num_repeats_spikes = 0;
          int num_repeats_ring = 0;

          if (Feno->molecularCorrection && ((rc=Analyse_Molecular_Ring_Init(Feno,Feno->LambdaK,n_wavel))>THREAD_EVENT_STOP))
            goto restore_specrange;

          do {
            if ((num_repeats_ring && ((rc=Analyse_Molecular_Ring_Calculate(Feno,Feno->LambdaK,n_wavel,molecularRing_a))!=ERROR_ID_NO)) ||
               ((rc=ANALYSE_CurFitMethod(indexFenoColumn,
                                    (Feno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC)?SpectreK:Spectre, // raw spectrum
                                    (pRecord->useErrors)?pBuffers->sigmaSpec:NULL, // error on raw spectrum
                                    Sref, // reference spectrum
                                    n_wavel,
                                    residuals,
                                    &Feno->chiSquare, // returned stretch order 2
                                    &Niter, // number of iterations in Curfit
                                    speNormFact,
                                    Feno->refNormFact,
                                    &Feno->fit_properties))!=ERROR_ID_NO))
             break;

            rms_residual_old=rms_residual;
            rms_residual = root_mean_square(residuals, Feno->fit_properties.specrange);
          }
          while(!Feno->hidden && !rc && // no spike removal, no molecular ring for calibration

               ((!num_repeats_ring &&                                                           // spike removal should be performed only one time before molecular ring loop
                 remove_spikes(residuals, rms_residual * pAnalysisOptions->spike_tolerance, Feno->fit_properties.specrange, Feno->spikes) && // repeat as long as spikes are found
              (++num_repeats_spikes < max_repeats_spikes)) ||
              (Feno->molecularCorrection && (++num_repeats_ring <= max_repeats_ring))) &&

              ((num_repeats_ring<=1) || ((fabs(rms_residual_old)>EPSILON) && (fabs(rms_residual-rms_residual_old)>pAnalysisOptions->convergence))) &&
              !(rc=reinit_analysis(Feno, n_wavel))); // SVD matrix must be initialized again when pixels are removed.
          free(residuals);

          #if defined(__DEBUG_) && __DEBUG_
          // DEBUG_Stop("Test");
          #endif

          if (Feno->molecularCorrection)
           Analyse_Molecular_Ring_End(Feno,n_wavel);

          if (rc == THREAD_EVENT_STOP || ERROR_Fatal())
           goto restore_specrange;
          else if (rc>THREAD_EVENT_STOP)
           Feno->rc=rc;

          pRecord->BestShift+=(double)Feno->TabCrossResults[Feno->indexSpectrum].Shift;
          Feno->nIter=Niter;

          Feno->RMS = root_mean_square(ANALYSE_absolu, Feno->fit_properties.specrange);

          // Display residual spectrum

          if  (Feno->displayResidue) {
            if (Feno->analysisMethod!=OPTICAL_DENSITY_FIT)
              for (int j=SvdPDeb;j<=SvdPFin;j++)
                ANALYSE_absolu[j]=(ANALYSE_tc[j]!=(double)0.)?ANALYSE_absolu[j]/ANALYSE_tc[j]:(double)0.;

            sprintf(graphTitle,"%s (%.2le)",(Feno->analysisMethod!=OPTICAL_DENSITY_FIT)?"Normalized Residual":"Residual",Feno->RMS);

            double *curves[1][2] = {{Feno->LambdaK,ANALYSE_absolu}};
            plot_curves(indexPage,curves,1,Residual,0,graphTitle, responseHandle, Feno->fit_properties.specrange);
          }

          if (Feno->analysisMethod!=OPTICAL_DENSITY_FIT)
           for (int j=SvdPDeb;j<=SvdPFin;j++)
             ANALYSE_absolu[j]= (ANALYSE_t[j]>0.) ? log(ANALYSE_t[j]) : 0.;

          if (Feno->saveResidualsFlag && (Feno->residualSpectrum!=NULL))
           memcpy(Feno->residualSpectrum,&ANALYSE_absolu[SvdPDeb],sizeof(double)*Feno->fit_properties.DimL);
          else if (strlen(Feno->residualsFile))
           rc=AnalyseSaveResiduals(Feno->residualsFile,pEngineContext,n_wavel);

          if (rc!=ERROR_ID_NO)  
           goto restore_specrange;

          if  (Feno->displayResidue && (Feno->analysisMethod!=OPTICAL_DENSITY_FIT))
           {
            double * curves[1][2] = {{Feno->LambdaK,ANALYSE_absolu}};
            plot_curves(indexPage,curves,1,Residual,allowFixedScale,"OD Residual", responseHandle, Feno->fit_properties.specrange);
           }

          // Store fits

          memcpy(ANALYSE_secX,ANALYSE_zeros,sizeof(double)*n_wavel);
          memcpy(Trend,ANALYSE_zeros,sizeof(double)*n_wavel);
          memcpy(offset,ANALYSE_zeros,sizeof(double)*n_wavel);
          maxOffset=(double)0.;

          // if analysis has failed, don't try to plot results (some
          // buffers contain garbage data which can make Qwt crash)
          if (Feno->rc) goto SKIP_PLOTS;

          // Display Offset

          if  (Feno->displayPredefined &&
               ((Feno->indexOffsetConst!=ITEM_NONE) ||
                (Feno->indexOffsetOrder1!=ITEM_NONE) ||
                (Feno->indexOffsetOrder2!=ITEM_NONE)) &&

               ((TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE) ||
                (TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE) ||
                (TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE) ||
                (TabCross[Feno->indexOffsetConst].InitParam!=0.) ||
                (TabCross[Feno->indexOffsetOrder1].InitParam!=0.) ||
                (TabCross[Feno->indexOffsetOrder2].InitParam!=0.))) {
             lambda0=Feno->lambda0;

            doas_iterator my_iterator;
            for (int l=iterator_start(&my_iterator, Feno->fit_properties.specrange); l != ITERATOR_FINISHED; l=iterator_next(&my_iterator)) {
              // log(I+offset)=log(I)+log(1+offset/I)
              newVal = 1.-Feno->xmean*(Results[Feno->indexOffsetConst].Param+
                                       Results[Feno->indexOffsetOrder1].Param*(ANALYSE_splineX[l]-lambda0)+
                                       Results[Feno->indexOffsetOrder2].Param*(ANALYSE_splineX[l]-lambda0)*(ANALYSE_splineX[l]-lambda0))/Spectre[l];

              ANALYSE_absolu[l]+=(newVal > 0. ? log(newVal) : 0. )-ANALYSE_secX[l];
              ANALYSE_secX[l]=newVal > 0. ? log(newVal) : 0.;
            }

            double *curves[2][2] = {{Feno->LambdaK, ANALYSE_absolu},
                                    {Feno->LambdaK, ANALYSE_secX}};
            plot_curves(indexPage,curves,2,Residual,allowFixedScale,"Offset", responseHandle, Feno->fit_properties.specrange);
          }

          // Display fits

          for (int i=0;i<Feno->NTabCross;i++) {
            if (TabCross[i].IndSvdA) {
              if (((WorkSpace[TabCross[i].Comp].type==WRK_SYMBOL_CROSS) ||
                   (WorkSpace[TabCross[i].Comp].type==WRK_SYMBOL_PREDEFINED)) &&
                  Feno->displayFits && TabCross[i].display) {

                doas_iterator my_iterator;
                for (int k=1,l=iterator_start(&my_iterator, Feno->fit_properties.specrange); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator)) {
                  newVal=x[TabCross[i].IndSvdA]*Feno->fit_properties.A[TabCross[i].IndSvdA][k];
                  ANALYSE_absolu[l]+=newVal-ANALYSE_secX[l];
                  ANALYSE_secX[l]=newVal;
                }

                sprintf(graphTitle,"%s (%.2le)",WorkSpace[TabCross[i].Comp].symbolName,Results[i].SlntCol);
                double *curves[2][2] = {{Feno->LambdaK, ANALYSE_absolu},
                                        {Feno->LambdaK, ANALYSE_secX}};
                plot_curves(indexPage,curves,2,Residual,allowFixedScale,graphTitle, responseHandle, Feno->fit_properties.specrange);

              } else if ((WorkSpace[TabCross[i].Comp].type==WRK_SYMBOL_CONTINUOUS) && Feno->displayTrend) {

                doas_iterator my_iterator;
                if ((tolower(WorkSpace[TabCross[i].Comp].symbolName[0])=='x') ||
                    (tolower(WorkSpace[TabCross[i].Comp].symbolName[2])=='x')) { // polynomial
                  for (int k=1,l=iterator_start(&my_iterator, Feno->fit_properties.specrange); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator))
                    Trend[l]+=x[TabCross[i].IndSvdA]*Feno->fit_properties.A[TabCross[i].IndSvdA][k];
                } else if (!strncmp( WorkSpace[TabCross[i].Comp].symbolName, "offl", 4) ) { // linear offset
                  for (int k=1,l=iterator_start(&my_iterator, Feno->fit_properties.specrange); l != ITERATOR_FINISHED; k++,l=iterator_next(&my_iterator))
                    offset[l]+=x[TabCross[i].IndSvdA]*Feno->fit_properties.A[TabCross[i].IndSvdA][k];

                  for (int l=iterator_start(&my_iterator, Feno->fit_properties.specrange); l != ITERATOR_FINISHED; l=iterator_next(&my_iterator))
                    if (fabs(offset[l])>maxOffset)
                      maxOffset=fabs(offset[l]);
                }
              }
            }
          }

          // Display Trend

          if (Feno->displayTrend) {
            doas_iterator my_iterator;
            for (int l=iterator_start(&my_iterator, Feno->fit_properties.specrange); l != ITERATOR_FINISHED; l=iterator_next(&my_iterator))
             ANALYSE_absolu[l]+=Trend[l]-ANALYSE_secX[l];

            double *curves[2][2] = {{Feno->LambdaK,ANALYSE_absolu},
                                    {Feno->LambdaK,Trend}};
            plot_curves(indexPage,curves,2,Residual,allowFixedScale,"Polynomial", responseHandle, Feno->fit_properties.specrange);

            if (maxOffset > 0.) {
              doas_iterator my_iterator;
              if (Feno->analysisMethod==INTENSITY_FIT)
               for (int l=iterator_start(&my_iterator, Feno->fit_properties.specrange); l != ITERATOR_FINISHED; l=iterator_next(&my_iterator))
                offset[l]=-offset[l];           // inverse the sign in order to have the same display as in SVD method


              for (int l=iterator_start(&my_iterator, Feno->fit_properties.specrange); l != ITERATOR_FINISHED; l=iterator_next(&my_iterator))
               ANALYSE_absolu[l]+=offset[l]-Trend[l];

              double *curves[2][2] = {{Feno->LambdaK, ANALYSE_absolu},
                                      {Feno->LambdaK, offset}};
              plot_curves(indexPage, curves, 2, Residual, allowFixedScale, "Linear offset",responseHandle, Feno->fit_properties.specrange);
            }
          }  // end displayTrend

         SKIP_PLOTS:

          if (!Feno->rc)
           nrc++;
          else
           irc++;

          if (displayFlag && saveFlag) {
            indexLine = Feno->displayLineIndex;
            indexColumn=2;

            mediateResponseCellDataString(indexPage,indexLine,indexColumn,tabTitle,responseHandle);

            indexLine +=2;
            if(num_repeats_spikes) {
             mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn,responseHandle,
                                            "Spike removal: the following pixels were excluded after %d iterations",num_repeats_spikes);
             for (i = 0; i< n_wavel; i++)
              if(Feno->spikes[i])
               mediateResponseCellInfoNoLabel(indexPage,indexLine++,indexColumn+1, responseHandle,"%d",i);

             indexLine++;
            }

            if (Feno->molecularCorrection)
             {
              mediateResponseCellInfoNoLabel(indexPage,indexLine++,indexColumn,responseHandle,
                                            "Molecular ring processed in %d iterations",num_repeats_ring-1);
              indexLine++;
             }

            mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"OD ChiSquare","%.5le",Feno->chiSquare);
            mediateResponseCellInfo(indexPage,indexLine++,indexColumn,responseHandle,"RMS Residual","%.5le",Feno->RMS);
            mediateResponseCellInfo(indexPage,indexLine,indexColumn,responseHandle,"Iterations","%d",Niter);

            indexLine+=2;

            mediateResponseCellDataString(indexPage,indexLine,indexColumn+1,"[CONC/Param]",responseHandle);
            mediateResponseCellDataString(indexPage,indexLine,indexColumn+2,"Shift",responseHandle);
            mediateResponseCellDataString(indexPage,indexLine,indexColumn+3,"Stretch",responseHandle);

            indexLine++;

            for (i=0;i<Feno->NTabCross;i++) {
              mediateResponseCellDataString(indexPage,indexLine,indexColumn,WorkSpace[TabCross[i].Comp].symbolName,responseHandle);
              // -------------------------------------------------------------------
              if (TabCross[i].IndSvdA)
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+1,responseHandle,"%10.3e +/-%10.3e",Results[i].SlntCol,Results[i].SlntErr);
              else if (TabCross[i].FitParam!=ITEM_NONE)
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+1,responseHandle,"%10.3e +/-%10.3e",Results[i].Param,Results[i].SigmaParam);
              else
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+1,responseHandle,"%10.3e",Results[i].Param);
              // -------------------------------------------------------------------
              if (TabCross[i].FitShift!=ITEM_NONE)
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+2,responseHandle,"%10.3e +/-%10.3e",Results[i].Shift,Results[i].SigmaShift);
              else
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+2,responseHandle,"%10.3e",Results[i].Shift);
              // -------------------------------------------------------------------
              if (TabCross[i].FitStretch!=ITEM_NONE)
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+3,responseHandle,"%10.3e +/-%10.3e",Results[i].Stretch,Results[i].SigmaStretch);
              else
               mediateResponseCellInfoNoLabel(indexPage,indexLine,indexColumn+3,responseHandle,"%10.3e",Results[i].Stretch);
              indexLine++;
             }  // for (i=0;i<Feno->NTabCross;i++)
           }  // if (displayFlag && saveFlag)

          // Recover spectral window limits and gaps which were possibly modified after spike removal
        restore_specrange:
          if (!spectrum_isequal(old_range,Feno->fit_properties.specrange)) {
            //AnalyseCopyFenetre(Feno->fit_properties.Fenetre,&Feno->fit_properties.Z,oldFenetre,oldZ);
            spectrum_destroy(Feno->fit_properties.specrange);
            Feno->fit_properties.specrange = old_range;
            Feno->fit_properties.DimL = spectrum_length(Feno->fit_properties.specrange);
            Feno->Decomp = 1;
          } else {
            spectrum_destroy(old_range);
          }

         }  // if (!Feno->hidden && (Feno->rcKurucz==ERROR_ID_NO) &&
       }  // for (WrkFeno=0;(WrkFeno<NFeno) && (rc!=THREAD_EVENT_STOP);WrkFeno++)
     }  // if (THRD_id==THREAD_TYPE_ANALYSIS)

    if (NbFeno)
      pRecord->BestShift/=(double)NbFeno;

    // Output : save all records (including those with a not null return code for all analysis windows - for example : log error on spectrum)
    //
    // OUTPUT_SaveResults : moved after EndAnalysis
    // test on nrc (no return code due to fatal error) removed but in this case, initialize the return code to -1 in order to force the output

    EndAnalysis :

    if (!nrc)
     {
      for (int WrkFeno=0;WrkFeno<NFeno;WrkFeno++)
        if (!TabFeno[indexFenoColumn][WrkFeno].hidden)
         TabFeno[indexFenoColumn][WrkFeno].rc=rc;

      if (pRecord->rc==ERROR_ID_NO)
       pRecord->rc=-1;
     }

//    if ((pEngineContext->mfcDoasisFlag || (pEngineContext->lastSavedRecord!=pEngineContext->indexRecord)) &&
//        (   ((THRD_id==THREAD_TYPE_ANALYSIS) && pProject->asciiResults.analysisFlag && (!pEngineContext->project.asciiResults.successFlag /* || nrc */))
//            || ((THRD_id==THREAD_TYPE_KURUCZ) && pProject->asciiResults.calibFlag) ) )

//      rcOutput=OUTPUT_SaveResults(pEngineContext,indexFenoColumn);

//    if (!rc)
//      rc=rcOutput;
   }

  if (Spectre!=NULL)
   MEMORY_ReleaseDVector(__func__,"Spectre",Spectre,0);
  if (SpectreK!=NULL)
   MEMORY_ReleaseDVector(__func__,"SpectreK",SpectreK,0);
  if (LambdaK!=NULL)
   MEMORY_ReleaseDVector(__func__,"LambdaK",LambdaK,0);
  if (Sref!=NULL)
   MEMORY_ReleaseDVector(__func__,"Sref",Sref,0);
  if (Trend!=NULL)
   MEMORY_ReleaseDVector(__func__,"Trend",Trend,0);
  if (offset!=NULL)
   MEMORY_ReleaseDVector(__func__,"offset",offset,0);

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  //  TO SIMULATE ERROR ON SPECTRA
  //  if ((pEngineContext->indexRecord%2)==0)
  //   rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_LOG,analyseIndexRecord);

  return (irc)?-1:rc;
}

// ===============
// DATA PROCESSING
// ===============

// --------------------------------------------------------------------------
// ANALYSE_ResetData : Release and reset all data used for a project analysis
// --------------------------------------------------------------------------

void ANALYSE_ResetData(void)
{
  // Declarations

  CROSS_REFERENCE *pTabCross;
  FENO *pTabFeno;
  INDEX indexWorkSpace,indexFeno,indexTabCross,indexParam,indexFenoColumn;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Filter

  if (ANALYSE_plFilter->filterFunction!=NULL)
   {
    MEMORY_ReleaseDVector(__func__,"FILTER_function",ANALYSE_plFilter->filterFunction,1);
    ANALYSE_plFilter->filterFunction=NULL;
   }

  if (ANALYSE_phFilter->filterFunction!=NULL)
   {
    MEMORY_ReleaseDVector(__func__,"FILTER_function",ANALYSE_phFilter->filterFunction,1);
    ANALYSE_phFilter->filterFunction=NULL;
   }

  // List of all symbols in a project

  for (indexWorkSpace=0;indexWorkSpace<NWorkSpace;indexWorkSpace++)
   MATRIX_Free(&WorkSpace[indexWorkSpace].xs,__func__);

  memset(WorkSpace,0,sizeof(WRK_SYMBOL)*MAX_SYMB);
  NWorkSpace=0;

  // List of analysis windows in a project

  for (indexFenoColumn=0;indexFenoColumn<MAX_SWATHSIZE;indexFenoColumn++)
   {
    for (indexFeno=0;indexFeno<MAX_FENO;indexFeno++)
     {
      pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

      // Reference spectra and wavelength scale

      free(pTabFeno->ref_description);
      pTabFeno->ref_description = NULL;

      if (pTabFeno->SrefEtalon!=NULL)
       MEMORY_ReleaseDVector(__func__,"SrefEtalon",pTabFeno->SrefEtalon,0);
      if (pTabFeno->Sref!=NULL)
       MEMORY_ReleaseDVector(__func__,"Sref",pTabFeno->Sref,0);
      if (pTabFeno->SrefSigma!=NULL)
       MEMORY_ReleaseDVector(__func__,"SrefSigma",pTabFeno->SrefSigma,0);
      if (pTabFeno->Lambda!=NULL)
       MEMORY_ReleaseDVector(__func__,"Lambda",pTabFeno->Lambda,0);
      if (pTabFeno->LambdaK!=NULL)
       MEMORY_ReleaseDVector(__func__,"LambdaK",pTabFeno->LambdaK,0);
      if (pTabFeno->LambdaRef!=NULL)
       MEMORY_ReleaseDVector(__func__,"LambdaRef",pTabFeno->LambdaRef,0);
      if (pTabFeno->LambdaRadAsRef1!=NULL)
       MEMORY_ReleaseDVector(__func__,"LambdaRadAsRef1",pTabFeno->LambdaRadAsRef1,0);
      if (pTabFeno->LambdaRadAsRef2!=NULL)
       MEMORY_ReleaseDVector(__func__,"LambdaRadAsRef2",pTabFeno->LambdaRadAsRef2,0);
      if (pTabFeno->SrefRadAsRef1!=NULL)
       MEMORY_ReleaseDVector(__func__,"SrefRadAsRef1",pTabFeno->SrefRadAsRef1,0);
      if (pTabFeno->SrefRadAsRef2!=NULL)
       MEMORY_ReleaseDVector(__func__,"SrefRadAsRef2",pTabFeno->SrefRadAsRef2,0);
      if (pTabFeno->Deriv2RadAsRef1!=NULL)
       MEMORY_ReleaseDVector(__func__,"Deriv2RadAsRef1",pTabFeno->Deriv2RadAsRef1,0);
      if (pTabFeno->Deriv2RadAsRef2!=NULL)
       MEMORY_ReleaseDVector(__func__,"Deriv2RadAsRef2",pTabFeno->Deriv2RadAsRef2,0);
      if (pTabFeno->residualSpectrum!=NULL)
       MEMORY_ReleaseDVector(__func__,"residualSpectrum",pTabFeno->residualSpectrum,0); 

      // SVD matrices

      FIT_PROPERTIES_free(__func__,&pTabFeno->fit_properties);

      // spike array
      if(pTabFeno->spikes != NULL)
       MEMORY_ReleaseBuffer(__func__, "spikes", pTabFeno->spikes);

      // Coefficients for building polynomial fitting fwhm

      for (indexParam=0;(indexParam<MAX_KURUCZ_FWHM_PARAM);indexParam++)
       {
        if (pTabFeno->fwhmPolyRef[indexParam]!=NULL)
         MEMORY_ReleaseDVector(__func__,"fwhmPolyRef",pTabFeno->fwhmPolyRef[indexParam],0);
        if (pTabFeno->fwhmVector[indexParam]!=NULL)
         MEMORY_ReleaseDVector(__func__,"fwhmVector",pTabFeno->fwhmVector[indexParam],0);
        if (pTabFeno->fwhmDeriv2[indexParam]!=NULL)
         MEMORY_ReleaseDVector(__func__,"fwhmDeriv2",pTabFeno->fwhmDeriv2[indexParam],0);
       }

      // Cross sections and derivatives

      for (indexTabCross=0;indexTabCross<MAX_FIT;indexTabCross++)
       {
        pTabCross=&pTabFeno->TabCross[indexTabCross];

        if (pTabCross->vector!=NULL)
         MEMORY_ReleaseDVector(__func__,"vector",pTabCross->vector,0);
        if (pTabCross->Deriv2!=NULL)
         MEMORY_ReleaseDVector(__func__,"Deriv2",pTabCross->Deriv2,0);
        if (pTabCross->vectorBackup!=NULL)
         MEMORY_ReleaseDVector(__func__,"vectorBackup",pTabCross->vectorBackup,0);
        if (pTabCross->Deriv2Backup!=NULL)
         MEMORY_ReleaseDVector(__func__,"Deriv2Backup",pTabCross->Deriv2Backup,0);
        if (pTabCross->molecularCrossSection!=NULL)
         MEMORY_ReleaseDVector(__func__,"molecularCrossSection",pTabCross->molecularCrossSection,0);
       }

      memset(pTabFeno,0,sizeof(FENO));

      pTabFeno->Shift=pTabFeno->Stretch=pTabFeno->Stretch2=0.;
      pTabFeno->refNormFact=1.;

      pTabFeno->refMaxdoasSelectionMode=ANLYS_MAXDOAS_REF_SZA;

      pTabFeno->indexSpectrum=
        pTabFeno->indexReference=
        pTabFeno->indexFwhmParam[0]=
        pTabFeno->indexFwhmParam[1]=
        pTabFeno->indexFwhmParam[2]=
        pTabFeno->indexFwhmParam[3]=
        pTabFeno->indexFwhmConst=
        pTabFeno->indexFwhmOrder1=
        pTabFeno->indexFwhmOrder2=
        pTabFeno->indexSol=
        pTabFeno->indexOffsetConst=
        pTabFeno->indexOffsetOrder1=
        pTabFeno->indexOffsetOrder2=
        pTabFeno->indexCommonResidual=
        pTabFeno->indexUsamp1=
        pTabFeno->indexUsamp2=
        pTabFeno->indexResol=ITEM_NONE;

      pTabFeno->indexRefMorning=
        pTabFeno->indexRefAfternoon=
        pTabFeno->indexRef=
        pTabFeno->indexRefScanBefore=
        pTabFeno->indexRefScanAfter=ITEM_NONE;

      pTabFeno->ZmRefMorning=
        pTabFeno->ZmRefAfternoon=
        pTabFeno->Zm=
        pTabFeno->refSZA=
        pTabFeno->refSZADelta=
        pTabFeno->refMaxdoasSZA=
        pTabFeno->refMaxdoasSZADelta=
        pTabFeno->TDet=
        pTabFeno->Tm=
        pTabFeno->TimeDec=(double)9999.;

      memset(&pTabFeno->refDate,0,sizeof(pTabFeno->refDate));

      // Cross reference

      for (indexTabCross=0;indexTabCross<MAX_FIT;indexTabCross++)
       {
        pTabCross=&pTabFeno->TabCross[indexTabCross];

        // -------------------------------------------
        pTabCross->IndOrthog=
          pTabCross->IndSubtract=
          pTabCross->indexPukite1=
          pTabCross->indexPukite2=
          pTabCross->FitConc=
          pTabCross->FitParam=
          pTabCross->FitShift=
          pTabCross->FitStretch=
          pTabCross->FitStretch2=
          pTabCross->Comp=
          pTabCross->amfType=ITEM_NONE;
        // -------------------------------------------
        pTabCross->TypeStretch=(int)0;
        // -------------------------------------------
        pTabCross->display=(char)0;
        // -------------------------------------------
        pTabCross->InitConc=
          pTabCross->InitParam=
          pTabCross->InitShift=
          pTabCross->InitStretch=
          pTabCross->InitStretch2=
          pTabCross->DeltaConc=
          pTabCross->DeltaParam=
          pTabCross->DeltaShift=
          pTabCross->DeltaStretch=
          pTabCross->DeltaStretch2=
          pTabCross->I0Conc=
          pTabCross->MinConc=
          pTabCross->MaxConc=(double)0.;
        // -------------------------------------------
        pTabCross->MinParam=
          pTabCross->MinShift=(double)-99.;
        pTabCross->MaxParam=
          pTabCross->MaxShift=(double)99.;
        // -------------------------------------------
        pTabCross->Fact=1.;
       }
     }
   }

  // Kurucz buffers

  KURUCZ_Free();
  ANALYSE_UsampGlobalFree();

  // Output part

  OUTPUT_ResetData();

  NFeno=0;
  ANALYSE_swathSize=1;                                                          // by default, one column at least for analysis windows

  // Release global buffers

  if (Fitp!=NULL)
   MEMORY_ReleaseDVector(__func__,"Fitp",Fitp,0);
  if (FitDeltap!=NULL)
   MEMORY_ReleaseDVector(__func__,"FitDeltap",FitDeltap,0);
  if (FitMinp!=NULL)
   MEMORY_ReleaseDVector(__func__,"FitMinp",FitMinp,0);
  if (FitMaxp!=NULL)
   MEMORY_ReleaseDVector(__func__,"FitMaxp",FitMaxp,0);
  if (a!=NULL)
   MEMORY_ReleaseDVector(__func__,"a",a,1);
  if (b!=NULL)
   MEMORY_ReleaseDVector(__func__,"b",b,1);
  if (x!=NULL)
   MEMORY_ReleaseDVector(__func__,"x",x,0);
  if (Sigma!=NULL)
   MEMORY_ReleaseDVector(__func__,"Sigma",Sigma,0);
  if (ANALYSE_shift!=NULL)
   MEMORY_ReleaseDVector(__func__,"ANALYSE_shift",ANALYSE_shift,0);
  if (ANALYSE_pixels!=NULL)
   MEMORY_ReleaseDVector(__func__,"ANALYSE_pixels",ANALYSE_pixels,0);
  if (ANALYSE_splineX!=NULL)
   MEMORY_ReleaseDVector(__func__,"ANALYSE_splineX",ANALYSE_splineX,0);
  if (ANALYSE_absolu!=NULL)
   MEMORY_ReleaseDVector(__func__,"ANALYSE_absolu",ANALYSE_absolu,0);
  if (ANALYSE_t!=NULL)
   MEMORY_ReleaseDVector(__func__,"ANALYSE_t",ANALYSE_t,0);
  if (ANALYSE_tc!=NULL)
   MEMORY_ReleaseDVector(__func__,"ANALYSE_tc",ANALYSE_tc,0);
  if (ANALYSE_xsTrav!=NULL)
   MEMORY_ReleaseDVector(__func__,"ANALYSE_xsTrav",ANALYSE_xsTrav,0);
  if (ANALYSE_xsTrav2!=NULL)
   MEMORY_ReleaseDVector(__func__,"ANALYSE_xsTrav2",ANALYSE_xsTrav2,0);
  if (ANALYSE_secX!=NULL)
   MEMORY_ReleaseDVector(__func__,"ANALYSE_secX",ANALYSE_secX,0);
  if (SplineSpec!=NULL)
   MEMORY_ReleaseDVector(__func__,"SplineSpec",SplineSpec,0);
  if (SplineRef!=NULL)
   MEMORY_ReleaseDVector(__func__,"SplineRef",SplineRef,0);
  if (ANALYSE_zeros!=NULL)
   MEMORY_ReleaseDVector(__func__,"ANALYSE_zeros",ANALYSE_zeros,0);
  if (ANALYSE_ones!=NULL)
   MEMORY_ReleaseDVector(__func__,"ANALYSE_ones",ANALYSE_ones,0);

  Fitp=
    FitDeltap=
    FitMinp=
    FitMaxp=
    a=
    b=
    x=
    Sigma=
    ANALYSE_shift=
    ANALYSE_pixels=
    ANALYSE_splineX=
    ANALYSE_absolu=
    ANALYSE_secX=
    ANALYSE_t=
    ANALYSE_tc=
    ANALYSE_xsTrav=
    ANALYSE_xsTrav2=
    SplineSpec=
    SplineRef=
    ANALYSE_zeros=
    ANALYSE_ones=NULL;

  for (int i=0;i<NSFP;i++)
   {
    MATRIX_Free(&ANALYSIS_slitMatrix[i],"ANALYSE_ResetData (2)");
    ANALYSIS_slitParam[i]=(double)0.;
   }

  MATRIX_Free(&ANALYSIS_slitK,"ANALYSE_ResetData (4)");


  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,0);
  #endif
}

// --------------------------------------------------------
// ANALYSE_LoadSlit : Load slit function for fwhm correction
// --------------------------------------------------------

RC ANALYSE_LoadSlit(const PRJCT_SLIT *pSlit,int kuruczFlag)
{
  // Declarations

  const SLIT *pSlitFunction;
  RC    rc;

  // Initializations

  pSlitFunction=&pSlit->slitFunction;
  rc=ERROR_ID_NO;

  if (pSlitFunction->slitType!=SLIT_TYPE_NONE) {
    // Slit type selection

    if ((pSlitFunction->slitType==SLIT_TYPE_FILE) || pSlitFunction->slitWveDptFlag) {
      // Load file

      if (!strlen(pSlitFunction->slitFile))
        rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_MSGBOX_FIELDEMPTY,"Slit File");
      else
        rc=MATRIX_Load(pSlitFunction->slitFile,&ANALYSIS_slitMatrix[0], 0, 0, 0., 0., 1, 0, __func__);
    }

    if (pSlitFunction->slitWveDptFlag && ((pSlitFunction->slitType==SLIT_TYPE_FILE) ||(pSlitFunction->slitType==SLIT_TYPE_ERF) || (pSlitFunction->slitType==SLIT_TYPE_AGAUSS) || (pSlitFunction->slitType==SLIT_TYPE_SUPERGAUSS) || (pSlitFunction->slitType==SLIT_TYPE_VOIGT))) {
      if (!strlen(pSlitFunction->slitFile2))
        rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_MSGBOX_FIELDEMPTY,"Slit File 2");
      else
        rc=MATRIX_Load(pSlitFunction->slitFile2, &ANALYSIS_slitMatrix[1], 0, 0, 0., 0., 1, 0, "ANALYSE_LoadSlit 2");
    }

    if (pSlitFunction->slitWveDptFlag && (pSlitFunction->slitType==SLIT_TYPE_SUPERGAUSS)) {
      if (!strlen(pSlitFunction->slitFile3))
        rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_MSGBOX_FIELDEMPTY,"Slit File 3");
      else
        rc=MATRIX_Load(pSlitFunction->slitFile3, &ANALYSIS_slitMatrix[2], 0, 0, 0., 0., 1, 0, "ANALYSE_LoadSlit 3");
    }

    ANALYSIS_slitParam[0]=pSlitFunction->slitParam;
    ANALYSIS_slitParam[1]=pSlitFunction->slitParam2;
    ANALYSIS_slitParam[2]=pSlitFunction->slitParam3;
  }

  if (!rc && kuruczFlag) {
    if (!strlen(pSlit->kuruczFile))
      rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_MSGBOX_FIELDEMPTY,"Slit solar ref. file");
    else {
      // check filename extension:
      const char *ext = strrchr(pSlit->kuruczFile, '.');
      if (ext == NULL) {
        ext = "";
      }
      if (!strcmp(ext, ".nc")) { // netCDF file
        rc=MATRIX_netcdf_Load(pSlit->kuruczFile, &ANALYSIS_slitK, 0, 0, 0., 0., 1, 0, NULL, __func__);
      } else {
        rc=MATRIX_Load(pSlit->kuruczFile, &ANALYSIS_slitK, 0, 0, 0., 0., 1, 0, __func__);
      }

      int n_columns = (pSlitFunction->slitType==SLIT_TYPE_NONE) ? 1+ANALYSE_swathSize : 2;

      if (ANALYSIS_slitK.nc != n_columns) {
        rc=ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_XS_COLUMNS, "Slit solar ref. file", ANALYSIS_slitK.nc, n_columns);
      }
    }
  }

  // Return

  return rc;
} 

RC ANALYSE_CheckLambda(WRK_SYMBOL *pWrkSymbol, const double *lambda, const int n_wavel)
{
  // Declarations

  char fileName[MAX_ITEM_TEXT_LEN];
  RC rc;

  // Initialization

  rc=ERROR_ID_NO;

  FILES_RebuildFileName(fileName,pWrkSymbol->crossFileName,1);

  if ((pWrkSymbol->xs.nl!=n_wavel) || !VECTOR_Equal(pWrkSymbol->xs.matrix[0],lambda,n_wavel,(double)1.e-7))
   rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_XS_BAD_WAVELENGTH,pWrkSymbol->crossFileName);

  // Return

  return rc;
}

INDEX AnalyseGetSymbolIndex(CROSS_REFERENCE *TabCross,INDEX firstTabCross,INDEX endTabCross,INDEX indexTabCross,const char *symbolName)
 {
  // Declarations

  INDEX indexSymbol;
  unsigned int symbolLength;

  // Initialization

  symbolLength=strlen(symbolName);

  // Search for symbol in list

  for (indexSymbol=firstTabCross;indexSymbol<endTabCross;indexSymbol++)
    if ((indexTabCross!=indexSymbol) &&
        (symbolLength==strlen(WorkSpace[TabCross[indexSymbol].Comp].symbolName)) &&
        !strcasecmp(symbolName,WorkSpace[TabCross[indexSymbol].Comp].symbolName))
      break;

  // Return

  return (indexSymbol<endTabCross)?indexSymbol:ITEM_NONE;
 }

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_LoadCross
// -----------------------------------------------------------------------------
// PURPOSE       Load data from the molecules pages
// -----------------------------------------------------------------------------

RC ANALYSE_LoadCross(ENGINE_CONTEXT *pEngineContext, const ANALYSIS_CROSS *crossSectionList,int nCross, const double *lambda,INDEX indexFenoColumn)
{
  // Declarations

  CROSS_REFERENCE *pEngineCross;                                     // pointer of the current cross section in the engine list
  const ANALYSIS_CROSS *pCross;                                      // pointer of the current cross section in the mediate list
  FENO *pTabFeno;                                                    // pointer to the current analysis window
  const char *pOrthoDiffSymbol[MAX_FIT], *symbolName,                // for each cross section in list, hold cross section to use for orthogonalization or difference
    *pRingSymbol[MAX_FIT];
  const char* diffOrtho = "Differential XS";
  INDEX indexSymbol,indexSvd,                                        // resp. indexes of item in list and of symbol
    firstTabCross,endTabCross,indexTabCross,i,indexTabCrossOriginal; // indexes for browsing list of cross sections symbols
  WRK_SYMBOL *pWrkSymbol;                                            // pointer to a general description of a symbol
  RC rc;

  // Debug

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_CONFIG|DEBUG_FCTTYPE_MEM);
#endif
  
  // Initializations
  const int n_wavel = NDET[indexFenoColumn];
  pWrkSymbol=NULL;
  pTabFeno=&TabFeno[indexFenoColumn][NFeno];
  firstTabCross=pTabFeno->NTabCross;
  rc=ERROR_ID_NO;
  
  for (indexTabCross=0;
       (indexTabCross<nCross) && (pTabFeno->NTabCross<MAX_FIT) && !rc;
       indexTabCross++) {
    pEngineCross=&pTabFeno->TabCross[pTabFeno->NTabCross];
    pCross=&crossSectionList[indexTabCross];

    // Get cross section name from analysis properties dialog box

    symbolName=pCross->symbol;

    // Search for symbol in list

    for (indexSymbol=0;indexSymbol<NWorkSpace;indexSymbol++) {
      pWrkSymbol=&WorkSpace[indexSymbol];

      bool same_file = false;

      if ((pWrkSymbol->type==WRK_SYMBOL_CROSS) &&
          !strcasecmp(pWrkSymbol->symbolName,symbolName) &&
          !strcasecmp(pWrkSymbol->amfFileName,pCross->amfFile) &&
          ( (rc=is_same_file(pWrkSymbol->crossFileName,pCross->crossSectionFile, &same_file) != ERROR_ID_NO)
            || same_file ) // stop loop (with error) if file comparison returns error, or (without error) if files are same
          )

        break;
    }

    // Add a new cross section

    if (rc==ERROR_ID_NO && (indexSymbol==NWorkSpace) && (NWorkSpace<MAX_SYMB)) {
      // Allocate a new symbol

      pWrkSymbol=&WorkSpace[indexSymbol];

      pWrkSymbol->type=WRK_SYMBOL_CROSS;
      strcpy(pWrkSymbol->symbolName,symbolName);
      strcpy(pWrkSymbol->crossFileName,pCross->crossSectionFile);
      strcpy(pWrkSymbol->amfFileName,pCross->amfFile);

      // Load cross section from file

      if (strcasecmp(pWrkSymbol->symbolName,"1/Ref")) {
        if (!strlen(pWrkSymbol->crossFileName)) {
          return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_XS_FILENAME,pWrkSymbol->symbolName);
        }
        
       char *ext;
       
       if ((ext=strrchr(pWrkSymbol->crossFileName,'.'))!=NULL) 
        ext++;
       else
        ext="xs";
       
       if ((strcmp(ext,"nc") && ((rc=MATRIX_Load(pCross->crossSectionFile,&pWrkSymbol->xs,0,0,0.,0.,
                                                 (pCross->crossType!=ANLYS_CROSS_ACTION_NOTHING) ?1:0,1,__func__))!=0)) ||
          (!strcmp(ext,"nc") && ((rc=MATRIX_netcdf_Load(pCross->crossSectionFile,&pWrkSymbol->xs,0,0,0.,0.,
                                                        (pCross->crossType!=ANLYS_CROSS_ACTION_NOTHING) ? 1 : 0,
                                                        1, pEngineContext->project.instrumental.use_row,__func__))!=0)))
        {
          return rc;
        }

        NWorkSpace++;
      }

      // Check cross sections have the correct number of columns.
      // We have the following possibilities:
      //
      //  - normal or I0 convolution: 2 columns
      //  - ring convolution: 4 columns
      //  - no convolution:  1 + ANALYSE_swathSize columns
      //    (preconvolved xs for each detector row)
      if ( ( (pTabFeno->hidden || pTabFeno->useKurucz) // we are in the calibration procedure, or in an analysis window which relies on Kuruz calibration
             && pEngineContext->project.kurucz.fwhmFit) // slit function was fit in Kurucz calibration procedure
           || pEngineContext->project.slit.slitFunction.slitType != SLIT_TYPE_NONE) { // slit function is specified in the project's slit tab
        // the analysis uses convolution (Kurucz-fitted slit function or slit function set in slit tab):
        switch (pCross->crossType) {
        case ANLYS_CROSS_ACTION_CONVOLUTE:
        case ANLYS_CROSS_ACTION_CONVOLUTE_I0:
          if (pWrkSymbol->xs.nc != 2) {
            rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_XS_COLUMNS, pCross->symbol, pWrkSymbol->xs.nc, 2);
          }
          break;
        case ANLYS_CROSS_ACTION_CONVOLUTE_RING:
          if (pWrkSymbol->xs.nc != 4) {
            rc = ERROR_SetLast(__func__, ERROR_TYPE_FATAL, ERROR_ID_XS_RING, pCross->symbol);
          }
          break;
        case ANLYS_CROSS_ACTION_NOTHING:
        case ANLYS_CROSS_ACTION_INTERPOLATE:
          // interpolated/preset cross section is allowed in a project
          // using online convolution, but normally there should be 1
          // column for each detector row:
          if (pWrkSymbol->xs.nc != (1+ANALYSE_swathSize) ) {
            int tmp_rc = ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_XS_COLUMNS, pCross->symbol, pWrkSymbol->xs.nc, 1+ANALYSE_swathSize);
            if (pWrkSymbol->xs.nc != 2) {
              // we allow cross sections with 2 columns, but give a
              // warning. When the number of columns does not match 2
              // or (1+ANALYSE_swathSize), something is not right =>
              // return the error:
              rc = tmp_rc;
            }
          }
          break;
        }
      } else {
        // analysis uses preconvolved cross sections
        if (pWrkSymbol->xs.nc != (1+ANALYSE_swathSize) ) {
          int tmp_rc = ERROR_SetLast(__func__, ERROR_TYPE_WARNING, ERROR_ID_XS_COLUMNS, pCross->crossSectionFile, pWrkSymbol->xs.nc, 1+ANALYSE_swathSize);
          if (pWrkSymbol->xs.nc != 2) {
            // same as above for ACTION_NOTHING/ACTION_INTERPOLATE:
            // for preconvolved cross sections, if the number of
            // columns does not match 1+indexFenocolumn, we allow
            // 2-column cross sections with a warning, otherwise,
            // return an error
            rc = tmp_rc;
          }
        } else if (pCross->crossType == ANLYS_CROSS_ACTION_CONVOLUTE ||
                   pCross->crossType == ANLYS_CROSS_ACTION_CONVOLUTE_I0 ||
                   pCross->crossType == ANLYS_CROSS_ACTION_CONVOLUTE_RING) {
          rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_CONVOLUTION, pCross->symbol);
        }
      }
    }

    if ((rc==ERROR_ID_NO) && (indexSymbol<NWorkSpace) && (pTabFeno->NTabCross<MAX_FIT)) {
      // Allocate vectors for cross section and its second derivative for analysis processing

      if (
          ((pEngineCross->vector=(double *)MEMORY_AllocDVector(__func__,"vector",0,n_wavel-1))==NULL) ||
          ((pEngineCross->Deriv2=(double *)MEMORY_AllocDVector(__func__,"Deriv2",0,n_wavel-1))==NULL) ||
         (((pCross->correctionType==ANLYS_CORRECTION_TYPE_MOLECULAR_RING) || (pCross->correctionType==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE)) &&
         (((pEngineCross->vectorBackup=(double *)MEMORY_AllocDVector(__func__,"vectorBackup",0,n_wavel-1))==NULL) ||
          ((pEngineCross->Deriv2Backup=(double *)MEMORY_AllocDVector(__func__,"Deriv2Backup",0,n_wavel-1))==NULL)))
         )

       rc=ERROR_ID_ALLOC;

      else
       {
        pEngineCross->crossAction=pCross->crossType;
        pEngineCross->crossCorrection=pCross->correctionType;
        pEngineCross->amfType=pCross->amfType;
        pEngineCross->filterFlag=pCross->requireFilter;
        pEngineCross->isPukite=0;
        pEngineCross->molecularCrossIndex=ITEM_NONE;
        pEngineCross->indexPukite1=ITEM_NONE;
        pEngineCross->indexPukite2=ITEM_NONE;

        if ((pEngineCross->crossAction==ANLYS_CROSS_ACTION_NOTHING) && ((pTabFeno->gomeRefFlag && pTabFeno->useRadAsRef2!=1) || pEngineContext->refFlag))
          rc=ANALYSE_CheckLambda(pWrkSymbol,lambda,n_wavel);

        if (rc==ERROR_ID_NO)
         {
          pEngineCross->Comp=indexSymbol;
          pEngineCross->IndSvdA=++pTabFeno->fit_properties.DimC;
          pTabFeno->xsToConvolute+=((pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE) ||
                                    (pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0) ||
                                    (pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_RING))?1:0;

          pTabFeno->xsToConvoluteI0+=(pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?1:0;
          pTabFeno->xsPukite+=((pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_SLOPE) || (pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_PUKITE) || (pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE))?1:0;
          pTabFeno->molecularCorrection+=((pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING) || (pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE))?1:0;
          
          pOrthoDiffSymbol[pTabFeno->NTabCross]=(pCross->subtractFlag)?pCross->subtract:pCross->orthogonal;
          pRingSymbol[pTabFeno->NTabCross]=((pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING) || (pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE))?pCross->molecularRing:NULL;

          pEngineCross->display=pCross->requireFit;                    // fit display
          pEngineCross->InitConc=pCross->initialCc;                    // initial concentration
          pEngineCross->FitConc=pCross->requireCcFit;                  // modify concentration
          pEngineCross->FitFromPrevious=pCross->constrainedCc;

          pEngineCross->DeltaConc=(pEngineCross->FitConc)?pCross->deltaCc:(double)0.;   // delta on concentration
          pEngineCross->I0Conc=(pEngineCross->crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0)?pCross->ccIo:(double)0.;
          pEngineCross->MinConc=pCross->ccMin;
          pEngineCross->MaxConc=pCross->ccMax;

          // Swap columns of original matrix A in order to have in the end of the matrix, cross sections with fixed concentrations

          if (pEngineCross->FitConc!=0)   // the difference between SVD and Marquardt+SVD hasn't to be done yet but later
           {
            for (i=pTabFeno->NTabCross-1;i>=0;i--)
             if (((indexSvd=pTabFeno->TabCross[i].IndSvdA)!=0) && !pTabFeno->TabCross[i].FitConc)
              {
               pTabFeno->TabCross[i].IndSvdA=pEngineCross->IndSvdA;
               pEngineCross->IndSvdA=indexSvd;
              }

            if (pTabFeno->analysisMethod!=OPTICAL_DENSITY_FIT)     // In the intensity fitting method, FitConc is an index
             pEngineCross->FitConc=pTabFeno->fit_properties.NF++;                   // in the non linear parameters vectors

            pTabFeno->fit_properties.nFit++;
           }
          else if (pTabFeno->analysisMethod!=OPTICAL_DENSITY_FIT)
           pEngineCross->FitConc=ITEM_NONE;                              // so if the parameter hasn't to be fitted, index is ITEM_NONE

          pTabFeno->NTabCross++;
         }
       }
     }
   }

  // Process symbols dependent on other symbols (orthogonalization, pukite, molecular ring...)

  if (rc==ERROR_ID_NO)
   {
    CROSS_REFERENCE *TabCross=pTabFeno->TabCross;
    for (indexTabCrossOriginal=0,indexTabCross=firstTabCross,endTabCross=pTabFeno->NTabCross;(indexTabCross<endTabCross) && !rc;indexTabCross++)
     {
      pEngineCross=&TabCross[indexTabCross];
      pCross=&crossSectionList[indexTabCrossOriginal];

      if (!pEngineCross->isPukite)
       indexTabCrossOriginal++;

      // !!! Pukite -> add additional vectors for Pukite (except for pre-convolved cross sections)

      if ((pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_SLOPE) || (pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_PUKITE) || (pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE))
       {
        if ((pEngineCross->indexPukite1=AnalyseGetPukiteIndex(&TabCross[firstTabCross],endTabCross,pCross->symbol,1))!=ITEM_NONE)
         TabCross[pEngineCross->indexPukite1].isPukite=2;
        if ((pEngineCross->indexPukite2=AnalyseGetPukiteIndex(&TabCross[firstTabCross],endTabCross,pCross->symbol,2))!=ITEM_NONE)
         TabCross[pEngineCross->indexPukite2].isPukite=2;
        
        // !!! Pukite, check the different cases and generate error messages for inconsistency

        if (((pEngineCross->indexPukite1==ITEM_NONE) && AnalyseAddPukiteTerm(pEngineContext,pEngineCross,indexFenoColumn,1,&pEngineCross->indexPukite1)) ||
               ((pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_PUKITE) && (pEngineCross->indexPukite2==ITEM_NONE) && AnalyseAddPukiteTerm(pEngineContext,pEngineCross,indexFenoColumn,2,&pEngineCross->indexPukite2)))

            rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_PUKITE,pTabFeno->windowName,WorkSpace[pEngineCross->Comp].symbolName);
           
          

        else
         {
          if (pEngineCross->indexPukite1!=ITEM_NONE)
           pOrthoDiffSymbol[pEngineCross->indexPukite1]=(strncasecmp(pCross->orthogonal,"None",4))?diffOrtho:pCross->orthogonal;
          if (pEngineCross->indexPukite2!=ITEM_NONE)
           pOrthoDiffSymbol[pEngineCross->indexPukite2]=(strncasecmp(pCross->orthogonal,"None",4))?diffOrtho:pCross->orthogonal;
         }
       }

      // Molecular ring -> use additional vectors for the molecular cross section and its pukite terms

      if (((pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING) ||(pEngineCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE)) && (pRingSymbol[indexTabCross]!=NULL))
       {
        int molecularCrossIndex;
        if (((molecularCrossIndex=pEngineCross->molecularCrossIndex=AnalyseGetSymbolIndex(TabCross,firstTabCross,endTabCross,indexTabCross,pRingSymbol[indexTabCross]))!=ITEM_NONE) &&
           (((TabCross[molecularCrossIndex].molecularCrossSection=(double *)MEMORY_AllocDVector(__func__,"molecularCrossSection",0,n_wavel-1))==NULL) )) // lambdaEff,sigmaEff ||
// lambdaEff,sigmaEff            ((TabCross[molecularCrossIndex].crossCorrection==ANLYS_CORRECTION_TYPE_PUKITE) &&
// lambdaEff,sigmaEff             (TabCross[molecularCrossIndex].crossAction==ANLYS_CROSS_ACTION_CONVOLUTE_I0) &&
// lambdaEff,sigmaEff           ((((indexPukite1=TabCross[molecularCrossIndex].indexPukite1)!=ITEM_NONE) && ((TabCross[indexPukite1].molecularCrossSection=(double *)MEMORY_AllocDVector(__func__,"molecularCrossSection",0,n_wavel-1))==NULL)) ||
// lambdaEff,sigmaEff            (((indexPukite2=TabCross[molecularCrossIndex].indexPukite2)!=ITEM_NONE) && ((TabCross[indexPukite2].molecularCrossSection=(double *)MEMORY_AllocDVector(__func__,"molecularCrossSection",0,n_wavel-1))==NULL)))   )))

         rc=ERROR_ID_ALLOC;
       }

      // No orthogonalization

      if (!strncasecmp(pOrthoDiffSymbol[indexTabCross],"None",4) || pEngineCross->isPukite)
       {
        pEngineCross->IndOrthog=ITEM_NONE;
        pEngineCross->IndSubtract=ITEM_NONE;
       }

      // Orthogonalization to orthogonal base

      else if (!strncasecmp(pOrthoDiffSymbol[indexTabCross],"Differential XS",15))
       pEngineCross->IndOrthog=ORTHOGONAL_BASE;

      // Orthogonalization to another cross section

      else if (!pEngineCross->isPukite)
       {
        indexSymbol=AnalyseGetSymbolIndex(TabCross,firstTabCross,endTabCross,indexTabCross,pOrthoDiffSymbol[indexTabCross]);

        if ((indexSymbol!=ITEM_NONE) && (indexSymbol<endTabCross))
         {
          pEngineCross->IndOrthog=(pCross->subtractFlag)?TabCross[indexSymbol].IndOrthog:indexSymbol;
          pEngineCross->IndSubtract=(pCross->subtractFlag)?indexSymbol:ITEM_NONE;
         }
        else
         pEngineCross->IndOrthog=ITEM_NONE;
       }
     }

    for (indexTabCross=firstTabCross,endTabCross=pTabFeno->NTabCross;indexTabCross<endTabCross;indexTabCross++)
     {
      pEngineCross=&pTabFeno->TabCross[indexTabCross];

      // Symbol should be set to be orthogonalized to base
      if (pEngineCross->IndOrthog>=0)
       {
        if (pTabFeno->TabCross[pEngineCross->IndOrthog].IndOrthog==ITEM_NONE)
         {
          rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_ORTHOGONAL_BASE,
                           WorkSpace[pTabFeno->TabCross[pEngineCross->IndOrthog].Comp].symbolName,
                           WorkSpace[pEngineCross->Comp].symbolName);

          pTabFeno->TabCross[pEngineCross->IndOrthog].IndOrthog=ORTHOGONAL_BASE;
         }
       }
     }
   }

#if defined(__DEBUG_) && __DEBUG_ && defined(__DEBUG_DOAS_CONFIG_) && __DEBUG_DOAS_CONFIG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  // Return

  return rc;
}

// -------------------------------------------------
// ANALYSE_LoadLinear : Load continuous functions
// -------------------------------------------------

RC ANALYSE_LoadLinear(ANALYSE_LINEAR_PARAMETERS *linearList,int nLinear,INDEX indexFenoColumn)
{
  // Declarations

  INDEX indexItem,indexSymbol,indexOrder;                                       // indexes for loops and arrays
  CROSS_REFERENCE *pTabCross;                                                   // pointer to an element of the symbol cross reference table of an analysis window
  CROSS_RESULTS *pResults;                                                      // pointer to results
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a general description of a symbol
  char buttonText[15];                                                         // term in polynomial
  ANALYSE_LINEAR_PARAMETERS *pList;                                             // pointer to description of an item in list
  FENO *pTabFeno;                                                               // pointer to description of the current analysis window
  INDEX indexSvd,indexTabCross;                                                 // extra index for swapping
  int polyFlag;                                                                 // polynomial flag (-1 for invpoly, 0 for offset, 1 for poly)
  int polyOrder,baseOrder;                                                      // polynomial order, base order
  RC rc;                                                                        // return code

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  pTabFeno=&TabFeno[indexFenoColumn][NFeno];
  rc=ERROR_ID_NO;
  const int n_wavel = NDET[indexFenoColumn];

  // Browse lines

  for (indexItem=0;(indexItem<nLinear) && !rc;indexItem++) {
    pList=&linearList[indexItem];

    polyFlag=0;
    polyOrder=pList->polyOrder;
    baseOrder=pList->baseOrder;

    if (polyOrder < 0) {
      // if polyorder is "none", do nothing for this linear component
      continue;
    }

    if (!strcasecmp(pList->symbolName,"Polynomial (x)")) {
      polyFlag=1;
    } else if (!strcasecmp(pList->symbolName, "Offset (rad)") ) {
      pTabFeno->linear_offset_mode = LINEAR_OFFSET_RAD;
    } else if (!strcasecmp(pList->symbolName, "Offset (ref)") ) {
      pTabFeno->linear_offset_mode = LINEAR_OFFSET_REF;
    }

    for (indexOrder=0;indexOrder<=polyOrder;indexOrder++) {
      // Set symbol name

      if (polyFlag!=0)
       sprintf(buttonText,"x%d",indexOrder);
      else
       sprintf(buttonText,"offl%d",indexOrder);

      // Search for symbol in list

      for (indexSymbol=0;indexSymbol<NWorkSpace;indexSymbol++)
       {
        pWrkSymbol=&WorkSpace[indexSymbol];

        if ((pWrkSymbol->type==WRK_SYMBOL_CONTINUOUS) &&
            !strcasecmp(pWrkSymbol->symbolName,buttonText))

         break;
       }

      if ((indexSymbol==NWorkSpace) && (NWorkSpace<MAX_SYMB))
       {
        // Allocate a new symbol (but do not allocate a vector !)

        pWrkSymbol=&WorkSpace[indexSymbol];

        pWrkSymbol->type=WRK_SYMBOL_CONTINUOUS;
        strcpy(pWrkSymbol->symbolName,buttonText);

        NWorkSpace++;
       }

      if ((indexSymbol<NWorkSpace) && (pTabFeno->NTabCross<MAX_FIT))
       {
        pTabCross=&pTabFeno->TabCross[pTabFeno->NTabCross];
        pResults=&pTabFeno->TabCrossResults[pTabFeno->NTabCross];

        if ((pTabCross->vector=(double *)MEMORY_AllocDVector(__func__,"vector",0,n_wavel-1))==NULL)
          rc=ERROR_ID_ALLOC;

        else
         {
          memcpy(pTabCross->vector,ANALYSE_ones,sizeof(double)*n_wavel);

          if (polyFlag && (baseOrder>=indexOrder))
           {
            pTabFeno->OrthoSet[pTabFeno->NOrtho++]=pTabFeno->NTabCross;
            pTabCross->IndOrthog=ITEM_NONE;
           }
          else if (polyFlag)
           pTabCross->IndOrthog=ORTHOGONAL_BASE;
          else
           pTabCross->IndOrthog=ITEM_NONE;

          pTabCross->Comp=indexSymbol;
          pTabCross->IndSvdA=++pTabFeno->fit_properties.DimC;
          pTabCross->crossAction=ANLYS_CROSS_ACTION_NOTHING;

          pTabFeno->NTabCross++;

          // Swap columns of original matrix A in order to have in the end of the matrix, cross sections with fixed concentrations

          for (indexTabCross=pTabFeno->NTabCross-2;indexTabCross>=0;indexTabCross--)
           if (((indexSvd=pTabFeno->TabCross[indexTabCross].IndSvdA)!=0) &&
               (pTabFeno->TabCross[indexTabCross].FitConc==0) &&
               (pTabFeno->TabCross[indexTabCross].DeltaConc==(double)0.))
            {
             pTabFeno->TabCross[indexTabCross].IndSvdA=pTabCross->IndSvdA;
             pTabCross->IndSvdA=indexSvd;
            }

          if (pTabFeno->analysisMethod==INTENSITY_FIT)        // Marquardt-Levenberg + SVD
           pTabCross->IndSvdP=++(pTabFeno->fit_properties.DimP);

          pTabFeno->fit_properties.nFit++;

          // Results

          pResults->StoreSlntCol=pList->storeFit;                               // flag set if slant column is to be written into output file
          pResults->StoreSlntErr=pList->storeError;                             // flag set if error on slant column is to be written into output file
          pResults->SlntFact=1.;
         }
       }
     }
   }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
  #endif

  // Return

  return rc;
}

// ------------------------------------------------------------------------------------------------
// ANALYSE_LoadShiftStretch : Load shift and stretch for cross sections implied in SVD decomposition
// ------------------------------------------------------------------------------------------------

RC ANALYSE_LoadShiftStretch(const ANALYSIS_SHIFT_STRETCH *shiftStretchList,int nShiftStretch,INDEX indexFenoColumn)
{
  // Declarations

  INDEX indexItem,indexSymbol,indexTabCross,indexSymbolInList,indexCross,       // indexes for loops and arrays
    indexShift,indexStretch,indexStretch2;
  const char *symbol;                                                                // copy of list of symbols
  CROSS_REFERENCE *pTabCross;                                                   // pointer to an element of the symbol cross reference table of an analysis window
  CROSS_RESULTS *pResults;                                                      // pointer to results part relative to the symbol
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a general description of a symbol
  SZ_LEN symbolLength;                                                          // length in characters of a symbol name
  const ANALYSIS_SHIFT_STRETCH *pList;                                          // pointer to description of an item in list
  FENO *pTabFeno;                                                               // pointer to description of the current analysis window
  int oldNF;
  RC rc;                                                                        // return code

  // Initializations

  pTabFeno=&TabFeno[indexFenoColumn][NFeno];
  indexShift=indexStretch=indexStretch2=oldNF=pTabFeno->fit_properties.NF;
  rc=ERROR_ID_NO;

  for (indexItem=0;(indexItem<nShiftStretch) && !rc;indexItem++)
   {
    pList=&shiftStretchList[indexItem];

    for (indexSymbolInList=0;indexSymbolInList<pList->nSymbol;indexSymbolInList++)
     {
      symbol=pList->symbol[indexSymbolInList];
      symbolLength=strlen(symbol);

      // Search for the symbols in the cross sections list

      for (indexCross=0;indexCross<pTabFeno->NTabCross;indexCross++)
       {
        pWrkSymbol=&WorkSpace[pTabFeno->TabCross[indexCross].Comp];

        if ((strlen(pWrkSymbol->symbolName)==symbolLength) &&
            !strcasecmp(pWrkSymbol->symbolName,symbol))

         break;
       }

      if (indexCross<pTabFeno->NTabCross)
       indexSymbol=pTabFeno->TabCross[indexCross].Comp;
      else
       {
        for (indexSymbol=0;indexSymbol<NWorkSpace;indexSymbol++)
         {
          pWrkSymbol=&WorkSpace[indexSymbol];

          if ((strlen(pWrkSymbol->symbolName)==symbolLength) &&
              !strcasecmp(pWrkSymbol->symbolName,symbol))

           break;
         }
       }

      // Allocate a new symbol

      if ((indexSymbol==NWorkSpace) && (NWorkSpace<MAX_SYMB))
       {
        pWrkSymbol=&WorkSpace[indexSymbol];

        pWrkSymbol->type=WRK_SYMBOL_SPECTRUM;
        strcpy(pWrkSymbol->symbolName,symbol);

        NWorkSpace++;
       }

      if (indexSymbol<NWorkSpace)
       {
        // Search for symbol in symbol cross reference

        for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
         if (pTabFeno->TabCross[indexTabCross].Comp==indexSymbol)
          break;

        // Add symbol into symbol cross reference

        if ((indexTabCross==pTabFeno->NTabCross) && (pTabFeno->NTabCross<MAX_FIT))
         {
          if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_SPECTRUM].name)) && !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_SPECTRUM].name))
           pTabFeno->indexSpectrum=pTabFeno->NTabCross;
          else if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_REF].name)) && !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_REF].name))
           pTabFeno->indexReference=pTabFeno->NTabCross;

          pTabFeno->TabCross[indexTabCross].Comp=indexSymbol;
          pTabFeno->NTabCross++;
         }

        // Load shift and stretch parameters into symbol cross reference

        if (indexTabCross<pTabFeno->NTabCross)
         {
          pTabCross=&pTabFeno->TabCross[indexTabCross];
          pResults=&pTabFeno->TabCrossResults[indexTabCross];

          // Shift

          pTabCross->InitShift=pList->shInit;                                   // initial value for shift

          // !!! SEPARATE SHIFT LINEAR AND NON LINEAR ???

          if (pList->shFit!=ANLYS_SHIFT_TYPE_NONE)
           indexShift=pTabCross->FitShift=(!indexSymbolInList)?                 // flag set when shift is to be fit
             pTabFeno->fit_properties.NF++:indexShift;

          pTabCross->DeltaShift=(pTabCross->FitShift!=ITEM_NONE)?
            pList->shDelta:(double)0.;                                           // delta value for shift

          pTabCross->MinShift=pList->shMin;                                     // minimum value for shift
          pTabCross->MaxShift=pList->shMax;                                     // maximum value for shift

          // Stretch

          pTabCross->InitStretch=pList->stInit;                                 // initial value for stretch order 1
          pTabCross->InitStretch2=pList->stInit2;                               // initial value for stretch order 2

          pTabCross->TypeStretch=pList->stFit;                                  // type of stretch method

          if (pTabCross->TypeStretch!=ANLYS_STRETCH_TYPE_NONE)
           indexStretch=pTabCross->FitStretch=(!indexSymbolInList)?          // flag set when stretch is to be fit
             pTabFeno->fit_properties.NF++:indexStretch;

          if (pTabCross->TypeStretch==ANLYS_STRETCH_TYPE_SECOND_ORDER)
           indexStretch2=pTabCross->FitStretch2=(!indexSymbolInList)?        // flag set when stretch is to be fit
             pTabFeno->fit_properties.NF++:indexStretch2;

          pTabCross->DeltaStretch=(pTabCross->FitStretch!=ITEM_NONE)?        // delta value for stretch order 1
            pList->stDelta:(double)0.;
          pTabCross->DeltaStretch2=(pTabCross->FitStretch2!=ITEM_NONE)?      // delta value for stretch order 2
            pList->stDelta2:(double)0.;

          if (pTabCross->IndSvdA)
           {
            if (pTabCross->FitShift!=ITEM_NONE)
             pTabFeno->fit_properties.NP++;
            if (pTabCross->FitStretch!=ITEM_NONE)
             pTabFeno->fit_properties.NP++;
            if (pTabCross->FitStretch2!=ITEM_NONE)
             pTabFeno->fit_properties.NP++;
           }

          pResults->StoreShift=((!indexSymbolInList) && (pList->shStore==1))?(char)1:(char)0;                  // flag set if shift is to be written into output file
          pResults->StoreStretch=((!indexSymbolInList) && (pList->stStore==1))?(char)1:(char)0;                // flag set if stretch is to be written into output file
          pResults->StoreError=((!indexSymbolInList) && (pList->errStore==1))?(char)1:(char)0;                 // flag set if errors on linear parameters are to be written into output file
         }
       }
     }
   }

  // Return

  pTabFeno->fit_properties.nFit+=(pTabFeno->fit_properties.NF-oldNF);

  return rc;
}

// --------------------------------------------------
// AnalyseLoadPredefined : Load predefined parameters
// --------------------------------------------------
RC ANALYSE_LoadNonLinear(ENGINE_CONTEXT *pEngineContext,ANALYSE_NON_LINEAR_PARAMETERS *nonLinearList,int nNonLinear,double *lambda,INDEX indexFenoColumn)
{
  // Declarations

  INDEX indexItem,indexSymbol,indexTabCross,indexSvd;                           // indexes for loops and arrays
  char *symbol;                                                               // browse symbols
  CROSS_REFERENCE *pTabCross;                                                   // pointer to an element of the symbol cross reference table of an analysis window
  CROSS_RESULTS *pResults;                                                      // pointer to results part relative to the symbol
  ANALYSE_NON_LINEAR_PARAMETERS *pListItem;                                     // pointer to the current item in the non linear parameters list
  WRK_SYMBOL *pWrkSymbol;                                                       // pointer to a general description of a symbol
  SZ_LEN symbolLength,fileLength;                                               // length in characters of a symbol name
  FENO *pTabFeno;                                                               // pointer to description of the current analysis window
  RC rc,rcTmp;                                                                  // return code

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations
  const int n_wavel = NDET[indexFenoColumn];
  pWrkSymbol=NULL;
  pTabFeno=&TabFeno[indexFenoColumn][NFeno];
  rc=ERROR_ID_NO;

  for (indexItem=0;(indexItem<nNonLinear) && !rc;indexItem++)
   {
    pListItem=&nonLinearList[indexItem];

    if (pListItem->fitFlag || !strcasecmp(pListItem->symbolName,"SFP 2") || !strcasecmp(pListItem->symbolName,"SFP 3") || (!strcasecmp(pListItem->symbolName,"SFP 1") && (pKuruczOptions->fwhmType==SLIT_TYPE_FILE)) || (pListItem->initialValue!=(double)0.))
     {
      if ((pTabFeno->analysisMethod==OPTICAL_DENSITY_FIT) || strnicmp(pListItem->symbolName,"offset",6))
       {
        symbol=pListItem->symbolName;
        symbolLength=strlen(symbol);
        fileLength=strlen(pListItem->crossFileName);
        rcTmp=0;

        // Search for symbol in list

        for (indexSymbol=0;indexSymbol<NWorkSpace;indexSymbol++)
         {
          pWrkSymbol=&WorkSpace[indexSymbol];

          if ((pWrkSymbol->type==WRK_SYMBOL_PREDEFINED) &&
              (strlen(pWrkSymbol->symbolName)==symbolLength) &&
              (strlen(pWrkSymbol->crossFileName)==fileLength) &&
              !strcasecmp(pWrkSymbol->symbolName,pListItem->symbolName) &&
              !strcasecmp(pWrkSymbol->crossFileName,pListItem->crossFileName))
           break;
         }

        // Allocate a new symbol

        if ((indexSymbol==NWorkSpace) && (NWorkSpace<MAX_SYMB))
         {
          pWrkSymbol=&WorkSpace[indexSymbol];

          pWrkSymbol->type=WRK_SYMBOL_PREDEFINED;
          strcpy(pWrkSymbol->symbolName,symbol);
          strcpy(pWrkSymbol->crossFileName,pListItem->crossFileName);

          // Load cross section from file

          NWorkSpace++;
         }

        if ((indexSymbol<NWorkSpace) &&
            ((indexTabCross=pTabFeno->NTabCross)<MAX_FIT) &&
             ((strcasecmp(symbol,"SFP 1") && strcasecmp(symbol,"SFP 2") && strcasecmp(symbol,"SFP 3")) ||
              (pKuruczOptions->fwhmFit && (!strcasecmp(symbol,"SFP 1") ||
                                           (((pKuruczOptions->fwhmType==SLIT_TYPE_FILE) ||
                                             (pKuruczOptions->fwhmType==SLIT_TYPE_ERF) ||
                                             (pKuruczOptions->fwhmType==SLIT_TYPE_AGAUSS) ||
                                             (pKuruczOptions->fwhmType==SLIT_TYPE_SUPERGAUSS) ||
                                             (pKuruczOptions->fwhmType==SLIT_TYPE_VOIGT)) && !strcasecmp(symbol,"SFP 2")) ||

                                            ((pKuruczOptions->fwhmType==SLIT_TYPE_SUPERGAUSS) && !strcasecmp(symbol,"SFP 3"))
                                           )
              )
             )
            )

         {
          // Add symbol into symbol cross reference

          if ((symbolLength==strlen("SFP 1")) && !strcasecmp(symbol,"SFP 1"))
           pTabFeno->indexFwhmParam[0]=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("SFP 2")) && !strcasecmp(symbol,"SFP 2"))
           pTabFeno->indexFwhmParam[1]=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("SFP 3")) && !strcasecmp(symbol,"SFP 3"))
           pTabFeno->indexFwhmParam[2]=pTabFeno->NTabCross;
          //          else if ((symbolLength==strlen("Fwhm (Constant)")) && !strcasecmp(symbol,"Fwhm (Constant)"))
          //           pTabFeno->indexFwhmConst=pTabFeno->NTabCross;
          //          else if ((symbolLength==strlen("Fwhm (Order 1)")) && !strcasecmp(symbol,"Fwhm (Order 1)"))
          //           pTabFeno->indexFwhmOrder1=pTabFeno->NTabCross;
          //          else if ((symbolLength==strlen("Fwhm (order 2)")) && !strcasecmp(symbol,"Fwhm (Order 2)"))
          //           pTabFeno->indexFwhmOrder2=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("Sol")) && !strcasecmp(symbol,"Sol"))
           pTabFeno->indexSol=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("Offset (Constant)")) && !strcasecmp(symbol,"Offset (Constant)"))
           pTabFeno->indexOffsetConst=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("Offset (Order 1)")) && !strcasecmp(symbol,"Offset (Order 1)"))
           pTabFeno->indexOffsetOrder1=pTabFeno->NTabCross;
          else if ((symbolLength==strlen("Offset (Order 2)")) && !strcasecmp(symbol,"Offset (Order 2)"))
           pTabFeno->indexOffsetOrder2=pTabFeno->NTabCross;
          else if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_COM].name)) &&
                   !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_COM].name) &&
                   !pTabFeno->hidden)

           pTabFeno->indexCommonResidual=pTabFeno->NTabCross;

          else if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP1].name)) &&
                   !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP1].name) &&
                   !pTabFeno->hidden)

           pTabFeno->indexUsamp1=pTabFeno->NTabCross;

          else if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP2].name)) &&
                   !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_USAMP2].name) &&
                   !pTabFeno->hidden)

           pTabFeno->indexUsamp2=pTabFeno->NTabCross;

          else if ((symbolLength==strlen(SYMB_itemCrossList[SYMBOL_PREDEFINED_RESOL].name)) &&
                   !strcasecmp(symbol,SYMB_itemCrossList[SYMBOL_PREDEFINED_RESOL].name) &&
                   !pTabFeno->hidden)
           pTabFeno->indexResol=pTabFeno->NTabCross;
          else
           rcTmp=1;

          if (!rcTmp)
           {
            pTabFeno->TabCross[indexTabCross].Comp=indexSymbol;

            // Load parameters into cross reference structures

            pTabCross=&pTabFeno->TabCross[indexTabCross];
            pResults=&pTabFeno->TabCrossResults[indexTabCross];
            pTabCross->InitParam=pListItem->initialValue;

            // DOAS -> the parameters are fitted non linearly (except undersampling, see further NF--)
            // Marquardt-Levenberg method -> the parameters are fitted linearly

            if ((pTabFeno->analysisMethod==INTENSITY_FIT) &&
                ((pTabFeno->indexOffsetConst==pTabFeno->NTabCross) ||
                 (pTabFeno->indexOffsetOrder1==pTabFeno->NTabCross) ||
                 (pTabFeno->indexOffsetOrder2==pTabFeno->NTabCross) ||
                 (pTabFeno->indexCommonResidual==pTabFeno->NTabCross) ||
                 (pTabFeno->indexUsamp1==pTabFeno->NTabCross) ||
                 (pTabFeno->indexUsamp2==pTabFeno->NTabCross) ||
                 (pTabFeno->indexResol==pTabFeno->NTabCross)) &&
                (((pTabCross->FitParam=((pListItem->fitFlag)?1:0))!=ITEM_NONE) ||
                 (pTabCross->InitParam!=(double)0.)))
             {
              pTabCross->IndSvdA=++pTabFeno->fit_properties.DimC;
              pTabCross->IndSvdP=++pTabFeno->fit_properties.DimP;
             }
            else
             pTabCross->FitParam=(pListItem->fitFlag)?pTabFeno->fit_properties.NF++:ITEM_NONE;

            if (pTabCross->FitParam!=ITEM_NONE)  // Increase the number of parameters to fit
             pTabFeno->fit_properties.nFit++;               //    -> this information is useful to calculate the number of degrees of freedom

            pTabCross->MinParam=pListItem->minValue;
            pTabCross->MaxParam=pListItem->maxValue;
            pTabCross->DeltaParam=(pTabCross->FitParam!=ITEM_NONE)?pListItem->deltaValue:(double)0.;

            if ((pTabCross->FitParam==ITEM_NONE) && (pTabCross->InitParam==(double)0.))
             {
              if (pTabFeno->indexCommonResidual==pTabFeno->NTabCross)
               pTabFeno->indexCommonResidual=ITEM_NONE;
              else if (pTabFeno->indexUsamp1==pTabFeno->NTabCross)
               pTabFeno->indexUsamp1=ITEM_NONE;
              else if (pTabFeno->indexUsamp2==pTabFeno->NTabCross)
               pTabFeno->indexUsamp2=ITEM_NONE;
              else if (pTabFeno->indexResol==pTabFeno->NTabCross)
               pTabFeno->indexResol=ITEM_NONE;
              // else if (pTabFeno->indexFwhmParam[2]==pTabFeno->NTabCross)
              //  pTabFeno->indexFwhmParam[2]=ITEM_NONE;
              // else if (pTabFeno->indexFwhmParam[3]==pTabFeno->NTabCross)
              //  pTabFeno->indexFwhmParam[3]=ITEM_NONE;
             }

            // Load the cross section file if any

            if (((pTabFeno->indexCommonResidual==pTabFeno->NTabCross) ||          // common residual
                 (pTabFeno->indexResol==pTabFeno->NTabCross) ||                   // resol
                 (pTabFeno->indexUsamp1==pTabFeno->NTabCross) ||                  // undersampling phase 1
                 (pTabFeno->indexUsamp2==pTabFeno->NTabCross)) &&                 // undersampling phase 2
                ((pTabCross->FitParam!=ITEM_NONE) || (pTabCross->InitParam!=(double)0.))) {
              if (pTabFeno->indexCommonResidual==pTabFeno->NTabCross ||
                  ( (pTabFeno->indexUsamp1==pTabFeno->NTabCross ||                  // undersampling phase 1
                     pTabFeno->indexUsamp2==pTabFeno->NTabCross) &&                 // undersampling phase 2
                    pUsamp->method==PRJCT_USAMP_FILE) ) {

                if (!strlen(pListItem->crossFileName) ) {
                  return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_SPECIFIED,"undersampling cross section");
                }
                if ( (rc=MATRIX_Load(pListItem->crossFileName,&pWrkSymbol->xs,0,0,0.,0.,0,0,__func__) )
                     || ( (pTabFeno->gomeRefFlag || pEngineContext->refFlag) &&
                          (rc=ANALYSE_CheckLambda(pWrkSymbol,lambda, n_wavel) ) ) )   // grid of the reference spectrum
                  goto EndLoadPredefined;
               }
              else if ((pTabFeno->indexUsamp1==pTabFeno->NTabCross) || (pTabFeno->indexUsamp2==pTabFeno->NTabCross))
               pTabFeno->useUsamp=1;

              if ((pTabCross->vector=(double *)MEMORY_AllocDVector("ANALYSE_LoadNonLinear ","vector",0,n_wavel-1))==NULL)

               rc=ERROR_ID_ALLOC;
              else if (rc==ERROR_ID_NO) {
                if ((pTabFeno->indexCommonResidual==pTabFeno->NTabCross) ||
                    (((pTabFeno->indexUsamp1==pTabFeno->NTabCross) ||                  // undersampling phase 1
                      (pTabFeno->indexUsamp2==pTabFeno->NTabCross)) &&                 // undersampling phase 2
                     (pUsamp->method==PRJCT_USAMP_FILE)))
                  memcpy(pTabCross->vector,pWrkSymbol->xs.matrix[1],sizeof(double)*n_wavel);
                else
                  memcpy(pTabCross->vector,ANALYSE_zeros,sizeof(double)*n_wavel);

                if (((pTabFeno->analysisMethod!=INTENSITY_FIT) || (pTabCross->FitParam==ITEM_NONE)))
                  pTabCross->IndSvdA=++pTabFeno->fit_properties.DimC;

                pTabCross->crossAction=ANLYS_CROSS_ACTION_NOTHING; // For raman interpolation ((pTabFeno->indexRing1==pTabFeno->NTabCross)) ? ANLYS_CROSS_ACTION_intERPOLATE : ANLYS_CROSS_ACTION_NOTHING;
                pTabCross->display=(char)pTabFeno->displayPredefined;

                // DOAS fitting : only the Raman spectrum is fitted non linearly, other parameters are considered as cross sections

                if (pTabFeno->analysisMethod==OPTICAL_DENSITY_FIT) {
                  pTabCross->InitConc=pTabCross->InitParam;
                  pTabCross->DeltaConc=pTabCross->DeltaParam;
                  pTabCross->FitConc=(pTabCross->FitParam!=ITEM_NONE)?1:0;

                  // Swap columns of original matrix A in order to have in the end of the matrix, cross sections with fixed concentrations

                  if (pTabCross->FitConc!=0)
                   {
                    for (indexTabCross=pTabFeno->NTabCross-1;indexTabCross>=0;indexTabCross--)
                     if (((indexSvd=pTabFeno->TabCross[indexTabCross].IndSvdA)!=0) &&
                         !pTabFeno->TabCross[indexTabCross].FitConc)
                      {
                       pTabFeno->TabCross[indexTabCross].IndSvdA=pTabCross->IndSvdA;
                       pTabCross->IndSvdA=indexSvd;
                      }
                   }

                  if (pTabCross->FitParam!=ITEM_NONE)
                   pTabFeno->fit_properties.NF--;

                  pTabCross->FitParam=ITEM_NONE;
                  pTabCross->InitParam=(double)0.;
                } else { // Intensity fit
                  pTabCross->InitConc=(double)0.;
                  pTabCross->FitConc=ITEM_NONE;
                  pTabCross->DeltaConc=(double)0.;
                }
               }
             }

            if ((pTabCross->FitParam!=ITEM_NONE) || (pTabCross->InitParam!=(double)0.))
             {
              pResults->StoreParam=pListItem->storeFit;
              pResults->StoreParamError=pListItem->storeError;
             }
            else
             {
              pResults->StoreSlntCol=pListItem->storeFit;
              pResults->StoreSlntErr=pListItem->storeError;
             }

            if (((pTabFeno->indexOffsetConst==pTabFeno->NTabCross) ||
                 (pTabFeno->indexOffsetOrder1==pTabFeno->NTabCross) ||
                 (pTabFeno->indexOffsetOrder2==pTabFeno->NTabCross)) && (pTabFeno->linear_offset_mode != NO_LINEAR_OFFSET) &&
                ((pTabCross->FitParam!=ITEM_NONE) || (fabs(pTabCross->InitConc)>(double)1.e-6)))

             rc=ERROR_SetLast("ANALYSE_LoadNonLinear",ERROR_TYPE_FATAL,ERROR_ID_OPTIONS,"Offset (linear <-> non linear fit)",pTabFeno->windowName);

            pTabFeno->NTabCross++;
           }
         }
       }
     }
   }

 EndLoadPredefined :

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
  #endif

  // Return
  return rc;
}

// ----------------------------------------------------------
// AnalyseLoadGaps : Load gaps defined in an analysis windows
// ----------------------------------------------------------
RC ANALYSE_LoadGaps(ENGINE_CONTEXT *pEngineContext, const ANALYSIS_GAP *gapList,int nGaps,double *lambda,double lambdaMin,double lambdaMax,INDEX indexFenoColumn)
{
  // Declarations

  const ANALYSIS_GAP *pGap;
  int Z;
  INDEX indexItem,indexWindow,i;
  double swap,lambda1,lambda2,(*LFenetre)[2];
  FENO *pTabFeno;
  RC rc;

  // Initializations

  pTabFeno=&TabFeno[indexFenoColumn][NFeno];
  LFenetre=pTabFeno->fit_properties.LFenetre;    // gaps in wavelength units
  rc=ERROR_ID_NO;
  Z=0;

  // Get window limits

  lambda1=lambdaMin;
  lambda2=lambdaMax;

  if (lambda1==lambda2)
   rc=ERROR_SetLast("ANALYSIS_LoadGaps",ERROR_TYPE_FATAL,ERROR_ID_GAPS,lambdaMin,lambdaMax);
  else
   {
    LFenetre[Z][0]=min(lambda1,lambda2);
    LFenetre[Z][1]=max(lambda1,lambda2);

    // Get gaps

    for (indexItem=0,Z=1;(indexItem<nGaps);indexItem++)
     {
      pGap=&gapList[indexItem];

      lambda1=pGap->minimum;
      lambda2=pGap->maximum;

      if (lambda2<lambda1)
       {
        swap=lambda2;
        lambda2=lambda1;
        lambda1=swap;
       }

      for (indexWindow=0;indexWindow<Z;indexWindow++)
       if ((lambda1>LFenetre[indexWindow][0]) && (lambda2<LFenetre[indexWindow][1]))
        break;

      if (indexWindow==Z)
       rc=ERROR_SetLast("ANALYSIS_LoadGaps",ERROR_TYPE_WARNING,ERROR_ID_GAPS,pGap->minimum,pGap->maximum);
      else if (Z<MAX_FEN)
       {
        for (i=Z;i>indexWindow;i--)
         {
          LFenetre[i][0]=LFenetre[i-1][0];
          LFenetre[i][1]=LFenetre[i-1][1];
         }

        LFenetre[indexWindow][1]=lambda1;
        LFenetre[indexWindow+1][0]=lambda2;

        Z++;
       }
     }

    if (pTabFeno->gomeRefFlag || pEngineContext->refFlag)
     {
      if(pTabFeno->fit_properties.specrange != NULL)
       spectrum_destroy(pTabFeno->fit_properties.specrange);
      pTabFeno->fit_properties.specrange = spectrum_new();

      for (indexWindow=0;indexWindow<Z;indexWindow++)
       {
        int start = FNPixel(lambda,LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
        int end = FNPixel(lambda,LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);
        spectrum_append(pTabFeno->fit_properties.specrange, start, end);
       }
      pTabFeno->fit_properties.DimL= spectrum_length(pTabFeno->fit_properties.specrange);
     }

    pTabFeno->fit_properties.Z=Z;
   }

  // Return

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_LoadOutput
// -----------------------------------------------------------------------------
// PURPOSE       Load output part relative to a cross section from analysis windows properties
//
// INPUT         outputList   options from the output page of analysis windows properties
//               nOutput      number of entries in the list before
//               hidden     1 for the calibration options, 0 for analysis options
//
// OUTPUT        pAmfFlag   non zero if wavelength dependent AMF have to be accounted for
//
// RETURN        return code
// -----------------------------------------------------------------------------

RC ANALYSE_LoadOutput(const ANALYSIS_OUTPUT *outputList,int nOutput,INDEX indexFenoColumn)
{
  // Declarations

  const ANALYSIS_OUTPUT *pOutput;
  INDEX indexOutput,indexTabCross;
  CROSS_REFERENCE *TabCross,*pTabCross;                                         //  symbol cross reference
  CROSS_RESULTS   *TabCrossResults,*pResults;                                   //  results stored per symbol in previous list
  FENO *pTabFeno;
  RC rc;

  // Initializations

  pTabFeno=&TabFeno[indexFenoColumn][NFeno];
  TabCross=pTabFeno->TabCross;
  TabCrossResults=pTabFeno->TabCrossResults;
  rc=ERROR_ID_NO;

  pTabFeno->amfFlag=0;

  // Browse output

  for (indexOutput=0;indexOutput<nOutput;indexOutput++)
   {
    pOutput=&outputList[indexOutput];

    // Search for the equivalence of symbols between the molecules and output pages
    // Probably, that indexOutput==indexTabCross would be OK but it's safer

    for (indexTabCross=0;indexTabCross<pTabFeno->NTabCross;indexTabCross++)
     if ((strlen(pOutput->symbol)==strlen(WorkSpace[TabCross[indexTabCross].Comp].symbolName)) &&
         !strcasecmp(pOutput->symbol,WorkSpace[TabCross[indexTabCross].Comp].symbolName))
      break;

    // Symbol found

    if (indexTabCross<pTabFeno->NTabCross)
     {
      pTabCross=&TabCross[indexTabCross];
      pResults=&TabCrossResults[indexTabCross];

      if (!(rc=OUTPUT_ReadAmf(pOutput->symbol,
                              WorkSpace[pTabCross->Comp].amfFileName,
                              pTabCross->amfType,&pResults->indexAmf)))
       {
        // Load fields dependent on AMF

        if ((pResults->indexAmf!=ITEM_NONE) && (OUTPUT_AmfSpace!=NULL))
         {
          if (OUTPUT_AmfSpace[pResults->indexAmf].type==ANLYS_AMF_TYPE_WAVELENGTH)
           pTabFeno->amfFlag++;

          pResults->StoreAmf=pOutput->amf;                                      // flag set if AMF is to be written into output file
          pResults->StoreVrtCol=pOutput->vertCol;                               // flag set if vertical column is to be written into output file
          pResults->StoreVrtErr=pOutput->vertErr;                               // flag set if error on vertical column is to be written into output file
          pResults->VrtFact=pOutput->vertFactor;
          pResults->ResCol=(double)pOutput->resCol;                             // residual column
         }
       }

      pResults->StoreSlntCol=pOutput->slantCol;                                 // flag set if slant column is to be written into output file
      pResults->StoreSlntErr=pOutput->slantErr;                                 // flag set if error on slant column is to be written into output file
      pResults->SlntFact=pOutput->slantFactor;

      if ((pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_SLOPE) ||
          (pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_PUKITE) ||
          (pTabCross->crossCorrection==ANLYS_CORRECTION_TYPE_MOLECULAR_RING_SLOPE))
       {
        if (pTabCross->indexPukite1!=ITEM_NONE)
         memcpy(&TabCrossResults[pTabCross->indexPukite1],pResults,sizeof(CROSS_RESULTS));
        if (pTabCross->indexPukite2!=ITEM_NONE)
         memcpy(&TabCrossResults[pTabCross->indexPukite2],pResults,sizeof(CROSS_RESULTS));
       }
     }
   }

  // Return

  return rc;
}

// ---------------------------------------
// AnalyseLoadRef : Load reference spectra
// ---------------------------------------

RC ANALYSE_LoadRef(ENGINE_CONTEXT *pEngineContext,INDEX indexFenoColumn)
{
  // Declarations

  FENO *pTabFeno;
  double *Sref;
  double *SrefEtalon,*lambdaRef,*lambdaRefEtalon;
  int temp_use_row;
  char *ptr;
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  const int n_wavel = NDET[indexFenoColumn];

  pTabFeno=&TabFeno[indexFenoColumn][NFeno];
  SrefEtalon=pTabFeno->Sref=pTabFeno->SrefEtalon=NULL;
  pTabFeno->SrefRadAsRef1=NULL;
  pTabFeno->SrefRadAsRef2=NULL;
  pTabFeno->Zm=(double)-1.;
  pTabFeno->TDet=(double)0.;
  lambdaRef=lambdaRefEtalon=NULL;
  pTabFeno->LambdaRadAsRef1=NULL;
  pTabFeno->LambdaRadAsRef2=NULL;
  pTabFeno->useEtalon=0;
  pTabFeno->displayRef=0;
  pEngineContext->refFlag=0;
  pTabFeno->longPathFlag=ANALYSE_LONGPATH;                                      // !!! Anoop
  pTabFeno->useRadAsRef1=0;
  pTabFeno->useRadAsRef2=0;
  pTabFeno->useRefRow=((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GOME1_NETCDF) &&
                       (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) &&
                       (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMIV4) &&
                       (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_APEX) &&
                       (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_GEMS) &&
                       (pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_TROPOMI))?1:pEngineContext->project.instrumental.use_row[indexFenoColumn];

  pTabFeno->gomeRefFlag=(!is_satellite(pEngineContext->project.instrumental.readOutFormat))?1:0;

  //
  // in the case of satellites measurements :
  //
  //      gomeRefFlag=0 means that irradiance from the radiance input file is used as etalon spectrum
  //      gomeRefFlag=1 means that an external (irradiance) reference spectrum is given
  //
  // in other formats, gomeRefFlag is always equal to 1 even though no
  // reference is given (tropospheric measurements)
  //

  memset(pTabFeno->refFile,0,MAX_ITEM_TEXT_LEN);
  rc=0;

  // Allocate memory for reference spectra

  if (((Sref=pTabFeno->Sref=(double *)MEMORY_AllocDVector(__func__,"Sref",0,n_wavel-1))==NULL) ||
      ((SrefEtalon=pTabFeno->SrefEtalon=(double *)MEMORY_AllocDVector(__func__,"SrefEtalon",0,n_wavel))==NULL) ||

      ((is_satellite(pEngineContext->project.instrumental.readOutFormat) ||
      ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_ASCII) &&                     // column-extended ascii format is used for ground-based activitiesor any synthetic spectra
       (pEngineContext->project.instrumental.ascii.format==PRJCT_INSTR_ASCII_FORMAT_COLUMN_EXTENDED) &&      // a third columns can be used for errors only and only if errors are fitted
       (pEngineContext->project.analysis.fitWeighting!=PRJCT_ANLYS_FIT_WEIGHTING_NONE))) &&                  // otherwise, the reference spectrum should contain only two columns

       ((pTabFeno->SrefSigma=(double *)MEMORY_AllocDVector(__func__,"SrefSigma",0,n_wavel))==NULL)) ||

      ((lambdaRef=(double *)MEMORY_AllocDVector(__func__,"lambdaRef",0,n_wavel))==NULL) ||
      ((lambdaRefEtalon=(double *)MEMORY_AllocDVector(__func__,"lambdaRefEtalon",0,n_wavel))==NULL) )

   rc=ERROR_ID_ALLOC;

  else if (((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC) ||
            (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MFC_STD)) &&
           (strlen(pTabFeno->ref1) && !strrchr(pTabFeno->ref1,PATH_SEP)))
   {
    strcpy(pTabFeno->refFile,pTabFeno->ref1);
    pTabFeno->gomeRefFlag=0;
    pEngineContext->refFlag++;
   }
  else if ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_MKZY) && !strlen(pTabFeno->ref1) && !strlen(pTabFeno->ref2))
   {
    pTabFeno->gomeRefFlag=0;
    pEngineContext->refFlag++;
   }
  else
   {
    if (pTabFeno->SrefSigma!=NULL)
     memcpy(pTabFeno->SrefSigma,ANALYSE_ones,sizeof(double)*n_wavel);
    
    // ====
    // Ref1
    // ====

    memcpy(lambdaRefEtalon,pTabFeno->LambdaRef,sizeof(double)*n_wavel);
    memcpy(SrefEtalon,ANALYSE_ones,sizeof(double)*n_wavel);

    int n_wavel_ref = n_wavel; // for OMI, Tropomi, APEX: n_wavel is determined after reading the input file
    if ((pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC) &&                             // if the wavelength calibration procedure is applied on the measured
        (pTabFeno->useKurucz!=ANLYS_KURUCZ_REF_AND_SPEC) &&                     // spectrum, the ref1 has no sense.
        strlen(pTabFeno->ref1)) {

      switch(pEngineContext->project.instrumental.readOutFormat) {
      case PRJCT_INSTR_FORMAT_OMI:
        rc=OMI_GetReference(pEngineContext->project.instrumental.omi.spectralType, pTabFeno->ref1,indexFenoColumn,
                            lambdaRefEtalon,SrefEtalon,pTabFeno->SrefSigma,&n_wavel_ref);
        break;
      case PRJCT_INSTR_FORMAT_OMIV4:
        rc=OMIV4_get_irradiance_reference(pTabFeno->ref1, indexFenoColumn, lambdaRefEtalon, SrefEtalon, pTabFeno->SrefSigma);
        break;
      case PRJCT_INSTR_FORMAT_GEMS:
        rc=GEMS_LoadReference(pTabFeno->ref1,indexFenoColumn,lambdaRefEtalon,SrefEtalon,&n_wavel_ref);
        break;
      case PRJCT_INSTR_FORMAT_APEX:
        rc=apex_get_reference(pTabFeno->ref1,indexFenoColumn,lambdaRefEtalon,SrefEtalon,&n_wavel_ref);
        break;
      case PRJCT_INSTR_FORMAT_TROPOMI:
        rc=tropomi_get_reference_irrad(pTabFeno->ref1,indexFenoColumn,
                                       lambdaRefEtalon,SrefEtalon,pTabFeno->SrefSigma,pTabFeno->n_wavel_ref1);
        n_wavel_ref=pTabFeno->n_wavel_ref1;
        if (!n_wavel_ref) {
          pEngineContext->project.instrumental.use_row[indexFenoColumn] = false;
        }
        break;
      case PRJCT_INSTR_FORMAT_GOME1_NETCDF: // we read GOME1 reference in the apex format
        pTabFeno->useRadAsRef1=1;
        temp_use_row = 1;
        if ((pTabFeno->SrefRadAsRef1=(double *)MEMORY_AllocDVector(__func__,"SrefRadAsRef1",0,pTabFeno->n_wavel_ref1))==NULL ||
            (pTabFeno->LambdaRadAsRef1=(double *)MEMORY_AllocDVector(__func__,"LambdaRadAsRef1",0,pTabFeno->n_wavel_ref1))==NULL ||
            (pTabFeno->Deriv2RadAsRef1=(double *)MEMORY_AllocDVector(__func__,"Deriv2RadAsRef1",0,pTabFeno->n_wavel_ref1))==NULL) {
          rc=ERROR_ID_ALLOC;
        } else if (!(rc=radiance_ref_load(pTabFeno->ref1,indexFenoColumn,pTabFeno->LambdaRadAsRef1,pTabFeno->SrefRadAsRef1,pTabFeno->n_wavel_ref1,&temp_use_row))) {
          rc=SPLINE_Deriv2(pTabFeno->LambdaRadAsRef1,pTabFeno->SrefRadAsRef1,pTabFeno->Deriv2RadAsRef1,pTabFeno->n_wavel_ref1,__func__);
        }
        break;

      default:
        // rc=AnalyseLoadVector("ANALYSE_LoadRef (SrefEtalon) ",pTabFeno->ref1,lambdaRefEtalon,SrefEtalon,n_wavel,indexFenoColumn);
        
        if (!(rc=AnalyseLoadVector("ANALYSE_LoadRef (SrefEtalon) ",pTabFeno->ref1,lambdaRefEtalon,SrefEtalon,n_wavel,indexFenoColumn)) &&
             (pTabFeno->SrefSigma!=NULL) && !is_satellite(pEngineContext->project.instrumental.readOutFormat))
         rc=AnalyseLoadVector("ANALYSE_LoadRef (SrefEtalon) ",pTabFeno->ref1,lambdaRefEtalon,pTabFeno->SrefSigma,n_wavel,1);
          
        break;
      }


      if (!rc &&
          !(rc=THRD_SpectrumCorrection(pEngineContext,SrefEtalon,n_wavel)) &&
          !(rc=VECTOR_NormalizeVector(SrefEtalon-1,n_wavel_ref,&pTabFeno->refNormFact,"ANALYSE_LoadRef (SrefEtalon) "))) {
        pTabFeno->NDET = n_wavel_ref;
        pTabFeno->displayRef=pTabFeno->useEtalon=1;
        pTabFeno->gomeRefFlag=(pTabFeno->useRadAsRef1)?0:1;
        strcpy(pTabFeno->refFile,pTabFeno->ref1);

      }
    }
    
    // ====
    // Ref2
    // ====

    memcpy(lambdaRef,lambdaRefEtalon,sizeof(double)*n_wavel_ref);
    memcpy(Sref,SrefEtalon,sizeof(double)*n_wavel_ref);

    if (!rc &&
        (pTabFeno->useKurucz!=ANLYS_KURUCZ_SPEC) &&
        (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) &&
        strlen(pTabFeno->ref2) &&

        ((pEngineContext->project.instrumental.readOutFormat!=PRJCT_INSTR_FORMAT_OMI) ||
         (((ptr=strrchr(pTabFeno->ref2,'.'))!=NULL) &&
          (strlen(ptr)==4) && !strcasecmp(ptr,".ref"))))
     {
      ANALYSE_plotRef=1;

      switch(pEngineContext->project.instrumental.readOutFormat) {
        // Tropomi, GEMS, GOME1_NETCDF use the same radiance reference code.
      case PRJCT_INSTR_FORMAT_TROPOMI:
      case PRJCT_INSTR_FORMAT_GEMS:
      case PRJCT_INSTR_FORMAT_GOME1_NETCDF:
           pTabFeno->useRadAsRef2=1;
           temp_use_row = 1;
           if ((pTabFeno->SrefRadAsRef2=(double *)MEMORY_AllocDVector(__func__,"SrefRadAsRef2",0,pTabFeno->n_wavel_ref2))==NULL ||
               (pTabFeno->LambdaRadAsRef2=(double *)MEMORY_AllocDVector(__func__,"LambdaRadAsRef2",0,pTabFeno->n_wavel_ref2))==NULL ||
               (pTabFeno->Deriv2RadAsRef2=(double *)MEMORY_AllocDVector(__func__,"Deriv2RadAsRef2",0,pTabFeno->n_wavel_ref2))==NULL)
               rc=ERROR_ID_ALLOC;
           else {
             rc=radiance_ref_load(pTabFeno->ref2,indexFenoColumn,pTabFeno->LambdaRadAsRef2,pTabFeno->SrefRadAsRef2,pTabFeno->n_wavel_ref2,&temp_use_row);
             if (rc) break;
             if (!temp_use_row) {
               pTabFeno->useRefRow = false;
               break;
             }
             memcpy(pTabFeno->LambdaRef,lambdaRefEtalon,sizeof(double)*n_wavel_ref);
             rc=SPLINE_Deriv2(pTabFeno->LambdaRadAsRef2,pTabFeno->SrefRadAsRef2,pTabFeno->Deriv2RadAsRef2,pTabFeno->n_wavel_ref2,__func__);
             if(rc) break;
             rc=SPLINE_Vector(pTabFeno->LambdaRadAsRef2,pTabFeno->SrefRadAsRef2,pTabFeno->Deriv2RadAsRef2,pTabFeno->n_wavel_ref2,
                              pTabFeno->LambdaRef,pTabFeno->Sref,n_wavel_ref,SPLINE_CUBIC);
           }
           break;
        case PRJCT_INSTR_FORMAT_APEX:
           rc=apex_get_reference(pTabFeno->ref2,indexFenoColumn,lambdaRef,Sref,&n_wavel_ref);
           break;
        default:
           rc=AnalyseLoadVector("ANALYSE_LoadRef (Sref) ",pTabFeno->ref2,lambdaRef,Sref,n_wavel,indexFenoColumn);
           break;
        }
      if (!rc &&
          !(rc=THRD_SpectrumCorrection(pEngineContext,Sref,n_wavel)) &&
          !(rc=VECTOR_NormalizeVector(Sref-1,n_wavel_ref,&pTabFeno->refNormFact,"ANALYSE_LoadRef (Sref) "))) {
         if (!pTabFeno->useEtalon && (!is_satellite(pEngineContext->project.instrumental.readOutFormat) ||
             ((pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI) ||
              (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_GEMS) ||
              (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_TROPOMI) ||
              (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_APEX))))
          {
            memcpy(SrefEtalon,Sref,sizeof(double)*n_wavel);
            memcpy(lambdaRefEtalon,lambdaRef,sizeof(double)*n_wavel);
          }
      }
      strcpy(pTabFeno->refFile,pTabFeno->ref2);

      // Ground based measurements + 2 reference spectra

      if (!is_satellite(pEngineContext->project.instrumental.readOutFormat) && pTabFeno->useEtalon)
       pTabFeno->newrefFlag=1;
      
      pTabFeno->displayRef=pTabFeno->useEtalon=1;
     }

    if (!rc)
     memcpy(pTabFeno->LambdaRef,(pTabFeno->useEtalon)?lambdaRefEtalon:lambdaRef,sizeof(double)*n_wavel);
   }

  // Return

  if (lambdaRef!=NULL)
   MEMORY_ReleaseDVector(__func__,"lambdaRef",lambdaRef,0);
  if (lambdaRefEtalon!=NULL)
   MEMORY_ReleaseDVector(__func__,"lambdaRefEtalon",lambdaRefEtalon,0);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
  #endif

  return rc;
}

// -----------------------------------------------------------------------------------------
// AnalyseSetAnalysisType : Set the type of analysis to apply in the current analysis window
// -----------------------------------------------------------------------------------------

void ANALYSE_SetAnalysisType(INDEX indexFenoColumn)
{
  // Declarations

  CROSS_REFERENCE *TabCross;
  FENO *pTabFeno;

  // Initializations

  pTabFeno=&TabFeno[indexFenoColumn][NFeno];
  TabCross=pTabFeno->TabCross;
  pTabFeno->analysisType=ANALYSIS_TYPE_FWHM_NONE;

  // Fit fwhm in Kurucz procedure

  if (pTabFeno->hidden) 
   {
    if (pKuruczOptions->fwhmFit)
     pTabFeno->analysisType=ANALYSIS_TYPE_FWHM_KURUCZ;
    else 
     pTabFeno->analysisType=ANALYSIS_TYPE_FWHM_SLIT;
   }
  else // if (!pTabFeno->hidden)
   {
    // Apply a correction of resolution on reference or spectrum based on the temperature of these spectra

    if (pSlitOptions->fwhmCorrectionFlag)
     pTabFeno->analysisType=ANALYSIS_TYPE_FWHM_CORRECTION;

    // Fit of the difference of resolution between spectrum and reference
    // At least fit of fwhm order 0 should be set

    else if ((pTabFeno->indexFwhmConst!=ITEM_NONE) && ((TabCross[pTabFeno->indexFwhmConst].FitParam!=ITEM_NONE) || (TabCross[pTabFeno->indexFwhmConst].InitParam!=(double)0.)))
     pTabFeno->analysisType=ANALYSIS_TYPE_FWHM_NLFIT;
   }
}

// -------------------------------------------
// ANALYSE_LoadData : Load data from a project
// -------------------------------------------

RC ANALYSE_SetInit(ENGINE_CONTEXT *pEngineContext)
{
  // Declarations

  RC rc;                           // return code

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin("ANALYSE_SetInit",DEBUG_FCTTYPE_FILE|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  pEngineContext->analysisRef.refAuto=pEngineContext->analysisRef.refLon=0;
  analyseIndexRecord=pEngineContext->indexRecord;
  memset(&pEngineContext->analysisRef,0,sizeof(ANALYSIS_REF));

  rc=ERROR_ID_NO;

  // take maximum detector size to make sure that buffers allocated
  // here are big enough
  int max_ndet = 0;
  for (int i=0; i<ANALYSE_swathSize; ++i) {
    if (NDET[i] > max_ndet)
      max_ndet = NDET[i];
  }

  // Release all previously allocated buffers

  ANALYSE_plFilter=&pEngineContext->project.lfilter;
  ANALYSE_phFilter=&pEngineContext->project.hfilter;

  ANALYSE_plFilter->filterFunction=ANALYSE_phFilter->filterFunction=NULL;
  ANALYSE_plFilter->filterSize=ANALYSE_phFilter->filterSize=0;
  ANALYSE_plFilter->filterEffWidth=ANALYSE_phFilter->filterEffWidth=1.;

  if (pEngineContext->project.instrumental.readOutFormat==PRJCT_INSTR_FORMAT_OMI)
   OMI_ReleaseReference();

  ANALYSE_ResetData();

  // Allocate buffers for general use

  pAnalysisOptions=&pEngineContext->project.analysis;
  pSlitOptions=&pEngineContext->project.slit;
  pKuruczOptions=&pEngineContext->project.kurucz;
  pUsamp=&pEngineContext->project.usamp;

  if (pSlitOptions->fwhmCorrectionFlag && pKuruczOptions->fwhmFit)
   rc=ERROR_SetLast("ANALYSE_LoadData",ERROR_TYPE_FATAL,ERROR_ID_FWHM_INCOMPATIBLE_OPTIONS);
  else if (!(rc=FILTER_LoadFilter(&pEngineContext->project.lfilter)) &&   // low pass filtering
           !(rc=FILTER_LoadFilter(&pEngineContext->project.hfilter)) &&   // high pass filtering
           !(rc=AnalyseSvdGlobalAlloc()))
   {
    if (((ANALYSE_zeros=(double *)MEMORY_AllocDVector("ANALYSE_LoadData ","ANALYSE_zeros",0,max_ndet-1))==NULL) ||
        ((ANALYSE_ones=(double *)MEMORY_AllocDVector("ANALYSE_LoadData ","ANALYSE_ones",0,max_ndet-1))==NULL))

     rc=ERROR_ID_ALLOC;

    else
     {
      VECTOR_Init(ANALYSE_zeros,(double)0.,max_ndet);
      VECTOR_Init(ANALYSE_ones,(double)1.,max_ndet);
     }
   }

  // Return

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop("ANALYSE_SetInit",(RC)rc);
  analyseDebugMask=0;
#endif

  return rc;
}

// ====================
// RESOURCES MANAGEMENT
// ====================

// ------------------------------------------------------------------
// ANALYSE_Alloc : All analysis buffers allocation and initialization
// ------------------------------------------------------------------

RC ANALYSE_Alloc(void)
{
  // Declarations

  INDEX indexFenoColumn;
  RC rc;

  // Initialization

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  rc=ERROR_ID_NO;

  // Allocate all buffers need for analysis

  if (((WorkSpace=(WRK_SYMBOL *)MEMORY_AllocBuffer(__func__,"WorkSpace",MAX_SYMB,sizeof(WRK_SYMBOL),0,MEMORY_TYPE_STRUCT))==NULL) ||
      ((TabFeno=(FENO **)MEMORY_AllocBuffer(__func__,"TabFeno",MAX_SWATHSIZE,sizeof(FENO *),0,MEMORY_TYPE_PTR))==NULL))


   rc=ERROR_ID_ALLOC;

  else
   {
    memset(WorkSpace,0,sizeof(WRK_SYMBOL)*MAX_SYMB);

    ANALYSE_swathSize=1;                                                        // Allocate only for one reference spectrum

    memset(ANALYSIS_slitMatrix,0,sizeof(MATRIX_OBJECT)*NSFP);
    memset(&ANALYSIS_slitK,0,sizeof(MATRIX_OBJECT));

    for (int i=0;i<NSFP;i++)
     ANALYSIS_slitParam[i]=(double)0.;

    for (indexFenoColumn=0;(indexFenoColumn<MAX_SWATHSIZE) && !rc;indexFenoColumn++)
     if ((TabFeno[indexFenoColumn]=(FENO *)MEMORY_AllocBuffer("ANALYSE_Alloc ","TabFeno",MAX_FENO,sizeof(FENO),0,MEMORY_TYPE_STRUCT))==NULL)
      rc=ERROR_ID_ALLOC;
     else
      memset(TabFeno[indexFenoColumn],0,sizeof(FENO)*MAX_FENO);
   }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
  #endif

  // Return

  return rc;
}

// ------------------------------------------------
// ANALYSE_Free : Release buffers used for analysis
// ------------------------------------------------

void ANALYSE_Free(void)
{
#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif
  int i;

  if (WorkSpace!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_Free ","WorkSpace",WorkSpace);

  for (i=0;i<MAX_SWATHSIZE;i++)
   MEMORY_ReleaseBuffer("ANALYSE_Free ","TabFeno",TabFeno[i]);

  if (TabFeno!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_Free ","TabFeno",TabFeno);

  WorkSpace=NULL;
  TabFeno=NULL;

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,0);
  #endif
}


// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_UsampBuild
// -----------------------------------------------------------------------------
// PURPOSE       Build undersampling cross sections during analysis process.
//
// INPUT         analysisFlag :
//
//                   0 : file reference selection mode
//                   1 : automatic reference selection mode
//                   2 : automatic undersampling mode
//
//               gomeFlag :
//
//                   0 : Ref1 is the irradiance spectrum
//                   1 : Ref1 is a user-defined spectrum
// -----------------------------------------------------------------------------
// DESCRIPTION
//
// This function is called from different functions during analysis process
// according to selected reference options and undersampling method.
// In order to avoid multiple creation of undersampling cross sections, the
// calling function set input flags.
//
// Available combinations :
//
// 0,1 : called only one time from ANALYSE_LoadData because reference is already
//       available (Ref1 is a user-defined spectrum);
//       no automatic reference selection.
//
// 0,0 : called from GOME_LoadAnalysis for every spectra file;
//       the Ref1 is the irradiance spectrum;
//       no automatic reference selection.
//
// 1,ITEM_NONE : in automatic reference selection mode, the undersampling
//               must be calculated before aligning Ref1 sur Ref2; this function
//               is called from GOME_LoadAnalysis.

//
// 2,ITEM_NONE : used several times during analysis process in the following
//               cases :
//
//               called from Function for every iteration
//               in automatic undersampling method
//               -> apply current calibration with current fitted shift
//               -> independent from the reference selection mode
//
//               also called from ANALYSE_Spectrum each time the reference
//               spectrum is changed because the calibration used to calculate
//               undersampling cross sections should account for possible shift
//               between Ref2 and Ref1;
//               so this function is called after ANALYSE_Spectrum.
// -----------------------------------------------------------------------------
RC ANALYSE_UsampBuild(int analysisFlag,int gomeFlag,int indexFenoColumn)
{
  // Declarations

  MATRIX_OBJECT khrConvoluted,slitMatrix[NSFP];
  INDEX indexFeno,i,indexPixMin,indexPixMax,j;
  double slitParam[NSFP],*lambda,*lambda2,lambda0,x0;
  FENO *pTabFeno;
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  lambda2=NULL;
  memset(&slitMatrix,0,sizeof(MATRIX_OBJECT)*NSFP);
  memset(&khrConvoluted,0,sizeof(MATRIX_OBJECT));

  for (i=0;i<NSFP;i++)
   slitParam[i]=(double)0.;

  if (pKuruczOptions->fwhmType==SLIT_TYPE_INVPOLY)
   slitParam[1]=(double)pKuruczOptions->invPolyDegree;

  rc=ERROR_ID_NO;

  const int n_wavel = NDET[indexFenoColumn];

  // Buffer allocation

  if (((lambda=(double *)MEMORY_AllocDVector(__func__,"lambda",0,n_wavel-1))==NULL) ||
      ((lambda2=(double *)MEMORY_AllocDVector(__func__,"lambda2",0,n_wavel-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else

   // Browse analysis windows

   for (indexFeno=0;(indexFeno<NFeno)&&!rc;indexFeno++)
    {
     pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

     // Check options combinations

     if (!pTabFeno->hidden && (pTabFeno->useUsamp) &&
         ((gomeFlag==ITEM_NONE) || (pTabFeno->gomeRefFlag==gomeFlag)) &&
         (((analysisFlag==0) && (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) && (pUsamp->method==PRJCT_USAMP_FIXED)) ||
          ((analysisFlag==1) && (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_AUTOMATIC) && (pUsamp->method==PRJCT_USAMP_FIXED)) ||
          ((analysisFlag==2) && (pTabFeno==Feno))))
      {
       // Build lambda for second phase

       memcpy(lambda,pTabFeno->LambdaK,sizeof(double)*n_wavel);

       if ((analysisFlag==2) && (pUsamp->method==PRJCT_USAMP_FIXED))
        {
         lambda0=pTabFeno->lambda0;
         for (j=0;j<n_wavel;j++)
          {
           x0=lambda[j]-lambda0;
           lambda[j]-=(pTabFeno->Shift+(pTabFeno->Stretch+pTabFeno->Stretch2*x0)*x0);
          }
        }

       memcpy(lambda2,ANALYSE_zeros,sizeof(double)*n_wavel);

       indexPixMin=0;
       indexPixMax=n_wavel;

       if (pUsamp->method==PRJCT_USAMP_FIXED)
        for (i=indexPixMin+1,lambda2[indexPixMin]=lambda[indexPixMin]-pUsamp->phase;i<indexPixMax;i++)
         lambda2[i]=lambda[i]-pUsamp->phase; //  (double)(1.-pUsamp->phase)*lambda[i-1]+pUsamp->phase*lambda[i];
       else
        memcpy(lambda2,ANALYSE_shift,sizeof(double)*pTabFeno->NDET);

       // Not allowed combinations :
       //
       //     - fit slit function with calibration and apply calibration on spec only or on ref and spec;
       //     - fwhmCorrectionFlag and no calibration or fit slit function with the calibration

       // convolve the high resolution solar spectrum on its own grid

       if (MATRIX_Allocate(&khrConvoluted,ANALYSE_usampBuffers.lambdaRange[1][indexFeno],2,0,0,1,__func__))
        rc=ERROR_ID_ALLOC;

       else if ((((pTabFeno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC) || (pTabFeno->useKurucz==ANLYS_KURUCZ_SPEC)) && pKuruczOptions->fwhmFit) ||
                ((!pTabFeno->useKurucz || pKuruczOptions->fwhmFit) && pSlitOptions->fwhmCorrectionFlag))

        rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_OPTIONS,__func__,pTabFeno->windowName);


       // Convolution with user-defined slit function

       else if (!pTabFeno->useKurucz ||                                         // don't apply calibration
             !pKuruczOptions->fwhmFit)                                          // apply calibration but don't fit the slit function
        {
         memcpy(khrConvoluted.matrix[0],&ANALYSE_usampBuffers.hrSolar.matrix[0][ANALYSE_usampBuffers.lambdaRange[0][indexFeno]],sizeof(double)*khrConvoluted.nl);

         // Convolution with slit function from slit tab page of project properties

         rc=XSCONV_TypeStandard(&khrConvoluted,0,khrConvoluted.nl,&ANALYSE_usampBuffers.hrSolar,&ANALYSE_usampBuffers.hrSolar,NULL,
                                 pSlitOptions->slitFunction.slitType,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitWveDptFlag);
        }

       else if (!(rc=MATRIX_Allocate(&slitMatrix[0],pTabFeno->NDET,(pKuruczOptions->fwhmType!=SLIT_TYPE_FILE)?2:3,0,0,1,__func__)) &&
                ((pTabFeno->fwhmVector[1]==NULL) || (pKuruczOptions->fwhmType==SLIT_TYPE_FILE) || !(rc=MATRIX_Allocate(&slitMatrix[1],pTabFeno->NDET,2,0,0,1,__func__))))
        {
         // Local initializations

         memcpy(khrConvoluted.matrix[0],&ANALYSE_usampBuffers.hrSolar.matrix[0][ANALYSE_usampBuffers.lambdaRange[0][indexFeno]],sizeof(double)*khrConvoluted.nl);

         memcpy(slitMatrix[0].matrix[0],pTabFeno->LambdaK,sizeof(double)*pTabFeno->NDET);
         memcpy(slitMatrix[0].matrix[1],pTabFeno->fwhmVector[0],sizeof(double)*pTabFeno->NDET);
         memcpy(slitMatrix[0].deriv2[1],pTabFeno->fwhmDeriv2[0],sizeof(double)*pTabFeno->NDET);
         slitMatrix[0].nl=pTabFeno->NDET;

         if (pKuruczOptions->fwhmType==SLIT_TYPE_FILE)
          {
           memcpy(slitMatrix[0].matrix[2],pTabFeno->fwhmVector[1],sizeof(double)*pTabFeno->NDET);
           memcpy(slitMatrix[0].deriv2[2],pTabFeno->fwhmDeriv2[1],sizeof(double)*pTabFeno->NDET);
          }

         if ((pKuruczOptions->fwhmType!=SLIT_TYPE_FILE) && (pTabFeno->fwhmVector[1]!=NULL))
          {
           memcpy(slitMatrix[1].matrix[0],pTabFeno->LambdaK,sizeof(double)*pTabFeno->NDET);
           memcpy(slitMatrix[1].matrix[1],pTabFeno->fwhmVector[1],sizeof(double)*pTabFeno->NDET);
           memcpy(slitMatrix[1].deriv2[1],pTabFeno->fwhmDeriv2[1],sizeof(double)*pTabFeno->NDET);
           slitMatrix[1].nl=pTabFeno->NDET;
          }

         if ((pKuruczOptions->fwhmType==SLIT_TYPE_SUPERGAUSS) && (pTabFeno->fwhmVector[2]!=NULL))
          {
           memcpy(slitMatrix[2].matrix[0],pTabFeno->LambdaK,sizeof(double)*pTabFeno->NDET);
           memcpy(slitMatrix[2].matrix[1],pTabFeno->fwhmVector[2],sizeof(double)*pTabFeno->NDET);
           memcpy(slitMatrix[2].deriv2[1],pTabFeno->fwhmDeriv2[2],sizeof(double)*pTabFeno->NDET);
           slitMatrix[2].nl=pTabFeno->NDET;
          }

         rc=XSCONV_TypeStandard(&khrConvoluted,0,khrConvoluted.nl,&ANALYSE_usampBuffers.hrSolar,&ANALYSE_usampBuffers.hrSolar,NULL,pKuruczOptions->fwhmType,slitMatrix,slitParam,1);

         for (i=0;i<NSFP;i++)
          MATRIX_Free(&slitMatrix[i],__func__);
        }

       // Pre-Interpolation on analysis window wavelength scale

       if (!rc &&
           !(rc=SPLINE_Deriv2(khrConvoluted.matrix[0],khrConvoluted.matrix[1],khrConvoluted.deriv2[1],khrConvoluted.nl,"ANALYSE_UsampBuild (1) ")))
        {
         memcpy(ANALYSE_usampBuffers.kuruczInterpolated[indexFeno],ANALYSE_zeros,sizeof(double)*pTabFeno->NDET);
         memcpy(ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno],ANALYSE_zeros,sizeof(double)*pTabFeno->NDET);

         indexPixMin=ANALYSE_usampBuffers.lambdaRange[2][indexFeno]=FNPixel(lambda,khrConvoluted.matrix[0][0],pTabFeno->NDET,PIXEL_AFTER);
         indexPixMax=FNPixel(lambda,khrConvoluted.matrix[0][khrConvoluted.nl-1],pTabFeno->NDET,PIXEL_BEFORE);
         ANALYSE_usampBuffers.lambdaRange[3][indexFeno]=indexPixMax-ANALYSE_usampBuffers.lambdaRange[2][indexFeno]+1;

         if (!(rc=SPLINE_Vector(khrConvoluted.matrix[0],khrConvoluted.matrix[1],khrConvoluted.deriv2[1],khrConvoluted.nl,
                                &lambda[indexPixMin],&ANALYSE_usampBuffers.kuruczInterpolated[indexFeno][indexPixMin],(indexPixMax-indexPixMin+1),
                                pAnalysisOptions->interpol)))

          rc=SPLINE_Deriv2(&lambda[indexPixMin],&ANALYSE_usampBuffers.kuruczInterpolated[indexFeno][indexPixMin],
                           &ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno][indexPixMin],(indexPixMax-indexPixMin+1),"ANALYSE_UsampBuild (3) ");
        }

       // Build undersampling correction

       if (!rc)

        rc=USAMP_BuildCrossSections
          ((pTabFeno->indexUsamp1!=ITEM_NONE)?&pTabFeno->TabCross[pTabFeno->indexUsamp1].vector[ANALYSE_usampBuffers.lambdaRange[2][indexFeno]]:NULL,
           (pTabFeno->indexUsamp2!=ITEM_NONE)?&pTabFeno->TabCross[pTabFeno->indexUsamp2].vector[ANALYSE_usampBuffers.lambdaRange[2][indexFeno]]:NULL,
           &lambda[ANALYSE_usampBuffers.lambdaRange[2][indexFeno]],
           &lambda2[ANALYSE_usampBuffers.lambdaRange[2][indexFeno]],
           &ANALYSE_usampBuffers.kuruczInterpolated[indexFeno][ANALYSE_usampBuffers.lambdaRange[2][indexFeno]],
           &ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno][ANALYSE_usampBuffers.lambdaRange[2][indexFeno]],
           ANALYSE_usampBuffers.lambdaRange[3][indexFeno],
           khrConvoluted.matrix[0],
           khrConvoluted.matrix[1],
           khrConvoluted.deriv2[1],
           khrConvoluted.nl,
           pTabFeno->analysisMethod);

       MATRIX_Free(&khrConvoluted,__func__);
      }
    }

  // Return

  if (lambda!=NULL)
   MEMORY_ReleaseDVector(__func__,"lambda",lambda,0);
  if (lambda2!=NULL)
   MEMORY_ReleaseDVector(__func__,"lambda2",lambda2,0);

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
  #endif

  return rc;
 }

// ==================
// BUFFERS ALLOCATION
// ==================

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_UsampGlobalAlloc
// -----------------------------------------------------------------------------
// PURPOSE       allocate buffers (not depending on analysis windows) for the calculation of the undersampling XS
//
// INPUT         lambdaMin, lambdaMax : range of wavelengths for the kurucz spectrum
//               size                 : the size of the final wavelength calibration
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_USAMP if another called function returns an error;
//               ERROR_ID_NO on success
// -----------------------------------------------------------------------------

RC ANALYSE_UsampGlobalAlloc(double lambdaMin,double lambdaMax,int size)
{
  // Declarations

  char kuruczFile[MAX_ITEM_TEXT_LEN];
  FENO *pTabFeno;
  INDEX indexFeno,indexFenoColumn;
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initialization

  rc=ERROR_ID_NO;

  // Load high resolution kurucz spectrum
  // NB : if the high resolution spectrum is the same as used in Kurucz, don't reload it from file but only make a copy

  if ((strlen(pKuruczOptions->file)==strlen(pUsamp->kuruczFile)) &&
      !strcasecmp(pKuruczOptions->file,pUsamp->kuruczFile) &&
      KURUCZ_buffers[0].hrSolar.nl)
   {
    if ((rc=MATRIX_Allocate(&ANALYSE_usampBuffers.hrSolar,KURUCZ_buffers[0].hrSolar.nl,2,0,0,1,"ANALYSE_UsampGlobalAlloc")))
     rc=ERROR_ID_ALLOC;
    else
     {
      memcpy(ANALYSE_usampBuffers.hrSolar.matrix[0],KURUCZ_buffers[0].hrSolar.matrix[0],sizeof(double)*KURUCZ_buffers[0].hrSolar.nl);
      memcpy(ANALYSE_usampBuffers.hrSolar.matrix[1],KURUCZ_buffers[0].hrSolar.matrix[1],sizeof(double)*KURUCZ_buffers[0].hrSolar.nl);
      memcpy(ANALYSE_usampBuffers.hrSolar.deriv2[1],KURUCZ_buffers[0].hrSolar.deriv2[1],sizeof(double)*KURUCZ_buffers[0].hrSolar.nl);
     }
   }
  else
   {
    FILES_RebuildFileName(kuruczFile,pUsamp->kuruczFile,1);

    if (!(rc=XSCONV_LoadCrossSectionFile(&ANALYSE_usampBuffers.hrSolar,kuruczFile,lambdaMin-7.,lambdaMax+7.,(double)0.,CONVOLUTION_CONVERSION_NONE)))
     rc=VECTOR_NormalizeVector(ANALYSE_usampBuffers.hrSolar.matrix[1]-1,ANALYSE_usampBuffers.hrSolar.nl,NULL,"ANALYSE_UsampGlobalAlloc ");
   }

  if (rc!=ERROR_ID_NO)
   rc=ERROR_SetLast("ANALYSE_UsampGlobalAlloc",ERROR_TYPE_FATAL,ERROR_ID_USAMP,"ANALYSE_UsampGlobalAlloc (0)");

  // Buffers allocation

  else if (((ANALYSE_usampBuffers.lambdaRange[0]=(int *)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","lambdaRange[0]",NFeno,sizeof(int),0,MEMORY_TYPE_INT))==NULL) ||
           ((ANALYSE_usampBuffers.lambdaRange[1]=(int *)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","lambdaRange[1]",NFeno,sizeof(int),0,MEMORY_TYPE_INT))==NULL) ||
           ((ANALYSE_usampBuffers.lambdaRange[2]=(int *)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","lambdaRange[2]",NFeno,sizeof(int),0,MEMORY_TYPE_INT))==NULL) ||
           ((ANALYSE_usampBuffers.lambdaRange[3]=(int *)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","lambdaRange[3]",NFeno,sizeof(int),0,MEMORY_TYPE_INT))==NULL) ||
           ((ANALYSE_usampBuffers.kuruczInterpolated=(double **)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","kuruczInterpolated",NFeno,sizeof(double *),0,MEMORY_TYPE_DOUBLE))==NULL) ||
           ((ANALYSE_usampBuffers.kuruczInterpolated2=(double **)MEMORY_AllocBuffer("ANALYSE_UsampGlobalAlloc ","kuruczInterpolated2",NFeno,sizeof(double *),0,MEMORY_TYPE_DOUBLE))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    for (indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++)
     for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
      {
       pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

       ANALYSE_usampBuffers.lambdaRange[0][indexFeno]=
         ANALYSE_usampBuffers.lambdaRange[1][indexFeno]=
         ANALYSE_usampBuffers.lambdaRange[2][indexFeno]=
         ANALYSE_usampBuffers.lambdaRange[3][indexFeno]=ITEM_NONE;

       ANALYSE_usampBuffers.kuruczInterpolated[indexFeno]=
         ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno]=NULL;

       if (!pTabFeno->hidden && pTabFeno->useUsamp &&
           (((ANALYSE_usampBuffers.kuruczInterpolated[indexFeno]=(double *)MEMORY_AllocDVector("ANALYSE_UsampGlobalAlloc ","kuruczInterpolated",0,size-1))==NULL) ||
            ((ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno]=(double *)MEMORY_AllocDVector("ANALYSE_UsampGlobalAlloc ","kuruczInterpolated2",0,size-1))==NULL)))

        rc=ERROR_ID_ALLOC;

       else
        ANALYSE_usampBuffers.lambdaRange[0][indexFeno]=ANALYSE_usampBuffers.lambdaRange[1][indexFeno]=ITEM_NONE;
      }
   }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
  #endif

  // Return

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_UsampLocalAlloc
// -----------------------------------------------------------------------------
// PURPOSE       allocate buffers (depending on analysis windows) for the calculation of the undersampling XS
//
// INPUT         gomeFlag = 0 : Ref1 is the irradiance spectrum
//                          1 : Ref1 is a user-defined spectrum
//
// RETURN        ERROR_ID_ALLOC if the allocation of a buffer failed;
//               ERROR_ID_NO on success
// -----------------------------------------------------------------------------
RC ANALYSE_UsampLocalAlloc(int gomeFlag)
{
  // Declarations

  INDEX indexFeno,indexFenoColumn;
  int endPixel;
  FENO *pTabFeno;
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  rc=ERROR_ID_NO;

  // for (indexFenoColumn=0;(indexFenoColumn<ANALYSE_swathSize) && !rc;indexFenoColumn++)

  indexFenoColumn=0;  // per row, the configuration of the analysis shouldn't change   // TO DO LATER

   for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
    {
     pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

     if (!pTabFeno->hidden && pTabFeno->useUsamp && (pTabFeno->gomeRefFlag==gomeFlag))
      {
       ANALYSE_usampBuffers.lambdaRange[0][indexFeno]=FNPixel(ANALYSE_usampBuffers.hrSolar.matrix[0],(double)pTabFeno->fit_properties.LFenetre[0][0]-7.,ANALYSE_usampBuffers.hrSolar.nl,PIXEL_AFTER);

       endPixel=FNPixel(ANALYSE_usampBuffers.hrSolar.matrix[0],(double)pTabFeno->fit_properties.LFenetre[pTabFeno->fit_properties.Z-1][1]+7.,ANALYSE_usampBuffers.hrSolar.nl,PIXEL_BEFORE);
       ANALYSE_usampBuffers.lambdaRange[1][indexFeno]=endPixel-ANALYSE_usampBuffers.lambdaRange[0][indexFeno]+1;
      }
    }

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
  #endif

  // Return

  return rc;
}

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_UsampLocalFree
// -----------------------------------------------------------------------------
// PURPOSE       release the buffers previously allocated by the ANALYSE_UsampLocalAlloc function
// -----------------------------------------------------------------------------

// void ANALYSE_UsampLocalFree(void)
// {
//   // Declarations
//
//   INDEX indexFeno,indexFenoColumn;
//
// }

// -----------------------------------------------------------------------------
// FUNCTION      ANALYSE_UsampGlobalFree
// -----------------------------------------------------------------------------
// PURPOSE       release the buffers previously allocated by the ANALYSE_UsampGlobalAlloc function
// -----------------------------------------------------------------------------

void ANALYSE_UsampGlobalFree(void)
{
  INDEX indexFeno;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  MATRIX_Free(&ANALYSE_usampBuffers.hrSolar,"ANALYSE_UsampGlobalFree");

  if (ANALYSE_usampBuffers.lambdaRange[0]!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","lambdaRange[0]",ANALYSE_usampBuffers.lambdaRange[0]);
  if (ANALYSE_usampBuffers.lambdaRange[1]!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","lambdaRange[1]",ANALYSE_usampBuffers.lambdaRange[1]);
  if (ANALYSE_usampBuffers.lambdaRange[2]!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","lambdaRange[2]",ANALYSE_usampBuffers.lambdaRange[2]);
  if (ANALYSE_usampBuffers.lambdaRange[3]!=NULL)
   MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","lambdaRange[3]",ANALYSE_usampBuffers.lambdaRange[3]);

  if (ANALYSE_usampBuffers.kuruczInterpolated!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (ANALYSE_usampBuffers.kuruczInterpolated[indexFeno]!=NULL)
      MEMORY_ReleaseDVector("ANALYSE_UsampGlobalFree ","kuruczInterpolated",ANALYSE_usampBuffers.kuruczInterpolated[indexFeno],0);

    MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","kuruczInterpolated",ANALYSE_usampBuffers.kuruczInterpolated);
   }

  if (ANALYSE_usampBuffers.kuruczInterpolated2!=NULL)
   {
    for (indexFeno=0;indexFeno<NFeno;indexFeno++)
     if (ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno]!=NULL)
      MEMORY_ReleaseDVector("ANALYSE_UsampGlobalFree ","kuruczInterpolated2",ANALYSE_usampBuffers.kuruczInterpolated2[indexFeno],0);

    MEMORY_ReleaseBuffer("ANALYSE_UsampGlobalFree ","kuruczInterpolated2",ANALYSE_usampBuffers.kuruczInterpolated2);
   }

  memset(&ANALYSE_usampBuffers,0,sizeof(USAMP));

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,0);
  #endif
}
