
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  CROSS SECTIONS CONVOLUTION TOOLS
//  Name of module    :  XSCONV.C
//  Creation date     :  This module was already existing in old DOS versions and
//                       has been added in WinDOAS package in June 97
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
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  =====================
//  SUPPORTED LINE SHAPES
//  =====================
//
//  XSCONV_FctGauss - gaussian function;
//  XsconvFctApod - apodisation function (FTS);
//  XsconvFctApodNBS - apodisation function Norton Beer Strong (FTS);
//
//  XsconvFctBuild - build a line shape in a vector;
//
//  ======================
//  TAB CONTROL PROCESSING
//  ======================
//
//  XsconvTabChange - TCN_SELCHANGE notification message processing;
//  XsconvTabReSize - resize pages in tab control;
//
//  ===============
//  DATA PROCESSING
//  ===============
//
//  XSCONV_GetFwhm - get slit function full width half way up;
//
//  =====================
//  CONVOLUTION FUNCTIONS
//  =====================
//
//  XSCONV_TypeNone - apply no convolution, interpolation only;
//  XSCONV_TypeGauss - gaussian convolution with variable half way up width;
//  XSCONV_TypeStandard - standard convolution of cross section with a slit function;
//  XSCONV_RealTimeXs - real time cross sections convolution;
//
//  XsconvTypeI0Correction - convolution of cross sections with I0 correction;
//  XsconvRebuildSlitFunction - rebuild slit function onto a regular wavelength scale;
//  XsconvPowFFTMin - return index of the first minimum found in the power spectrum obtained by FFT;
//  XsconvFreqFilter - frequencies filtering;
//  XSCONV_NewSlitFunction - replace slit function by a new one when a deconvolution slit function is given;
//
//  XSCONV_Convolution - main convolution function;
//
//  ================
//  FILES PROCESSING
//  ================
//
//  XSCONV_FileSelection - file selection;
//  XSCONV_LoadCalibrationFile - final wavelength scale read out;
//  XSCONV_LoadSlitFunction - slit function read out;
//  XSCONV_LoadCrossSectionFile - load a cross section file;
//
//  ==========================================
//  CONVOLUTION DIALOG BOX MESSAGES PROCESSING
//  ==========================================
//
//  GENERAL TAB PAGE MESSAGES PROCESSING
//  ------------------------------------
//
//  XsconvGeneralKurucz - enable/Disable Kurucz fields on convolution type;
//  XsconvGeneralInit - WM_INIT message processing;
//  XsconvGeneralCommand - WM_COMMAND message processing;
//  XsconvGeneralDestroy - WM_DESTROY message processing;
//
//  XSCONV_GeneralWndProc - dispatch messages from the page related to general options;
//
//  SLIT FUNCTION TAB PAGE MESSAGES PROCESSING
//  ------------------------------------------
//
//  XsconvSlitInit - WM_INIT message processing;
//  XsconvSlitCommand - WM_COMMAND message processing;
//  XsconvSlitDestroy - WM_DESTROY message processing;
//
//  XSCONV_SlitWndProc - dispatch messages from the page related to the slit function;
//  XSCONV_SlitType - according to the selected slit function, show/hide slit options;
//
//  CONVOLUTION DIALOG BOX MESSAGES PROCESSING
//  ------------------------------------------
//
//  XsconvInit - initialization of the convolution tools dialog box (WM_INITDIALOG);
//  XsconvNotify - WM_NOTIFY message processing;
//  XsconvOK- validate the convolution options on OK command;
//  XsConvHelp - dispatch help messages from the convolution tools dialog box;
//  XsconvCommand - dispatch command messages from the convolution tools dialog box;
//
//  XSCONV_WndProc - dispatch messages from the convolution tools dialog box;
//
//  ==================
//  CONFIGURATION FILE
//  ==================
//
//  XSCONV_ResetConfiguration- initialize the convolution options to default values;
//  XSCONV_LoadConfiguration - load last used information in the convolution tool box from the wds configuration file;
//  XSCONV_SaveConfiguration - save the last information from the convolution tool box in the wds configuration file.
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string.h>
#include <math.h>

#include "xsconv.h"

#include "engine_context.h"
#include "doas.h"
#include "winfiles.h"
#include "spline.h"
#include "filter.h"
#include "erf.h"
#include "vector.h"

// =====================
// CONSTANTS DEFINITIONS
// =====================

#define XSCONV_SECTION "Convolution"
#define NFWHM          18                 // number of pixels/FWHM

double Voigtx(double x,double y);

// Slit types

const char *XSCONV_slitTypes[SLIT_TYPE_MAX]=
 {
     "None",
  "File",
  "Gaussian (FTS)",
  "2n-Lorentz",
  "Voigt",
  "Error function (FTS)",
  "Asymmetric gaussian",
  "Super gaussian",
  "Boxcar (FTS)",
  "Norton Beer Strong (FTS)",
//  "Gaussian, wavelength dependent",
//  "2n-Lorentz, wavelength dependent",
//  "Voigt, wavelength dependent",
//  "Error function, wavelength dependent",
//  "Gaussian, wavelength+t° dependent",
//  "Error function, wavelength+t° dependent"
 };

// =====================
// SUPPORTED LINE SHAPES
// =====================

// -----------------------------------------------------------------------------
// FUNCTION      XSCONV_FctGauss
// -----------------------------------------------------------------------------
// PURPOSE       Gaussian function
//
// INPUT         fwhm   : full-width at half maximum of the function;
//               step   : resolution of the line shape (usefule for normalisation);
//               delta  : the distance to the centre wavelength (or
//                        wavenumber);
//
// OUTPUT        pValue : the value of the function calculated at delta;
//
// RETURN        rc     : ERROR_ID_BAD_ARGUMENTS if one of the argument is not correct
//                        ERROR_ID_NO if the function is successful.
// -----------------------------------------------------------------------------

RC XSCONV_FctGauss(double *pValue,double fwhm,double step,double delta)
 {
  // Declarations

  // use sigma2=fwhm/2 because in the S/W user manual sigma=fwhm

  double sigma2,a,ia;                                                           // temporary variables
  RC rc;                                                                        // return code

  // Initializations

  *pValue=(double)0.;
  rc=ERROR_ID_NO;

  // Gaussian function

  if (fwhm<=(double)0.)
   rc=ERROR_SetLast("XSCONV_FctGauss",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"fwhm<=0");
  else if (step<=(double)0.)
   rc=ERROR_SetLast("XSCONV_FctGauss",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"step<=0");
  else
   {
    sigma2=fwhm*0.5;
    a=sigma2/sqrt(log(2.));
    ia=(double)step/(a*sqrt(DOAS_PI));

    *pValue=ia*exp(-(delta*delta)/(a*a));
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      XsconvFctApod
// -----------------------------------------------------------------------------
// PURPOSE       Apodisation function (FTS)
//
// INPUT         resolution  : the resolution;
//               dispersion  : the dispersion;
//               phase       : the phase;
//               delta       : the distance to the centre wavelength (or
//                             wavenumber);
//
// OUTPUT        pValue      : the value of the function calculated in delta;
//
// RETURN        rc  ERROR_ID_BAD_ARGUMENTS if one of the argument is not correct
//                   ERROR_ID_NO if the function is successful.
// -----------------------------------------------------------------------------
// PROCESSING
//
// This function is implemented as provided in apod.m MATLAB function by
// Ann-Carine VANDAELE (IASB/BIRA, ULB)
// -----------------------------------------------------------------------------

RC XsconvFctApod(double *pValue,double resolution,double phase,double dispersion,double delta)
 {
  // Declarations

  double phi,d,a1,sinphi,cosphi,b,eps;                                          // temporary variables
  RC rc;                                                                        // return code

  // Initializations

  *pValue=(double)0.;
  eps=(double)1.e-6;
  rc=ERROR_ID_NO;

  // Apodisation function

  if (resolution<=(double)0.)
   rc=ERROR_SetLast("XsconvFctApod",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"resolution<=0");
  else
   {
    phi=(double)phase*DOAS_PI/180.0;                                                 // degree -> radians
    d=(double)1.8*dispersion/resolution;
    a1=(double)DOAS_PI*1.8/resolution;
    sinphi=sin(phi);
    cosphi=cos(phi);
    b=a1*delta;

    *pValue=(fabs(b)<eps)?d*cosphi:d*(sinphi-sin(phi-b))/b;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      XsconvFctApodNBS
// -----------------------------------------------------------------------------
// PURPOSE       Apodisation function Norton Beer Strong (FTS)
//
// INPUT         resolution  : the resolution;
//               dispersion  : the dispersion;
//               phase       : the phase;
//               delta       : the distance to the centre wavelength (or
//                             wavenumber);
//
// OUTPUT        pValue      : the value of the function calculated in delta;
//
// RETURN        rc     : ERROR_ID_BAD_ARGUMENTS if one of the argument is not correct
//                        ERROR_ID_NO if the function is successful.
// -----------------------------------------------------------------------------
// PROCESSING
//
// This function is implemented as provided in apodNBS.m MATLAB function by
// Ann-Carine VANDAELE (IASB/BIRA, ULB)
// -----------------------------------------------------------------------------

RC XsconvFctApodNBS(double *pValue,double resolution,double phase,double dispersion,double delta)
 {
  // Declarations

  double phi,d,a1,sinphi,cosphi,b,                                              // temporary variables
         phib,c0,c1,c2,c4,
         btwo,bfour,beight,dtempm,eps;

  RC rc;                                                                        // return code

  // Initializations

  *pValue=(double)0.;
  eps=(double)1.e-6;
  rc=ERROR_ID_NO;

  // Apodisation function (NBS)

  if (resolution<=(double)0.)
   rc=ERROR_SetLast("XsconvFctApodNBS",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"resolution<=0");
  else
   {
    phi=(double)phase*DOAS_PI/180.0;                                                 // degree -> radians
    d=(double)1.8*dispersion/resolution;
    a1=(double)DOAS_PI*1.8/resolution;
    sinphi=sin(phi);
    cosphi=cos(phi);
    b=a1*delta;
    phib=phi-b;

    c0=0.09;
    c1=0.0;
    c2=0.5875*d*8.0;
    c4=0.3225*d*128.0;

    btwo=b*b;
    bfour=btwo*btwo;
    beight=bfour*bfour;

    dtempm=c0*(sinphi-sin(phib))/b+
           c2*((3.0+btwo/2.0+bfour/8.0)*sinphi-(3.0-btwo)*sin(phib)-3.0*cos(phib)*b)/(bfour*b)+
           c4*((315.0+22.5*btwo+9.0*bfour/8.0+bfour*btwo/16.0+beight/128.0)*sinphi-(315.0-135.0*btwo+3.0*bfour)*sin(phib)-(315.0-30.0*btwo)*cos(phib)*b)/(beight*b);

    *pValue=(fabs(b)<eps)?(c0+c1/3.0+c2/15.0+c4/315.0)*cosphi:dtempm;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      XsconvFctBuild
// -----------------------------------------------------------------------------
// PURPOSE       Build a line shape in a vector
//
// INPUT         slitLambda  : the calibration (wavelength, wavenumber or pixel);
//               slitSize    : the size of previous vector;
//               slitType    : the type of line shape to build;
//               slitParam   : the list of slit function parameters
//               nSlitParam  : the number of elements in the previous list
//
// OUTPUT        slitVector  : the calculated line shape
//
// RETURN        rc          : return code
// -----------------------------------------------------------------------------

RC XsconvFctBuild(double *slitLambda,double *slitVector,int slitSize,int slitType,double *slitParam,int nSlitParam)
 {
  // Declarations

  double sum,                                                                   // calculate the integral of the line shape for normalisation
         invSum,                                                                // inverse of the sum (in order to optimize the calculation time)
         slitStep;                                                              // the slit function dispersion
  INDEX  i;                                                                     // browse wavelength
  RC     rc;                                                                    // return code

  // Initializations

  sum=(double)0.;
  rc=ERROR_ID_NO;

  // Get the dispersion

  if (slitSize<=1)
   rc=ERROR_SetLast("XsconvFctBuild",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"slitSize<=1");
 else
   {
    // Calculate the resolution of the slit function

    slitStep=(double)(slitLambda[slitSize-1]-slitLambda[0])/(slitSize-1);

    // Build the slit function; the message is dispatched according to the selected type of line shape

    switch(slitType)
     {
   // --------------------------------------------------------------------------
      case SLIT_TYPE_GAUSS :

       if (nSlitParam<1)
        rc=ERROR_SetLast("XsconvFctBuild",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"Gauss function needs 1 parameter");
       else
        for (i=0;(i<slitSize) && !rc;i++)
         rc=XSCONV_FctGauss(&slitVector[i],
                             slitParam[0],                                      // FWHM
                             slitStep,                                          // resolution of the line shape
                             slitLambda[i]);                                    // distance to the centre wavelength

      break;
   // --------------------------------------------------------------------------
      case SLIT_TYPE_APOD :
      case SLIT_TYPE_APODNBS :

       if (nSlitParam<3)
        rc=ERROR_SetLast("XsconvFctBuild",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"Apod function needs 3 parameters");

       else if (slitType==SLIT_TYPE_APOD)
        for (i=0;(i<slitSize) && !rc;sum+=slitVector[i],i++)
         rc=XsconvFctApod(&slitVector[i],
                               slitParam[0],                                    // resolution
                               slitParam[1],                                    // phase
                               slitParam[2],                                    // dispersion
                               slitLambda[i]);                                  // distance to the centre wavelength

       else if (slitType==SLIT_TYPE_APODNBS)
        for (i=0;(i<slitSize) && !rc;sum+=slitVector[i],i++)
         rc=XsconvFctApodNBS(&slitVector[i],
                                  slitParam[0],                                 // resolution
                                  slitParam[1],                                 // phase
                                  slitParam[2],                                 // dispersion
                                  slitLambda[i]);                               // distance to the centre wavelength

      break;
   // --------------------------------------------------------------------------
      default :
       rc=ERROR_SetLast("XsconvFctBuild",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"the type of line shape is not supported");
      break;
   // --------------------------------------------------------------------------
     }

    // Normalisation of the slit function

    if (!rc && (slitType==SLIT_TYPE_APOD))
     {
      if (sum==(double)0.)
       rc=ERROR_SetLast("XsconvFctBuild",ERROR_TYPE_FATAL,ERROR_ID_DIVISION_BY_0,"the type of line shape is not supported");
      else if (slitType==SLIT_TYPE_APOD)
       for (invSum=(double)1./sum,i=0;i<slitSize;i++)
        slitVector[i]*=invSum;
     }
   }

  return rc;
 }

// ===============
// DATA PROCESSING
// ===============

// --------------------------------------------------------
// XSCONV_GetFwhm : Get slit function full width half way up
// --------------------------------------------------------

RC XSCONV_GetFwhm(double *lambda,double *slit,double *deriv2,int nl,int slitType,double *slitParam)
 {
  // Declarations

  double max,lmin,lmax,l1,l2,x,x0;                         // search for wavelengths giving function at half way up

  // Initializations

  x0=(double)0.;

  if (slitType==SLIT_TYPE_FILE) {
    // Get the value of function half way up

    SPLINE_Vector(lambda,slit,deriv2,nl,&x0,&max,1,SPLINE_CUBIC);

    max*=(double)0.5;

    // Search for the wavelength in the left part giving the value of function half way up

    for (lmin=lambda[0],lmax=(double)0.,l1=lmin*0.5,x=(double)0.;
        (lmin<lmax) && fabs(x-max)>1.e-5;l1=(lmin+lmax)*0.5) {
      SPLINE_Vector(lambda,slit,deriv2,nl,&l1,&x,1,SPLINE_CUBIC);

      if (x<max)
       lmin=l1;
      else if (x>max)
       lmax=l1;
     }

    // Search for the wavelength in the right part giving the value of function half way up

    for (lmin=(double)0.,lmax=lambda[nl-1],l2=lmax*0.5,x=(double)0.;
        (lmin<lmax) && fabs(x-max)>1.e-5;l2=(lmin+lmax)*0.5) {
      SPLINE_Vector(lambda,slit,deriv2,nl,&l2,&x,1,SPLINE_CUBIC);

      if (x<max)
       lmax=l2;
      else if (x>max)
       lmin=l2;
     }

    // Width of slit function half way up

    *slitParam=(double)(l2-l1);
   }

  return ERROR_ID_NO;
 }

// ================
// FILES PROCESSING
// ================

// ------------------------------------------------------------
// XSCONV_LoadCalibrationFile : Final wavelength scale read out
// ------------------------------------------------------------

RC XSCONV_LoadCalibrationFile(MATRIX_OBJECT *pLambda,char *lambdaFile,int nextraPixels)
 {
  // Declarations

  char  lambdaBuffer[MAX_ITEM_TEXT_LEN];
  double step;
  FILE *lambdaFp;
  int npts;
  INDEX i;
  RC rc;

  // Initializations

  memset(pLambda,0,sizeof(MATRIX_OBJECT));
  rc=ERROR_ID_NO;
  npts=0;

  // Open BINARY file

  FILES_RebuildFileName(lambdaBuffer,lambdaFile,1);

  if ((lambdaFp=fopen(FILES_RebuildFileName(lambdaBuffer,lambdaFile,1),"rt"))==NULL)
   rc=ERROR_SetLast("XSCONV_LoadCalibrationFile",ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_FOUND,lambdaBuffer);
  else
   {
    // Get the number of lines

    while (!feof(lambdaFp) && fgets(lambdaBuffer,MAX_ITEM_TEXT_LEN,lambdaFp))
     if ((strchr(lambdaBuffer,';')==NULL) && (strchr(lambdaBuffer,'*')==NULL))
      npts++;

    if (!npts)
     rc=ERROR_SetLast("XSCONV_LoadCalibrationFile",ERROR_TYPE_FATAL,ERROR_ID_FILE_EMPTY,lambdaFile);

    // Buffers allocation

    else if (MATRIX_Allocate(pLambda,npts+2*nextraPixels,2,0,0,1,"XSCONV_LoadCalibrationFile"))
     rc=ERROR_ID_ALLOC;

    // File read out

    else
     {
      // File read out

      VECTOR_Init(pLambda->matrix[0],(double)0.,npts+2*nextraPixels);
      fseek(lambdaFp,0L,SEEK_SET);

      for (i=0;(i<npts) && !feof(lambdaFp);)
       if (fgets(lambdaBuffer,MAX_ITEM_TEXT_LEN,lambdaFp) && (strchr(lambdaBuffer,';')==NULL) && (strchr(lambdaBuffer,'*')==NULL))
        {
         sscanf(lambdaBuffer,"%lf ",&pLambda->matrix[0][i+nextraPixels]);
         i++;
        }

      step=(pLambda->matrix[0][nextraPixels+npts-1]-pLambda->matrix[0][nextraPixels])/npts;

      for (i=0;i<nextraPixels;i++)
       {
        pLambda->matrix[0][i]=pLambda->matrix[0][nextraPixels]-step*(nextraPixels-i);
        pLambda->matrix[0][nextraPixels+npts+i]=pLambda->matrix[0][nextraPixels+npts-1]+step*i;
       }
     }
   }

  // Close file

  if (lambdaFp!=NULL)
   fclose(lambdaFp);

  // Return

  return rc;
 }

// ----------------------------------------------------------------------------
// FUNCTION        XSCONV_LoadSlitFunction
// ----------------------------------------------------------------------------
// PURPOSE         Load slit function parameters from file(s) or build the slit
//                 function
//
// INPUT           slitXs      slit matrix (array of matrices depending on the number of parameters);
//                 pSlit       the slit functions parameters (from the interface)
//
// OUTPUT          pGaussWidth the FWHM
//                 pSlitType
//
// RETURN          return code
// ----------------------------------------------------------------------------

RC XSCONV_LoadSlitFunction(MATRIX_OBJECT *slitXs,SLIT *pSlit,double *pGaussWidth,int *pSlitType)
 {
  // Declarations

  MATRIX_OBJECT *pSlitXs;
  char  slitBuffer[MAX_ITEM_TEXT_LEN],
        slitBuffer2[MAX_ITEM_TEXT_LEN],
        slitBuffer3[MAX_ITEM_TEXT_LEN];
  RC     rc;
  INDEX  i;
  double x,a,a2,delta,sigma2,invapi,sumPoly,slitStep,norm1,slitParam[4],fwhm;
  int    slitType,slitSize,nFwhm;

  // Initializations

  for (i=0;i<NSFP;i++)
   memset(&slitXs[i],0,sizeof(MATRIX_OBJECT));

  pSlitXs=&slitXs[0];

  invapi=norm1=(double)0.;
  sumPoly=(double)0.;

  slitParam[0]=pSlit->slitParam;
  slitParam[1]=pSlit->slitParam2;
  slitParam[2]=pSlit->slitParam3;    // NSFP should be >=3 (cfr constants.h)

  slitType=pSlit->slitType;

  if (pSlitType!=NULL)
   *pSlitType=slitType;

  nFwhm=NFWHM;         // number of pixels by FWHM
  rc=ERROR_ID_NO;

  // Input read out

  if ((slitType==SLIT_TYPE_FILE) || pSlit->slitWveDptFlag)
   {
    FILES_RebuildFileName(slitBuffer,pSlit->slitFile,1);
    FILES_RebuildFileName(slitBuffer2,pSlit->slitFile2,1);
    FILES_RebuildFileName(slitBuffer3,pSlit->slitFile3,1);

    // Load the file

    if  ((strlen(pSlit->slitFile)  && !(rc=MATRIX_Load(slitBuffer,&slitXs[0],0,0,-9999.,9999.,1,0,"XSCONV_LoadSlitFunction"))) && // the first file (fhwm) should be present
        (!strlen(pSlit->slitFile2) || !(rc=MATRIX_Load(slitBuffer2,&slitXs[1],0,0,-9999.,9999.,1,0,"XSCONV_LoadSlitFunction"))) &&
        (!strlen(pSlit->slitFile3) || !(rc=MATRIX_Load(slitBuffer3,&slitXs[2],0,0,-9999.,9999.,1,0,"XSCONV_LoadSlitFunction"))) &&
         (slitType==SLIT_TYPE_FILE))
     {
      for (i=1,sumPoly=(double)0.;i<pSlitXs->nl;i++)
       sumPoly+=(pSlitXs->matrix[1][i]+pSlitXs->matrix[1][i-1])*(pSlitXs->matrix[0][i]-pSlitXs->matrix[0][i-1])*0.5;

      if (pSlitXs->nc==2)
       {
        for (i=0;i<pSlitXs->nl;i++)
         pSlitXs->matrix[1][i]/=sumPoly;

           if (!(rc=SPLINE_Deriv2(pSlitXs->matrix[0],pSlitXs->matrix[1],pSlitXs->deriv2[1],pSlitXs->nl,"XSCONV_LoadSlitFunction")))
            rc=XSCONV_GetFwhm(pSlitXs->matrix[0],pSlitXs->matrix[1],pSlitXs->deriv2[1],pSlitXs->nl,slitType,pGaussWidth);
       }
      else if (pSlitXs->nc>2)
       {
           if (!(rc=SPLINE_Deriv2(pSlitXs->matrix[0]+1,pSlitXs->matrix[1]+1,pSlitXs->deriv2[1]+1,pSlitXs->nl-1,"XSCONV_LoadSlitFunction")))
            rc=XSCONV_GetFwhm(pSlitXs->matrix[0]+1,pSlitXs->matrix[1]+1,pSlitXs->deriv2[1]+1,pSlitXs->nl-1,slitType,pGaussWidth);
       }
     }
   }
  else if (pSlit->slitParam<=(double)EPSILON)
   rc=ERROR_SetLast("XSCONV_LoadSlitFunction",ERROR_TYPE_WARNING,ERROR_ID_FWHM,"SlitParam");   // impossible to build slit function if FHHM is <=0.

  // Precalculate the slit function

  else if ((slitType!=SLIT_TYPE_APOD) && (slitType!=SLIT_TYPE_APODNBS))
   {
       fwhm=(slitType!=SLIT_TYPE_ERF)?pSlit->slitParam:sqrt(pSlit->slitParam*pSlit->slitParam+pSlit->slitParam2*pSlit->slitParam2);
    slitStep=fwhm/nFwhm;                               // number of points /FWHM

 //   slitSize=(int)(ceil((nFwhm*pSlit->slitParam)/slitStep)+1.);

    slitSize=nFwhm*nFwhm+1;

//    slitSize=1001;
//    slitStep=(6*pSlit->slitParam)/(slitSize-1);

    if ((pSlitType!=NULL) && !MATRIX_Allocate(pSlitXs,slitSize,2,0,0,1,"XSCONV_LoadSlitFunction"))
     {
         // use sigma2=fwhm/2 because in the S/W user manual sigma=fwhm

      sigma2=pSlit->slitParam*0.5;
      a=((slitType!=SLIT_TYPE_SUPERGAUSS) || (fabs(pSlit->slitParam2)<EPSILON))?sigma2/sqrt(log(2.)):sigma2/pow(log(2.),(double)1./pSlit->slitParam2);
      delta=pSlit->slitParam2*0.5;

      if (slitType==SLIT_TYPE_GAUSS)
       invapi=(double)1./(a*sqrt(DOAS_PI))*slitStep;
      else if (slitType==SLIT_TYPE_VOIGT)
       {
        norm1=(double)Voigtx((double)0.,pSlit->slitParam2);

        if (norm1==(double)0.)
         rc=ERROR_SetLast("XSCONV_LoadSlitFunction",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"Voigt norm");
        else
         norm1=(double)1./norm1;
       }
      else
       norm1=(double)1.;

      for (i=0,x=(double)-0.5*nFwhm*fwhm;(i<slitSize) && !rc;i++,x+=(double)slitStep)
       {
        pSlitXs->matrix[0][i]=(double)x;

        if (slitType==SLIT_TYPE_INVPOLY)
         sumPoly+=pSlitXs->matrix[1][i]=(double)pow(sigma2,pSlit->slitParam2)/(pow(x,pSlit->slitParam2)+pow(sigma2,pSlit->slitParam2));
        else if (slitType==SLIT_TYPE_ERF)
         pSlitXs->matrix[1][i]=(double)(ERF_GetValue((x+delta)/a)-ERF_GetValue((x-delta)/a))/(4.*delta)*slitStep;
        else if (slitType==SLIT_TYPE_AGAUSS)
         {
             a2=(x<(double)0.)?a*(1.-pSlit->slitParam2):a*(1.+pSlit->slitParam2);
          pSlitXs->matrix[1][i]=(double)exp(-(x*x)/(a2*a2));
         }
        else if (slitType==SLIT_TYPE_SUPERGAUSS)
         {
             a2=(x<(double)0.)?a*(1.-pSlit->slitParam3):a*(1.+pSlit->slitParam3);     // for super gaussian, asymmetry factor is the third one
          pSlitXs->matrix[1][i]=(double)exp(-pow(fabs(x/a2),pSlit->slitParam2));
         }
        else if (slitType==SLIT_TYPE_GAUSS)
         pSlitXs->matrix[1][i]=(double)invapi*exp(-(x*x)/(a*a));
        else if (slitType==SLIT_TYPE_VOIGT)
         {
          pSlitXs->matrix[1][i]=(double)Voigtx(x/a,pSlit->slitParam2)*norm1;
          sumPoly+=pSlitXs->matrix[1][i];
         }
       }

   //   exit(1);
      if (slitType==SLIT_TYPE_GAUSS)
       rc=XsconvFctBuild(pSlitXs->matrix[0],pSlitXs->matrix[1],slitSize,pSlit->slitType,slitParam,NSFP);

      if (!rc)
       {
        if ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_VOIGT))
         for (i=0;i<slitSize;i++)
          pSlitXs->matrix[1][i]/=sumPoly;

        rc=SPLINE_Deriv2(pSlitXs->matrix[0],pSlitXs->matrix[1],pSlitXs->deriv2[1],slitSize,"XSCONV_LoadSlitFunction ");

        if (pSlitType!=NULL)
         *pSlitType=SLIT_TYPE_FILE;
       }
     }

    if (pGaussWidth!=NULL)
     *pGaussWidth=pSlit->slitParam;
   }

  // for apodisation function, no pre-calculation

  else if (pGaussWidth!=NULL)
   *pGaussWidth=pSlit->slitParam;

// {
//     FILE *fp;
//     fp=fopen("slit.dat","w+t");
//     fprintf(fp,"; slitSize %d\n",slitSize);
//     // for (i=0;i<pSlitXs->nl;i++)
//     for (i=0;(i<slitSize) && !rc;i++)
//      fprintf(fp,"%g %g %g\n",pSlitXs->matrix[0][i],pSlitXs->matrix[1][i],pSlitXs->deriv2[1][i]);
//     fclose(fp);
// }

  // Return

  return rc;
 }

RC XSCONV_ConvertCrossSectionFile(MATRIX_OBJECT *pCross, double lambdaMin,double lambdaMax,double shift,int conversionMode) {
   // Declarations

   INDEX i;
   double lambda;
   RC rc = ERROR_ID_NO;

   // Load file

   if ( conversionMode!=CONVOLUTION_CONVERSION_NONE || (fabs(shift)>EPSILON)) {
     for (i=0;i<pCross->nl;i++) {
       lambda=pCross->matrix[0][i];

       // Air <-> vacuum correction

       if (conversionMode==CONVOLUTION_CONVERSION_VAC2AIR)
           lambda=(double)0.1*(9.9972683*lambda+0.0107-(19.625/lambda));
       else if (conversionMode==CONVOLUTION_CONVERSION_AIR2VAC)
         lambda=(double)0.1*(10.0027325*lambda-0.0107+(19.625/lambda));

       // Apply shift

       pCross->matrix[0][i]=lambda+shift;
     }

     // Recalculate second derivatives

     rc = SPLINE_Deriv2(pCross->matrix[0],pCross->matrix[1],pCross->deriv2[1],pCross->nl,"XSCONV_LoadCrossSectionFile");
   }
   return rc;
}

// -------------------------------------------------------
// XSCONV_LoadCrossSectionFile : Load a cross section file
// -------------------------------------------------------

RC XSCONV_LoadCrossSectionFile(MATRIX_OBJECT *pCross,char *crossFile,double lambdaMin,double lambdaMax,double shift,int conversionMode)
{
  RC rc = rc=MATRIX_Load(crossFile,pCross,0,0,lambdaMin,lambdaMax,1,0,__func__);
  if(!rc) {
    rc = XSCONV_ConvertCrossSectionFile(pCross, lambdaMin, lambdaMax, shift, conversionMode);
  }
  return rc;
}

// =====================
// CONVOLUTION FUNCTIONS
// =====================

// ----------------------------------------------------------
// XSCONV_TypeNone : Apply no convolution, interpolation only
// ----------------------------------------------------------

RC XSCONV_TypeNone(MATRIX_OBJECT *pXsnew,MATRIX_OBJECT *pXshr)
 {
  return SPLINE_Vector(pXshr->matrix[0],pXshr->matrix[1],pXshr->deriv2[1],pXshr->nl,pXsnew->matrix[0],pXsnew->matrix[1],pXsnew->nl,SPLINE_CUBIC);
 }

// -----------------------------------------------------------------------
// XSCONV_TypeGauss : Gaussian convolution with variable half way up width
// -----------------------------------------------------------------------

 RC XSCONV_TypeGauss(const double *lambda, const double *Spec, const double *SDeriv2,double lambdaj,
                     double dldj,double *SpecConv,double fwhm,double slitParam2,int slitType, int ndet)
 {
   // Declarations

   double h,oldF,newF,Lim,ld_inc,ldi,dld,a,delta,
     lambdaMax,SpecOld, SpecNew,sigma2,
     crossFIntegral, FIntegral;

   RC rc = ERROR_ID_NO;

   // Initializations

   fwhm=fabs(fwhm);
   crossFIntegral=FIntegral=(double)0.;
   sigma2=fwhm*0.5;                                                             // use sigma2=fwhm/2 because in the S/W user manual sigma=fwhm
   a=sigma2/sqrt(log(2.));
   delta=slitParam2*0.5;

   Lim=(double)2.*fwhm;

   if ((ld_inc=(double)fwhm/3.)>dldj)
    ld_inc=dldj;

   h=(double)ld_inc*0.5;

   // Browse wavelengths in the final calibration vector

   ldi=lambdaj-Lim;
   lambdaMax=lambdaj+Lim;

   // Search for first pixel in high resolution cross section in the wavelength range delimited by slit function

   dld = -(ldi-lambdaj);

   if (slitType==SLIT_TYPE_GAUSS) {
    //   oldF=(double)exp(-4.*log(2.)*(dld*dld)/(fwhm*fwhm));
     rc=XSCONV_FctGauss(&oldF,fwhm,ld_inc,dld);
   } else if (slitType==SLIT_TYPE_INVPOLY) {
    oldF=(double)pow(sigma2,(double)slitParam2)/(pow(dld,(double)slitParam2)+pow(sigma2,(double)slitParam2));
   } else if (slitType==SLIT_TYPE_ERF) {
    oldF=(double)(ERF_GetValue((dld+delta)/a)-ERF_GetValue((dld-delta)/a))/(4.*delta);
   }

   if (!rc) {
     SPLINE_Vector(lambda,Spec,SDeriv2,ndet,&ldi,&SpecOld,1,SPLINE_CUBIC);
   }

   while (!rc && (ldi<=lambdaMax))
    {
     ldi += (double) ld_inc;
     dld = -(ldi-lambdaj);

     if (slitType==SLIT_TYPE_GAUSS) {
      //     newF=(double)exp(-4.*log(2.)*(dld*dld)/(fwhm*fwhm));
      rc=XSCONV_FctGauss(&newF,fwhm,ld_inc,dld);
      if (rc != 0)
        return rc;
     } else if (slitType==SLIT_TYPE_INVPOLY) {
      newF=(double)pow(sigma2,(double)slitParam2)/(pow(dld,(double)slitParam2)+pow(sigma2,(double)slitParam2));
     } else if (slitType==SLIT_TYPE_ERF) {
      newF=(double)(ERF_GetValue((dld+delta)/a)-ERF_GetValue((dld-delta)/a))/(4.*delta);
     }

     SPLINE_Vector(lambda,Spec,SDeriv2,ndet,&ldi,&SpecNew,1,SPLINE_CUBIC);

     crossFIntegral += (SpecOld*oldF+SpecNew*newF)*h;
     FIntegral      += (oldF+newF)*h;

     oldF=newF;
     SpecOld=SpecNew;
    }

   *SpecConv=(FIntegral!=(double)0.)?(double)crossFIntegral/FIntegral:(double)1.;

   // Return

   return rc;
 }

RC XSCONV_TypeStandardFFT(FFT *pFFT,int fwhmType,double slitParam,double slitParam2,double *lambda, double *target,int size)
 {
  // Declarations

  double F,G,w,a,sigma2,delta,step;
  INDEX i;
  int ndemi;
  RC rc;

  // Initializations

  if ((slitParam>(double)0.) &&
     ((fwhmType==SLIT_TYPE_GAUSS) || ((fwhmType==SLIT_TYPE_ERF) && (slitParam2>(double)0.))))
   {
    ndemi=pFFT->fftSize>>1;
    step=(pFFT->fftIn[pFFT->oldSize]-pFFT->fftIn[1])/(pFFT->oldSize-1.);

    sigma2=slitParam*0.5;                                                       // use sigma2=fwhm/2 because in the S/W user manual sigma=fwhm
    a=sigma2/sqrt(log(2.));
    delta=slitParam2*0.5;

    w=(double)DOAS_PI/step;
    F=exp(-a*a*w*w*0.25);
    G=(fwhmType==SLIT_TYPE_GAUSS)?(double)1.:sin(w*delta)/(w*delta);

    pFFT->invFftIn[1]=pFFT->fftOut[1];
    pFFT->invFftIn[2]=pFFT->fftOut[2]*F*G;

    for (i=2;i<=ndemi;i++)
     {
      w=(double)DOAS_PI*(i-1.)/(ndemi*step);

      F=(double)exp(-a*a*w*w*0.25);
      G=(double)(fwhmType==SLIT_TYPE_GAUSS)?(double)1.:(double)sin(w*delta)/(w*delta);

      pFFT->invFftIn[(i<<1) /* i*2 */-1]=pFFT->fftOut[(i<<1) /* i*2 */-1]*F*G;      // Real part
      pFFT->invFftIn[(i<<1) /* i*2 */]=pFFT->fftOut[(i<<1) /* i*2 */]*F*G;          // Imaginary part
     }

    realft(pFFT->invFftIn,pFFT->invFftOut,pFFT->fftSize,-1);

//    for (i=1;i<=pFFT->fftSize;i++)
//     pFFT->invFftOut[i]/=step;

    if (!(rc=SPLINE_Deriv2(pFFT->fftIn+1,pFFT->invFftOut+1,pFFT->invFftIn+1,pFFT->oldSize,"XSCONV_TypeStandardFFT ")))
      SPLINE_Vector(pFFT->fftIn+1,pFFT->invFftOut+1,pFFT->invFftIn+1,pFFT->oldSize,lambda,target,size,SPLINE_CUBIC);
   }
  else
   rc=ERROR_SetLast("XSCONV_TypeStandardFFT",ERROR_TYPE_WARNING,ERROR_ID_BAD_ARGUMENTS);

  // Return

  return rc;
 }

// GetNewF : convolve at a given point
//           wavelength dependent slit function have been pre-calculated

RC GetNewF(double *pNewF,
           int     slitType,
           const double *slitLambda,
           const double *slitVector,
           const double *slitDeriv2,
           int     slitNDET,
           double  dist,
           double  slitParam,
           double  slitParam2,
           double  slitParam3,
           double  step)
 {
  double sigma2,a,a2,newF,norm1,delta;
  int rc;

  newF=(double)0.;
  sigma2=(double)slitParam*0.5;                                                 // use sigma2=fwhm/2 because in the S/W user manual sigma=fwhm
  a=((slitType!=SLIT_TYPE_SUPERGAUSS) || (fabs(slitParam2)<EPSILON))?sigma2/sqrt(log(2.)):sigma2/pow(log(2.),(double)1./slitParam2);
  //a=(double)sigma2/sqrt(log(2.));
  delta=(double)slitParam2*0.5;

  rc=ERROR_ID_NO;

  if (slitType==SLIT_TYPE_VOIGT)
   {
    norm1=(double)Voigtx((double)0.,slitParam2);

    if (norm1==(double)0.)
     rc=ERROR_SetLast("GetNewF",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"calculation of the voigt function");
    else
     norm1=(double)1./norm1;
   }
  else
   norm1=(double)1.;

  if (slitType==SLIT_TYPE_INVPOLY)
   newF=(double)pow(sigma2,slitParam2)/(pow(dist,slitParam2)+pow(sigma2,slitParam2));
  else if ((slitType==SLIT_TYPE_ERF) && (slitParam2!=(double)0.))
   newF=(double)(ERF_GetValue((dist+delta)/a)-ERF_GetValue((dist-delta)/a))/(4.*delta);
  else if (slitType==SLIT_TYPE_AGAUSS)
   {
    a2=(dist<(double)0.)?(double)a*(1.-slitParam2):(double)a*(1.+slitParam2);
    newF=(double)exp(-(dist*dist)/(a2*a2));
   }
  else if (slitType==SLIT_TYPE_SUPERGAUSS)
   {
    a2=(dist<(double)0.)?(double)a*(1.-slitParam3):(double)a*(1.+slitParam3);
    newF=(double)exp(-pow(fabs(dist/a2),slitParam2));
   }
  else if (slitType==SLIT_TYPE_VOIGT)
   newF=(double)Voigtx(dist/a,slitParam2)*norm1;
  else if (slitType==SLIT_TYPE_FILE)
    SPLINE_Vector(slitLambda,slitVector,slitDeriv2,slitNDET,&dist,&newF,1,SPLINE_CUBIC);
  else if (slitType==SLIT_TYPE_APOD)
   rc=XsconvFctApod(&newF,slitParam,slitParam2,0.01,dist);
  else if (slitType==SLIT_TYPE_APODNBS)
   rc=XsconvFctApodNBS(&newF,slitParam,slitParam2,0.01,dist);
  else // if (slitType==SLIT_TYPE_GAUSS)
   newF=(double)exp(-4.*log(2.)*(dist*dist)/(slitParam*slitParam));

//   rc=XSCONV_FctGauss(&newF,slitParam,step,dist);

  *pNewF=newF;

  return rc;
 }

// --------------------------------------------------------------------------------
// XSCONV_TypeStandard : Standard convolution of cross section with a slit function
// --------------------------------------------------------------------------------

//
// RC XSCONV_TypeStandard(XS *pXsnew,XS *pXshr,XS *pSlit,XS *pI,double *Ic,
//                        int slitType,double slitWidth,double slitParam)
//
// with :
//
//  - pXsnew->lambda : final wavelength scale (input);
//  - pXsnew->vector : pXshr->vector after convolution (output);
//  - indexLambdaMin,indexLambdaMax : the calibration range to calculate
//
//  - pXshr : cross section high resolution (wavelength scale,slit vector and second derivatives);
//
//  - pSlit : if wveDptFlag=1 : wavelength dependent slit function parameters (wavelength scale, slit vector and second derivatives);
//  - slitParam : if wveDptFlag=0 : slit function parameters (constants)
//
//  - pI,Ic : these extra parameters are mainly used when I0 correction is applied in order to speed up total convolution
//            work because integrals of I and I0 can be computed simultaneously;
//
//    if I0 correction is applied, Ic is the I convoluted vector;
//    if no I0 correction is applied, Ic is set to NULL but pI is set to pXshr in order to avoid extra tests in loops;
//
//    the computed integral is then the same as pXshr's one and is not used;
//
//  - slitType : type of slit function;
//
// NB : pI->lambda==pXshr->lambda.
//

RC XSCONV_TypeStandard(MATRIX_OBJECT *pXsnew,INDEX indexLambdaMin,INDEX indexLambdaMax,const MATRIX_OBJECT *pXshr,
                          const MATRIX_OBJECT *pI, double *Ic,int slitType,const MATRIX_OBJECT *slitMatrix, double *slitParam,int wveDptFlag)
 {
  // Declarations
  MATRIX_OBJECT slitTmp;
  double *xsnewLambda,*xsnewVector,
         *xshrLambda,*xshrVector,*xshrDeriv2,
         *slitLambda[NSFP],*slitVector[NSFP],*slitDeriv2[NSFP],
          slitWidth,dist,
         *IVector,*IDeriv2,
          crossFIntegral,IFIntegral,FIntegral,
          oldF,newF,oldIF,newIF,stepF,h,fwhm,
          slitCenter,
          stepXshr,slitStretch1,slitStretch2,
          lambdaMin,lambdaMax,oldXshr,newXshr;
  INDEX   xshrPixMin,
          xsnewIndex,indexOld,indexNew,
          klo,khi,i;
  int     xshrNDET,xsnewNDET,slitNDET[NSFP];
  RC      rc;

  memset(&slitTmp,0,sizeof(MATRIX_OBJECT));
  fwhm=slitWidth=(double)0.;
  rc=ERROR_ID_NO;

  // Use substitution variables

  xsnewLambda=pXsnew->matrix[0];
  xsnewVector=pXsnew->matrix[1];

  xshrLambda=pXshr->matrix[0];
  xshrVector=pXshr->matrix[1];
  xshrDeriv2=pXshr->deriv2[1];

  memset(slitNDET,0,sizeof(int)*NSFP);

  for (i=0;i<NSFP;i++)
   slitLambda[i]=slitVector[i]=slitDeriv2[i]=(double *)NULL;

  // Slit function files and wavelength dependent functions require vectors

  if (slitMatrix!=NULL)
   {
       fwhm=(double)0.;    // will be calculated later

       for (i=0;i<NSFP;i++)
        {
            if ((slitNDET[i]=slitMatrix[i].nl)>0)
             {
           slitLambda[i]=slitMatrix[i].matrix[0];
           slitVector[i]=slitMatrix[i].matrix[1];
           slitDeriv2[i]=slitMatrix[i].deriv2[1];
          }
         else
          slitLambda[i]=slitVector[i]=slitDeriv2[i]=NULL;
        }

       if ((slitType!=SLIT_TYPE_FILE) || (slitMatrix[0].nc==2))
        {
      if (wveDptFlag)
       {
        if ((rc=MATRIX_Allocate(&slitTmp,slitMatrix[0].nl,2,0,0,1,"XSCONV_TypeStandard"))!=0)
         goto EndTypeStandard;
        else
         {
          slitLambda[0]=slitTmp.matrix[0];
          slitVector[0]=slitTmp.matrix[1];
          slitDeriv2[0]=slitTmp.deriv2[1];
          slitNDET[0]=slitTmp.nl;

          memcpy(slitTmp.matrix[0],(double *)slitMatrix[0].matrix[0],sizeof(double)*(slitMatrix[0].nl));
          memcpy(slitTmp.matrix[1],(double *)slitMatrix[0].matrix[1],sizeof(double)*(slitMatrix[0].nl));
          memcpy(slitTmp.deriv2[1],(double *)slitMatrix[0].deriv2[1],sizeof(double)*(slitMatrix[0].nl));
         }
       }

      if (slitType==SLIT_TYPE_FILE)
       rc=XSCONV_GetFwhm(slitLambda[0],slitVector[0],slitDeriv2[0],slitNDET[0],SLIT_TYPE_FILE,&fwhm);
     }

    // Multicolumns files

    else if ((rc=MATRIX_Allocate(&slitTmp,slitMatrix[0].nl-1,2,0,0,1,"XSCONV_TypeStandard"))!=0)
     goto EndTypeStandard;
    else
     {
      slitLambda[0]=slitTmp.matrix[0];
      slitVector[0]=slitTmp.matrix[1];
      slitDeriv2[0]=slitTmp.deriv2[1];
      slitNDET[0]=slitTmp.nl;

      memcpy(slitTmp.matrix[0],(double *)slitMatrix[0].matrix[0]+1,sizeof(double)*(slitMatrix[0].nl-1));
     }
   }
  else
   fwhm=(slitType!=SLIT_TYPE_ERF)?slitParam[0]:sqrt(slitParam[0]*slitParam[0]+slitParam[1]*slitParam[1]);

  xsnewNDET=pXsnew->nl;
  xshrNDET=pXshr->nl;

  IVector=pI->matrix[1];
  IDeriv2=pI->deriv2[1];

  // Initializations

  if (slitType==SLIT_TYPE_FILE)
   {
    for (i=1,stepF=(double)0.;i<slitNDET[0];i++)
     stepF+=(slitLambda[0][i]-slitLambda[0][i-1]);

    stepF/=(slitNDET[0]-1);
   }
  else if ((slitType==SLIT_TYPE_APOD) ||(slitType==SLIT_TYPE_APODNBS))
   stepF=fwhm/50.;
  else
   stepF=fwhm/(double)NFWHM;

// Add an option later  if ((slitType==SLIT_TYPE_FILE) || (slitType==SLIT_TYPE_VOIGT) || (slitType==SLIT_TYPE_AGAUSS))
// Add an option later   {
// Add an option later    // Calculation of the center of the slit function
// Add an option later
// Add an option later    for (i=0,c=si=(double)0.;i<slitNDET;i++)
// Add an option later     {
// Add an option later      c+=(double)slitLambda[i]*slitVector[i];
// Add an option later      si+=(double)slitVector[i];
// Add an option later     }
// Add an option later
// Add an option later    slitCenter=(si>(double)EPSILON)?(double)c/si:(double)0.;
// Add an option later   }
// Add an option later  else
   slitCenter=(double)0.;

  // average wavelength step in Xshr:
  stepXshr = (xshrLambda[xshrNDET-1] - xshrLambda[0])/(xshrNDET-1);

  // Browse wavelengths in the final calibration vector

  for (xsnewIndex=max(0,indexLambdaMin);(xsnewIndex<xsnewNDET) && (xsnewIndex<indexLambdaMax) && !rc;xsnewIndex++) {
    double lambda=xsnewLambda[xsnewIndex];
    slitStretch1=slitStretch2=(double)1.;

    if ((slitType==SLIT_TYPE_FILE) && (slitMatrix[0].nc>2))
     {
      for (i=0;i<slitTmp.nl;i++)
       slitVector[0][i]=(double)VECTOR_Table2((double **)slitMatrix[0].matrix,slitMatrix[0].nl,slitMatrix[0].nc,slitMatrix[0].matrix[0][i+1],lambda);

      // memcpy(slitLambda[0],(double *)slitMatrix[0].matrix[0]+1,sizeof(double)*(slitMatrix[0].nl-1));
      if (!(rc=SPLINE_Deriv2(slitLambda[0],slitVector[0],slitDeriv2[0],slitNDET[0],"XSCONV_TypeStandard")))
       rc=XSCONV_GetFwhm(slitLambda[0],slitVector[0],slitDeriv2[0],slitNDET[0],SLIT_TYPE_FILE,&fwhm);
     }

    // Interpolate wavelength dependent parameters at value lambda:
    if (wveDptFlag) {

      if ((slitType==SLIT_TYPE_SUPERGAUSS) && (slitLambda[2]!=NULL))
        SPLINE_Vector(slitLambda[2],slitVector[2],slitDeriv2[2],slitNDET[2],&lambda,&slitParam[2],1,SPLINE_CUBIC);
      else
        slitParam[2]=(double)0.;

      if (slitLambda[1]!=NULL) {
        if (slitType!=SLIT_TYPE_FILE)
          SPLINE_Vector(slitLambda[1],slitVector[1],slitDeriv2[1],slitNDET[1],&lambda,&slitParam[1],1,SPLINE_CUBIC);
        else {
          SPLINE_Vector(slitLambda[1],slitMatrix[1].matrix[1],slitMatrix[1].deriv2[1],slitNDET[1],&lambda,&slitStretch1,1,SPLINE_CUBIC);
          if (slitMatrix[1].nc>2)
            SPLINE_Vector(slitLambda[1],slitMatrix[1].matrix[2],slitMatrix[1].deriv2[2],slitNDET[1],&lambda,&slitStretch2,1,SPLINE_CUBIC);
          else
            slitStretch2=slitStretch1;
        }
      }

      if (slitType==SLIT_TYPE_FILE) {
        // Apply slitStretch1 or slitStretch2 depending on whether we
        // are in the "left" or "right" wing of the slit function.  We
        // do this by recalculating the wavelength grid

        const double *lambda_orig = slitMatrix[0].matrix[0];
        const double *slit_col1 = slitMatrix[0].matrix[1];
        if (slitMatrix[0].nc>2) {
          // If we have a matrix of slit functions for different
          // central wavelengths, the matrix[0:ncols][0] contains the
          // central wavelenghts -> shift lambda_orig and slit_col1 by
          // one position:
          lambda_orig += 1;
          slit_col1 += 1;
        }

        // calculate center wavelength, defined as wavelength
        // corresponding to the maximum value
        //
        // We always use slit_col1 here, to match what is done during
        // calibration (KuruczConvolveSolarSpectrum in kurucz.c).
        //
        // TODO: for low-sampled slit functions, lambda of maximum
        // might not be a good measure of the center => perform an
        // interpolation here and in KuruczConvolveSolarSpectrum.
        double lambda_center = 0.;
        double slit_max = 0.;
        for (int i=0; i<slitTmp.nl; ++i) {
          if (slit_col1[i] > slit_max) {
            slit_max = slit_col1[i];
            lambda_center = lambda_orig[i];
          }
        }

        for (i=0;i<slitTmp.nl;i++) {
          // stretch wavelength grid around the center wavelength,
          // using slitStretch1 on the left, and slitStretch2 on the
          // right:
          double delta_lambda = lambda_orig[i] - lambda_center;
          delta_lambda *= (delta_lambda < 0.) ? slitStretch1 : slitStretch2;
          slitLambda[0][i] = lambda_center + delta_lambda;
        }

        // Recalculate second derivatives and the FWHM
        if (!(rc=SPLINE_Deriv2(slitLambda[0],slitVector[0],slitDeriv2[0],slitNDET[0],"XSCONV_TypeStandard ")))
          rc=XSCONV_GetFwhm(slitLambda[0],slitVector[0],slitDeriv2[0],slitNDET[0],SLIT_TYPE_FILE,&slitParam[0]);
      }
      else if (slitLambda[0]!=NULL)
        SPLINE_Vector(slitLambda[0],slitVector[0],slitDeriv2[0],slitNDET[0],&lambda,&slitParam[0],1,SPLINE_CUBIC);
    }

    if (slitType!=SLIT_TYPE_FILE) {
      fwhm=(slitType!=SLIT_TYPE_ERF)?slitParam[0]:sqrt(slitParam[0]*slitParam[0]+slitParam[1]*slitParam[1]);
      stepF=fwhm/(double)NFWHM;            // number of points/FWHM
      slitWidth=(double)0.5*NFWHM*fwhm; // 3.*fwhm; // slitWidth=(double)3.*fwhm;

      lambdaMin=lambda-slitWidth;
      lambdaMax=lambda+slitWidth;
    } else {
      lambdaMin=lambda-max(fabs(slitLambda[0][0]),fabs(slitLambda[0][slitNDET[0]-1]));   // use max between both limits in order to account for asymetric line shapes
      lambdaMax=lambda+max(fabs(slitLambda[0][0]),fabs(slitLambda[0][slitNDET[0]-1]));   // use max between both limits in order to account for asymetric line shapes
    }

    // Search for first pixel in high resolution cross section in the wavelength range delimited by slit function

    for (klo=0,khi=xshrNDET-1;khi-klo>1;) {
      xshrPixMin=(khi+klo)>>1;

      if (xshrLambda[xshrPixMin]>lambdaMin)
        khi=xshrPixMin;
      else
        klo=xshrPixMin;
    }

    xshrPixMin=(xshrLambda[klo]<lambdaMin)?khi:klo;
    crossFIntegral=IFIntegral=FIntegral=(double)0.;

    if (xshrPixMin==xshrNDET-1)
      newF=newXshr=(double)0.;

    // Case 1 : the resolution of cross section is better than the resolution of slit function => slit function interpolation only

    else if (2*stepF-stepXshr>EPSILON)
     {
      // set indexes to browse wavelengths in the grid of the high resolution cross section

      indexOld=xshrPixMin;
      indexNew=indexOld+1;

      // distance to the central wavelength

      dist=(double)slitCenter-(xshrLambda[indexOld]-lambda); // !!! slit function is inversed for convolution

      rc=GetNewF(&newF,slitType,slitLambda[0],slitVector[0],slitDeriv2[0],slitNDET[0],
                  dist,slitParam[0],slitParam[1],slitParam[2],xshrLambda[xshrPixMin+1]-xshrLambda[xshrPixMin]);

      // browse the grid of the high resolution cross section

      while ((indexNew<xshrNDET) && (xshrLambda[indexNew]<=lambdaMax) && !rc)
       {
        oldF=newF;

        dist=(double)slitCenter-(xshrLambda[indexNew]-lambda);

        // Convolution

        if (!(rc=GetNewF(&newF,slitType,slitLambda[0],slitVector[0],slitDeriv2[0],slitNDET[0],
                          dist,slitParam[0],slitParam[1],slitParam[2],xshrLambda[indexNew]-xshrLambda[indexOld])))
         {
          h=(xshrLambda[indexNew]-xshrLambda[indexOld])*0.5;  // use trapezium formula for surface computation (B+b)*H/2
          crossFIntegral+=(xshrVector[indexOld]*oldF+xshrVector[indexNew]*newF)*h;
          IFIntegral+=(IVector[indexOld]*oldF+IVector[indexNew]*newF)*h;
          FIntegral+=(oldF+newF)*h;
         }

        indexOld=indexNew++;
       }

     }

    // Case 2 : the resolution of slit function is better than the resolution of cross section => cross section interpolation

    else
     {
      // set indexes to browse wavelengths in the grid of the slit function if pre-calculated

      indexOld=0;
      indexNew=1;
      rc=0;

      // Calculate first value for the slit function

      if (slitType==SLIT_TYPE_FILE)
       {
        dist=lambda-slitLambda[0][indexOld];           // !!! Hilke : - -> +
        oldF=slitVector[0][indexOld];
       }
      else
       {
        dist=lambda-slitWidth;

        if ((rc=GetNewF(&oldF,slitType,slitLambda[0],slitVector[0],slitDeriv2[0],slitNDET[0],
                        dist-lambda,slitParam[0],slitParam[1],slitParam[2],stepF))!=ERROR_ID_NO)

         goto EndTypeStandard;
       }

      // Calculate first value for the high resolution cross section

      oldF=oldIF=oldXshr=(double)0.;

      if ((dist<xshrLambda[0]) || (dist>xshrLambda[xshrNDET-1]))
       oldF=oldIF=oldXshr=(double)0.;
      else if (!(rc=SPLINE_Vector(xshrLambda,xshrVector,xshrDeriv2,xshrNDET,&dist,&oldXshr,1,SPLINE_CUBIC)) && (Ic!=NULL))
       rc=SPLINE_Vector(xshrLambda,IVector,IDeriv2,xshrNDET,&dist,&oldIF,1,SPLINE_CUBIC);

      // browse the grid of the slit function

      while ((((slitType==SLIT_TYPE_FILE) && (indexNew<slitNDET[0])) ||
              ((slitType!=SLIT_TYPE_FILE) && (dist+stepF<=lambda+slitWidth))) && !rc)
       {
        // the slit function is pre-calculated

        if (slitType==SLIT_TYPE_FILE)
         {
          dist=lambda-slitLambda[0][indexNew];                                     // !!! Hilke : - -> +
          newF=slitVector[0][indexNew];

          h=fabs((slitLambda[0][indexNew]-slitLambda[0][indexOld])*0.5);
          indexOld=indexNew++;
         }

        // the slit function is calculated now

        else
         {
          dist+=stepF;
          h=stepF*0.5;

          if ((rc=GetNewF(&newF,slitType,slitLambda[0],slitVector[0],slitDeriv2[0],slitNDET[0],
                           dist-lambda,slitParam[0],slitParam[1],slitParam[2],stepF))!=ERROR_ID_NO)

           goto EndTypeStandard;

         }

        if ((dist>=xshrLambda[0]) && (dist<=xshrLambda[xshrNDET-1])) {
          // interpolation of the high resolution cross section

          SPLINE_Vector(xshrLambda,xshrVector,xshrDeriv2,xshrNDET,&dist,&newXshr,1,SPLINE_CUBIC);

          // Convolution

          crossFIntegral+=(oldF*oldXshr+newF*newXshr)*h;
          FIntegral+=(oldF+newF)*h;

          // I0 correction

          if ((Ic!=NULL) && !(rc=SPLINE_Vector(xshrLambda,IVector,IDeriv2,xshrNDET,&dist,&newIF,1,SPLINE_CUBIC)))
           {
            IFIntegral+=(oldF*oldIF+newF*newIF)*h;
            oldIF=newIF;
           }
         }
        else
         newF=newXshr=(double)0.;

        oldXshr=newXshr;
        oldF=newF;
       }
     }

    xsnewVector[xsnewIndex]=(FIntegral!=(double)0.)?crossFIntegral/FIntegral:(double)0.;

    if (Ic!=NULL)
     Ic[xsnewIndex]=(FIntegral!=(double)0.)?IFIntegral/FIntegral:(double)0.;
   }

  EndTypeStandard :

  MATRIX_Free(&slitTmp,"XSCONV_TypeStandard");

  // Return

  return rc;
 }

// -------------------------------------------------------------------------
// XsconvTypeI0Correction : Convolution of cross sections with I0 correction
// -------------------------------------------------------------------------

RC XSCONV_TypeI0Correction(MATRIX_OBJECT *pXsnew,MATRIX_OBJECT *pXshr,MATRIX_OBJECT *pI0,double conc,int slitType,MATRIX_OBJECT *slitMatrix,double *slitParam,int wveDptFlag)
 {
  // Declarations

  double  sigma,
         *xshrLambda,*xshrVector,*xshrDeriv2,
         *xsnewLambda,*xsnewVector,
         *ILambda,*IVector,*IDeriv2,
         *I0Lambda,*I0Vector,*I0cVector,*IcVector;

  int INDET,xshrNDET,xsnewNDET;
  INDEX i;
  MATRIX_OBJECT I,I0c;
  RC rc;

  // Use substitution variables

  I0Lambda=pI0->matrix[0];
  I0Vector=pI0->matrix[1];

  xshrLambda=pXshr->matrix[0];
  xshrVector=pXshr->matrix[1];
  xshrDeriv2=pXshr->deriv2[1];

  xsnewLambda=pXsnew->matrix[0];
  xsnewVector=pXsnew->matrix[1];

  xshrNDET=pXshr->nl;
  xsnewNDET=pXsnew->nl;

  // Initializations

  memset(&I,0,sizeof(MATRIX_OBJECT));
  memset(&I0c,0,sizeof(MATRIX_OBJECT));

  INDET=pI0->nl;
  IcVector=NULL;
  rc=ERROR_ID_NO;

  if (conc<=(double)0.)

   rc=ERROR_SetLast("XSCONV_TypeI0Correction",ERROR_TYPE_FATAL,ERROR_ID_BAD_ARGUMENTS,"General tab page : Conc. <= 0.");

  // Buffers allocation

  else if (MATRIX_Allocate(&I,INDET,2,0,0,1,"XSCONV_TypeI0Correction") ||
           MATRIX_Allocate(&I0c,xsnewNDET,2,0,0,1,"XSCONV_TypeI0Correction") ||
         ((IcVector=(double *)MEMORY_AllocDVector("XSCONV_TypeI0Correction ","IcVector",0,xsnewNDET-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    memcpy(I0c.matrix[0],xsnewLambda,sizeof(double)*xsnewNDET);

    // Use substitution variables

    ILambda=I.matrix[0];
    IVector=I.matrix[1];
    IDeriv2=I.deriv2[1];

    I0cVector=I0c.matrix[1];

    // Build I from I0 (solar spectrum) with the specified cross section absorption

    memcpy(ILambda,I0Lambda,sizeof(double)*INDET);

    VECTOR_Init(I0cVector,(double)0.,sizeof(double));
    VECTOR_Init(IcVector,(double)0.,sizeof(double));

    for (i=0;(i<INDET) && !rc;i++) {
      SPLINE_Vector(xshrLambda,xshrVector,xshrDeriv2,xshrNDET,&I0Lambda[i],&sigma,1,SPLINE_CUBIC);
      if (-sigma*conc>(double)700.)
        rc=ERROR_SetLast("XSCONV_TypeI0Correction",ERROR_TYPE_FATAL,ERROR_ID_OVERFLOW);
      else
        IVector[i]=(double)I0Vector[i]*exp(-sigma*conc);
    }

    // I and I0 convolution

    if (!rc &&
        !(rc=SPLINE_Deriv2(ILambda,IVector,IDeriv2,INDET,"XSCONV_TypeI0Correction ")) &&               // I second derivatives calculation
        !(rc=XSCONV_TypeStandard(&I0c,0,xsnewNDET,pI0,&I,IcVector,slitType,slitMatrix,slitParam,wveDptFlag)))    // I0 and I convolution
     {
      // Cross section convolution

      for (i=0;(i<xsnewNDET) && !rc;i++)
       {
        if ((I0cVector[i]<=(double)0.) || (IcVector[i]<=(double)0.) || (conc<=(double)0.))
         xsnewVector[i]=(double)0.;
//         THRD_Error(ERROR_TYPE_FATAL,(rc=ERROR_ID_LOG),"XSCONV_TypeI0Correction ");
        else
         xsnewVector[i]=(double)log(I0cVector[i]/IcVector[i])/conc;
       }
     }
   }

  // Release allocated buffers

  MATRIX_Free(&I,"XSCONV_TypeI0Correction");
  MATRIX_Free(&I0c,"XSCONV_TypeI0Correction");

  if (IcVector!=NULL)
   MEMORY_ReleaseDVector("XSCONV_TypeI0Correction ","IcVector",IcVector,0);

  // Return

  return rc;
 }

// ---------------------------------------------------------------------------------
// XsconvRebuildSlitFunction : Rebuild slit function onto a regular wavelength scale
// ---------------------------------------------------------------------------------

RC XsconvRebuildSlitFunction(double *lambda,double *slit,int nslit,SLIT *pSlit,MATRIX_OBJECT *pSlitXs)
 {
  // Declarations

  double slitParam,n,sigma2,delta,a,a2;                                         // use sigma2=fwhm/2 because in the S/W user manual sigma=fwhm
  INDEX i;
  RC rc;

  // Initialization

  sigma2=pSlit->slitParam*0.5;
  a=((pSlit->slitType!=SLIT_TYPE_SUPERGAUSS) || (fabs(pSlit->slitParam2)<EPSILON))?sigma2/sqrt(log(2.)):sigma2/pow(log(2.),(double)1./pSlit->slitParam2);
  // a=sigma2/sqrt(log(2.));
  rc=ERROR_ID_NO;

  // Build deconvolution slit function

  if (pSlit->slitType==SLIT_TYPE_GAUSS)
   for (i=0,slitParam=pSlit->slitParam;i<nslit;i++)
    slit[i]=(double)exp(-4.*log(2.)*(lambda[i]*lambda[i])/(slitParam*slitParam));

  else if (pSlit->slitType==SLIT_TYPE_INVPOLY)
   for (i=0,sigma2=pSlit->slitParam*0.5,n=(double)pSlit->slitParam2;i<nslit;i++)
    slit[i]=(double)pow(sigma2,n)/(pow(lambda[i],n)+pow(sigma2,n));

  else if (pSlit->slitType==SLIT_TYPE_ERF)
   for (i=0,sigma2=pSlit->slitParam*0.5,a=(double)sigma2/sqrt(log(2.)),delta=(double)pSlit->slitParam2*0.5;i<nslit;i++)
    slit[i]=(double)(ERF_GetValue((lambda[i]+delta)/a)-ERF_GetValue((lambda[i]-delta)/a))/(4.*delta);

  else if (pSlit->slitType==SLIT_TYPE_SUPERGAUSS)
   for (i=0,slitParam=pSlit->slitParam;i<nslit;i++)
    slit[i]=(double)exp(-pow(fabs(lambda[i]/a),pSlit->slitParam2));           // normalization ????


  else if (pSlit->slitType==SLIT_TYPE_AGAUSS)
    for (i=0;i<nslit;i++)
     {
         a2=(lambda[i]<(double)0.)?a*(1.-pSlit->slitParam2):a*(1+pSlit->slitParam2);
      slit[i]=(double)exp(-4.*log(2.)*(lambda[i]*lambda[i])/(a2*a2));                                                 // normalization -4*log(2.) is correct ????
     }

  else // slit type == SLIT_TYPE_FILE
   rc=SPLINE_Vector(pSlitXs->matrix[0],pSlitXs->matrix[1],pSlitXs->deriv2[1],pSlitXs->nl,lambda,slit,nslit,SPLINE_CUBIC);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------------------------
// XsconvPowFFTMin : return index of the first minimum found in the power spectrum obtained by FFT
// -----------------------------------------------------------------------------------------------

INDEX XsconvPowFFTMin(double *fft,int n2)
 {
  // Declarations

  double minold,minnew;
  INDEX i,ndemi;

  // Initialization

  minold=(double)fft[1]*fft[1];  // the lowest frequency (no imaginary part)

  // Search for the minimum of the power spectrum

  for (i=2,ndemi=n2/2;i<=ndemi;i++,minold=minnew)

   if ((minnew=fft[(i<<1) /* i*2 */-1]*fft[(i<<1) /* i*2 */-1]+        // real part
               fft[(i<<1) /* i*2 */]*fft[(i<<1) /* i*2 */])>=minold)   // imaginary part

    break;

  if ((--i==ndemi) && (fft[2]*fft[2]<minold))
   i++;

  // Return

  return i;
 }

// ----------------------------------------
// XsconvFreqFilter : Frequencies filtering
// ----------------------------------------

double XsconvFreqFilter(double freq,double fc,double bp)
 {
  double f;

  if (freq<fc-bp)
   f=(double)1.;
  else if (freq<=fc+bp)
   f=(double)0.5*(1.+cos(PIDEMI*(1+(freq-fc)/bp)));
  else
   f=(double)0.;

  return f;
 }

// ------------------------------------------------------------------------------------------------------
// XSCONV_NewSlitFunction : Replace slit function by a new one when a deconvolution slit function is given
// ------------------------------------------------------------------------------------------------------

RC XSCONV_NewSlitFunction(SLIT *pSlitOptions,MATRIX_OBJECT *pSlit,double slitParam,SLIT *pSlit2Options,MATRIX_OBJECT *pSlit2,double slitParam2)
 {
  // Declarations

  double *slit,*slit2,*newSlit,*lambda,              // substitution vectors
         *slitFFTin,*slit2FFTin,                     // FFT input vectors
         *slitFFTout,*slit2FFTout,                   // FFT output vectors
          slitStep,fc,bp,
          norm,norm2,                                // normalization
          a,b,c,d,div;                               // substitution variables for complex calculation
  INDEX i,i0;
  int n2,ndemi,nslit,nslitOld;
  RC rc;

  // Initializations

  slit=slit2=newSlit=slitFFTin=slit2FFTin=slitFFTout=slit2FFTout=NULL;
  rc=ERROR_ID_NO;

  // With the slit function width half way up, give a preliminary sampling

  slitStep=slitParam2*0.2;                                // force a sampling about five pixels at half way up
  nslit=(int)floor(slitParam*4./slitStep);                // convert the step from wavelength to a number of pixels
  nslit=(nslit%2!=0)?nslit:nslit+1;                        // force the number of points to be odd

  // Buffers allocation

  if (((lambda=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","lambda",0,nslit))==NULL) ||
      ((slit=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slit",0,nslit))==NULL) ||
      ((slit2=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slit2",0,nslit))==NULL) ||
      ((newSlit=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","newSlit",0,nslit))==NULL) ||
      ((slitFFTin=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slitFFTin",1,
              (n2=(int)pow((double)2.,ceil(log((double)nslit)/log((double)2.))))))==NULL) ||

      ((slit2FFTin=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slit2FFTin",1,n2))==NULL) ||
      ((slitFFTout=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slitFFTout",1,n2))==NULL) ||
      ((slit2FFTout=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction ","slit2FFTout",1,n2))==NULL))

   rc=ERROR_ID_ALLOC;

  // Both convolution and deconvolution slit functions are gaussian functions

  else if (((pSlit2Options->slitType==SLIT_TYPE_GAUSS) && (pSlitOptions->slitType==SLIT_TYPE_GAUSS)) ||
           ((pSlit2Options->slitType==SLIT_TYPE_ERF) && (pSlitOptions->slitType==SLIT_TYPE_ERF) && (pSlit2Options->slitParam2==pSlitOptions->slitParam2)))
   {
    if (slitParam2<slitParam)

     for (i=0,i0=nslit>>1,slitParam=slitParam*slitParam-slitParam2*slitParam2;i<nslit;i++)
      {
       lambda[i]=(double)slitStep*(i-i0);
       newSlit[i]=(double)exp(-4.*log(2.)*(lambda[i]*lambda[i])/slitParam);
      }
    else
     {
         rc=ERROR_SetLast("XSCONV_NewSlitFunction",ERROR_TYPE_FATAL,ERROR_ID_GAUSSIAN,slitParam,slitParam2);
      goto EndNewSlit;
     }
   }
  else
   {
    // ==========
    // First pass
    // ==========

    // Resample deconvolution slit function onto the regular stepped wavelength scale

    for (i=0,i0=nslit>>1;i<nslit;i++)
     lambda[i]=(double)slitStep*(i-i0);

    if ((rc=XsconvRebuildSlitFunction(lambda,slit2,nslit,pSlit2Options,pSlit2))!=0)
     goto EndNewSlit;

    // Vector normalization

    VECTOR_Init(slit2FFTin+1,(double)0.,n2);

    for (i=1,norm2=(double)0.;i<nslit;i++)
     norm2+=(slit2[i]+slit2[i-1]);

    if (norm2==(double)0.)
     {
         rc=ERROR_SetLast("XSCONV_NewSlitFunction",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"calculation of a norm");
      goto EndNewSlit;
     }

    norm2=(double)1./(norm2*0.5*slitStep);
    ndemi=nslit/2;

    // Split deconvolution function for FFT calculation

    slit2FFTin[1]=slit2[ndemi]*norm2;

    for (i=ndemi+1;i<nslit;i++)
     {
      slit2FFTin[i-ndemi+1]=slit2[i]*norm2;
      slit2FFTin[n2-(i-ndemi)+1]=slit2[nslit-i-1]*norm2;
     }

    // FFT calculation of deconvolution slit function

    realft(slit2FFTin,slit2FFTout,n2,1);

    // Search for the first minimum of power spectrum

    if (((i=XsconvPowFFTMin(slit2FFTout,n2))<=ndemi) && (i>1))
     {
      nslitOld=nslit;
      slitStep=(double)ceil(100*(ndemi*slitStep)/(i-1))*0.01;

      nslit=(int)floor(slitParam*4./slitStep);                 // convert the step from wavelength to a number of pixels
      nslit=(nslit%2!=0)?nslit:nslit+1;                        // force the number of points to be odd

      if (nslit>nslitOld)
       {
        // Release allocated buffers

        if (lambda!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","lambda",lambda,0);
        if (slit!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit",slit,0);
        if (slit2!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2",slit2,0);
        if (newSlit!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","newSlit",newSlit,0);
        if (slitFFTin!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slitFFTin",slitFFTin,1);
        if (slit2FFTin!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2FFTin",slit2FFTin,1);
        if (slitFFTout!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slitFFTout",slitFFTout,1);
        if (slit2FFTout!=NULL)
         MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2FFTout",slit2FFTout,1);

        slit=slit2=newSlit=slitFFTin=slit2FFTin=slitFFTout=slit2FFTout=NULL;

        // Allocate buffers with the new size

        if (((lambda=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","lambda",0,nslit))==NULL) ||
            ((slit=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slit",0,nslit))==NULL) ||
            ((slit2=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slit2",0,nslit))==NULL) ||
            ((newSlit=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","newSlit",0,nslit))==NULL) ||
            ((slitFFTin=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slitFFTin",1,
                    (n2=(int)pow((double)2.,ceil(log((double)nslit)/log((double)2.))))))==NULL) ||

            ((slit2FFTin=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slit2FFTin",1,n2))==NULL) ||
            ((slitFFTout=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slitFFTout",1,n2))==NULL) ||
            ((slit2FFTout=(double *)MEMORY_AllocDVector("XSCONV_NewSlitFunction (2) ","slit2FFTout",1,n2))==NULL))
         {
          rc=ERROR_ID_ALLOC;
          goto EndNewSlit;
         }
       }
     }

    // ===========
    // Second pass
    // ===========

    // Resample convolution and deconvolution slit functions onto the regular stepped wavelength scale

    for (i=0,i0=nslit>>1;i<nslit;i++)
     lambda[i]=(double)slitStep*(i-i0);

    // Build deconvolution slit function

    if (((rc=XsconvRebuildSlitFunction(lambda,slit,nslit,pSlitOptions,pSlit))!=0) ||
        ((rc=XsconvRebuildSlitFunction(lambda,slit2,nslit,pSlit2Options,pSlit2))!=0))

     goto EndNewSlit;

    // Vectors initializations

    VECTOR_Init(slitFFTin+1,(double)0.,n2);
    VECTOR_Init(slit2FFTin+1,(double)0.,n2);

    // Normalization factors calculation

    for (i=1,norm=norm2=(double)0.;i<nslit;i++)
     {
      norm+=slit[i]+slit[i-1];
      norm2+=slit2[i]+slit2[i-1];
     }

    if ((norm==(double)0.) || (norm2==(double)0.))
     {
         rc=ERROR_SetLast("XSCONV_NewSlitFunction",ERROR_TYPE_WARNING,ERROR_ID_DIVISION_BY_0,"calculation of a norm");
      goto EndNewSlit;
     }

    norm=(double)1./(norm*0.5*slitStep);
    norm2=(double)1./(norm2*0.5*slitStep);

    ndemi=nslit/2;

    slitFFTin[1]=slit[ndemi]*norm;
    slit2FFTin[1]=slit2[ndemi]*norm2;

    for (i=ndemi+1;i<nslit;i++)
     {
      slitFFTin[i-ndemi+1]=slit[i]*norm;
      slitFFTin[n2-(i-ndemi)+1]=slit[nslit-i-1]*norm;
      slit2FFTin[i-ndemi+1]=slit2[i]*norm2;
      slit2FFTin[n2-(i-ndemi)+1]=slit2[nslit-i-1]*norm2;
     }

    // FFT calculations of slit functions

    realft(slitFFTin,slitFFTout,n2,1);
    realft(slit2FFTin,slit2FFTout,n2,1);

    // Convolution slit function filtering

    if (((i=XsconvPowFFTMin(slitFFTout,n2))<=(ndemi=n2/2)) && (i>1))
     {
      fc=bp=(double)(i-1)/(n2*slitStep);

      fc*=(double)0.8;
      bp*=(double)0.2;

      slitFFTout[1]*=XsconvFreqFilter((double)1./(n2*slitStep),fc,bp);
      slitFFTout[2]*=XsconvFreqFilter((double)(ndemi+1.)/(n2*slitStep),fc,bp);

      for (i=2;i<=ndemi;i++)
       {
        slitFFTout[(i<<1)-1]*=XsconvFreqFilter((double)(i-1)/(n2*slitStep),fc,bp);   // real part
        slitFFTout[(i<<1)]*=XsconvFreqFilter((double)(i-1)/(n2*slitStep),fc,bp);     // imaginary part
       }
     }

    // Complex division slitFFTout/slit2FFTout

    slitFFTout[1]/=(double)slit2FFTout[1];   // the lowest frequency (no imaginary part)
    slitFFTout[2]/=(double)slit2FFTout[2];   // the highest frequency (no imaginary part)

    for (i=2;i<=ndemi;i++)
     {
      // Use substitution variables

      a=slitFFTout[(i<<1) /* i*2 */-1];   // real part
      b=slitFFTout[(i<<1) /* i*2 */];     // imaginary part
      c=slit2FFTout[(i<<1) /* i*2 */-1];  // real part
      d=slit2FFTout[(i<<1) /* i*2 */];    // imaginary part

      // Perform (a+bi)/(c+di)=(a+bi)*(c-di)/(c2+d2)=(ac+bd)/(c2+d2)+i*(bc-ad)/(c2+d2)

      if ((div=c*c+d*d)!=(double)0.)
       {
        slitFFTout[(i<<1) /* i*2 */-1]=(a*c+b*d)/div;
        slitFFTout[(i<<1) /* i*2 */]=(b*c-a*d)/div;
       }
     }

    // FFT inverse calculation for the effective slit function

    realft(slitFFTout,slitFFTin,n2,-1);

    // Rebuild the new slit function

    newSlit[(ndemi=nslit/2)]=slitFFTin[1];

    for (i=ndemi+1;i<nslit;i++)
     {
      newSlit[i]=slitFFTin[i-ndemi+1];
      newSlit[nslit-i-1]=slitFFTin[n2-(i-ndemi-1)];
     }
   }

  // Release previous buffers

  MATRIX_Free(pSlit,"XSCONV_NewSlitFunction");

  // Allocate

  if (MATRIX_Allocate(pSlit,nslit,2,0,0,1,"XSCONV_NewSlitFunction"))
   rc=ERROR_ID_ALLOC;
  else
   {
    memcpy(pSlit->matrix[0],lambda,sizeof(double)*nslit);
    memcpy(pSlit->matrix[1],newSlit,sizeof(double)*nslit);

    rc=SPLINE_Deriv2(pSlit->matrix[0],pSlit->matrix[1],pSlit->deriv2[1],nslit,"XSCONV_NewSlitFunction ");
   }

  EndNewSlit :

  // Release allocated buffers

  if (lambda!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","lambda",lambda,0);
  if (slit!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit",slit,0);
  if (slit2!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2",slit2,0);
  if (newSlit!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","newSlit",newSlit,0);
  if (slitFFTin!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slitFFTin",slitFFTin,1);
  if (slit2FFTin!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2FFTin",slit2FFTin,1);
  if (slitFFTout!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slitFFTout",slitFFTout,1);
  if (slit2FFTout!=NULL)
   MEMORY_ReleaseDVector("XSCONV_NewSlitFunction ","slit2FFTout",slit2FFTout,1);

  // Return

  return rc;
 }
