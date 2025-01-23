//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  KURUCZ PROCESSING
//  Name of module    :  KURUCZ.C
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
//
//  FUNCTIONS
//
//  =================
//  KURUCZ PROCEDURES
//  =================
//
//  KURUCZ_SearchReference - search for a reference spectrum in analysis windows on which Kurucz has already been applied;
//  KURUCZ_Spectrum - apply Kurucz for building a new wavelength scale to a spectrum;
//  KURUCZ_ApplyCalibration - apply the new calibration to cross sections to interpolate or to convolute and recalculate gaps;
//  KURUCZ_Reference - browse analysis windows and apply Kurucz if needed on reference spectrum;
//
//  ====================
//  RESOURCES MANAGEMENT
//  ====================
//
//  KURUCZ_Init - set the limits for each window of the calibration interval;
//  KURUCZ_Alloc - allocate all buffers needed for applying Kurucz procedures to a project;
//  KURUCZ_Free - release buffers allocated for Kurucz procedures;
//  ----------------------------------------------------------------------------

#include <string.h>
#include <math.h>

#include "kurucz.h"
#include "analyse.h"
#include "mediate.h"
#include "xsconv.h"
#include "engine.h"
#include "spline.h"
#include "filter.h"
#include "winfiles.h"
#include "xsconv.h"
#include "vector.h"
#include "winthrd.h"
#include "output.h"
#include "nelmin.h"

// ================
// GLOBAL VARIABLES
// ================

KURUCZ KURUCZ_buffers[MAX_SWATHSIZE];
FFT *pKURUCZ_fft;
int KURUCZ_indexLine=1;

// ===========================
// CALCULATION OF THE PRESHIFT
// ===========================

// Given two arrays x[1..n] and y[1..n], this routine computes their correlation coefficient
// r (returned as r), the significance level at which the null hypothesis of zero correlation is
// disproved (prob whose small value indicates a significant correlation), and Fisher�s z (returned
// as z), whose value can be used in further statistical tests as described above.

// Reference : Numerical Recipes in C

#define TINY 1.0e-20 // Will regularize the unusual case of complete correlation#

double corrcoef(double *x,double *y,int n)
 {
     // Declarations

  int j;
  double yt,xt,r;   // df,t
  double syy=0.0,sxy=0.0,sxx=0.0,ay=0.0,ax=0.0;

  // Find the means.

  for (j=0;j<n;j++)
   {
    ax += x[j];
    ay += y[j];
   }

  ax /= n;
  ay /= n;

  // Compute the correlation coefficient.

  for (j=0;j<n;j++)
   {
    xt=x[j]-ax;
    yt=y[j]-ay;
    sxx += xt*xt;
    syy += yt*yt;
    sxy += xt*yt;
   }

  r=sxy/(sqrt(sxx*syy)+TINY);

  // *z=0.5*log((1.0+(*r)+TINY)/(1.0-(*r)+TINY)); Fisher�s z transformation.
  // df=n-2;
  // t=(*r)*sqrt(df/((1.0-(*r)+TINY)*(1.0+(*r)+TINY))); Equation (14.5.5).
  // *prob=betai(0.5*df,0.5,df/(df+t*t)); Student�s t probability.
/* *prob=erfcc(fabs((*z)*sqrt(n-1.0))/1.4142136) */
 // For large n, this easier computation of prob, using the short routine erfcc, would give approximately
 // the same value.

  // Return

  return r;
 }

double ShiftCorrel(double *lambda,double *ref,double *spec,double *spec2,double *lambdas,double *specInt,int n,int imin,int imax,double *x,int *pRc)
 {
     // Declarations

     int i,npix;
     double correl;
     RC rc;

     // Initializations

     npix=(imax-imin+1);
     correl=(double)0.;

     rc=ERROR_ID_NO;

     // Shift the wavelength calibration

  for (i=0;i<n;i++)
   lambdas[i]=lambda[i]+x[0];

  // Interpolate the spectrum on the original wavelength calibration

  if (!(rc=SPLINE_Deriv2(&lambdas[imin],&spec[imin],&spec2[imin],npix,__func__)) &&
      !(rc=SPLINE_Vector(&lambdas[imin],&spec[imin],&spec2[imin],npix,&lambda[imin],&specInt[imin],npix,PRJCT_ANLYS_INTERPOL_SPLINE)))

   correl=(double)1.-corrcoef(&ref[imin],&specInt[imin],npix);

  *pRc=rc;

  return correl;
 }

void KuruczSmooth(double *lambda,double *spectrum,int n,int width,int imin,int imax,int normFlag,double *smoothed)
 {
     // Declarations

     int i,j,jmin,jmax;
  double sum,norm;

  // Initializations

  for (i=0;i<n;i++)
   smoothed[i]=(double)0.;

     for (i=imin,norm=(double)0.;i<=imax;i++)
      {
          jmin=max(0,i-width);
          jmax=min(n-1,i+width);

          for (j=jmin,sum=(double)0.;j<=jmax;j++)
           sum+=spectrum[j];

          if (fabs(sum)>EPSILON)
        smoothed[i]=spectrum[i]/sum;
          norm+=smoothed[i]*smoothed[i];
      }

     // Normalization

     if (normFlag && (norm>EPSILON))
      for (i=imin;i<=imax;i++)
       smoothed[i]/=norm;
 }

// TO DO LATER
// !!! The following function could be called by ShiftVector -> some adaptations necessary
//

// ----------------------------------------------------------------------------
// FUNCTION        KuruczConvolveSolarSpectrum
// ----------------------------------------------------------------------------
// PURPOSE         Convolve solar spectrum with initial fwhm parameters before
//                 calculating the preshift.  The resulting spectrum will be used
//                 as calibrated ref.
//
// INPUT           newlambda             the calibrated grid
//                 n_wavel               the size of the calibrated grid
//
// OUTPUT          pSolar                pointer to the convolved solar spectrum
//
// RETURN          ERROR_ID_NO in case of success
// ----------------------------------------------------------------------------

RC KuruczConvolveSolarSpectrum(MATRIX_OBJECT *pSolar,double *newlambda,int n_wavel,int indexFenoColumn)
 {
     // Declarations

     CROSS_REFERENCE *TabCross;
  MATRIX_OBJECT slitMatrix[NSFP],*pSlitMatrix;
  double slitParam[NSFP];
  SLIT slitOptions;
  int slitType;
  int shiftIndex,i;
  RC rc;
  KURUCZ *pKurucz;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  pKurucz=&KURUCZ_buffers[indexFenoColumn];
  Feno=&TabFeno[indexFenoColumn][pKurucz->indexKurucz];
  TabCross=Feno->TabCross;
  rc=ERROR_ID_NO;

  memset(slitMatrix,0,sizeof(MATRIX_OBJECT)*NSFP);

  memcpy(pSolar->matrix[0],newlambda,sizeof(double)*n_wavel);
  memcpy(pSolar->matrix[1],ANALYSE_zeros,sizeof(double)*n_wavel);

  for (i=0;i<NSFP;i++)
   slitParam[i]=(double)0.;

  slitType=pKuruczOptions->fwhmType;

  if (slitType==SLIT_TYPE_FILE)
   {
    int nc=KURUCZ_buffers[indexFenoColumn].slitFunction.nc;
    int nl=KURUCZ_buffers[indexFenoColumn].slitFunction.nl;
    double fwhmStretch1,fwhmStretch2;

    shiftIndex=(nc==2)?0:1;

    fwhmStretch1=(Feno->indexFwhmParam[0]!=ITEM_NONE)?(double)TabCross[Feno->indexFwhmParam[0]].InitParam:(double)1.;
    fwhmStretch2=(Feno->indexFwhmParam[1]!=ITEM_NONE)?(double)TabCross[Feno->indexFwhmParam[1]].InitParam:(double)1.;

    pSlitMatrix=&slitMatrix[0];

    if (MATRIX_Allocate(pSlitMatrix,nl,nc,0,0,1,__func__))
      rc=ERROR_ID_ALLOC;
    else {
      // make a backup of the matrix
      for (i=0;i<nc;i++)
        memcpy(pSlitMatrix->matrix[i],KURUCZ_buffers[indexFenoColumn].slitFunction.matrix[i],sizeof(double)*nl);

      // determine slit center wavelength, defined as wavelength
      // corresponding to the maximum value
      double lambda_center = 0.;
      double slit_max = 0.;
      XSCONV_get_slit_center(pSlitMatrix->matrix[0] + shiftIndex, pSlitMatrix->matrix[1] + shiftIndex, pSlitMatrix->nl - shiftIndex,
                             &lambda_center, &slit_max);

      // Apply the stretch on the slit wavelength calibration
      for (i=shiftIndex;i<pSlitMatrix->nl;i++) {
        // stretch wavelength grid around the center wavelength,
        // using fwhmStretch1 on the left, and fwhmStretch2 on the
        // right:
        double delta_lambda = pSlitMatrix->matrix[0][i] - lambda_center;
        delta_lambda *= (delta_lambda < 0.) ? fwhmStretch1 : fwhmStretch2;
        pSlitMatrix->matrix[0][i]= lambda_center + delta_lambda;
      }

      // Recalculate second derivatives and the FWHM
      for (i=1;i<pSlitMatrix->nc;i++)
        rc=SPLINE_Deriv2(pSlitMatrix->matrix[0]+shiftIndex,pSlitMatrix->matrix[i]+shiftIndex,pSlitMatrix->deriv2[i]+shiftIndex,pSlitMatrix->nl-shiftIndex,__func__);
     }
   }
  else
   {
    memset(&slitOptions,0,sizeof(SLIT));

    slitOptions.slitType=slitType;
    slitOptions.slitFile[0]=0;
    slitOptions.slitParam=TabCross[Feno->indexFwhmParam[0]].InitParam;
    slitOptions.slitParam2=(slitType==SLIT_TYPE_GAUSS)?(double)0.:TabCross[Feno->indexFwhmParam[1]].InitParam;
    slitOptions.slitParam3=(slitType==SLIT_TYPE_SUPERGAUSS)?TabCross[Feno->indexFwhmParam[2]].InitParam:(double)0.;

    rc=XSCONV_LoadSlitFunction(slitMatrix,&slitOptions,NULL,&slitType);
   }

  // convolution of the solar spectrum

  if (!rc)
   rc=XSCONV_TypeStandard(pSolar,0,n_wavel,&KURUCZ_buffers[indexFenoColumn].hrSolar,&KURUCZ_buffers[indexFenoColumn].hrSolar,NULL,slitType,slitMatrix,slitParam,0);

  // Release allocated buffers

  for (i=0;i<NSFP;i++)
   MATRIX_Free(&slitMatrix[i],__func__);

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  // Return

  return rc;
 }

// ----------------------------------------------------------------------------
// FUNCTION        KuruczCalculatePreshift
// ----------------------------------------------------------------------------
// PURPOSE         Calculate the preshift between the reference spectrum to calculate and a already well calibrated spectrum
//
// INPUT           calibratedLambda      wavelength calibration of the well calibrated spectrum
//                 calibratedRef         well calibrated spectrum
//                 newRef                reference spectrum to calibrate
//                 ndet                  size of the detector
//                 preshiftMin           minimum value for the preshift
//                 preshiftMax           maximum value for the preshift
//                 step                  shift step in the wavelength shift window for correlation caluclation
//                 lambdaMin, lambdaMax  wavelengths range to calculate the correlation between the two spectra
//
// OUTPUT          newLambda             new grid
//                 pShift                initial shift for wavelength calibration
//
// RETURN          indexFeno      index of the analysis window in which reference has been found;
// ----------------------------------------------------------------------------

RC KuruczCalculatePreshift(double *calibratedLambda,double *calibratedRef,double *newRef,int ndet,double preshiftMin,double preshiftMax,double step,double lambdaMin,double lambdaMax,double *pShift)
 {
     // Declarations

  double *lambdas,*Sref1,*Sref2,*Sref2Deriv,*Sref2Interp,*shift,*coeff;
  int ishift,ishiftMin,nshift,nfuncEval,nrestart,imin,imax,nsmooth;
  double shiftIni,shiftMin,coefMin,varstep;
     RC rc,rc2;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

     // Initialization

  Sref1=Sref2=Sref2Deriv=Sref2Interp=shift=NULL;
  nshift=(double)((preshiftMax-preshiftMin)/step)+1.;

  varstep=1;
  *pShift=(double)0.;
     rc=ERROR_ID_NO;

     // Allocate temporary buffers

     if (((lambdas=(double *)MEMORY_AllocDVector("KuruczCalculatePreshift","lambdas",0,ndet-1))==NULL) ||
         ((Sref1=(double *)MEMORY_AllocDVector("KuruczCalculatePreshift","Sref1",0,ndet-1))==NULL) ||
         ((Sref2=(double *)MEMORY_AllocDVector("KuruczCalculatePreshift","Sref2",0,ndet-1))==NULL) ||
         ((Sref2Deriv=(double *)MEMORY_AllocDVector("KuruczCalculatePreshift","Sref2Deriv",0,ndet-1))==NULL) ||
         ((Sref2Interp=(double *)MEMORY_AllocDVector("KuruczCalculatePreshift","Sref2Interp",0,ndet-1))==NULL) ||
         ((shift=(double *)MEMORY_AllocDVector("KuruczCalculatePreshift","shift",0,nshift-1))==NULL) ||
         ((coeff=(double *)MEMORY_AllocDVector("KuruczCalculatePreshift","coeff",0,nshift-1))==NULL))

      rc=ERROR_ID_ALLOC;

     // Load the reference spectrum at minimum SZA

     else // if (!(rc=EngineBuildRefList(pEngineContext)) && ((indexRef=pEngineContext->analysisRef.zmMinIndex)!=ITEM_NONE) &&
          //   ((!pEngineContext->mfcDoasisFlag && !(rc=EngineReadFile(&ENGINE_contextRef,indexRef,0,0))) ||
          //     (pEngineContext->mfcDoasisFlag && !(rc=EngineLoadRefMFC(&ENGINE_contextRef,pEngineContext,indexRef)))))
      {
    imin=max(0,FNPixel(calibratedLambda,lambdaMin,ndet,PIXEL_CLOSEST));
       imax=min(ndet-1,FNPixel(calibratedLambda,lambdaMax,ndet,PIXEL_CLOSEST));

       nsmooth=(int)floor((double)(imax-imin+1)/(calibratedLambda[imax]-calibratedLambda[imin])*2.5+0.5);

       for (int i=0;i<ndet;i++)
        Sref1[i]=Sref2[i]=Sref2Deriv[i]=Sref2Interp[i]=(double)0.;

          // Smooth structures in both reference spectra; spectra are also normalized

          KuruczSmooth(calibratedLambda,calibratedRef,ndet,nsmooth,imin,imax,1,Sref1);
    KuruczSmooth(calibratedLambda,newRef,ndet,nsmooth,imin,imax,1,Sref2);

          for (ishift=ishiftMin=0;(ishift<nshift) && !rc;ishift++)
     {
         shift[ishift]=preshiftMin+(double)ishift*step;
         coeff[ishift]=ShiftCorrel(calibratedLambda,Sref1,Sref2,Sref2Deriv,lambdas,Sref2Interp,ndet,imin,imax,&shift[ishift],&rc);

         if (!rc && (coeff[ishift]<coeff[ishiftMin]))
          ishiftMin=ishift;
     }

    shiftIni=shift[ishiftMin];

    nelmin  (ShiftCorrel,                                                                                                // I   double fn ( double x[] )    the name of the routine which evaluates the function to be minimized.
             calibratedLambda,Sref1,Sref2,Sref2Deriv,lambdas,Sref2Interp,ndet,imin,imax,&shift[ishiftMin],&rc,           // I                               arguments of ShiftCorrel
             1,                                                                                                          // I   int n                       the number of variables -> in this case, the shift
             &shiftIni,                                                                                                  // I/O double start[]              starting points for the iteration.  This data may be overwritten
             &shiftMin,                                                                                                  // O   double xmin[]               the coordinates of the point which is estimated to minimize the function.
             &coefMin,                                                                                                   // O   double *ynewlo              the minimum value of the function
     (double)1.e-3,                                                                                                      // I   double reqmin               the terminating limit for the variance of function values.
             &varstep,                                                                                                   // I   double STEP[N]              determines the size and shape of the initial simplex.  The relative magnitudes of its elements should reflect the units of the variables.
             10,                                                                                                         // I   int konvge,                 the convergence check is carried out every KONVGE iterations.
              200,                                                                                                       // I   int kcount,                 the maximum number of function evaluations.
             &nfuncEval,                                                                                                 // O   int *icount,                the number of function evaluations  used.
             &nrestart,                                                                                                  // O   int *numres,                the number of restarts.
             &rc2);                                                                                                      // O   int *ifault                 error indicator :
                                                                                                                         //                                       0, no errors detected.
                                                                                                                         //                                       1, REQMIN, N, or KONVGE has an illegal value.
                                                                                                                         //                                       2, iteration terminated because KCOUNT was exceeded without convergence.

    *pShift=shiftMin;
   }

  // Release allocated buffers

  if (lambdas!=NULL)
   MEMORY_ReleaseDVector("KuruczCalculatePreshift","lambdas",lambdas,0);
  if (Sref1!=NULL)
   MEMORY_ReleaseDVector("KuruczCalculatePreshift","Sref1",Sref1,0);
  if (Sref2!=NULL)
   MEMORY_ReleaseDVector("KuruczCalculatePreshift","Sref2",Sref2,0);
  if (Sref2Deriv!=NULL)
   MEMORY_ReleaseDVector("KuruczCalculatePreshift","Sref2Deriv",Sref2Deriv,0);
  if (Sref2Interp!=NULL)
   MEMORY_ReleaseDVector("KuruczCalculatePreshift","Sref2Interp",Sref2Interp,0);
  if (shift!=NULL)
   MEMORY_ReleaseDVector("KuruczCalculatePreshift","shift",shift,0);
  if (coeff!=NULL)
   MEMORY_ReleaseDVector("KuruczCalculatePreshift","coeff",coeff,0);

  // Return

  return rc;
 }

// =================
// KURUCZ PROCEDURES
// =================

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_SearchReference
// ----------------------------------------------------------------------------
// PURPOSE         search for a reference spectrum in analysis windows on which Kurucz has already been applied
//
// INPUT           indexRefFeno   index of the analysis window with the reference to search for in other analysis windows;
//                 indexRefColumn index to account to the second dimension of FENO (new with OMI)
//
// RETURN          indexFeno      index of the analysis window in which reference has been found;
// ----------------------------------------------------------------------------

INDEX KuruczSearchReference(INDEX indexRefFeno,INDEX indexRefColumn)
 {
  // Declarations

  INDEX indexFeno;
  FENO *pTabFeno;

  // Initialization

  const double * const reference=(TabFeno[indexRefColumn][indexRefFeno].useEtalon)?TabFeno[indexRefColumn][indexRefFeno].SrefEtalon:TabFeno[indexRefColumn][indexRefFeno].Sref;

  // Search for reference in analysis windows

  for (indexFeno=0;indexFeno<NFeno;indexFeno++)
   {
    pTabFeno=&TabFeno[indexRefColumn][indexFeno];

    if (!pTabFeno->hidden && pTabFeno->useKurucz && (indexFeno!=indexRefFeno) &&
         VECTOR_Equal((pTabFeno->useEtalon)?pTabFeno->SrefEtalon:pTabFeno->Sref, // Etalon has highest priority
                      reference,NDET[indexRefColumn],(double)1.e-7))

     break;
   }

  // Return

  return indexFeno;
 }

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_Spectrum
// ----------------------------------------------------------------------------
// PURPOSE         apply Kurucz for building a new wavelength scale to a spectrum
//
// INPUT           oldLambda      old calibration associated to reference;
//                 spectrum       spectrum to shift;
//                 reference      reference spectrum;
//                 instrFunction  instrumental function for correcting reference
//                 displayFlag    1 to display the results of the fit in the Graphs window
//                 windowTitle    title of the graph window
//                 saveFlag       1 to save the calibration results in the data Window
//                 indexFeno      index of the current sub-window
//
// OUTPUT          newLambda      new wavelength scale
//
// RETURN          return code
// ----------------------------------------------------------------------------

RC KURUCZ_Spectrum(const double *oldLambda,double *newLambda,double *spectrum,const double *reference,double *instrFunction,
                   char displayFlag, const char *windowTitle,double **coeff,double **fwhmVector,double **fwhmDeriv2,int saveFlag,INDEX indexFeno,void *responseHandle,INDEX indexFenoColumn)
{
  // Declarations

  char            string[MAX_ITEM_TEXT_LEN];
  CROSS_REFERENCE *TabCross,*pTabCross;
  CROSS_RESULTS   *pResults,*Results;                                           // pointer to results associated to a symbol
  double slitParam[NSFP],
    *shiftPoly,
    *dispAbsolu,*dispSecX,
    **fwhm,**fwhmSigma,                                            // substitution vectors
    *solar,                                                       // solar spectrum
    *offset,                                                      // offset
    Square;                                                      // Chi square returned by 'CurFitMethod'
  int              Nb_Win,maxParam,pixMin,pixMax,                                             // number of little windows
                  *NIter;                                                       // number of iterations
  INDEX            indexWindow,                                                 // browse little windows
                   indexParam,
                   indexTabCross,
                   indexCrossFit,
                   indexLine,indexColumn,                                       // position in the spreadsheet for information to write
                   i,j,k;                                                    // temporary indexes
  double j0,lambda0,shiftSign;
  RC               rc;                                                          // return code
  struct curve_data *spectrumData;
  KURUCZ *pKurucz;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  rc=ERROR_ID_NO;
  pKurucz=&KURUCZ_buffers[indexFenoColumn];
  indexLine=KURUCZ_indexLine;
  indexColumn=2;
  solar=NULL;
  const int oldNDET = NDET[indexFenoColumn];
  spectrumData=NULL;

  slitParam[0]=pSlitOptions->slitFunction.slitParam;
  slitParam[1]=pSlitOptions->slitFunction.slitParam2;
  slitParam[2]=pSlitOptions->slitFunction.slitParam3;

  pKurucz->KuruczFeno[indexFeno].have_calibration = true;

  // store calibration shift/stretch factors:
  double *calib_shift = malloc(pKurucz->Nb_Win * sizeof(*calib_shift));
  double *calib_stretch = malloc(pKurucz->Nb_Win * sizeof(*calib_stretch));
  double *calib_stretch2 = malloc(pKurucz->Nb_Win * sizeof(*calib_stretch2));

  if (((shiftPoly=(double *)MEMORY_AllocDVector("KURUCZ_Spectrum ","shiftPoly",0,oldNDET-1))==NULL) ||
      ((spectrumData=MEMORY_AllocBuffer(__func__,"spectrumData",pKurucz->Nb_Win*2,sizeof(*spectrumData),0,MEMORY_TYPE_STRUCT))==NULL))
   {
    rc=ERROR_ID_ALLOC;
    goto EndKuruczSpectrum;
  }

  // Use substitution variables

  Feno=&TabFeno[indexFenoColumn][pKurucz->indexKurucz];
  shiftSign=(Feno->indexSpectrum!=ITEM_NONE)?(double)-1.:(double)1.;            // very important !!!
  TabCross=Feno->TabCross;

  memcpy(Feno->LambdaK,oldLambda,sizeof(double)*oldNDET);
  rc=ANALYSE_XsInterpolation(Feno,oldLambda,indexFenoColumn);

  Results=Feno->TabCrossResults;
  pResults=&Feno->TabCrossResults[(Feno->indexSpectrum!=ITEM_NONE)?Feno->indexSpectrum:Feno->indexReference];

  double *VSig = pKurucz->VSig;
  double *Pcalib = pKurucz->Pcalib; // polynomial coefficients computation
  double *pixMid = pKurucz->pixMid;
  double *VLambda = pKurucz->VLambda;
  double *VShift = pKurucz->VShift;

  struct fit_properties *subwindow_fit    = pKurucz->KuruczFeno[indexFeno].subwindow_fits;

  Nb_Win     = pKurucz->Nb_Win;
  fwhm       = pKurucz->fwhm;
  fwhmSigma  = pKurucz->fwhmSigma;
  offset     = pKurucz->offset;
  NIter      = pKurucz->NIter;

  NDET[indexFenoColumn] = TabFeno[indexFenoColumn][indexFeno].NDET; // required??
  const int n_wavel = TabFeno[indexFenoColumn][indexFeno].NDET;

  for (maxParam=0;maxParam<MAX_KURUCZ_FWHM_PARAM;maxParam++)
    if ((fwhmVector[maxParam]!=NULL) && (Feno->indexFwhmParam[maxParam]!=ITEM_NONE))
      VECTOR_Init(fwhmVector[maxParam],TabCross[Feno->indexFwhmParam[maxParam]].InitParam,n_wavel);
    else
      break;

  // Instrumental correction

  memcpy(ANALYSE_absolu,ANALYSE_zeros,sizeof(double)*n_wavel);
  memcpy(offset,ANALYSE_zeros,sizeof(double)*n_wavel);

  if (instrFunction!=NULL)
    for (i=0;i<n_wavel;i++)
      spectrum[i]/=(double)instrFunction[i];

  // Always restart from the original calibration

  Lambda=newLambda;
  LambdaSpec=newLambda;
  memcpy(Lambda,oldLambda,sizeof(double)*n_wavel);
  memcpy(ANALYSE_secX,spectrum,sizeof(double)*n_wavel);

  // Set solar spectrum

  if ((solar=MEMORY_AllocDVector(__func__,"solar",0,n_wavel))==NULL)
    rc=ERROR_ID_ALLOC;
  else if (!pKuruczOptions->fwhmFit) 
   {
    if (pSlitOptions->slitFunction.slitType==SLIT_TYPE_NONE) 
     {
      if ((pKurucz->hrSolar.nl==n_wavel) && VECTOR_Equal(pKurucz->hrSolar.matrix[0],oldLambda,n_wavel,(double)1.e-7))
        memcpy(solar,pKurucz->hrSolar.matrix[1],sizeof(double)*n_wavel);
      else if (!(rc=SPLINE_Vector(pKurucz->hrSolar.matrix[0],pKurucz->hrSolar.matrix[1],pKurucz->hrSolar.deriv2[1],pKurucz->hrSolar.nl,
                                  pKurucz->hrSolarGridded.matrix[0],pKurucz->hrSolarGridded.matrix[1],pKurucz->hrSolarGridded.nl,pAnalysisOptions->interpol)))
       rc=SPLINE_Deriv2(pKurucz->hrSolarGridded.matrix[0],pKurucz->hrSolarGridded.matrix[1],pKurucz->hrSolarGridded.deriv2[1],pKurucz->hrSolarGridded.nl,__func__);
      //  SPLINE_Vector(pKurucz->hrSolar.matrix[0],pKurucz->hrSolar.matrix[1],pKurucz->hrSolar.deriv2[1],pKurucz->hrSolar.nl,
      //                oldLambda,solar,n_wavel,pAnalysisOptions->interpol);
     } 
    else 
     {
      // 20130208 : a high resolution spectrum is now loaded from the slit page of project properties and convolved
      // 20210924 : convolved on a high resolution grid (0.01 nm)
//       rc=ANALYSE_ConvoluteXs(NULL,ANLYS_CROSS_ACTION_CONVOLUTE,(double)0.,&pKurucz->hrSolar,
//                              ANALYSIS_slitMatrix,slitParam,pSlitOptions->slitFunction.slitType,
//                              oldLambda,solar,0,n_wavel,n_wavel,0,pSlitOptions->slitFunction.slitWveDptFlag);
//      
      if (!(rc=ANALYSE_ConvoluteXs(NULL,ANLYS_CROSS_ACTION_CONVOLUTE,(double)0.,&pKurucz->hrSolar,
                             ANALYSIS_slitMatrix,slitParam,pSlitOptions->slitFunction.slitType,
                             pKurucz->hrSolarGridded.matrix[0],pKurucz->hrSolarGridded.matrix[1],0,pKurucz->hrSolarGridded.nl,pKurucz->hrSolarGridded.nl,0,pSlitOptions->slitFunction.slitWveDptFlag)))
       rc=SPLINE_Deriv2(pKurucz->hrSolarGridded.matrix[0],pKurucz->hrSolarGridded.matrix[1],pKurucz->hrSolarGridded.deriv2[1],pKurucz->hrSolarGridded.nl,__func__);
     }
   }  
  else 
   {
    memcpy(solar,reference,sizeof(double)*n_wavel);
   }

  if (rc!=ERROR_ID_NO)
    goto EndKuruczSpectrum;

  // Buffers for fits initialization

  if (pKurucz->crossFits.matrix!=NULL)
    for (indexTabCross=0;indexTabCross<pKurucz->crossFits.nc;indexTabCross++)
      memcpy(pKurucz->crossFits.matrix[indexTabCross],ANALYSE_zeros,sizeof(double)*n_wavel);

  memcpy(ANALYSE_t,ANALYSE_zeros,sizeof(double)*n_wavel);
  memcpy(ANALYSE_tc,ANALYSE_zeros,sizeof(double)*n_wavel);

  ANALYSE_plotKurucz=(pKurucz->displaySpectra || pKurucz->displayResidual || pKurucz->displayFit || pKurucz->displayShift)?1:0;

  if (ANALYSE_plotKurucz) {
    if (ANALYSE_swathSize>1) {
      sprintf(string,"Row %d/%d",indexFenoColumn+1,ANALYSE_swathSize);
      mediateResponseCellDataString(plotPageCalib,indexLine++,1,string,responseHandle);
    }

    if (!TabFeno[indexFenoColumn][indexFeno].hidden)
      mediateResponseCellInfo(plotPageCalib,indexLine++,indexColumn,responseHandle,"KURUCZ alignment for window ","%s",TabFeno[indexFenoColumn][indexFeno].windowName);
    else
      mediateResponseCellDataString(plotPageCalib,indexLine++,indexColumn,"Kurucz",responseHandle);

    if (pKuruczOptions->preshiftFlag)
     mediateResponseCellInfo(plotPageCalib,indexLine++,indexColumn,responseHandle,"Preshift (nm)","%g",TabFeno[indexFenoColumn][indexFeno].preshift);

    mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Window",responseHandle);
    mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Pixel",responseHandle);
    mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Wavelength",responseHandle);
    mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Niter",responseHandle);
    mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Shift",responseHandle);

    if (pKuruczOptions->fwhmFit)
      for (indexParam=0;indexParam<maxParam;indexParam++) {
        sprintf(string,"SFP %d",indexParam+1);
        mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,string,responseHandle);
      }

    if ((Feno->indexOffsetConst!=ITEM_NONE) && (Feno->TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE))
      mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,"Offset",responseHandle);

    for (indexTabCross=0;indexTabCross<Feno->NTabCross;indexTabCross++) {
      pTabCross=&TabCross[indexTabCross];

      if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS))
        mediateResponseCellDataString(plotPageCalib,indexLine,indexColumn++,WorkSpace[pTabCross->Comp].symbolName,responseHandle);
    }
  }

  // Browse little windows

  for (indexWindow=0,Square=(double)0.;(indexWindow<Nb_Win) && (rc<THREAD_EVENT_STOP);indexWindow++) {
    Feno->Decomp=1;
    NIter[indexWindow]=0;

    dispAbsolu=pKurucz->dispAbsolu[indexWindow];
    dispSecX=pKurucz->dispSecX[indexWindow];

    for (int i=0;i<n_wavel;i++)
     dispAbsolu[i]=dispSecX[i]=(double)0.;

    if (pKuruczOptions->fwhmFit)
      pKURUCZ_fft=&pKurucz->KuruczFeno[indexFeno].fft[indexWindow];

    // Global initializations

#if defined(__DEBUG_) && __DEBUG_
    // DEBUG_Start(ENGINE_dbgFile,"Kurucz",DEBUG_FCTTYPE_MATH|DEBUG_FCTTYPE_APPL,5,DEBUG_DVAR_YES,0); // !debugResetFlag++);
#endif

    if (((rc=ANALYSE_SvdInit(&TabFeno[indexFenoColumn][pKurucz->indexKurucz], &subwindow_fit[indexWindow], n_wavel, Lambda))!=ERROR_ID_NO) ||

        // Analysis method

        ((rc=ANALYSE_CurFitMethod(indexFenoColumn,                            // to change a little bit later for OMI
                                  spectrum,                                   // spectrum
                                  NULL,                                       // no error on previous spectrum
                                  solar,                                      // reference (Kurucz)
                                  n_wavel,
                                  NULL,
                                  &Square,                                     // returned stretch order 2
                                  &NIter[indexWindow],
                                  1.,1.,
                                  &subwindow_fit[indexWindow]))>0))
      break;

#if defined(__DEBUG_) && __DEBUG_
    // DEBUG_Stop("Kurucz");
#endif


    // Fill A SVD system

    pixMid[indexWindow+1]=(double)( spectrum_start(subwindow_fit[indexWindow].specrange)
                                    + spectrum_end(subwindow_fit[indexWindow].specrange) )*0.5;

    VSig[indexWindow+1]=pResults->SigmaShift;

    calib_shift[indexWindow] = pResults->Shift;
    calib_stretch[indexWindow] = pResults->Stretch;
    calib_stretch2[indexWindow] = pResults->Stretch2;

    VShift[indexWindow+1]=pResults->Shift;                          // In order to be in accordance with the preshift, we keep the sign now.  Before (Feno->indexSpectrum!=ITEM_NONE)?(double)-pResults->Shift:(double)pResults->Shift;

    // TODO:
    //
    //  - VLambda is used to fit FWHM as function of Lambda, but could be done as function of pixel, too?
    VLambda[indexWindow+1]=(fabs(pixMid[indexWindow+1]-floor(pixMid[indexWindow+1]))<(double)0.1)?
      (double)oldLambda[(INDEX)pixMid[indexWindow+1]]-shiftSign*VShift[indexWindow+1]:
      (double)0.5*(oldLambda[(INDEX)floor(pixMid[indexWindow+1])]+oldLambda[(INDEX)floor(pixMid[indexWindow+1]+1.)])-shiftSign*VShift[indexWindow+1];

    // Store fwhm for future use

    if (pKuruczOptions->fwhmFit)
      for (indexParam=0;indexParam<maxParam;indexParam++) {
        if ((indexParam==1) && (pKuruczOptions->fwhmType==SLIT_TYPE_AGAUSS))
          fwhm[indexParam][indexWindow]=Feno->TabCrossResults[Feno->indexFwhmParam[indexParam]].Param;  // asymmetric factor can be negatif for asymmetric gaussian
        else
          fwhm[indexParam][indexWindow]=fabs(Feno->TabCrossResults[Feno->indexFwhmParam[indexParam]].Param);

        fwhmSigma[indexParam][indexWindow]=Feno->TabCrossResults[Feno->indexFwhmParam[indexParam]].SigmaParam;
      }

    // Store fit for display

    if (displayFlag) {
      if (pKurucz->method==OPTICAL_DENSITY_FIT)
       {
        for (i=SvdPDeb;i<=SvdPFin;i++)
         {
          dispAbsolu[i]=ANALYSE_absolu[i];
          dispSecX[i]=ANALYSE_secX[i]=exp(log(spectrum[i])+ANALYSE_absolu[i]);
         }
       }
      else
       {
           for (i=SvdPDeb;i<=SvdPFin;i++)
            {
          dispAbsolu[i]=ANALYSE_absolu[i]=(ANALYSE_tc[i]!=(double)0.)?ANALYSE_absolu[i]/ANALYSE_tc[i]:(double)0.;
          dispSecX[i]=ANALYSE_secX[i]=exp(log(spectrum[i])+ANALYSE_absolu[i]/ANALYSE_tc[i]); // spectrum[i]+solar[i]*ANALYSE_absolu[i]/ANALYSE_tc[i];
         }
       }

      j0=(double)(SvdPDeb+SvdPFin)*0.5;
      lambda0=(fabs(j0-floor(j0))<(double)0.1)?
        (double)ANALYSE_splineX[(INDEX)j0]:
        (double)0.5*(ANALYSE_splineX[(INDEX)floor(j0)]+ANALYSE_splineX[(INDEX)floor(j0+1.)]);

      if ((Feno->indexOffsetConst!=ITEM_NONE) &&
          (Feno->indexOffsetOrder1!=ITEM_NONE) &&
          (Feno->indexOffsetOrder2!=ITEM_NONE) &&

          ((TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE) ||
           (TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE) ||
           (TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE) ||
           (TabCross[Feno->indexOffsetConst].InitParam!=(double)0.) ||
           (TabCross[Feno->indexOffsetOrder1].InitParam!=(double)0.) ||
           (TabCross[Feno->indexOffsetOrder2].InitParam!=(double)0.)))

        for (i=SvdPDeb;i<=SvdPFin;i++) {
          offset[i]=(double)1.-Feno->xmean*(Results[Feno->indexOffsetConst].Param+
                                            Results[Feno->indexOffsetOrder1].Param*(ANALYSE_splineX[i]-lambda0)+
                                            Results[Feno->indexOffsetOrder2].Param*(ANALYSE_splineX[i]-lambda0)*(ANALYSE_splineX[i]-lambda0))/spectrum[i];
          offset[i]=(offset[i]>(double)0.)?log(offset[i]):(double)0.;
        }

      if (pKurucz->crossFits.matrix!=NULL)

        for (indexTabCross=indexCrossFit=0;(indexTabCross<Feno->NTabCross) && (indexCrossFit<pKurucz->crossFits.nc);indexTabCross++) {
          pTabCross=&TabCross[indexTabCross];

          if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) && pTabCross->display) {
            for (i=SvdPDeb,k=1;i<=SvdPFin;i++,k++)
              pKurucz->crossFits.matrix[indexCrossFit][i]=x[pTabCross->IndSvdA]*subwindow_fit[indexWindow].A[pTabCross->IndSvdA][k];

            indexCrossFit++;
          }
        }
    }

    // Results safe keeping

    memcpy(pKurucz->KuruczFeno[indexFeno].results[indexWindow],Feno->TabCrossResults,sizeof(CROSS_RESULTS)*Feno->NTabCross);

    pKurucz->KuruczFeno[indexFeno].wve[indexWindow]=VLambda[indexWindow+1];
    pKurucz->KuruczFeno[indexFeno].chiSquare[indexWindow]=Square;
    pKurucz->KuruczFeno[indexFeno].rms[indexWindow]=(Square>(double)0.)?sqrt(Square):(double)0.;
    pKurucz->KuruczFeno[indexFeno].nIter[indexWindow]=NIter[indexWindow];

    pKurucz->KuruczFeno[indexFeno].rc=rc;
  }  // End for (indexWindow=...

  if (rc)
    goto EndKuruczSpectrum;

  SvdPDeb=spectrum_start(subwindow_fit[0].specrange);
  SvdPFin=spectrum_end(subwindow_fit[Nb_Win-1].specrange);

  // New wavelength scale (corrected calibration)
  // NB : we fit a polynomial in Lambda+shift point but it's possible to fit a polynomial in shift points by replacing
  //      VLambda by VShift in the following instruction

  if ((rc=LINEAR_fit_poly(Nb_Win, pKurucz->shiftDegree, pixMid, NULL, VShift, Pcalib))!=ERROR_ID_NO)
    goto EndKuruczSpectrum;

  if (pKuruczOptions->fwhmFit) {
    for (indexParam=0;indexParam<maxParam;indexParam++)
      if (TabCross[Feno->indexFwhmParam[indexParam]].FitParam!=ITEM_NONE) {
        memcpy(coeff[indexParam],ANALYSE_zeros,sizeof(double)*(pKurucz->fwhmDegree+1));
        memcpy(fwhmVector[indexParam],ANALYSE_zeros,sizeof(double)*n_wavel);

        if ((rc=LINEAR_fit_poly(Nb_Win, pKurucz->fwhmDegree,VLambda,NULL,fwhm[indexParam]-1,coeff[indexParam]-1))!=ERROR_ID_NO)
          goto EndKuruczSpectrum;
      }

    if (rc!=ERROR_ID_NO)
      goto EndKuruczSpectrum;
  }

  if (Nb_Win > 1) { // multiple subwindows -> fit a polynomial through the shift values.
    for (i=0;i<n_wavel;i++) {
      shiftPoly[i]=Pcalib[pKurucz->shiftDegree+1];
      for (j=pKurucz->shiftDegree;j>=1;j--) {
        shiftPoly[i]=shiftPoly[i]*(double)i+Pcalib[j];
      }
      Lambda[i]=oldLambda[i]-shiftSign*shiftPoly[i];
    }
  } else {
    // if Nb_Win == 1, use fitted shift/stretch from calibration procedure to create corrected lambda grid:

    int i_center = floor(0.5*(SvdPDeb + SvdPFin));
    double lambda0 = 0.5*(oldLambda[i_center] + oldLambda[1+i_center]);

    for (int i=0; i<n_wavel; ++i) {
      const double x0 = oldLambda[i] - lambda0;
      shiftPoly[i] = calib_shift[0] + calib_stretch[0]*x0 + calib_stretch2[0]*x0*x0;
      Lambda[i] = oldLambda[i] - shiftSign*shiftPoly[i];
    }
  }

  if (displayFlag) {
    // Display complete fit

    if (pKurucz->displaySpectra) {
      if (ANALYSE_swathSize==1)
        strcpy(string,"Complete fit");
      else
        sprintf(string,"Complete fit (%d/%d)",indexFenoColumn+1,ANALYSE_swathSize);

     if (pKuruczOptions->divisionMode==PRJCT_CALIB_WINDOWS_CONTIGUOUS) {
       MEDIATE_PLOT_CURVES(plotPageCalib, Spectrum, forceAutoScale, string, "Wavelength (nm)", "Intensity", responseHandle,
                           CURVE(.name="Spectrum", .x=Lambda, .y=spectrum, .length=n_wavel),
                           CURVE(.name="Adjusted Kurucz", .x=Lambda, .y=ANALYSE_secX, .length=n_wavel));
      }
     else
      {
       for (indexWindow=0;indexWindow<Nb_Win;indexWindow++)
        {
         pixMin=spectrum_start(subwindow_fit[indexWindow].specrange);
         pixMax=spectrum_end(subwindow_fit[indexWindow].specrange);

         spectrumData[2*indexWindow] = CURVE(.name="Spectrum", .x=&Lambda[pixMin], .y=&spectrum[pixMin], .length=pixMax-pixMin+1, .style=(indexWindow%2)?DashLine:Line, .number=0);
         spectrumData[2*indexWindow+1] = CURVE(.name="Adjusted Kurucz", .x=&Lambda[pixMin], .y=&pKurucz->dispSecX[indexWindow][pixMin], .length=pixMax-pixMin+1, .style=(indexWindow%2)?DashLine:Line, .number=1);
        }
       mediateResponsePlotData(plotPageCalib,spectrumData,Nb_Win*2,Spectrum,forceAutoScale,string,"Wavelength (nm)","Intensity", responseHandle);
      }
    }

    // Display residual

    if (pKurucz->displayResidual) {

      if (ANALYSE_swathSize==1)
        strcpy(string,"Residual");
      else
        sprintf(string,"Residual (%d/%d)",indexFenoColumn+1,ANALYSE_swathSize);

      if (pKuruczOptions->divisionMode==PRJCT_CALIB_WINDOWS_CONTIGUOUS) {
        MEDIATE_PLOT_CURVES(plotPageCalib,Spectrum,forceAutoScale,string,"Wavelength (nm)","", responseHandle,
                            CURVE(.name="Residual", .x=&Lambda[0], .y=&ANALYSE_absolu[0], .length=n_wavel));
      } else {
        for (indexWindow=0;indexWindow<Nb_Win;indexWindow++) {
          pixMin=spectrum_start(subwindow_fit[indexWindow].specrange);
          pixMax=spectrum_end(subwindow_fit[indexWindow].specrange);
          spectrumData[indexWindow] = CURVE(.name="Residual", .x=&Lambda[pixMin], .y=&pKurucz->dispAbsolu[indexWindow][pixMin], .length=pixMax-pixMin+1, .style=(indexWindow%2)?DashLine:Line, .number=0);
        }
        mediateResponsePlotData(plotPageCalib,spectrumData,Nb_Win,Spectrum,forceAutoScale,string,"Wavelength (nm)","", responseHandle);
      }
    }

    memcpy(ANALYSE_secX,ANALYSE_zeros,sizeof(double)*n_wavel);

    // Display Offset

    if  (// Feno->displayOffset &&
         (Feno->indexOffsetConst!=ITEM_NONE) &&
         (Feno->indexOffsetOrder1!=ITEM_NONE) &&
         (Feno->indexOffsetOrder2!=ITEM_NONE) &&

         ((TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE) ||
          (TabCross[Feno->indexOffsetOrder1].FitParam!=ITEM_NONE) ||
          (TabCross[Feno->indexOffsetOrder2].FitParam!=ITEM_NONE) ||
          (TabCross[Feno->indexOffsetConst].InitParam!=(double)0.) ||
          (TabCross[Feno->indexOffsetOrder1].InitParam!=(double)0.) ||
          (TabCross[Feno->indexOffsetOrder2].InitParam!=(double)0.))) {

      if (pKurucz->displayFit) {
        if (ANALYSE_swathSize==1)
          strcpy(string,"Offset");
        else
          sprintf(string,"Offset (%d/%d)",indexFenoColumn+1,ANALYSE_swathSize);

        if (pKuruczOptions->divisionMode==PRJCT_CALIB_WINDOWS_CONTIGUOUS) {
          for (j=SvdPDeb;j<=SvdPFin;j++) // !!!!!!!!!!!!!!!!!!! to test
           {
            ANALYSE_absolu[j]+=offset[j]-ANALYSE_secX[j];
            ANALYSE_secX[j]=offset[j];
           }
          MEDIATE_PLOT_CURVES(plotPageCalib,Spectrum,forceAutoScale,string,"Wavelength (nm)","",responseHandle,
                              CURVE(.name="Measured", .x=Lambda, .y=ANALYSE_absolu, .length=n_wavel),
                              CURVE(.name="Calculated", .x=Lambda, .y=ANALYSE_secX, .length=n_wavel));
        } else {
          for (indexWindow=0;indexWindow<Nb_Win;indexWindow++) {
            pixMin=spectrum_start(subwindow_fit[indexWindow].specrange);
            pixMax=spectrum_end(subwindow_fit[indexWindow].specrange);

            for (j=pixMin;j<=pixMax;j++) {
              pKurucz->dispAbsolu[indexWindow][j]+=offset[j]-pKurucz->dispSecX[indexWindow][j];
              pKurucz->dispSecX[indexWindow][j]=offset[j];
            }

            spectrumData[(indexWindow<<1)] = CURVE(.name="Measured", .x=&Lambda[pixMin], .y=&pKurucz->dispAbsolu[indexWindow][pixMin], .length=pixMax-pixMin+1, .style=(indexWindow%2)?DashLine:Line, .number=0);
            spectrumData[(indexWindow<<1)+1] = CURVE(.name="Calculated", .x=&Lambda[pixMin], .y=&pKurucz->dispSecX[indexWindow][pixMin], .length=pixMax-pixMin+1, .style=(indexWindow%2)?DashLine:Line, .number=0);
          }
          mediateResponsePlotData(plotPageCalib,spectrumData,Nb_Win,Spectrum,forceAutoScale,string,"Wavelength (nm)","", responseHandle);
         }
      }
    }

    // Display subwindow_fits

    if (pKurucz->crossFits.matrix!=NULL) {
      for (indexTabCross=indexCrossFit=0;(indexTabCross<Feno->NTabCross) && (indexCrossFit<pKurucz->crossFits.nc);indexTabCross++) {
        pTabCross=&TabCross[indexTabCross];

        if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS) && pTabCross->display) {

          if (ANALYSE_swathSize==1)
            sprintf(string,"%s fit",WorkSpace[pTabCross->Comp].symbolName);
          else
            sprintf(string,"%s fit (%d/%d)",WorkSpace[pTabCross->Comp].symbolName,indexFenoColumn+1,ANALYSE_swathSize);

          if (pKuruczOptions->divisionMode==PRJCT_CALIB_WINDOWS_CONTIGUOUS) {
            for (j=SvdPDeb;j<=SvdPFin;j++) {
              ANALYSE_absolu[j]+=pKurucz->crossFits.matrix[indexCrossFit][j]-ANALYSE_secX[j];
              ANALYSE_secX[j]=pKurucz->crossFits.matrix[indexCrossFit][j];
            }

            MEDIATE_PLOT_CURVES(plotPageCalib,Spectrum,forceAutoScale,string,"Wavelength (nm)","", responseHandle,
                                CURVE(.name="Measured", .x=Lambda, .y=ANALYSE_absolu, .length=n_wavel),
                                CURVE(.name="Calculated", .x=Lambda, .y=ANALYSE_secX, .length=n_wavel));

          } else {
            for (indexWindow=0;indexWindow<Nb_Win;indexWindow++) {
              pixMin=spectrum_start(subwindow_fit[indexWindow].specrange);
              pixMax=spectrum_end(subwindow_fit[indexWindow].specrange);

              for (j=pixMin;j<=pixMax;j++) {
                pKurucz->dispAbsolu[indexWindow][j]+=pKurucz->crossFits.matrix[indexCrossFit][j]-pKurucz->dispSecX[indexWindow][j];
                pKurucz->dispSecX[indexWindow][j]=pKurucz->crossFits.matrix[indexCrossFit][j];
              }

              spectrumData[(indexWindow<<1)] = CURVE(.name="Measured", .x=&Lambda[pixMin], .y=&pKurucz->dispAbsolu[indexWindow][pixMin], .length=pixMax-pixMin+1, .style=(indexWindow%2)?DashLine:Line, .number=0);
              spectrumData[(indexWindow<<1)+1] = CURVE(.name="Calculated", .x=&Lambda[pixMin], .y=&pKurucz->dispSecX[indexWindow][pixMin], .length=pixMax-pixMin+1, .style=(indexWindow%2)?DashLine:Line, .number=1);
            }
            mediateResponsePlotData(plotPageCalib,spectrumData,Nb_Win,Spectrum,forceAutoScale,string,"Wavelength (nm)","", responseHandle);
          }
          indexCrossFit++;
        }
      }
    }

    // Display error on calibration

    if (pKurucz->displayShift) {
      if (ANALYSE_swathSize==1)
        strcpy(string,"Shift");
      else
        sprintf(string,"Shift (%d/%d)",indexFenoColumn+1,ANALYSE_swathSize);

      MEDIATE_PLOT_CURVES(plotPageCalib,Spectrum,forceAutoScale,string,"Wavelength (nm)","Shift (nm)", responseHandle,
                          CURVE(.name="Polynomial fitting individual shift points", .x=Lambda, .y=shiftPoly, .length=n_wavel),
                          CURVE(.name="Shift calculated in the individual small windows", .x=VLambda+1, .y=VShift+1, .length=Nb_Win, .style=Point));
    }

    // Display wavelength dependence of fwhm

    if (pKuruczOptions->fwhmFit) {
      for (indexParam=0;(indexParam<maxParam) && (rc<THREAD_EVENT_STOP);indexParam++) {
        if (ANALYSE_swathSize==1)
          sprintf(string,"SFP %d",indexParam+1);
        else
          sprintf(string,"SFP %d (%d/%d)",indexParam+1,indexFenoColumn+1,ANALYSE_swathSize);

        if (TabCross[Feno->indexFwhmParam[indexParam]].FitParam!=ITEM_NONE) {
          for (i=0;i<n_wavel;i++) {
            fwhmVector[indexParam][i]=(double)coeff[indexParam][pKurucz->fwhmDegree];
            for (j=pKurucz->fwhmDegree-1;j>=0;j--)
              fwhmVector[indexParam][i]=fwhmVector[indexParam][i]*(double)Lambda[i]+coeff[indexParam][j];
          }

          if ((rc=SPLINE_Deriv2(Lambda,fwhmVector[indexParam],fwhmDeriv2[indexParam],n_wavel,__func__))!=0)
            goto EndKuruczSpectrum;
        }

        if (pKurucz->displayShift) {
          MEDIATE_PLOT_CURVES(plotPageCalib,Spectrum,forceAutoScale,string,"Wavelength (nm)",(pKuruczOptions->fwhmType==SLIT_TYPE_FILE)?"":"SFP (nm)", responseHandle,
                              CURVE(.name="Polynomial fitting individual FWHM points", .x=Lambda, .y=fwhmVector[indexParam], .length=n_wavel),
                              CURVE(.name="FWHM calculated in the individual small windows", .x=VLambda+1, .y=fwhm[indexParam], .length=Nb_Win, .style=Point));
        }
      }
    }

    mediateResponseLabelPage(plotPageCalib, "", "Kurucz", responseHandle);

    for (indexWindow=0,indexLine+=1;indexWindow<Nb_Win;indexWindow++,indexLine++) {
      indexColumn=2;

      mediateResponseCellInfoNoLabel(plotPageCalib,indexLine,indexColumn++,responseHandle,"%2d/%d",indexWindow+1,Nb_Win);
      mediateResponseCellDataDouble(plotPageCalib,indexLine,indexColumn++,pixMid[indexWindow+1],responseHandle);
      mediateResponseCellDataDouble(plotPageCalib,indexLine,indexColumn++,VLambda[indexWindow+1],responseHandle);
      mediateResponseCellDataInteger(plotPageCalib,indexLine,indexColumn++,NIter[indexWindow],responseHandle);
      mediateResponseCellInfoNoLabel(plotPageCalib,indexLine,indexColumn++,responseHandle,"%10.3e+/-%10.3e",VShift[indexWindow+1],VSig[indexWindow+1]);

      for (indexParam=0;indexParam<maxParam;indexParam++)
        mediateResponseCellInfoNoLabel(plotPageCalib,indexLine,indexColumn++,responseHandle,"%10.3e+/-%10.3e",fwhm[indexParam][indexWindow],fwhmSigma[indexParam][indexWindow]);

      if ((Feno->indexOffsetConst!=ITEM_NONE) && (Feno->TabCross[Feno->indexOffsetConst].FitParam!=ITEM_NONE))
        mediateResponseCellInfoNoLabel(plotPageCalib,indexLine,indexColumn++,responseHandle,"%10.3e+/-%10.3e",Feno->TabCrossResults[Feno->indexOffsetConst].Param,Feno->TabCrossResults[Feno->indexOffsetConst].SigmaParam);

      for (indexTabCross=0;indexTabCross<Feno->NTabCross;indexTabCross++) {
        pTabCross=&TabCross[indexTabCross];

        if (pTabCross->IndSvdA && (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS))
          mediateResponseCellInfoNoLabel(plotPageCalib,indexLine,indexColumn++,responseHandle,"%10.3e+/-%10.3e",
                                         pKurucz->KuruczFeno[indexFeno].results[indexWindow][indexTabCross].SlntCol,
                                         pKurucz->KuruczFeno[indexFeno].results[indexWindow][indexTabCross].SlntErr);
      }
    }
  }

EndKuruczSpectrum:

  free(calib_shift);
  free(calib_stretch);
  free(calib_stretch2);

  KURUCZ_indexLine=indexLine+1;

  if (spectrumData!=NULL)
   MEMORY_ReleaseBuffer(__func__,"spectrumData",spectrumData);

  if (solar!=NULL)
   MEMORY_ReleaseDVector(__func__,"solar",solar,0);

  if (shiftPoly!=NULL)
   MEMORY_ReleaseDVector(__func__,"shiftPoly",shiftPoly,0);

  // Return

  NDET[indexFenoColumn]=oldNDET;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
}

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_ApplyCalibration
// ----------------------------------------------------------------------------
// PURPOSE         Apply the new calibration to cross sections to interpolate
//                 or to convolute and recalculate gaps
//
// INPUT           pTabFeno      original calibration
//                 newLambda     the new wavelength calibration to apply
//
// RETURN          return code
// ----------------------------------------------------------------------------

RC KURUCZ_ApplyCalibration(FENO *pTabFeno,double *newLambda,INDEX indexFenoColumn)
{
  // Declarations

  MATRIX_OBJECT slitMatrix[NSFP];
  double slitParam[NSFP];
  INDEX indexWindow,i;
  int newDimL = 0;
  RC rc;

  // Initializations
  const int n_wavel = NDET[indexFenoColumn];
  memset(slitMatrix,0,sizeof(MATRIX_OBJECT)*NSFP);
  rc=0;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Rebuild gaps

  doas_spectrum *new_range = spectrum_new();
  for (indexWindow = 0; indexWindow < pTabFeno->fit_properties.Z; indexWindow++) {
    int pixel_start = FNPixel(newLambda,pTabFeno->fit_properties.LFenetre[indexWindow][0],pTabFeno->NDET,PIXEL_AFTER);
    int pixel_end = FNPixel(newLambda,pTabFeno->fit_properties.LFenetre[indexWindow][1],pTabFeno->NDET,PIXEL_BEFORE);

    spectrum_append(new_range, pixel_start, pixel_end);

    newDimL += pixel_end - pixel_start +1;
  }

  if (newDimL != pTabFeno->fit_properties.DimL) {
    // reallocate complete FIT_PROPERTIES structure.
    FIT_PROPERTIES_free("KURUCZ_ApplyCalibration ",&pTabFeno->fit_properties);
    pTabFeno->fit_properties.DimL=newDimL;
    FIT_PROPERTIES_alloc("KURUCZ_ApplyCalibration ",&pTabFeno->fit_properties);
  } else if(pTabFeno->fit_properties.specrange != NULL) {
    // only update specrange
    spectrum_destroy(pTabFeno->fit_properties.specrange);
  }

  pTabFeno->fit_properties.specrange = new_range;

  // Force decomposition

  pTabFeno->Decomp=1;

  for (i=0;i<NSFP;i++)
   slitParam[i]=(double)0.;

  if (pKuruczOptions->fwhmType==SLIT_TYPE_INVPOLY)
   slitParam[1]=(double)pKuruczOptions->invPolyDegree;


  if (pTabFeno->xsToConvolute &&                                                                // slit function to convolute
      (pTabFeno->useKurucz==ANLYS_KURUCZ_REF || pTabFeno->useKurucz==ANLYS_KURUCZ_SPEC) &&    // calibration applied on the reference or the spectrum
      pKuruczOptions->fwhmFit)                                                               // fit of the slit function
   {
       if (pKuruczOptions->fwhmType==SLIT_TYPE_FILE)                                                // slit function is file type
        {
            if (!(rc=MATRIX_Copy(&slitMatrix[0],&KURUCZ_buffers[indexFenoColumn].slitFunction,__func__)) &&
                !(rc=MATRIX_Allocate(&slitMatrix[1],n_wavel,3,0,0,1,__func__)))
             {
        memcpy(slitMatrix[1].matrix[0],newLambda,sizeof(double)*n_wavel);
        memcpy(slitMatrix[1].matrix[1],pTabFeno->fwhmVector[0],sizeof(double)*n_wavel);
        memcpy(slitMatrix[1].matrix[2],pTabFeno->fwhmVector[1],sizeof(double)*n_wavel);

        if (!(rc=SPLINE_Deriv2(slitMatrix[1].matrix[0],slitMatrix[1].matrix[1],slitMatrix[1].deriv2[1],slitMatrix[1].nl,__func__)))
          rc=SPLINE_Deriv2(slitMatrix[1].matrix[0],slitMatrix[1].matrix[2],slitMatrix[1].deriv2[2],slitMatrix[1].nl,__func__);
             }
     }
    else
     {
         for (i=0;(i<NSFP) && !rc;i++)
          if (pTabFeno->fwhmVector[i]!=NULL)
           {
         if (MATRIX_Allocate(&slitMatrix[i],n_wavel,2,0,0,1,__func__)!=0)
          rc=ERROR_ID_ALLOC;
         else
          {
           memcpy(slitMatrix[i].matrix[0],newLambda,sizeof(double)*n_wavel);
           memcpy(slitMatrix[i].matrix[1],pTabFeno->fwhmVector[i],sizeof(double)*n_wavel);

           rc=SPLINE_Deriv2(slitMatrix[i].matrix[0],slitMatrix[i].matrix[1],slitMatrix[i].deriv2[1],slitMatrix[i].nl,__func__);
          }
        }
     }
   }

  if (!rc &&
     (((pTabFeno->rcKurucz=ANALYSE_XsInterpolation(pTabFeno,newLambda,indexFenoColumn))!=ERROR_ID_NO) ||
     (((pTabFeno->useKurucz==ANLYS_KURUCZ_REF) || (pTabFeno->useKurucz==ANLYS_KURUCZ_SPEC)) &&
      ((pKuruczOptions->fwhmFit && (pKuruczOptions->fwhmType!=SLIT_TYPE_FILE) && ((pTabFeno->rcKurucz=ANALYSE_XsConvolution(pTabFeno,newLambda,slitMatrix,slitParam,pKuruczOptions->fwhmType,indexFenoColumn,1))!=ERROR_ID_NO)) ||
       (pKuruczOptions->fwhmFit && (pKuruczOptions->fwhmType==SLIT_TYPE_FILE) && ((pTabFeno->rcKurucz=ANALYSE_XsConvolution(pTabFeno,newLambda,slitMatrix,slitParam,pKuruczOptions->fwhmType,indexFenoColumn,1))!=ERROR_ID_NO)) ||
      (!pKuruczOptions->fwhmFit && ((pTabFeno->rcKurucz=ANALYSE_XsConvolution(pTabFeno,newLambda,ANALYSIS_slitMatrix,ANALYSIS_slitParam,pSlitOptions->slitFunction.slitType,indexFenoColumn,pSlitOptions->slitFunction.slitWveDptFlag))!=ERROR_ID_NO))))))

   rc=pTabFeno->rcKurucz;

  // Release allocated matrix object

  for (i=0;i<NSFP;i++)
   MATRIX_Free(&slitMatrix[i],__func__);

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  // Return

  return rc;
 }

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_Reference
// ----------------------------------------------------------------------------
// PURPOSE         browse analysis windows and apply Kurucz if needed on reference spectrum
//
// INPUT           instrFunction  instrumental function to apply on reference spectrum
//                 refFlag        1 to apply Kurucz on the daily selected reference spectrum
//                 saveFlag       1 to save the calibration results in the data Window
//
//            in the case of satellites measurements :
//
//                 gomeRefFlag=0 means that irradiance is used as etalon spectrum
//                 gomeRefFlag=1 means that a reference spectrum is given
//
// RETURN          return code
// ----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
RC KURUCZ_Reference(double *instrFunction,INDEX refFlag,int saveFlag,int gomeFlag,void *responseHandle,INDEX indexFenoColumn) {
  // Declarations

  FENO            *pTabFeno,*pTabRef,                                           // browse analysis windows
                  *pKuruczFeno;                                                 // points to the calibration window

  double          *reference;                                                   // reference spectrum to align on Kurucz
  int              maxParam,
                   msgCount,
                   nKuruczFeno,
                   nBadKuruczFeno;
  INDEX            indexFeno,                                                   // browse analysis windows
                   indexRef;                                                    // index of another analysis window with the same reference spectrum
  KURUCZ *pKurucz;

  MATRIX_OBJECT    calibratedMatrix;                                            // for the calculation of the preshift
  CROSS_REFERENCE *TabCross;

  RC               rc;                                                          // return code

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initializations

  KURUCZ_indexLine=1;
  pKurucz=&KURUCZ_buffers[indexFenoColumn];
  const int n_wavel = NDET[indexFenoColumn];
  pKuruczFeno=&TabFeno[indexFenoColumn][pKurucz->indexKurucz];
  TabCross=pKuruczFeno->TabCross;

  rc=ERROR_ID_NO;
  msgCount=0;

  memset(&calibratedMatrix,0,sizeof(MATRIX_OBJECT));
  VECTOR_Init(ANALYSE_zeros,(double)0.,n_wavel);                                // To check later : sometimes, the last component of the vector is different from 0. and that causes a problem with the plot of "absolu"

  // Allocate buffers

  if ((reference=(double *)MEMORY_AllocDVector(__func__,"spectrum",0,n_wavel-1))==NULL)
    return ERROR_ID_ALLOC;

  // Browse analysis windows and apply Kurucz alignment on the specified reference spectrum if needed

  for (indexFeno=0,nKuruczFeno=nBadKuruczFeno=0;indexFeno<NFeno;indexFeno++) {
    pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

    if (!pTabFeno->hidden && ((pTabFeno->useKurucz==ANLYS_KURUCZ_REF) || (pTabFeno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC)) &&
        (pTabFeno->gomeRefFlag==gomeFlag) &&
        ((!refFlag && (pTabFeno->useEtalon || (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE))) ||
         ((refFlag==1) && !pTabFeno->useEtalon)))

      nKuruczFeno++;
  }

  for (indexFeno=0;indexFeno<NFeno;indexFeno++) {
    pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

    if (!pTabFeno->hidden && ((pTabFeno->useKurucz==ANLYS_KURUCZ_REF) || (pTabFeno->useKurucz==ANLYS_KURUCZ_REF_AND_SPEC)) &&
        (pTabFeno->gomeRefFlag==gomeFlag) && (pTabFeno->useRefRow) &&
        ((!refFlag && (pTabFeno->useEtalon || (pTabFeno->refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE))) ||
         ((refFlag==1) && !pTabFeno->useEtalon))) {
      memcpy(reference,(pTabFeno->useEtalon)?pTabFeno->SrefEtalon:pTabFeno->Sref,sizeof(double)*pTabFeno->NDET);

      if ((pTabFeno->NDET==pKurucz->hrSolar.nl) &&
          VECTOR_Equal(pKurucz->hrSolar.matrix[0],pTabFeno->LambdaRef,pTabFeno->NDET,(double)1.e-7) &&
          VECTOR_Equal(pKurucz->hrSolar.matrix[1],reference,pTabFeno->NDET,(double)1.e-7)) {
        // Reference spectrum equals high-resolution spectrum for Kurucz -> skip calibration.

        pTabFeno->rcKurucz=ERROR_ID_NO;
      } else {

        // If we have already calibrated an identical reference spectrum in another analysis window, re-use those results:
        // Apply instrumental corrections on reference spectrum
        if (((indexRef=KuruczSearchReference(indexFeno,indexFenoColumn))<NFeno) && (indexRef!=indexFeno) &&
            ((indexRef<indexFeno) ||
             ((refFlag && !pTabFeno->useEtalon) && ((TabFeno[indexFenoColumn][indexRef].refSpectrumSelectionMode==ANLYS_REF_SELECTION_MODE_FILE) || TabFeno[indexFenoColumn][indexRef].useEtalon)))) {
          int Nb_Win=KURUCZ_buffers[indexFenoColumn].Nb_Win;
          int indexWindow;

          pTabRef=&TabFeno[indexFenoColumn][indexRef];
          pTabFeno->rcKurucz=pTabRef->rcKurucz;

          memcpy(pTabFeno->LambdaK,pTabRef->LambdaK,sizeof(double)*pTabFeno->NDET);
          memcpy(pKurucz->KuruczFeno[indexFeno].rms,pKurucz->KuruczFeno[indexRef].rms,sizeof(double)*Nb_Win);
          memcpy(pKurucz->KuruczFeno[indexFeno].chiSquare,pKurucz->KuruczFeno[indexRef].chiSquare,sizeof(double)*Nb_Win);
          memcpy(pKurucz->KuruczFeno[indexFeno].wve,pKurucz->KuruczFeno[indexRef].wve,sizeof(double)*Nb_Win);
          memcpy(pKurucz->KuruczFeno[indexFeno].nIter,pKurucz->KuruczFeno[indexRef].nIter,sizeof(int)*Nb_Win);

          memcpy(pKurucz->KuruczFeno[indexFeno].chiSquare,pKurucz->KuruczFeno[indexRef].chiSquare,sizeof(double)*Nb_Win);

          if (TabFeno[indexFenoColumn][pKurucz->indexKurucz].NTabCross)
            for (indexWindow=0;indexWindow<Nb_Win;indexWindow++)
              memcpy(pKurucz->KuruczFeno[indexFeno].results[indexWindow],pKurucz->KuruczFeno[indexRef].results[indexWindow],sizeof(CROSS_RESULTS)*TabFeno[indexFenoColumn][pKurucz->indexKurucz].NTabCross);

          if (pKuruczOptions->fwhmFit) {
            for (maxParam=0;maxParam<MAX_KURUCZ_FWHM_PARAM;maxParam++)
              if (pTabFeno->fwhmVector[maxParam]!=NULL) {
                memcpy(pTabFeno->fwhmPolyRef[maxParam],pTabRef->fwhmPolyRef[maxParam],sizeof(double)*(pKuruczOptions->fwhmPolynomial+1));
                memcpy(pTabFeno->fwhmVector[maxParam],pTabRef->fwhmVector[maxParam],sizeof(double)*n_wavel);
                if (pTabFeno->fwhmDeriv2[maxParam]!=NULL)
                  memcpy(pTabFeno->fwhmDeriv2[maxParam],pTabRef->fwhmDeriv2[maxParam],sizeof(double)*n_wavel);
              }
              else
                break;
          }
        } else { // We have not yet calibrated this reference spectrum:

          // Calculate preshift
          if (pKuruczOptions->preshiftFlag && !(rc=MATRIX_Allocate(&calibratedMatrix,n_wavel,2,0,0,0,__func__))) {
            // Get solar spectrum

            memcpy(calibratedMatrix.matrix[0],pTabFeno->LambdaRef,n_wavel*sizeof(double));

            // For tests  --- MATRIX_Load("D:/My_GroundBased_Activities/GB_Stations/Bruxelles/miniDOAS_Uccle/BX_SPE_20131108_285.REF",&calibratedMatrix,pTabFeno->NDET,2,0.,0.,0,0,__func__);   // FOR TESTS

            if (pKuruczOptions->fwhmFit)
              rc=KuruczConvolveSolarSpectrum(&calibratedMatrix,pTabFeno->LambdaRef,n_wavel,indexFenoColumn);
            else
              SPLINE_Vector(pKurucz->hrSolar.matrix[0],pKurucz->hrSolar.matrix[1],pKurucz->hrSolar.deriv2[1],pKurucz->hrSolar.nl,pTabFeno->LambdaRef,calibratedMatrix.matrix[1],n_wavel,pAnalysisOptions->interpol);

            // Calculate preshift

            if (!rc &&
                !(rc=KuruczCalculatePreshift(calibratedMatrix.matrix[0],calibratedMatrix.matrix[1],reference,pTabFeno->NDET,pKuruczOptions->preshiftMin,pKuruczOptions->preshiftMax,0.2,(double)pKuruczOptions->lambdaLeft,(double)pKuruczOptions->lambdaRight,&pTabFeno->preshift))) {
              if ((pKuruczFeno->indexSpectrum!=ITEM_NONE) && (TabCross[Feno->indexSpectrum].FitShift!=ITEM_NONE))
                TabCross[Feno->indexSpectrum].InitShift=pTabFeno->preshift;
              if ((pKuruczFeno->indexReference!=ITEM_NONE) && (TabCross[Feno->indexReference].FitShift!=ITEM_NONE)) {
                pTabFeno->preshift=-pTabFeno->preshift;
                TabCross[Feno->indexReference].InitShift=pTabFeno->preshift;
              }
            }

            MATRIX_Free(&calibratedMatrix,__func__);
          }

          // Apply Kurucz for building new calibration for reference
          if ((rc=pTabFeno->rcKurucz=KURUCZ_Spectrum(pTabFeno->LambdaRef,pTabFeno->LambdaK,reference,pKurucz->solar,instrFunction,
                                                     1,pTabFeno->windowName,pTabFeno->fwhmPolyRef,pTabFeno->fwhmVector,pTabFeno->fwhmDeriv2,saveFlag,indexFeno,responseHandle,indexFenoColumn))!=ERROR_ID_NO)
            goto EndKuruczReference;
        }

        if (!rc && !pTabFeno->rcKurucz)
          rc=KURUCZ_ApplyCalibration(pTabFeno,pTabFeno->LambdaK,indexFenoColumn);

      EndKuruczReference :

        if (rc>0) {
          if (pTabFeno->rcKurucz<=THREAD_EVENT_STOP) {
            rc=pTabFeno->rcKurucz;
            break;
          } else {
            if (++nBadKuruczFeno==nKuruczFeno) {
              rc=pTabFeno->rcKurucz;
              break;
            }
            msgCount++;
          }
        }
      }

      if (pTabFeno->longPathFlag) { // !!! Anoop
        memcpy(pTabFeno->SrefEtalon,ANALYSE_ones,sizeof(double)*n_wavel);
        memcpy(pTabFeno->Sref,ANALYSE_ones,sizeof(double)*n_wavel);
      }
    }
  }

  if (rc<THREAD_EVENT_STOP)
    rc=ERROR_ID_NO;

  // Release allocated buffers

  if (reference!=NULL)
   MEMORY_ReleaseDVector(__func__,"reference",reference,0);

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
}

// ====================
// RESOURCES MANAGEMENT
// ====================

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_Init
// ----------------------------------------------------------------------------
// PURPOSE         set the limits for each window of the calibration interval
//
// INPUT           in the case of satellites measurements :
//
//                    gomeRefFlag=0 means that irradiance is used as etalon spectrum
//                    gomeRefFlag=1 means that a reference spectrum is given
//
// RETURN          return code
// ----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
void KURUCZ_Init(int gomeFlag,INDEX indexFenoColumn) {
  // Declarations

  INDEX indexWindow;
  int nbWin;
  KURUCZ *pKurucz;
  FENO *pTabFeno;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_APPL|DEBUG_FCTTYPE_MEM);
#endif

  // Initialization

  pKurucz=&KURUCZ_buffers[indexFenoColumn];

  nbWin=pKurucz->Nb_Win;

  // Browse analysis windows

  for (int indexFeno=0;indexFeno<NFeno;indexFeno++) {
    pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

    if ((pTabFeno->gomeRefFlag==gomeFlag) && (pTabFeno->useRefRow) &&
        (pKurucz->KuruczFeno[indexFeno].subwindow_fits!=NULL)) {

      for (indexWindow=0;indexWindow<nbWin;indexWindow++) {
        struct fit_properties *subwindow_fit=&pKurucz->KuruczFeno[indexFeno].subwindow_fits[indexWindow];

        int pixel_start=FNPixel(pTabFeno->LambdaRef,pKurucz->lambdaMin[indexWindow],pTabFeno->NDET,PIXEL_AFTER);
        int pixel_end=FNPixel(pTabFeno->LambdaRef,pKurucz->lambdaMax[indexWindow],pTabFeno->NDET,PIXEL_BEFORE);

        subwindow_fit->specrange = spectrum_new();
        spectrum_append(subwindow_fit->specrange, pixel_start, pixel_end);

        subwindow_fit->DimL=pixel_end - pixel_start + 1;
      }
    }
  }

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,0);
#endif
}

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_Alloc
// ----------------------------------------------------------------------------
// PURPOSE         allocate all buffers needed for applying Kurucz procedures to a project
//
// INPUT           pProject     pointer to the current project
//                 lambda       original wavelength calibration
//                 indexKurucz  index of analysis window with Kurucz description for the current project;
//                 lambdaMin    lower limit of the wavelength calibration interval
//                 lambdaMax    upper limit of the wavelength calibration interval
//                 hr_solar     pre-loaded contents of the high-resolution reference spectrum (from calibration or slit page)
//
// RETURN          return code
// ----------------------------------------------------------------------------

// Note : lambdaMin, lambdaMax not really used

RC KURUCZ_Alloc(const PROJECT *pProject, const double *lambda,INDEX indexKurucz,double lambdaMin,double lambdaMax,INDEX indexFenoColumn,
                const MATRIX_OBJECT *hr_solar, const MATRIX_OBJECT *slit_matrix)
 {
  // Declarations

  int hFilterFlag;
  CROSS_REFERENCE *pTabCross;                                                   // cross sections list

  char   slitFile[MAX_ITEM_TEXT_LEN];
  int    Nb_Win,shiftDegree,                                                    // substitution variables
         NTabCross;
  INDEX  i,indexFeno,indexParam,indexTabCross,indexWindow;                      // indexes for loops and arrays
  double Lambda_min,Lambda_max,                                                 // extrema in nm of a little window
         Win_size;                                                              // size of a little window in nm
  double step;
  KURUCZ *pKurucz;
  RC rc;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_MEM);
#endif

  // Initializations
  const int n_wavel = NDET[indexFenoColumn];
  pKurucz=&KURUCZ_buffers[indexFenoColumn];

  memset(&pKurucz->hrSolar,0,sizeof(MATRIX_OBJECT));
  memset(&pKurucz->slitFunction,0,sizeof(MATRIX_OBJECT));
  memset(&pKurucz->hrSolarGridded,0,sizeof(MATRIX_OBJECT));

  FENO *pKuruczFeno=&TabFeno[indexFenoColumn][indexKurucz]; // analysis window with Kurucz description

  step=(double)0.;

  FILES_RebuildFileName(slitFile,pKuruczOptions->slfFile,1);

  if ((hFilterFlag=((ANALYSE_phFilter->filterFunction!=NULL) &&                 // high pass filtering
                    (pKuruczFeno->analysisMethod==OPTICAL_DENSITY_FIT) &&    // implemented only in DOAS fitting
                    (pKuruczFeno->analysisType==ANALYSIS_TYPE_FWHM_KURUCZ) &&   // for calibration with slit function fitting
                   ((pKuruczFeno->indexSol==ITEM_NONE) ||                       // doesn't fit the Sol non linear parameter
                    (pKuruczFeno->TabCross[pKuruczFeno->indexSol].FitParam==ITEM_NONE)))?1:0)==1)

   pKurucz->solarFGap=(int)floor(ANALYSE_phFilter->filterWidth*sqrt(ANALYSE_phFilter->filterNTimes)+0.5);

  // Load options from Kurucz tab page from project properties

  KURUCZ_buffers[indexFenoColumn].Nb_Win=Nb_Win=pKuruczOptions->windowsNumber;
  shiftDegree=pKuruczOptions->shiftPolynomial;

  rc=ERROR_ID_NO;
  
  // Check if the degree of the polynomial for the shift of for the stretch is less than the number of calibration windows   
  
  if ((pKuruczOptions->shiftPolynomial>=Nb_Win) || (pKuruczOptions->fwhmFit && (pKuruczOptions->fwhmPolynomial>=Nb_Win)))
   {
    rc=ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_CALIBRATION_POLYNOMIAL,"Slit File");
    goto EndKuruczAlloc;
   }  
  
  // Check validity of entry fields in Kurucz tab page of project properties panel
  
  if (pKuruczOptions->fwhmFit)
   {
    pKurucz->fwhmDegree=pKuruczOptions->fwhmPolynomial;

    if (pKuruczOptions->fwhmType==SLIT_TYPE_FILE) {
      // copy the slit function matrix:
      rc = MATRIX_Copy(&pKurucz->slitFunction, slit_matrix, __func__);
    }

    if (rc!=ERROR_ID_NO)
     goto EndKuruczAlloc;
   }
   
  if (((pKurucz->KuruczFeno=(KURUCZ_FENO *)MEMORY_AllocBuffer(__func__,"KuruczFeno",NFeno,sizeof(KURUCZ_FENO),0,MEMORY_TYPE_STRUCT))==NULL) ||
      ((pKurucz->dispAbsolu=(double **)MEMORY_AllocBuffer(__func__,"dispAbsolu",Nb_Win,sizeof(double **),0,MEMORY_TYPE_PTR))==NULL) ||
      ((pKurucz->dispSecX=(double **)MEMORY_AllocBuffer(__func__,"dispSecX",Nb_Win,sizeof(double **),0,MEMORY_TYPE_PTR))==NULL) ||
      ((pKurucz->solar=(double *)MEMORY_AllocDVector(__func__,"solar",0,n_wavel-1))==NULL) ||           // solar spectrum
      ((pKurucz->offset=(double *)MEMORY_AllocDVector(__func__,"offset",0,n_wavel-1))==NULL) ||         // offset spectrum
      ((pKurucz->Pcalib=(double *)MEMORY_AllocDVector(__func__,"Pcalib",1,shiftDegree+1))==NULL) ||  // coefficients of the polynomial
      ((pKurucz->VLambda=(double *)MEMORY_AllocDVector(__func__,"VLambda",1,Nb_Win))==NULL) ||       // solution of the system
      ((pKurucz->pixMid=(double *)MEMORY_AllocDVector(__func__,"pixMid",1,Nb_Win))==NULL) ||         // pixels at the middle of little windows
      ((pKurucz->VShift=(double *)MEMORY_AllocDVector(__func__,"VShift",1,Nb_Win))==NULL) ||         // shift applied on pixels
      ((pKurucz->VSig=(double *)MEMORY_AllocDVector(__func__,"VSig",1,Nb_Win))==NULL) ||             // error on shift applied on pixels
      ((pKurucz->lambdaMin=(double *)MEMORY_AllocDVector(__func__,"lambdaMin",0,Nb_Win-1))==NULL) ||         // limits of the windows
      ((pKurucz->lambdaMax=(double *)MEMORY_AllocDVector(__func__,"lambdaMax",0,Nb_Win-1))==NULL) ||         // limits of the windows
      ((pKurucz->NIter=(int *)MEMORY_AllocBuffer(__func__,"NIter",Nb_Win,sizeof(int),0,MEMORY_TYPE_INT))==NULL) ||
      (hFilterFlag &&
       (((pKurucz->lambdaF=(double *)MEMORY_AllocDVector(__func__,"lambdaF",0,n_wavel+2*pKurucz->solarFGap-1))==NULL) ||
        ((pKurucz->solarF=(double *)MEMORY_AllocDVector(__func__,"solarF",0,n_wavel+2*pKurucz->solarFGap-1))==NULL) ||
        ((pKurucz->solarF2=(double *)MEMORY_AllocDVector(__func__,"solarF2",0,n_wavel+2*pKurucz->solarFGap-1))==NULL))))
   {
    rc=ERROR_ID_ALLOC;
    goto EndKuruczAlloc;
   } else {
    // null-initialize KuruczFeno structures.
    for (indexFeno=0;indexFeno<NFeno;indexFeno++) {
     memset(&pKurucz->KuruczFeno[indexFeno],0,sizeof(KURUCZ_FENO));
    }
  }

  // Determine the little windows

  Lambda_min=pKuruczOptions->lambdaLeft;
  Lambda_max=pKuruczOptions->lambdaRight;

  if (pKuruczOptions->divisionMode==PRJCT_CALIB_WINDOWS_CONTIGUOUS)
   Win_size=(double)(Lambda_max-Lambda_min)/Nb_Win;
  else if (pKuruczOptions->divisionMode==PRJCT_CALIB_WINDOWS_SLIDING)
   Win_size=pKuruczOptions->windowSize;
  else
   Win_size=(double)0.;

  for (indexWindow=0;indexWindow<Nb_Win;indexWindow++)
   {
    Lambda_max=(pKuruczOptions->divisionMode!=PRJCT_CALIB_WINDOWS_CUSTOM)?Lambda_min+Win_size:pKuruczOptions->customLambdaMax[indexWindow];

    pKurucz->lambdaMin[indexWindow]=Lambda_min;
    pKurucz->lambdaMax[indexWindow]=Lambda_max;

    if (pKuruczOptions->divisionMode==PRJCT_CALIB_WINDOWS_CONTIGUOUS)
     Lambda_min=Lambda_max;
    else if (pKuruczOptions->divisionMode==PRJCT_CALIB_WINDOWS_SLIDING)
     Lambda_min=pKuruczOptions->lambdaLeft+(indexWindow+1)*(pKuruczOptions->lambdaRight-pKuruczOptions->lambdaLeft-Win_size)/(Nb_Win-1.);
    else if (indexWindow<Nb_Win-1)
     Lambda_min=pKuruczOptions->customLambdaMin[indexWindow+1];

    pKurucz->dispAbsolu[indexWindow]=pKurucz->dispSecX[indexWindow]=NULL;

       if (((pKurucz->dispAbsolu[indexWindow]=(double *)MEMORY_AllocDVector(__func__,"dispAbsolu",0,n_wavel-1))==NULL) ||
           ((pKurucz->dispSecX[indexWindow]=(double *)MEMORY_AllocDVector(__func__,"dispSecX",0,n_wavel-1))==NULL))
        {
      rc=ERROR_ID_ALLOC;
      goto EndKuruczAlloc;
        }
   }

  if (hFilterFlag && pKurucz->solarFGap)
   {
    if (lambda[n_wavel-1]-lambda[0]+1==n_wavel)
     {
      for (i=0;i<n_wavel+2*pKurucz->solarFGap;i++)
       pKurucz->lambdaF[i]=(double)0.;
     }
    else
     {
      memcpy(&pKurucz->lambdaF[pKurucz->solarFGap],lambda,sizeof(double)*n_wavel);

      step=(lambda[n_wavel-1]-lambda[0])/n_wavel;

      for (i=0;i<pKurucz->solarFGap;i++)
       {
        pKurucz->lambdaF[i]=lambda[0]-step*(pKurucz->solarFGap-i);
        pKurucz->lambdaF[n_wavel+pKurucz->solarFGap+i]=lambda[n_wavel-1]+step*(i+1);
       }
     }
   }

  // Load and normalize solar spectrum

  // allocate matrix for pKurucz->hrSolar: (number of wavelengths)x(2)
  rc = MATRIX_Allocate(&pKurucz->hrSolar, hr_solar->nl, 2, 0, 0, 1, __func__);
  if( !rc) {
    // copy wavelengths and one spectrum column from pre-loaded hr_solar into pKurucz->hrSolar
    int index_row = (hr_solar->nc > 1+indexFenoColumn) ? 1+indexFenoColumn:1;
    memcpy(pKurucz->hrSolar.matrix[0], hr_solar->matrix[0], hr_solar->nl * sizeof(hr_solar->matrix[0][0]));
    memcpy(pKurucz->hrSolar.matrix[1], hr_solar->matrix[index_row], hr_solar->nl * sizeof(hr_solar->matrix[0][0]));

    rc=XSCONV_ConvertCrossSectionFile(&pKurucz->hrSolar,lambdaMin-7.-step*pKurucz->solarFGap,lambdaMax+7.+step*pKurucz->solarFGap,(double)0.,CONVOLUTION_CONVERSION_NONE);
  }
  
  if( !rc) {
    // If the fwhm of the slit function is fitted, then we can use the same high resolution solar
    // spectrum.  If we do not fit the slit function, the solar spectrum has to be preconvolved.
    // For OMI, the number of rows is 60 and the number of preconvolved spectra should be 60 too.
    // So in principle, a test if the slit function should be enough but why we couldn't use the
    // same spectrum ?

    if (((rc=VECTOR_NormalizeVector(pKurucz->hrSolar.matrix[1]-1,pKurucz->hrSolar.nl,NULL,__func__))!=ERROR_ID_NO) ||
        ((rc=SPLINE_Deriv2(pKurucz->hrSolar.matrix[0],pKurucz->hrSolar.matrix[1],pKurucz->hrSolar.deriv2[1],pKurucz->hrSolar.nl,__func__))!=ERROR_ID_NO))

     goto EndKuruczAlloc;

    if (!rc && !pKuruczOptions->fwhmFit) 
     {
      int hrKuruczLambdaN=ceil((pKuruczOptions->lambdaRight-pKuruczOptions->lambdaLeft)+6.)*100.; // grid of 0.01 nm; 3 nm security gap both sides
      if (!(rc = MATRIX_Allocate(&pKurucz->hrSolarGridded,hrKuruczLambdaN, 2, 0, 0, 1, __func__)))
       {
        for (int i=0;i<hrKuruczLambdaN;i++)
         pKurucz->hrSolarGridded.matrix[0][i]=pKuruczOptions->lambdaLeft-3.+0.01*i;
       }
     }    

    memcpy(pKurucz->solar,ANALYSE_zeros,sizeof(double)*n_wavel);

    // Initialize other fields of global structure

    pKurucz->indexKurucz=indexKurucz;

    pKurucz->Nb_Win=Nb_Win;
    pKurucz->shiftDegree=shiftDegree;

    pKurucz->displayFit=(char)pKuruczOptions->displayFit;                // display fit flag
    pKurucz->displayResidual=(char)pKuruczOptions->displayResidual;      // display new calibration flag
    pKurucz->displayShift=(char)pKuruczOptions->displayShift;            // display shift in each pixel flag
    pKurucz->displaySpectra=(char)pKuruczOptions->displaySpectra;        // display shift in each pixel flag

    pKurucz->method=(char)pKuruczOptions->analysisMethod;

    // Allocate one svd environment for each little window

    for (indexFeno=0;indexFeno<NFeno;indexFeno++) {
     memset(&pKurucz->KuruczFeno[indexFeno],0,sizeof(KURUCZ_FENO));
     FENO *pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

     if ((pTabFeno->hidden==1) ||
        ((THRD_id!=THREAD_TYPE_KURUCZ) && !pTabFeno->hidden && pTabFeno->useKurucz)) {
       int DimLMax=pTabFeno->NDET;

       if ((pKurucz->KuruczFeno[indexFeno].Grid=(double *)MEMORY_AllocDVector(__func__,"Grid",0,Nb_Win-1))==NULL)
        rc=ERROR_ID_ALLOC;
       else if ((pKurucz->KuruczFeno[indexFeno].subwindow_fits=MEMORY_AllocBuffer(__func__,"subwindow_fit",Nb_Win,sizeof(*pKurucz->KuruczFeno[indexFeno].subwindow_fits),0,MEMORY_TYPE_STRUCT))==NULL)                           // svd environments
        rc=ERROR_ID_ALLOC;
       else if (pKuruczOptions->fwhmFit && ((pKurucz->KuruczFeno[indexFeno].fft=(FFT *)MEMORY_AllocBuffer(__func__,"fft",Nb_Win,sizeof(FFT),0,MEMORY_TYPE_STRUCT))==NULL))                           // svd environments
        rc=ERROR_ID_ALLOC;
       else if ((pKurucz->KuruczFeno[indexFeno].chiSquare=(double *)MEMORY_AllocDVector(__func__,"chiSquare",0,Nb_Win-1))==NULL)
        rc=ERROR_ID_ALLOC;
       else if ((pKurucz->KuruczFeno[indexFeno].rms=(double *)MEMORY_AllocDVector(__func__,"rms",0,Nb_Win-1))==NULL)
        rc=ERROR_ID_ALLOC;
       else if ((pKurucz->KuruczFeno[indexFeno].wve=(double *)MEMORY_AllocDVector(__func__,"wve",0,Nb_Win-1))==NULL)
        rc=ERROR_ID_ALLOC;
       else if ((pKurucz->KuruczFeno[indexFeno].nIter=(int *)MEMORY_AllocBuffer(__func__,"nIter",Nb_Win,sizeof(int),0,MEMORY_TYPE_INT))==NULL)                           // svd environments
        rc=ERROR_ID_ALLOC;
       else if ((pKurucz->KuruczFeno[indexFeno].results=(CROSS_RESULTS **)MEMORY_AllocBuffer(__func__,"results",Nb_Win,sizeof(CROSS_RESULTS *),0,MEMORY_TYPE_STRUCT))==NULL)
        rc=ERROR_ID_ALLOC;

       if (rc)
        goto EndKuruczAlloc;
       else {
          memset(pKurucz->KuruczFeno[indexFeno].subwindow_fits,0,Nb_Win*sizeof(*pKurucz->KuruczFeno[0].subwindow_fits));
          if (pKuruczOptions->fwhmFit)
            memset(pKurucz->KuruczFeno[indexFeno].fft,0,Nb_Win*sizeof(FFT));
          memset(pKurucz->KuruczFeno[indexFeno].results,0,Nb_Win*sizeof(CROSS_RESULTS *));

          for (int i=0; i!=Nb_Win; ++i) { // Initialize calibration results with fill values
            pKurucz->KuruczFeno[indexFeno].chiSquare[i] = QDOAS_FILL_DOUBLE;
            pKurucz->KuruczFeno[indexFeno].rms[i] = QDOAS_FILL_DOUBLE;
            pKurucz->KuruczFeno[indexFeno].wve[i] = QDOAS_FILL_DOUBLE;
          }
       }

       for (indexWindow=0;indexWindow<Nb_Win;indexWindow++) {
         struct fit_properties *subwindow_fit=&pKurucz->KuruczFeno[indexFeno].subwindow_fits[indexWindow];
         memcpy(subwindow_fit,&pKuruczFeno->fit_properties,sizeof(*subwindow_fit));
         subwindow_fit->Z=1;
         subwindow_fit->DimL=DimLMax;

         pKurucz->KuruczFeno[indexFeno].Grid[indexWindow]=pKurucz->lambdaMax[indexWindow];

         if ((pKurucz->KuruczFeno[indexFeno].results[indexWindow]=(CROSS_RESULTS *)MEMORY_AllocBuffer(__func__,"KuruczFeno(results)",pKuruczFeno->NTabCross,sizeof(CROSS_RESULTS),0,MEMORY_TYPE_STRUCT))==NULL)
          {
           rc=ERROR_ID_ALLOC;
           goto EndKuruczAlloc;
          }
         else if ((rc=FIT_PROPERTIES_alloc("KURUCZ_Alloc (1)",subwindow_fit))!=ERROR_ID_NO)
          goto EndKuruczAlloc;
         else if (pKuruczOptions->fwhmFit)
          {
           int hrDeb,hrFin,hrN,fftSize;
           double *fftIn;
           FFT *pfft;
           INDEX i;

           pfft=&pKurucz->KuruczFeno[indexFeno].fft[indexWindow];

           hrDeb=FNPixel(pKurucz->hrSolar.matrix[0],pKurucz->lambdaMin[indexWindow]-3.,pKurucz->hrSolar.nl,PIXEL_AFTER);
           hrFin=FNPixel(pKurucz->hrSolar.matrix[0],pKurucz->lambdaMax[indexWindow]+3.,pKurucz->hrSolar.nl,PIXEL_BEFORE);

           if (hrDeb==hrFin)
            {
                rc=ERROR_SetLast(__func__,ERROR_TYPE_WARNING,ERROR_ID_POW);
             goto EndKuruczAlloc;
            }

           hrN=pfft->oldSize=(hrFin-hrDeb+1);
           fftSize=pfft->fftSize=(int)pow((double)2.,ceil(log((double)hrN)/log((double)2.)));

           if (((fftIn=pfft->fftIn=(double *)MEMORY_AllocDVector(__func__,"fftIn",1,fftSize))==NULL) ||
               ((pfft->fftOut=(double *)MEMORY_AllocDVector(__func__,"fftOut",1,fftSize))==NULL) ||
               ((pfft->invFftIn=(double *)MEMORY_AllocDVector(__func__,"invFftIn",1,fftSize))==NULL) ||
               ((pfft->invFftOut=(double *)MEMORY_AllocDVector(__func__,"invFftOut",1,fftSize))==NULL))
            {
             rc=ERROR_ID_ALLOC;
             goto EndKuruczAlloc;
            }

           memcpy(fftIn+1,pKurucz->hrSolar.matrix[1]+hrDeb,sizeof(double)*hrN);   // When the slit function is fitted, we use a high resolution solar spectrum (2 columns only)

           for (i=hrN+1;i<=fftSize;i++)
            fftIn[i]=fftIn[2*hrN-i];

           realft(pfft->fftIn,pfft->fftOut,fftSize,1);

           memcpy(fftIn+1,pKurucz->hrSolar.matrix[0]+hrDeb,sizeof(double)*hrN);  // Reuse fftIn for high resolution wavelength safe keeping
          }
        }
      }
    }

    // Allocate buffers for cross sections fits

    if (pKurucz->displayFit) {
      for (indexTabCross=0,NTabCross=0;indexTabCross<pKuruczFeno->NTabCross;indexTabCross++)
       {
        pTabCross=&pKuruczFeno->TabCross[indexTabCross];
        if (pTabCross->IndSvdA && pTabCross->display &&
           (WorkSpace[pTabCross->Comp].type==WRK_SYMBOL_CROSS))

         NTabCross++;
       }

      if (NTabCross && ((rc=MATRIX_Allocate(&pKurucz->crossFits,n_wavel,NTabCross,0,0,0,__func__))!=0))
       goto EndKuruczAlloc;
    }


    // Allocate buffers for coefficients of polynomials fitting fwhm

    if (pKuruczOptions->fwhmFit) {

      for (indexParam=0;(indexParam<MAX_KURUCZ_FWHM_PARAM) && !rc;indexParam++)
       {
        if (((pKurucz->fwhm[indexParam]=(double *)MEMORY_AllocDVector(__func__,"fwhm",0,Nb_Win-1))==NULL) ||
            ((pKurucz->fwhmSigma[indexParam]=(double *)MEMORY_AllocDVector(__func__,"fwhmSigma",0,Nb_Win-1))==NULL) ||
            ((pKurucz->fwhmPolySpec[indexParam]=(double *)MEMORY_AllocDVector(__func__,"fwhmPolySpec",0,pKurucz->fwhmDegree))==NULL) ||
            ((pKurucz->fwhmVector[indexParam]=(double *)MEMORY_AllocDVector(__func__,"fwhmVector",0,n_wavel-1))==NULL) ||
            ((pKurucz->fwhmDeriv2[indexParam]=(double *)MEMORY_AllocDVector(__func__,"fwhmVector",0,n_wavel-1))==NULL))

         rc=ERROR_ID_ALLOC;

        else
         {
          memcpy(pKurucz->fwhmVector[indexParam],ANALYSE_zeros,sizeof(double)*n_wavel);

          for (indexFeno=0;(indexFeno<NFeno) && !rc;indexFeno++)
           {
            FENO *pTabFeno=&TabFeno[indexFenoColumn][indexFeno];

            if ((pKuruczFeno->indexFwhmParam[indexParam]!=ITEM_NONE) &&
                 !pTabFeno->hidden && pTabFeno->useKurucz &&
              (((pTabFeno->fwhmPolyRef[indexParam]=(double *)MEMORY_AllocDVector(__func__,"fwhmPolyRef",0,pKurucz->fwhmDegree))==NULL) ||
               ((pTabFeno->fwhmVector[indexParam]=(double *)MEMORY_AllocDVector(__func__,"fwhmVector",0,n_wavel-1))==NULL) ||
               ((pKuruczFeno->TabCross[pKuruczFeno->indexFwhmParam[indexParam]].FitParam!=ITEM_NONE) &&
               ((pTabFeno->fwhmDeriv2[indexParam]=(double *)MEMORY_AllocDVector(__func__,"fwhmDeriv2",0,n_wavel-1))==NULL))))

             rc=ERROR_ID_ALLOC;
           }
         }
       }
     }

    if (rc)
     goto EndKuruczAlloc;

    if (hFilterFlag && pKurucz->solarFGap && (lambda[n_wavel-1]-lambda[0]+1!=n_wavel) &&
     (((rc=SPLINE_Vector(pKurucz->hrSolar.matrix[0],pKurucz->hrSolar.matrix[1],pKurucz->hrSolar.deriv2[1],pKurucz->hrSolar.nl,
                         pKurucz->lambdaF,pKurucz->solarF,n_wavel+2*pKurucz->solarFGap,pAnalysisOptions->interpol))!=0) ||
      ((rc=FILTER_Vector(ANALYSE_phFilter,pKurucz->solarF,pKurucz->solarF,NULL,n_wavel+2*pKurucz->solarFGap,PRJCT_FILTER_OUTPUT_LOW))!=0) ||
      ((rc=SPLINE_Deriv2(pKurucz->lambdaF,pKurucz->solarF,pKurucz->solarF2,n_wavel+2*pKurucz->solarFGap,"KURUCZ_Alloc (solarF) "))!=0)))

     goto EndKuruczAlloc;

    KURUCZ_Init(1,indexFenoColumn);
  }

  // Return

  EndKuruczAlloc :

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,rc);
#endif

  return rc;
 }

// ----------------------------------------------------------------------------
// FUNCTION        KURUCZ_Free
// ----------------------------------------------------------------------------
// PURPOSE         release buffers allocated for Kurucz procedures;
// ----------------------------------------------------------------------------

void KURUCZ_Free(void)
 {
  // Declarations

  INDEX indexWindow,indexParam,indexFeno,indexFenoColumn;
  KURUCZ_FENO *pKFeno;
  KURUCZ *pKurucz;

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionBegin(__func__,DEBUG_FCTTYPE_MEM);
#endif

  for (indexFenoColumn=0;indexFenoColumn<ANALYSE_swathSize;indexFenoColumn++)
   {
       pKurucz=&KURUCZ_buffers[indexFenoColumn];

    MATRIX_Free(&pKurucz->hrSolar,"KURUCZ_Free");
    MATRIX_Free(&pKurucz->hrSolarGridded,"KURUCZ_Free");
    MATRIX_Free(&pKurucz->slitFunction,"KURUCZ_Free");

    if (pKurucz->solar!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","solar",pKurucz->solar,0);
    if (pKurucz->lambdaF!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","lambdaF",pKurucz->lambdaF,0);
    if (pKurucz->solarF!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","solarF",pKurucz->solarF,0);
    if (pKurucz->solarF2!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","solarF2",pKurucz->solarF2,0);
    if (pKurucz->offset!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","offset",pKurucz->offset,0);
    if (pKurucz->Pcalib!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","Pcalib",pKurucz->Pcalib,1);
    if (pKurucz->VLambda!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","VLambda",pKurucz->VLambda,1);
    if (pKurucz->VShift!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","VShift",pKurucz->VShift,1);
    if (pKurucz->VSig!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","VSig",pKurucz->VSig,1);
    if (pKurucz->pixMid!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","pixMid",pKurucz->pixMid,1);
    if (pKurucz->lambdaMin!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","lambdaMin",pKurucz->lambdaMin,0);
    if (pKurucz->lambdaMax!=NULL)
     MEMORY_ReleaseDVector("KURUCZ_Free ","lambdaMax",pKurucz->lambdaMax,0);
    if (pKurucz->NIter!=NULL)
     MEMORY_ReleaseBuffer("KURUCZ_Free ","NIter",pKurucz->NIter);

    for (indexParam=0;(indexParam<MAX_KURUCZ_FWHM_PARAM);indexParam++)
     {
      if (pKurucz->fwhm[indexParam]!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","fwhm",pKurucz->fwhm[indexParam],0);
      if (pKurucz->fwhmSigma[indexParam]!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmSigma",pKurucz->fwhmSigma[indexParam],0);
      if (pKurucz->fwhmPolySpec[indexParam]!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmPolySpec",pKurucz->fwhmPolySpec[indexParam],0);
      if (pKurucz->fwhmVector[indexParam]!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmVector",pKurucz->fwhmVector[indexParam],0);
      if (pKurucz->fwhmDeriv2[indexParam]!=NULL)
       MEMORY_ReleaseDVector("KURUCZ_Free ","fwhmDeriv2",pKurucz->fwhmDeriv2[indexParam],0);
     }

    MATRIX_Free(&pKurucz->crossFits,"KURUCZ_Free");

    if (pKurucz->dispAbsolu!=NULL){
      for (indexWindow=0;indexWindow<pKurucz->Nb_Win;indexWindow++) {
        if (pKurucz->dispAbsolu[indexWindow]!=NULL)
           MEMORY_ReleaseDVector("KURUCZ_Free ","dispAbsolu",pKurucz->dispAbsolu[indexWindow],0);
      }
      MEMORY_ReleaseBuffer("KURUCZ_Free ","dispAbsolu",pKurucz->dispAbsolu);
    }
    if (pKurucz->dispSecX!=NULL) {
      for (indexWindow=0;indexWindow<pKurucz->Nb_Win;indexWindow++) {
        if (pKurucz->dispSecX[indexWindow]!=NULL) {
          MEMORY_ReleaseDVector("KURUCZ_Free ","dispSecX",pKurucz->dispSecX[indexWindow],0);
        }
      }
      MEMORY_ReleaseBuffer("KURUCZ_Free ","dispSecX",pKurucz->dispSecX);
    }

    if (pKurucz->KuruczFeno!=NULL)
     {
      for (indexFeno=0;indexFeno<NFeno;indexFeno++)
       {
        pKFeno=&pKurucz->KuruczFeno[indexFeno];

        // Grid

        if (pKFeno->Grid!=NULL)
         MEMORY_ReleaseDVector(__func__,"Grid",pKFeno->Grid,0);

        // fit

        if (pKFeno->subwindow_fits!=NULL) {
          for (indexWindow=0;indexWindow<pKurucz->Nb_Win;indexWindow++)
            FIT_PROPERTIES_free("KURUCZ_Free (1)",&pKFeno->subwindow_fits[indexWindow]);

          MEMORY_ReleaseBuffer(__func__,"subwindow_fits",pKFeno->subwindow_fits);
        }

        // fft

        if (pKFeno->fft!=NULL)
         {
          for (indexWindow=0;indexWindow<pKurucz->Nb_Win;indexWindow++)
           {
            if (pKFeno->fft[indexWindow].fftIn!=NULL)
             MEMORY_ReleaseDVector("KURUCZ_Free ","fftIn",pKFeno->fft[indexWindow].fftIn,1);
            if (pKFeno->fft[indexWindow].fftOut!=NULL)
             MEMORY_ReleaseDVector("KURUCZ_Free ","fftOut",pKFeno->fft[indexWindow].fftOut,1);
            if (pKFeno->fft[indexWindow].invFftIn!=NULL)
             MEMORY_ReleaseDVector("KURUCZ_Free ","invFftIn",pKFeno->fft[indexWindow].invFftIn,1);
            if (pKFeno->fft[indexWindow].invFftOut!=NULL)
             MEMORY_ReleaseDVector("KURUCZ_Free ","invFftOut",pKFeno->fft[indexWindow].invFftOut,1);
           }

          MEMORY_ReleaseBuffer("KURUCZ_Free ","fft",pKFeno->fft);
         }

        if (pKFeno->results!=NULL)
         {
          for (indexWindow=0;indexWindow<pKurucz->Nb_Win;indexWindow++)
           if (pKFeno->results[indexWindow]!=NULL)
            MEMORY_ReleaseBuffer("KURUCZ_Free ","results",pKFeno->results[indexWindow]);

          MEMORY_ReleaseBuffer("KURUCZ_Free ","results",pKFeno->results);
         }

        if (pKFeno->chiSquare!=NULL)
         MEMORY_ReleaseDVector("KURUCZ_Free ","chiSquare",pKFeno->chiSquare,0);
        if (pKFeno->wve!=NULL)
         MEMORY_ReleaseDVector("KURUCZ_Free ","wve",pKFeno->wve,0);
        if (pKFeno->rms!=NULL)
         MEMORY_ReleaseDVector("KURUCZ_Free ","rms",pKFeno->rms,0);
        if (pKFeno->nIter!=NULL)
         MEMORY_ReleaseBuffer("KURUCZ_Free ","nIter",pKFeno->nIter);
       }

      MEMORY_ReleaseBuffer("KURUCZ_Free ","KuruczFeno",pKurucz->KuruczFeno);
     }

    memset(pKurucz,0,sizeof(KURUCZ));
   }

#if defined(__DEBUG_) && __DEBUG_
  DEBUG_FunctionStop(__func__,0);
#endif
 }
