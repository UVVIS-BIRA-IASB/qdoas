
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  UNDERSAMPLING CORRECTION
//  Name of module    :  USAMP.C
//  Compiler          :  MinGW (GNU compiler)
//  Creation date     :  February 1999
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
//  FUNCTIONS
//
//  ============================
//  UNDERSAMPLING CROSS SECTIONS
//  ============================
//
//  USAMP_BuildCrossSections - build undersampling cross sections from preconvoluted high resolution Kurucz spectrum;
//  USAMP_Build - build undersampling cross sections (main function);
//  UsampWriteHeader - write options in the file header
//  UsampBuildFromTools - build undersampling cross sections from the tools dialog box
//
// -----------------------------------------------------------------------------
//
//  REFERENCE
//
//  Kelly Chance, from Harvard Smithsonian Institute
//  ----------------------------------------------------------------------------

#include <string.h>
#include <math.h>

#include "usamp.h"

#include "engine_context.h"
#include "doas.h"
#include "vector.h"
#include "matrix.h"
#include "spline.h"
#include "xsconv.h"

// =====================
// CONSTANTS DEFINITIONS
// =====================

#define USAMP_SECTION "Undersampling"
#define USAMP_METHOD_MAX 2

// =========
// FUNCTIONS
// =========

// -----------------------------------------------------------------------------
// FUNCTION      USAMP_BuildCrossSections
// -----------------------------------------------------------------------------
// PURPOSE       build undersampling cross sections from preconvoluted high resolution Kurucz spectrum
//
// INPUT/OUTPUT  see below
//
// PRECONDITIONS y2a second derivatives vector should be beforehand computed with SPLINE_deriv2 for cubic interpolation;
//
// RETURN CODE   ERROR_ID_NO            if no error;
//               ERROR_ID_ALLOC         if the allocation of a buffer failed
//               ERROR_ID_BAD_ARGUMENTS if at least one argument is bad;
//               ERROR_ID_SPLINE        if non increasing abscissa found in xa vector;
// -----------------------------------------------------------------------------

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
                            int     analysisMethod)                             // analysis method
 {
  // Declarations

  double *resample,*d2res,over,under;
  INDEX i,indexMin,indexMax;
  RC rc;

  // Initializations

  d2res=resample=NULL;
  rc=ERROR_ID_NO;

  for (indexMin=0;indexMin<nGome;indexMin++)
   if (gomeLambda2[indexMin]>(double)1.e-3)
    break;

  for (indexMax=nGome-1;indexMax>=0;indexMax--)
   if (gomeLambda2[indexMax]>(double)1.e-3)
    break;

  // Buffers allocation

  if ((nGome<=0) ||
      ((resample=MEMORY_AllocDVector("USAMP_BuildCrossSections ","resample",0,nGome-1))==NULL) ||
      ((d2res=MEMORY_AllocDVector("USAMP_BuildCrossSections ","d2res",0,nGome-1))==NULL))

   rc=ERROR_ID_ALLOC;

  else
   {
    // Phase 1 : calculate solar spectrum at GOME+phase positions, original and resampled

    if (phase1!=NULL)

     for (i=1,phase1[0]=(double)0.;(i<nGome) && !rc;i++)
      {
       if (gomeLambda2[i]<=(double)1.e-3)
        phase1[i]=(double)0.;
       else if (!(rc=SPLINE_Vector(kuruczLambda,kuruczConvolved,kuruczConvolvedDeriv2,nKurucz,&gomeLambda2[i],&over,1,SPLINE_CUBIC)) &&
                !(rc=SPLINE_Vector(gomeLambda,kuruczInterpolated,kuruczInterpolatedDeriv2,nGome,&gomeLambda2[i],&under,1,SPLINE_CUBIC)))

        phase1[i]=(analysisMethod==OPTICAL_DENSITY_FIT)?log(over/under):over-under;
      }

    // Phase 2 : calculate solar spectrum at GOME+phase positions

    if ((phase2!=NULL) &&
       !(rc=SPLINE_Vector(kuruczLambda,kuruczConvolved,kuruczConvolvedDeriv2,nKurucz,gomeLambda2,resample,nGome,SPLINE_CUBIC)) && // calculate solar spectrum at GOME positions
       !(rc=SPLINE_Deriv2(&gomeLambda2[indexMin],&resample[indexMin],&d2res[indexMin],(indexMax-indexMin+1),"USAMP_Build (resample 2) ")))

     for (i=0;(i<nGome) && !rc;i++)
      {
       over=kuruczInterpolated[i];

       if (gomeLambda2[i]<(double)1.e-3)
        phase2[i]=(double)0.;
       else if (!(rc=SPLINE_Vector(&gomeLambda2[indexMin],&resample[indexMin],&d2res[indexMin],(indexMax-indexMin+1),&gomeLambda[i],&under,1,SPLINE_CUBIC)))
        phase2[i]=(analysisMethod==OPTICAL_DENSITY_FIT)?log(over/under):over-under;
      }
   }

  // Buffers release

  if (resample!=NULL)
   MEMORY_ReleaseDVector("USAMP_BuildCrossSections ","resample",resample,0);
  if (d2res!=NULL)
   MEMORY_ReleaseDVector("USAMP_BuildCrossSections ","d2res",d2res,0);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      USAMP_Build
// -----------------------------------------------------------------------------
// PURPOSE       build undersampling cross sections (main function)
//
// INPUT/OUTPUT  see below
//
// RETURN CODE   ERROR_ID_NO            if no error;
//               ERROR_ID_ALLOC         if the allocation of a buffer failed
//               ERROR_ID_BAD_ARGUMENTS if at least one argument is bad;
//               ERROR_ID_SPLINE        if non increasing abscissa found in xa vector;
// -----------------------------------------------------------------------------

RC USAMP_Build(double *phase1,                                                  // OUTPUT : phase 1 calculation
               double *phase2,                                                  // OUTPUT : phase 2 calculation
               double *gomeLambda,                                              // GOME calibration
               int     nGome,                                                   // size of GOME calibration
               MATRIX_OBJECT *pKuruczMatrix,                                    // Kurucz matrix
               SLIT   *pSlit,                                                   // slit function
               double  fraction,                                                // tunes the phase
               int     analysisMethod)                                          // analysis method
 {
  // Declarations

  MATRIX_OBJECT xsnew,slitMatrix[NSFP];
  double *gomeLambda2,*kuruczLambda,slitParam[NSFP],
         *resample,*d2res;
  int     slitType,nKurucz;
  INDEX   i;
  RC      rc;

  // Initializations

  gomeLambda2=resample=d2res=NULL;
  memset(&xsnew,0,sizeof(MATRIX_OBJECT));
  memset(&slitMatrix,0,sizeof(MATRIX_OBJECT)*NSFP);
  slitType=pSlit->slitType;
  kuruczLambda=pKuruczMatrix->matrix[0];
  nKurucz=pKuruczMatrix->nl;

  slitParam[0]=pSlit->slitParam;
  slitParam[1]=pSlit->slitParam2;
  slitParam[2]=pSlit->slitParam3;
  slitParam[3]=pSlit->slitParam4;

  // Buffers allocation

  if (((gomeLambda2=MEMORY_AllocDVector("USAMP_Build ","gomeLambda2",0,nGome))==NULL) ||
      ((resample=MEMORY_AllocDVector("USAMP_Build ","resample",0,nGome))==NULL) ||
      ((d2res=MEMORY_AllocDVector("USAMP_Build ","d2res",0,nGome))==NULL) ||
      ((rc=MATRIX_Allocate(&xsnew,nKurucz,2,0,0,1,"USAMP_Build (xsnew)"))!=0))

   rc=ERROR_ID_ALLOC;

  else
   {
    memcpy(xsnew.matrix[0],kuruczLambda,nKurucz*sizeof(double));

    VECTOR_Init(xsnew.matrix[1],(double)0.,nKurucz);

    for (i=1,gomeLambda2[0]=gomeLambda[0]-fraction;i<nGome;i++)
     gomeLambda2[i]=gomeLambda[i]-fraction;
//     gomeLambda2[i]=(double)(1.-fraction)*gomeLambda[i-1]+fraction*gomeLambda[i];

    // Kurucz spectrum convolution with a gaussian

    if (!(rc=XSCONV_LoadSlitFunction(slitMatrix,pSlit,&slitParam[0],&slitType)) &&
//        !(rc=XSCONV_TypeStandardFFT(&usampFFT,slitType,pSlit->slitParam,pSlit->slitParam2,kuruczLambda,kuruczConvolved,nKurucz)) &&
        !(rc=XSCONV_TypeStandard(&xsnew,0,nKurucz,pKuruczMatrix,pKuruczMatrix,NULL,slitType,slitMatrix,slitParam,pSlit->slitWveDptFlag)) &&
        !(rc=SPLINE_Deriv2(kuruczLambda,xsnew.matrix[1],xsnew.deriv2[1],nKurucz,"USAMP_Build (kuruczConvolved) ")) &&
        !(rc=SPLINE_Vector(kuruczLambda,xsnew.matrix[1],xsnew.deriv2[1],nKurucz,gomeLambda,resample,nGome,SPLINE_CUBIC)) && // calculate solar spectrum at GOME positions
        !(rc=SPLINE_Deriv2(gomeLambda,resample,d2res,nGome,"USAMP_Build (resample 1) ")))

      rc=USAMP_BuildCrossSections(phase1,                                        // OUTPUT : phase 1 calculation
                                 phase2,                                        // OUTPUT : phase 2 calculation
                                 gomeLambda,                                    // GOME calibration
                                 gomeLambda2,                                   // shifted GOME calibration
                                 resample,                                      // preconvoluted Kurucz spectrum interpolated on gome calibration
                                 d2res,                                         // interpolated Kurucz spectrum second derivatives
                                 nGome,                                         // size of GOME calibration
                                 kuruczLambda,                                  // Kurucz high resolution wavelength scale
                                 xsnew.matrix[1],                               // preconvoluted Kurucz spectrum on high resolution wavelength scale
                                 xsnew.deriv2[1],                               // preconvoluted Kurucz second derivatives
                                 nKurucz,                                       // size of Kurucz vectors
                                 analysisMethod);                               // analysis method

  }

  // Buffers release

  for (i=0;i<NSFP;i++)
   MATRIX_Free(&slitMatrix[i],"USAMP_Build (slitFunction)");
  MATRIX_Free(&xsnew,"USAMP_Build (xsnew)");

  if (gomeLambda2!=NULL)
   MEMORY_ReleaseDVector("USAMP_Build ","gomeLambda2",gomeLambda2,0);

  if (resample!=NULL)
   MEMORY_ReleaseDVector("USAMP_Build ","resample",resample,0);
  if (d2res!=NULL)
   MEMORY_ReleaseDVector("USAMP_Build ","d2res",d2res,0);

  // Return

  return rc;
 }

