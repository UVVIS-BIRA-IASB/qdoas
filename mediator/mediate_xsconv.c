
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
#include <math.h>
#include <time.h>

#include "../mediator/mediate_response.h"
#include "../mediator/mediate_types.h"
#include "../mediator/mediate_xsconv.h"
#include "mediate_common.h"

#include "xsconv.h"
#include "filter.h"
#include "vector.h"
#include "spline.h"
#include "raman.h"
#include "usamp.h"

// ================
// STATIC VARIABLES
// ================

static const char *mediateConvolutionTypesStr[CONVOLUTION_TYPE_MAX]=
 {
  "Interpolation only",
  "Standard convolution",
  "Convolution with I0 correction"
 };

static const char *mediateConvolutionFileExt[CONVOLUTION_TYPE_MAX]=
   {
    "_none",                                                                     // CONVOLUTION_TYPE_NONE
    "_std",                                                                       // CONVOLUTION_TYPE_STANDARD
    "_i0",                                                                        // CONVOLUTION_TYPE_I0_CORRECTION
  //  "_ring"                         // CONVOLUTION_TYPE_RING
   };

const char *mediateConvolutionFilterTypes[PRJCT_FILTER_TYPE_MAX]={"None","Kaiser","Boxcar","Gaussian","Triangular","Savitzky-Golay","Odd-even pixels correction","Binomial"};
const char *mediateUsampAnalysisMethod[PRJCT_ANLYS_METHOD_MAX]={"Optical density","Intensity fitting"};

FFT    usampFFT;

// ----------------------------------------------------------
// mediateConvolutionSave : Save the convoluted cross section
// ----------------------------------------------------------

RC mediateConvolutionSave(void *engineContext)
 {
  // Declarations

  ENGINE_XSCONV_CONTEXT *pEngineContext=(ENGINE_XSCONV_CONTEXT*)engineContext;
  char fileName[MAX_ITEM_TEXT_LEN];
  PRJCT_FILTER *pLFilter,*pHFilter;
  SZ_LEN fileNameLength;
  char *ptr,*ptr2;
  FILE *fp;
  INDEX i,slitType;
  MATRIX_OBJECT *pXs;
  int nextraPixels;
  double *filterVector;
  int nsize;
  RC rc;

  pXs=&pEngineContext->xsNew;
  nextraPixels=pEngineContext->nFilter;
  filterVector=pEngineContext->filterVector;

  pLFilter=&pEngineContext->lfilter;
  pHFilter=&pEngineContext->hfilter;
  nsize=pXs->nl-nextraPixels;

  // Initializations

  strcpy(fileName,pEngineContext->path);
  fp=NULL;
  rc=ERROR_ID_NO;

  if (((ptr=strrchr(fileName,PATH_SEP))!=NULL) && !strlen(ptr+1))
   {
    // Concatenate file name to output path

    if ((ptr2=strrchr(pEngineContext->crossFile,PATH_SEP))!=NULL)
     ptr2++;
    else
     ptr2=pEngineContext->crossFile;

    sprintf(ptr,"%c%s",PATH_SEP,ptr2);

    // Replace file extension by the correct one

    if ((fileNameLength=strlen(fileName))!=0)
     {
      if ((ptr=strrchr(fileName,'.'))==NULL)
       {
        fileName[fileNameLength++]='.';
        fileName[fileNameLength]=0;
       }

      strcat(fileName,mediateConvolutionFileExt[pEngineContext->convolutionType]);
     }
   }

  // Save file

  if (!rc)
   {
    if ((fp=fopen(fileName,"w+t"))!=NULL)
     {
      if (!pEngineContext->noComment)
       {
        // Header

        fprintf(fp,";\n");
        fprintf(fp,"; High resolution cross section file : %s\n",pEngineContext->crossFile);
        fprintf(fp,"; Calibration file : %s\n",pEngineContext->calibrationFile);
        fprintf(fp,"; Shift applied : %g nm\n",pEngineContext->shift);
        fprintf(fp,"; Convolution type : %s\n",mediateConvolutionTypesStr[pEngineContext->convolutionType]);

        if (pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)
         {
          if (pEngineContext->convolutionType==CONVOLUTION_TYPE_I0_CORRECTION)
           {
            fprintf(fp,"; Kurucz file : %s\n",pEngineContext->kuruczFile);
            fprintf(fp,"; Concentration in I0 (mol/cm**2) : %g\n",pEngineContext->conc);
           }


          slitType=pEngineContext->slitConv.slitType;
          fprintf(fp,"; Slit function type : %s %s\n",XSCONV_slitTypes[slitType],(pEngineContext->slitConv.slitWveDptFlag)?"wavelength dependent":"");

          if (slitType==SLIT_TYPE_FILE)
           fprintf(fp,"; Slit function file : %s\n",pEngineContext->slitConv.slitFile);
          else if (pEngineContext->slitConv.slitWveDptFlag)
           {
            if (strlen(pEngineContext->slitConv.slitFile))
             fprintf(fp,"; Slit function file : %s\n",pEngineContext->slitConv.slitFile);
            if (strlen(pEngineContext->slitConv.slitFile2))
             fprintf(fp,"; Slit function file 2 : %s\n",pEngineContext->slitConv.slitFile2);
            if (strlen(pEngineContext->slitConv.slitFile3))
             fprintf(fp,"; Slit function file 3 : %s\n",pEngineContext->slitConv.slitFile3);          // Super gaussian
           }
          else
           {
            if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_AGAUSS) || (slitType==SLIT_TYPE_SUPERGAUSS))
             fprintf(fp,"; Gaussian FWHM : %.3f\n",pEngineContext->slitConv.slitParam);
            if (slitType==SLIT_TYPE_ERF)
             fprintf(fp,"; Boxcar width : %.3f\n",pEngineContext->slitConv.slitParam2);
            if (slitType==SLIT_TYPE_AGAUSS)
             fprintf(fp,"; Asymmetry factor : %.3f\n",pEngineContext->slitConv.slitParam2);
            if (slitType==SLIT_TYPE_SUPERGAUSS)
             {
                 fprintf(fp,"; Exponential term : %.3f\n",pEngineContext->slitConv.slitParam2);
              fprintf(fp,"; Asymmetry factor : %.3f\n",pEngineContext->slitConv.slitParam3);
             }

            if ((slitType== SLIT_TYPE_APOD) || (slitType== SLIT_TYPE_APODNBS))
             {
              fprintf(fp,"; Resolution : %.3lf\n",pEngineContext->slitConv.slitParam);
              fprintf(fp,"; Phase      : %.3lf\n",pEngineContext->slitConv.slitParam2);
             }

            if (slitType==SLIT_TYPE_VOIGT)
             {
              fprintf(fp,"; Gaussian FWHM : %.3f\n",pEngineContext->slitConv.slitParam);
              fprintf(fp,"; Lorentz/Gauss ratio : %.3f\n",pEngineContext->slitConv.slitParam2);
             }
           }

          if (slitType==SLIT_TYPE_INVPOLY)
           fprintf(fp,"; Polynomial degree : %d\n",(int)pEngineContext->slitConv.slitParam2);

          slitType=pEngineContext->slitDConv.slitType;

          if ((slitType!=SLIT_TYPE_FILE) || strlen(pEngineContext->slitDConv.slitFile))
           {
            fprintf(fp,"; Deconvolution Slit function type : %s %s\n",XSCONV_slitTypes[slitType],(pEngineContext->slitConv.slitWveDptFlag)?"wavelength dependent":"");

            if (slitType==SLIT_TYPE_FILE)
             fprintf(fp,"; Slit function file : %s\n",pEngineContext->slitDConv.slitFile);
            else if (pEngineContext->slitDConv.slitWveDptFlag)
             {
              if (strlen(pEngineContext->slitDConv.slitFile))
               fprintf(fp,"; Slit function file : %s\n",pEngineContext->slitDConv.slitFile);
              if (strlen(pEngineContext->slitDConv.slitFile2))
               fprintf(fp,"; Slit function file 2 : %s\n",pEngineContext->slitDConv.slitFile2);
              if (strlen(pEngineContext->slitDConv.slitFile3))
               fprintf(fp,"; Slit function file 3 : %s\n",pEngineContext->slitDConv.slitFile3);          // Super gaussian
             }
            else
             {
              if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_AGAUSS) || (slitType==SLIT_TYPE_SUPERGAUSS))
               fprintf(fp,"; Gaussian FWHM : %.3f\n",pEngineContext->slitDConv.slitParam);
              if (slitType==SLIT_TYPE_ERF)
               fprintf(fp,"; Boxcar width : %.3f\n",pEngineContext->slitDConv.slitParam2);
              if (slitType==SLIT_TYPE_AGAUSS)
               fprintf(fp,"; Asymmetry factor : %.3f\n",pEngineContext->slitDConv.slitParam2);
              if (slitType==SLIT_TYPE_SUPERGAUSS)
               {
                   fprintf(fp,"; Exponential term : %.3f\n",pEngineContext->slitDConv.slitParam2);
                fprintf(fp,"; Asymmetry factor : %.3f\n",pEngineContext->slitDConv.slitParam3);
               }

              if ((slitType== SLIT_TYPE_APOD) || (slitType== SLIT_TYPE_APODNBS))
               {
                fprintf(fp,"; Resolution : %.3lf\n",pEngineContext->slitDConv.slitParam);
                fprintf(fp,"; Phase      : %.3lf\n",pEngineContext->slitDConv.slitParam2);
               }

              if (slitType==SLIT_TYPE_VOIGT)
               {
                fprintf(fp,"; Gaussian FWHM : %.3f\n",pEngineContext->slitDConv.slitParam);
                fprintf(fp,"; Lorentz/Gauss ratio : %.3f\n",pEngineContext->slitDConv.slitParam2);
               }
             }

            if (slitType==SLIT_TYPE_INVPOLY)
             fprintf(fp,"; Polynomial degree : %d\n",(int)pEngineContext->slitDConv.slitParam2);
           }
         }

        // Low pass filtering

        if (pLFilter->type!=PRJCT_FILTER_TYPE_NONE)
         {
          fprintf(fp,"; Low-pass filtering\n");
          fprintf(fp,"; Filter applied : %s\n",mediateConvolutionFilterTypes[pLFilter->type]);
          fprintf(fp,"; Filter number of iterations : %d\n",pLFilter->filterNTimes);

          if (pLFilter->type==PRJCT_FILTER_TYPE_KAISER)
           {
            fprintf(fp,"; Cutoff frequency : %g\n",pLFilter->kaiserCutoff);
            fprintf(fp,"; Pass band : %g\n",pLFilter->kaiserPassBand);
            fprintf(fp,"; Tolerance : %g\n",pLFilter->kaiserTolerance);
           }
          else if (pLFilter->type==PRJCT_FILTER_TYPE_GAUSSIAN)
           fprintf(fp,"; Gaussian FWHM : %g\n",pLFilter->fwhmWidth);
          else if ((pLFilter->type==PRJCT_FILTER_TYPE_BOXCAR) || (pLFilter->type==PRJCT_FILTER_TYPE_TRIANGLE))
           fprintf(fp,"; Filter width : %d\n",pLFilter->filterWidth);
          else if (pLFilter->type==PRJCT_FILTER_TYPE_SG)
           {
            fprintf(fp,"; Filter width : %d\n",pLFilter->filterWidth);
            fprintf(fp,"; Filter order : %d\n",pLFilter->filterOrder);
           }
         }

        // High pass filtering

        if (pHFilter->type!=PRJCT_FILTER_TYPE_NONE)
         {
          fprintf(fp,"; High-pass filtering\n");
          fprintf(fp,"; Filter applied : %s\n",mediateConvolutionFilterTypes[pHFilter->type]);
          fprintf(fp,"; Filter number of iterations : %d\n",pHFilter->filterNTimes);

          if (pHFilter->type==PRJCT_FILTER_TYPE_KAISER)
           {
            fprintf(fp,"; Cutoff frequency : %g\n",pHFilter->kaiserCutoff);
            fprintf(fp,"; Pass band : %g\n",pHFilter->kaiserPassBand);
            fprintf(fp,"; Tolerance : %g\n",pHFilter->kaiserTolerance);
           }
          else if (pHFilter->type==PRJCT_FILTER_TYPE_GAUSSIAN)
           fprintf(fp,"; Gaussian FWHM : %g\n",pHFilter->fwhmWidth);
          else if ((pHFilter->type==PRJCT_FILTER_TYPE_BOXCAR) || (pHFilter->type==PRJCT_FILTER_TYPE_TRIANGLE))
           fprintf(fp,"; Filter width : %d\n",pHFilter->filterWidth);
          else if (pHFilter->type==PRJCT_FILTER_TYPE_SG)
           {
            fprintf(fp,"; Filter width : %d\n",pHFilter->filterWidth);
            fprintf(fp,"; Filter order : %d\n",pHFilter->filterOrder);
           }
         }

        fprintf(fp,";\n; Columns description :\n");
        fprintf(fp,"; Column 1 : calibration;\n");
        fprintf(fp,"; Column 2 : %s;\n",((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))?
                   "convoluted and filtered cross section" : "convoluted cross section");

        if ((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))
         fprintf(fp,"; Column 3 : convoluted only cross section.\n");
        fprintf(fp,";\n");
       }

      // Cross section

      if ((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))
       for (i=nextraPixels;i<nsize;i++)
        fprintf(fp,"%.14le %.14le %.14le\n",pXs->matrix[0][i],filterVector[i],pXs->matrix[1][i]);
      else
       for (i=nextraPixels;i<nsize;i++)
        fprintf(fp,"%.14le %.14le\n",pXs->matrix[0][i],pXs->matrix[1][i]);

      fclose(fp);
     }
    else
     rc=ERROR_SetLast("mediateConvolutionSave",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,fileName);
   }

  // Return

  return rc;
 }

// -------------------------------------------------------
// mediateConvolutionCalculate : Main convolution function
// -------------------------------------------------------

RC mediateConvolutionCalculate(void *engineContext,void *responseHandle)
 {
  // Declarations

  ENGINE_XSCONV_CONTEXT *pEngineContext=(ENGINE_XSCONV_CONTEXT*)engineContext;
  MATRIX_OBJECT XSCONV_slitMatrix[NSFP],XSCONV_slitDMatrix[NSFP];

  MATRIX_OBJECT XSCONV_xshr,                                                    // high resolution cross section
               *pXsnew,                                                         // convoluted cross section
                XSCONV_kurucz;                                                  // kurucz

  PRJCT_FILTER *plFilter,*phFilter;                                             // pointers to the low pass and high pass filtering parts of the engine context
  SLIT *pSlitConv,*pSlitDConv;                                                  // pointers to the convolution and deconvolution slit function parts of the engine context

  char windowTitle[MAX_ITEM_TEXT_LEN],pageTitle[MAX_ITEM_TEXT_LEN];
  double lambdaMin,lambdaMax,slitParam[NSFP],slitParamD[NSFP],slitWidth,*tmpVector;

  int slitType,slitType2,deconvFlag,dispConv;
  int lowFilterType,highFilterType,nFilter,i;
  RC rc;

  // Slit function

  pSlitDConv=&pEngineContext->slitDConv;
  pSlitConv=&pEngineContext->slitConv;
  pXsnew=&pEngineContext->xsNew;

  // Initialize buffers

  memset(XSCONV_slitMatrix,0,sizeof(MATRIX_OBJECT)*NSFP);
  memset(XSCONV_slitDMatrix,0,sizeof(MATRIX_OBJECT)*NSFP);

  slitParam[0]=pSlitConv->slitParam;
  slitParam[1]=pSlitConv->slitParam2;
  slitParam[2]=pSlitConv->slitParam3;

  slitParamD[0]=pSlitDConv->slitParam;
  slitParamD[1]=pSlitDConv->slitParam2;
  slitParamD[2]=pSlitDConv->slitParam3;

  memset(&XSCONV_xshr,0,sizeof(MATRIX_OBJECT));
  memset(&XSCONV_kurucz,0,sizeof(MATRIX_OBJECT));
  memset(pXsnew,0,sizeof(MATRIX_OBJECT));

  slitType=pSlitConv->slitType;
  slitType2=pSlitDConv->slitType;
  tmpVector=NULL;

  // Filtering

  plFilter=&pEngineContext->lfilter;
  phFilter=&pEngineContext->hfilter;

  lowFilterType=plFilter->type;            // low pass filtering
  highFilterType=phFilter->type;           // high pass filtering

  plFilter->filterFunction=phFilter->filterFunction=NULL;

  if ((((lowFilterType=plFilter->type)!=PRJCT_FILTER_TYPE_NONE) &&
        (lowFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) &&
       ((rc=FILTER_LoadFilter(plFilter))!=0)) ||

      (((highFilterType=phFilter->type)!=PRJCT_FILTER_TYPE_NONE) &&
        (highFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) &&
       ((rc=FILTER_LoadFilter(phFilter))!=0)))

   goto EndConvolution;

  nFilter=0;

  if ((lowFilterType!=PRJCT_FILTER_TYPE_NONE) && (lowFilterType!=PRJCT_FILTER_TYPE_ODDEVEN))
   nFilter+=(int)(plFilter->filterWidth*sqrt(plFilter->filterNTimes)+0.5);
  if ((highFilterType!=PRJCT_FILTER_TYPE_NONE) && (highFilterType!=PRJCT_FILTER_TYPE_ODDEVEN))
   nFilter+=(int)(phFilter->filterWidth*sqrt(phFilter->filterNTimes)+0.5);

  pEngineContext->nFilter=nFilter;

  if (pEngineContext->filterVector!=NULL)
   MEMORY_ReleaseDVector("mediateConvolutionCalculate","filterVector",pEngineContext->filterVector,0);

  // Display control

  deconvFlag=((pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE) &&
              !pSlitDConv->slitWveDptFlag && (pSlitDConv->slitType!=SLIT_TYPE_FILE || (strlen(pSlitDConv->slitFile)!=0)));

  // Load calibration file and slit function

  if (!(rc=XSCONV_LoadCalibrationFile(pXsnew,pEngineContext->calibrationFile,nFilter)) &&
     (((lowFilterType==PRJCT_FILTER_TYPE_NONE) && (highFilterType==PRJCT_FILTER_TYPE_NONE)) ||
     (((pEngineContext->filterVector=(double *)MEMORY_AllocDVector("mediateConvolutionCalculate","filterVector",0,pXsnew->nl-1))!=NULL) &&
      ((tmpVector=(double *)MEMORY_AllocDVector("mediateConvolutionCalculate","tmpVector",0,pXsnew->nl-1))!=NULL))) &&
      ((pEngineContext->convolutionType==CONVOLUTION_TYPE_NONE) ||
     (!(rc=XSCONV_LoadSlitFunction(XSCONV_slitMatrix,&pEngineContext->slitConv,&slitParam[0],&slitType)) &&

      (!deconvFlag || !(rc=XSCONV_LoadSlitFunction(XSCONV_slitDMatrix,pSlitDConv,&slitParamD[0],&slitType2))))))
   {
    slitWidth=(double)3.*slitParam[0];

    // Window in wavelength

    if ((slitType!=SLIT_TYPE_FILE) || (pEngineContext->convolutionType==CONVOLUTION_TYPE_NONE))
     {
      lambdaMin=pXsnew->matrix[0][0]-slitWidth-1.;                     // add 1 nm
      lambdaMax=pXsnew->matrix[0][pXsnew->nl-1]+slitWidth+1.;
     }
    else
     {
      lambdaMin=pXsnew->matrix[0][0]+XSCONV_slitMatrix[0].matrix[0][0]-1.;                     // add 1 nm
      lambdaMax=pXsnew->matrix[0][pXsnew->nl-1]+XSCONV_slitMatrix[0].matrix[0][XSCONV_slitMatrix[0].nl-1]+1.;
     }

    if (deconvFlag)
     slitType=SLIT_TYPE_FILE;  // the resulting effective slit function works as a slit file type one

    // Determine effective slit function when a deconvolution slit function is given

    if ((!deconvFlag || !(rc=XSCONV_NewSlitFunction(pSlitConv,XSCONV_slitMatrix,slitParam[0],pSlitDConv,XSCONV_slitDMatrix,slitParamD[0]))) &&                 // --- TO UPDATE

    // Load high resolution Kurucz file in convolution with I0 correction method

        ((pEngineContext->convolutionType!=CONVOLUTION_TYPE_I0_CORRECTION) ||
        !(rc=XSCONV_LoadCrossSectionFile(&XSCONV_kurucz,pEngineContext->kuruczFile,lambdaMin,lambdaMax,(double)0.,CONVOLUTION_CONVERSION_NONE))) &&
        !(rc=XSCONV_LoadCrossSectionFile(&XSCONV_xshr,pEngineContext->crossFile,lambdaMin,lambdaMax,(double)pEngineContext->shift,pEngineContext->conversionMode)))
     {
      dispConv=((pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE) ||
            (XSCONV_xshr.nl!=pXsnew->nl) ||
            !VECTOR_Equal(XSCONV_xshr.matrix[0],pXsnew->matrix[0],pXsnew->nl,(double)1.e-7))?1:0;


      // -----------
      // Convolution
      // -----------

      switch(pEngineContext->convolutionType)
       {
     // ----------------------------------------------------------------------
        case CONVOLUTION_TYPE_NONE :
         rc=XSCONV_TypeNone(pXsnew,&XSCONV_xshr);
        break;
     // ----------------------------------------------------------------------
        case CONVOLUTION_TYPE_STANDARD :
         rc=XSCONV_TypeStandard(pXsnew,0,pXsnew->nl,&XSCONV_xshr,&XSCONV_xshr,NULL,slitType,XSCONV_slitMatrix,slitParam,pSlitConv->slitWveDptFlag);
         break;
     // ----------------------------------------------------------------------
        case CONVOLUTION_TYPE_I0_CORRECTION :
          rc=XSCONV_TypeI0Correction(pXsnew,&XSCONV_xshr,&XSCONV_kurucz,pEngineContext->conc,slitType,XSCONV_slitMatrix,slitParam,pSlitConv->slitWveDptFlag);
        break;
     // ----------------------------------------------------------------------
     }

      // ------------------------------------------
      // Save resulting cross section and plot data
      // ------------------------------------------

      if ((lowFilterType==PRJCT_FILTER_TYPE_NONE) && (highFilterType==PRJCT_FILTER_TYPE_NONE))
       sprintf(pageTitle,(pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)?"Spectrum after convolution":"Spectrum after interpolation");
      else
       sprintf(pageTitle,(pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)?"Spectrum after convolution and filtering":"Spectrum after interpolation and filtering");

      if (!rc && dispConv)
       {
           plot_data_t spectrumData[2];

           sprintf(windowTitle,(pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)?"Spectrum after convolution":"Spectrum after interpolation");

        mediateAllocateAndSetPlotData(&spectrumData[0],"High resolution spectrum",XSCONV_xshr.matrix[0],XSCONV_xshr.matrix[1],XSCONV_xshr.nl,Line);
        mediateAllocateAndSetPlotData(&spectrumData[1],windowTitle,pXsnew->matrix[0]+nFilter,pXsnew->matrix[1]+nFilter,pXsnew->nl-2*nFilter,Line);
        mediateResponsePlotData(0,spectrumData,2,Spectrum,forceAutoScale,windowTitle,"Wavelength (nm)","",responseHandle);
        mediateResponseLabelPage(0,pageTitle,"",responseHandle);
        mediateReleasePlotData(&spectrumData[1]);
        mediateReleasePlotData(&spectrumData[0]);
       }

      if (pEngineContext->filterVector!=NULL)
       memcpy(pEngineContext->filterVector,pXsnew->matrix[1],pXsnew->nl*sizeof(double));

      // ------------------
      // Low-Pass filtering
      // ------------------

      if ((lowFilterType!=PRJCT_FILTER_TYPE_NONE) && (pEngineContext->filterVector!=NULL) && !rc &&
        (((lowFilterType==PRJCT_FILTER_TYPE_ODDEVEN) && !(rc=FILTER_OddEvenCorrection(pXsnew->matrix[0],pXsnew->matrix[1],pEngineContext->filterVector,pXsnew->nl))) ||
         ((lowFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) && !(rc=FILTER_Vector(plFilter,pEngineContext->filterVector,pEngineContext->filterVector,NULL,pXsnew->nl,PRJCT_FILTER_OUTPUT_LOW)))) && dispConv)
       {
           plot_data_t spectrumData[2];

           sprintf(windowTitle,"Spectrum after low-pass filtering");

        mediateAllocateAndSetPlotData(&spectrumData[0],"Convoluted spectrum before low pass filtering",pXsnew->matrix[0]+nFilter,pXsnew->matrix[1]+nFilter,pXsnew->nl-2*nFilter,Line);
        mediateAllocateAndSetPlotData(&spectrumData[1],"Convoluted spectrum after low pass filtering",pXsnew->matrix[0]+nFilter,pEngineContext->filterVector+nFilter,pXsnew->nl-2*nFilter,Line);
        mediateResponsePlotData(0,spectrumData,2,Spectrum,forceAutoScale,windowTitle,"Wavelength (nm)","",responseHandle);
        mediateResponseLabelPage(0,pageTitle,"",responseHandle);
        mediateReleasePlotData(&spectrumData[1]);
        mediateReleasePlotData(&spectrumData[0]);
       }

      // -------------------
      // High-Pass filtering
      // -------------------

      if ((highFilterType!=PRJCT_FILTER_TYPE_NONE) && (highFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) && (pEngineContext->filterVector!=NULL) && !rc &&
         !(rc=FILTER_Vector(phFilter,pEngineContext->filterVector,pEngineContext->filterVector,tmpVector,pXsnew->nl,phFilter->filterAction)) && dispConv)
       {
           plot_data_t spectrumData[2];

        sprintf(windowTitle,"Spectrum after high-pass filtering");

        mediateAllocateAndSetPlotData(&spectrumData[0],windowTitle,pXsnew->matrix[0]+nFilter,pXsnew->matrix[1]+nFilter,pXsnew->nl-2*nFilter,Line);
        mediateAllocateAndSetPlotData(&spectrumData[1],windowTitle,pXsnew->matrix[0]+nFilter,tmpVector+nFilter,pXsnew->nl-2*nFilter,Line);

        mediateResponsePlotData(0,spectrumData,2,Spectrum,forceAutoScale,windowTitle,"Wavelength (nm)","",responseHandle);
        mediateResponseLabelPage(0,pageTitle,"",responseHandle);
        mediateReleasePlotData(&spectrumData[0]);

           sprintf(windowTitle,"Spectrum after %s",(phFilter->filterAction==PRJCT_FILTER_OUTPUT_HIGH_SUB)?"subtraction":"division");

        mediateAllocateAndSetPlotData(&spectrumData[0],windowTitle,pXsnew->matrix[0]+nFilter,pEngineContext->filterVector+nFilter,pXsnew->nl-2*nFilter,Line);
        mediateResponsePlotData(1,spectrumData,1,Spectrum,forceAutoScale,windowTitle,"Wavelength (nm)","",responseHandle);
        mediateResponseLabelPage(1,pageTitle,"",responseHandle);
        mediateReleasePlotData(&spectrumData[0]);
       }

      // Result safe keeping

      if (!rc)
       rc=mediateConvolutionSave(pEngineContext);
     }
   }

  EndConvolution :

  // Release allocated buffers

  for (i=0;i<NSFP;i++)
   {
    MATRIX_Free(&XSCONV_slitMatrix[i],__func__);
    MATRIX_Free(&XSCONV_slitDMatrix[i],__func__);
   }

  MATRIX_Free(&XSCONV_xshr,"mediateConvolutionCalculate");
  MATRIX_Free(&XSCONV_kurucz,"mediateConvolutionCalculate");
  MATRIX_Free(pXsnew,"mediateConvolutionCalculate");

  if (plFilter->filterFunction!=NULL)
   {
    MEMORY_ReleaseDVector("mediateConvolutionCalculate","FILTER_function",plFilter->filterFunction,1);
    plFilter->filterFunction=NULL;
   }

  if (phFilter->filterFunction!=NULL)
   {
    MEMORY_ReleaseDVector("mediateConvolutionCalculate","FILTER_function",phFilter->filterFunction,1);
    phFilter->filterFunction=NULL;
   }

  if (tmpVector!=NULL)
   MEMORY_ReleaseDVector("mediateConvolutionCalculate","tmpVector",tmpVector,0);
  
  if (rc)
   ERROR_DisplayMessage(responseHandle);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestConvolution
// -----------------------------------------------------------------------------
// PURPOSE       Transfer the convolution options from the GUI to the engine
//
// RETURN        ERROR_ID_NO if no error found
// -----------------------------------------------------------------------------

RC mediateRequestConvolution(void *engineContext,mediate_convolution_t *pMediateConvolution,void *responseHandle)
 {
     // Declarations

  ENGINE_XSCONV_CONTEXT *pEngineContext = (ENGINE_XSCONV_CONTEXT*)engineContext;
  PRJCT_FILTER *plFilter,*phFilter;                                             // pointers to the low pass and high pass filter parts of the engine context
  SLIT *pSlitConv,*pSlitDConv;                                                  // pointers to the convolution and deconvolution slit function parts of the engine context
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  // General information

  pEngineContext->convolutionType=pMediateConvolution->general.convolutionType;
  pEngineContext->conversionMode=pMediateConvolution->general.conversionType;
  pEngineContext->shift=pMediateConvolution->general.shift;
  pEngineContext->conc=pMediateConvolution->general.conc;
  pEngineContext->noComment=pMediateConvolution->general.noheader;

  strcpy(pEngineContext->crossFile,pMediateConvolution->general.inputFile);                 // high resolution cross section file
  strcpy(pEngineContext->path,pMediateConvolution->general.outputFile);                     // output path
  strcpy(pEngineContext->calibrationFile,pMediateConvolution->general.calibrationFile);     // calibration file
  strcpy(pEngineContext->kuruczFile,pMediateConvolution->general.solarRefFile);             // Kurucz file used when I0 correction is applied

  // Description of the slit function

  setMediateSlit(&pEngineContext->slitConv,&pMediateConvolution->conslit);
  setMediateSlit(&pEngineContext->slitDConv,&pMediateConvolution->decslit);

  // Filtering configuration

  setMediateFilter(&pEngineContext->lfilter,&pMediateConvolution->lowpass,0,1);
  setMediateFilter(&pEngineContext->hfilter,&pMediateConvolution->highpass,1,1);

  // Check input

  if (!strlen(pEngineContext->crossFile))
   rc=ERROR_SetLast("mediateRequestConvolution",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Input (General tab page)","Input cross section file name is missing");
  else if (!strlen(pEngineContext->calibrationFile))
   rc=ERROR_SetLast("mediateRequestConvolution",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Calibration (General tab page)","Calibration file name is missing");
  else if (pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)
   {
       pSlitConv=&pEngineContext->slitConv;
       pSlitDConv=&pEngineContext->slitDConv;
       plFilter=&pEngineContext->lfilter;
       phFilter=&pEngineContext->hfilter;

       // Convolution slit function

       if (((pSlitConv->slitType==SLIT_TYPE_FILE) || pSlitConv->slitWveDptFlag) && !strlen(pSlitConv->slitFile))
     rc=ERROR_SetLast("mediateRequestConvolution",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Slit Function Type (Slit Function page, convolution part)","Convolution slit function file is missing");
    else if ((pSlitConv->slitType==SLIT_TYPE_INVPOLY) &&
            ((pSlitConv->slitParam2<=(double)0.) ||
              (pSlitConv->slitParam2-floor(pSlitConv->slitParam2)!=(double)0.) ||
              (fmod(pSlitConv->slitParam2,(double)2.)!=(double)0.)))

     rc=ERROR_SetLast("mediateRequestConvolution",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Degree (Slit Function page, convolution part)","Polynomial degree should be a positive integer and a multiple of 2");

    // Deconvolution slit function

    else if (pSlitDConv->slitWveDptFlag && !strlen(pSlitDConv->slitFile))

     rc=ERROR_SetLast("mediateRequestConvolution",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Slit Function Type (Slit Function page, Deconvolution part)","Convolution slit function file is missing");

    else if ((pSlitDConv->slitType==SLIT_TYPE_INVPOLY) &&
            ((pSlitDConv->slitParam2<=(double)0.) ||
             (pSlitDConv->slitParam2-floor(pSlitDConv->slitParam2)!=(double)0.) ||
             (fmod(pSlitDConv->slitParam2,(double)2.)!=(double)0.)))

     rc=ERROR_SetLast("mediateRequestConvolution",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Degree (Slit Function page, Deconvolution part)","Polynomial degree should be a positive integer and a multiple of 2");

    // Low pass filtering

    else if ((plFilter->type!=PRJCT_FILTER_TYPE_NONE) &&
            ((plFilter->type==PRJCT_FILTER_TYPE_BOXCAR) ||
             (plFilter->type==PRJCT_FILTER_TYPE_TRIANGLE) ||
             (plFilter->type==PRJCT_FILTER_TYPE_SG) ||
             (plFilter->type==PRJCT_FILTER_TYPE_BINOMIAL)) &&
             (plFilter->filterWidth%2!=1))

     rc=ERROR_SetLast("mediateRequestConvolution",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Filter width (Filtering page, low pass filter part)","Filter width should be an odd number");

    else if ((plFilter->type!=PRJCT_FILTER_TYPE_NONE) &&
             (plFilter->type==PRJCT_FILTER_TYPE_SG) &&
            ((plFilter->filterOrder%2!=0) || (plFilter->filterOrder==0)))

     rc=ERROR_SetLast("mediateRequestConvolution",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Filter order (Filtering page, low pass filter part)","Filter order should be an even strictly positive number");

    // High pass filtering

    else if ((phFilter->type!=PRJCT_FILTER_TYPE_NONE) &&
            ((phFilter->type==PRJCT_FILTER_TYPE_BOXCAR) ||
             (phFilter->type==PRJCT_FILTER_TYPE_TRIANGLE) ||
             (phFilter->type==PRJCT_FILTER_TYPE_SG) ||
             (phFilter->type==PRJCT_FILTER_TYPE_BINOMIAL)) &&
             (phFilter->filterWidth%2!=1))

     rc=ERROR_SetLast("mediateRequestConvolution",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Filter width (Filtering page, low pass filter part)","Filter width should be an odd number");

    else if ((phFilter->type!=PRJCT_FILTER_TYPE_NONE) &&
             (phFilter->type==PRJCT_FILTER_TYPE_SG) &&
            ((phFilter->filterOrder%2!=0) || (phFilter->filterOrder==0)))

     rc=ERROR_SetLast("mediateRequestConvolution",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Filter order (Filtering page, low pass filter part)","Filter order should be an even strictly positive number");
   }

  // Return
  
  if (rc)
   ERROR_DisplayMessage(responseHandle);

  return rc;

 }

// =========
// RING TOOL
// =========

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestRing
// -----------------------------------------------------------------------------
// PURPOSE       Transfer the convolution options from the GUI to the engine
//
// RETURN        ERROR_ID_NO if no error found
// -----------------------------------------------------------------------------

RC mediateRequestRing(void *engineContext,mediate_ring_t *pMediateRing,void *responseHandle)
 {
     // Declarations

  ENGINE_XSCONV_CONTEXT *pEngineContext = (ENGINE_XSCONV_CONTEXT*)engineContext;// pointer to the engine context
  SLIT *pSlitConv;                                                              // pointer to the convolution part of the engine context
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  // General information

  pEngineContext->noComment=pMediateRing->noheader;
  pEngineContext->saveRaman=pMediateRing->saveraman;
  pEngineContext->normalizeFlag=pMediateRing->normalize;
  pEngineContext->temperature=pMediateRing->temperature;

  strcpy(pEngineContext->path,pMediateRing->outputFile);                        // output path
  strcpy(pEngineContext->calibrationFile,pMediateRing->calibrationFile);        // calibration file
  strcpy(pEngineContext->kuruczFile,pMediateRing->solarRefFile);                // Kurucz file used when I0 correction is applied

  // Description of the slit function

  setMediateSlit(&pEngineContext->slitConv,&pMediateRing->slit);

  if (!strlen(pEngineContext->calibrationFile))
   rc=ERROR_SetLast("mediateRequestRing",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Calibration","Calibration file name is missing");
  else
   {
       pSlitConv=&pEngineContext->slitConv;

       // Convolution slit function

       if (((pEngineContext->slitConv.slitType==SLIT_TYPE_FILE) || pSlitConv->slitWveDptFlag) && !strlen(pSlitConv->slitFile))

     rc=ERROR_SetLast("mediateRequestRing",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Slit Function Type","Convolution slit function file is missing");

    else if ((pSlitConv->slitType==SLIT_TYPE_INVPOLY)  &&
             ((pSlitConv->slitParam2<=(double)0.) ||
              (pSlitConv->slitParam2-floor(pSlitConv->slitParam2)!=(double)0.) ||
              (fmod(pSlitConv->slitParam2,(double)2.)!=(double)0.)))

     rc=ERROR_SetLast("mediateRequestRing",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Degree","Polynomial degree should be a positive integer and a multiple of 2");
   }
   
  if (rc)
   ERROR_DisplayMessage(responseHandle);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateRingHeader
// -----------------------------------------------------------------------------
// PURPOSE       Write options in the file header
//
// INPUT         pEngineContext : pointer to the current engine context
//               fp    : pointer to the output file
// -----------------------------------------------------------------------------

void mediateRingHeader(ENGINE_XSCONV_CONTEXT *pEngineContext,FILE *fp)
 {
  // Declaration

  int     slitType;                                                 // type of the slit function

  // Header

  fprintf(fp,";\n");
  fprintf(fp,"; RING CROSS SECTION\n");
  fprintf(fp,"; Temperature : %g K\n",pEngineContext->temperature);
  fprintf(fp,"; High resolution Kurucz file : %s\n",pEngineContext->kuruczFile);
  fprintf(fp,"; Calibration file : %s\n",pEngineContext->calibrationFile);
  fprintf(fp,"; Slit function type : %s %s\n",XSCONV_slitTypes[(slitType=pEngineContext->slitConv.slitType)],(pEngineContext->slitConv.slitWveDptFlag)?"wavelength dependent":"");

  if (slitType==SLIT_TYPE_FILE)
   fprintf(fp,"; Slit function file : %s\n",pEngineContext->slitConv.slitFile);
  else if (pEngineContext->slitConv.slitWveDptFlag)
   {
    if (strlen(pEngineContext->slitConv.slitFile))
     fprintf(fp,"; Slit function file : %s\n",pEngineContext->slitConv.slitFile);
    if (strlen(pEngineContext->slitConv.slitFile2))
     fprintf(fp,"; Slit function file 2 : %s\n",pEngineContext->slitConv.slitFile2);
    if (strlen(pEngineContext->slitConv.slitFile3))
     fprintf(fp,"; Slit function file 3 : %s\n",pEngineContext->slitConv.slitFile3);          // Super gaussian
   }
  else
   {
    if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_AGAUSS) || (slitType==SLIT_TYPE_SUPERGAUSS))
     fprintf(fp,"; Gaussian FWHM : %.3f\n",pEngineContext->slitConv.slitParam);
    if (slitType==SLIT_TYPE_ERF)
     fprintf(fp,"; Boxcar width : %.3f\n",pEngineContext->slitConv.slitParam2);
    if (slitType==SLIT_TYPE_AGAUSS)
     fprintf(fp,"; Asymmetry factor : %.3f\n",pEngineContext->slitConv.slitParam2);
    if (slitType==SLIT_TYPE_SUPERGAUSS)
     {
         fprintf(fp,"; Exponential term : %.3f\n",pEngineContext->slitConv.slitParam2);
      fprintf(fp,"; Asymmetry factor : %.3f\n",pEngineContext->slitConv.slitParam3);
     }

    if ((slitType== SLIT_TYPE_APOD) || (slitType== SLIT_TYPE_APODNBS))
     {
      fprintf(fp,"; Resolution : %.3lf\n",pEngineContext->slitConv.slitParam);
      fprintf(fp,"; Phase      : %.3lf\n",pEngineContext->slitConv.slitParam2);
     }

    if (slitType==SLIT_TYPE_VOIGT)
     {
      fprintf(fp,"; Gaussian FWHM : %.3f\n",pEngineContext->slitConv.slitParam);
      fprintf(fp,"; Lorentz/Gauss ratio : %.3f\n",pEngineContext->slitConv.slitParam2);
     }
   }

  if (slitType==SLIT_TYPE_INVPOLY)
   fprintf(fp,"; Polynomial degree : %d\n",(int)pEngineContext->slitConv.slitParam2);

  fprintf(fp,"; column 1 : wavelength\n");
  fprintf(fp,"; column 2 : ring (raman/solar spectrum)\n");

  if (pEngineContext->saveRaman)
   {
    fprintf(fp,"; column 3 : interpolated raman\n");
    fprintf(fp,"; column 4 : convoluted solar spectrum\n");
   }

  fprintf(fp,";\n");
 }

// -------------------------------------------------------------------
// mediateRingCalculate : Main function to create a ring cross section
// -------------------------------------------------------------------

RC mediateRingCalculate(void *engineContext,void *responseHandle)
 {
  // Declarations

  MATRIX_OBJECT slitTmp;
  ENGINE_XSCONV_CONTEXT *pEngineContext=(ENGINE_XSCONV_CONTEXT*)engineContext;
  char   ringFileName[MAX_ITEM_TEXT_LEN],                                       // name of the output ring file
           pageTitle[MAX_ITEM_TEXT_LEN];
 // double *n2xref,*o2xref,                                                       // rotational Raman spectra
 //         gamman2,sigprimen2,n2xsec,sign2,sumn2xsec,                            // n2 working variables
 //         gammao2,sigprimeo2,o2xsec,sigo2,sumo2xsec,                            // o2 working variables
 //         sigsq,lambda1e7,solar,n2posj,o2posj,                                  // other working variables
  double  lambda,                                                               // range of wavelengths covered by slit function
          slitWidth,                                                            // width of the slit function
         *raman,*raman2,*ramanint,*ringEnd,                                     // output ring cross section
         *solarLambda,*solarVector,*solarDeriv2,                                // substitution vectors for solar spectrum
         *slitLambda,*slitVector,*slitDeriv2,                                   // substitution vectors for slit function
         *slitLambda2,*slitVector2,*slitDeriv22,                                // substitution vectors for slit function
         *ringLambda,*ringVector,                                               // substitution vectors for ring cross section
          temp,                                                                 // approximate average temperature in �K for scattering
          slitParam[NSFP];                                                      // gaussian full width at half maximum

  MATRIX_OBJECT  xsSolar,xsSolarConv,xsSlit[NSFP],xsRing,*pSlit,*pSlit2;// solar spectrum and slit function
  int     nsolar,nslit,nslit2,nring,                                            // size of previous vectors
          wveDptFlag,
          slitType;                                                             // type of the slit function
  INDEX   i,j;                                                                  // indexes for loops and arrays
  FILE   *fp;                                                                   // output file pointer
  RC      rc;                                                                   // return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Start("Ring.dbg","Ring",DEBUG_FCTTYPE_APPL,5,DEBUG_DVAR_YES,1);
  #endif

  // Initializations

  memset(&xsSolar,0,sizeof(MATRIX_OBJECT));
  memset(&xsSolarConv,0,sizeof(MATRIX_OBJECT));
  memset(xsSlit,0,sizeof(MATRIX_OBJECT)*NSFP);
  memset(&xsRing,0,sizeof(MATRIX_OBJECT));
  memset(&slitTmp,0,sizeof(MATRIX_OBJECT));

  slitParam[0]=pEngineContext->slitConv.slitParam;
  slitParam[1]=pEngineContext->slitConv.slitParam2;
  slitParam[2]=pEngineContext->slitConv.slitParam3;

  slitLambda=slitVector=slitDeriv2=NULL;
  slitLambda2=slitVector2=slitDeriv22=NULL;

  slitWidth=(double)RING_SLIT_WIDTH;                                            // NB : force slit width to 6 because of convolutions
  slitType=pEngineContext->slitConv.slitType;
  raman=raman2=ramanint=ringEnd=NULL;
  slitLambda2=slitVector2=slitDeriv22=NULL;
  temp=(double)pEngineContext->temperature;                                        // (double)250.;   May 2005/05/31
  nslit2=0;
  fp=NULL;
  rc=ERROR_ID_NO;

  #if defined(__DEBUG_) && __DEBUG_
  {
      // Declarations

   time_t today;                                                                // current date and time as a time_t number
   char datetime[20];                                                           // current date and time as a string

   // Get the current date and time

   today=time(NULL);

   // Convert into a string

   strftime(datetime,20,"%d/%m/%Y %H:%M:%S",localtime(&today));
   DEBUG_Print("Start at %s\n",datetime);
  }
  #endif

  if (!(rc=XSCONV_LoadCalibrationFile(&xsRing,pEngineContext->calibrationFile,0)) &&

  // Load slit function from file or pre-calculate the slit function

          ((slitType==SLIT_TYPE_NONE) || !(rc=XSCONV_LoadSlitFunction(xsSlit,&pEngineContext->slitConv,&slitParam[0],&slitType))) &&

  // Load solar spectrum in the range of wavelengths covered by the end wavelength scale corrected by the slit function

           !(rc=XSCONV_LoadCrossSectionFile(&xsSolar,pEngineContext->kuruczFile,(double)xsRing.matrix[0][0]-slitWidth-1.,
                                     (double)xsRing.matrix[0][xsRing.nl-1]+slitWidth+1.,(double)0.,CONVOLUTION_CONVERSION_NONE)) &&

           !(rc=MATRIX_Allocate(&xsSolarConv,xsSolar.nl,2,0,0,1,"mediateRingCalculate")))
   {
    // Use substitution variables

    for (i=0;i<xsSolarConv.nl;i++)
     xsSolarConv.matrix[0][i]=xsSolar.matrix[0][i];

    pSlit=&xsSlit[0];
    pSlit2=&xsSlit[1];

    solarLambda=xsSolarConv.matrix[0];
    solarVector=xsSolarConv.matrix[1];
    solarDeriv2=xsSolarConv.deriv2[1];
    nsolar=xsSolarConv.nl;

    wveDptFlag=pEngineContext->slitConv.slitWveDptFlag;

    if (slitType!=SLIT_TYPE_NONE) {
      if ((slitType!=SLIT_TYPE_FILE) || (pSlit->nc==2)) {
        slitLambda=pSlit->matrix[0];
        slitVector=pSlit->matrix[1];
        slitDeriv2=pSlit->deriv2[1];
        nslit=pSlit->nl;

        if (wveDptFlag)
         {
             // Make a backup of the slit function

          if ((rc=MATRIX_Allocate(&slitTmp,pSlit->nl,2,0,0,1,"XSCONV_TypeStandard"))!=0)
           goto EndRing;
          else
           {
               memcpy(slitTmp.matrix[0],slitLambda,sizeof(double)*pSlit->nl);
               memcpy(slitTmp.matrix[1],slitVector,sizeof(double)*pSlit->nl);
               memcpy(slitTmp.deriv2[1],slitDeriv2,sizeof(double)*pSlit->nl);
           }
         }
       }
      else if ((rc=MATRIX_Allocate(&slitTmp,pSlit->nl-1,2,0,0,1,"XSCONV_TypeStandard"))!=0)
       goto EndRing;
      else
       {
        slitLambda=slitTmp.matrix[0];
        slitVector=slitTmp.matrix[1];
        slitDeriv2=slitTmp.deriv2[1];
        nslit=slitTmp.nl;

        memcpy(slitTmp.matrix[0],(double *)pSlit->matrix[0]+1,sizeof(double)*(pSlit->nl-1));
       }

      if (wveDptFlag && ((slitType==SLIT_TYPE_FILE) || (slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_VOIGT) || (slitType==SLIT_TYPE_AGAUSS) || (slitType==SLIT_TYPE_SUPERGAUSS)))
       {
        slitLambda2=pSlit2->matrix[0];
        slitVector2=pSlit2->matrix[1];
        slitDeriv22=pSlit2->deriv2[1];
        nslit2=pSlit2->nl;
       }
      else
       {
           slitLambda2=slitVector2=slitDeriv22=NULL;
           nslit2=0;
       }
     }
    else
     {
         memcpy(xsSolarConv.matrix[0],xsSolar.matrix[0],sizeof(double)*xsSolar.nl);
         memcpy(xsSolarConv.matrix[1],xsSolar.matrix[1],sizeof(double)*xsSolar.nl);
     }

    ringLambda=xsRing.matrix[0];
    ringVector=xsRing.matrix[1];
    nring=xsRing.nl;

    if (!nsolar ||
       ((raman=MEMORY_AllocDVector("mediateRingCalculate","raman",0,nsolar-1))==NULL) ||
       ((raman2=MEMORY_AllocDVector("mediateRingCalculate","raman2",0,nsolar-1))==NULL) ||
       ((ramanint=MEMORY_AllocDVector("mediateRingCalculate","ramanint",0,nring-1))==NULL) || ((ringEnd=MEMORY_AllocDVector("mediateRingCalculate","ringEnd",0,nring-1))==NULL))
     rc=ERROR_ID_ALLOC;
    else
     {
      VECTOR_Init(raman,(double)0.,nsolar);
      VECTOR_Init(raman2,(double)0.,nsolar);

         // Start convolving the solar spectrum

      if (((slitType!=SLIT_TYPE_NONE) && ((rc=XSCONV_TypeStandard(&xsSolarConv,0,xsSolarConv.nl,&xsSolar,&xsSolar,NULL,slitType,xsSlit,slitParam,pEngineContext->slitConv.slitWveDptFlag))!=ERROR_ID_NO)) ||
          ((rc=SPLINE_Deriv2(solarLambda,solarVector,solarDeriv2,nsolar,"mediateRingCalculate"))!=0) ||
          ((rc=raman_convolution(solarLambda,solarVector,solarDeriv2,raman,nsolar,temp,pEngineContext->normalizeFlag))!=0))
       goto EndRing;

      strcpy(ringFileName,pEngineContext->path);

      #if defined(__DEBUG_) && __DEBUG_
      DEBUG_PrintVar("High resolution vectors",solarLambda,0,nsolar-1,solarVector,0,nsolar-1,raman,0,nsolar-1,NULL);
      #endif

      // Interpolate the high resolution spectrum

      if (!rc && !(rc=SPLINE_Deriv2(solarLambda,solarVector,solarDeriv2,nsolar,"mediateRingCalculate")) &&
         !(rc=SPLINE_Vector(solarLambda,solarVector,solarDeriv2,nsolar,ringLambda,ringVector,nring,SPLINE_CUBIC)) &&

      // Interpolate the raman spectrum

         !(rc=SPLINE_Deriv2(solarLambda,raman,raman2,nsolar,"mediateRingCalculate")) &&
         !(rc=SPLINE_Vector(solarLambda,raman,raman2,nsolar,ringLambda,ramanint,nring,SPLINE_CUBIC)))
       {
        if ((fp=fopen(ringFileName,"w+t"))!=NULL)
         {
             plot_data_t spectrumData[1];

          if (!pEngineContext->noComment)
           mediateRingHeader(pEngineContext,fp);

          // The final ring is the ratio between the interpolated raman and the interpolated convoluted solar spectrum

          for (i=0;i<nring;i++)
           ringEnd[i]=((ramanint[i]>(double)0.) && (ringVector[i]>(double)0.))?
                                    (double)ramanint[i] /* ring effect source spectrum */ /ringVector[i] /* solar spectrum */:(double)0.;

          strcpy(pageTitle,"Ring");

          if (pEngineContext->saveRaman)
           for (i=0;i<nring;i++)
            fprintf(fp,"%.14le %.14le %.14le %.14le\n",ringLambda[i],ringEnd[i],ramanint[i],ringVector[i]);
          else
           for (i=0;i<nring;i++)
            fprintf(fp,"%.14le %.14le\n",ringLambda[i],ringEnd[i]);

          mediateAllocateAndSetPlotData(&spectrumData[0],"Calculated ring cross section",ringLambda,ringEnd,nring,Line);
          mediateResponsePlotData(0,spectrumData,1,Spectrum,forceAutoScale,"Calculated ring cross section","Wavelength (nm)","",responseHandle);
          mediateResponseLabelPage(0,pageTitle,"",responseHandle);
          mediateReleasePlotData(&spectrumData[0]);

          if (fp!=NULL)
           fclose(fp);
         }
       }
     }
   }

  EndRing :

  // Release allocated buffers

  if (raman!=NULL)
   MEMORY_ReleaseDVector("mediateRingCalculate","raman",raman,0);
  if (raman2!=NULL)
   MEMORY_ReleaseDVector("mediateRingCalculate","raman2",raman2,0);
  if (ramanint!=NULL)
   MEMORY_ReleaseDVector("mediateRingCalculate","ramanint",ramanint,0);
  if (ringEnd!=NULL)
   MEMORY_ReleaseDVector("mediateRingCalculate","ringEnd",ringEnd,0);

  MATRIX_Free(&xsSolar,"mediateRingCalculate");
  MATRIX_Free(&xsSolarConv,"mediateRingCalculate");

  for (i=0;i<NSFP;i++)
   MATRIX_Free(&xsSlit[i],"mediateRingCalculate");


  MATRIX_Free(&xsRing,"mediateRingCalculate");
  MATRIX_Free(&slitTmp,"mediateRingCalculate");

  #if defined(__DEBUG_) && __DEBUG_
  {
      // Declarations

   time_t today;                                                                // current date and time as a time_t number
   char datetime[20];                                                           // current date and time as a string

   // Get the current date and time

   today=time(NULL);

   // Convert into a string

   strftime(datetime,20,"%d/%m/%Y %H:%M:%S",localtime(&today));
   DEBUG_Print("End at %s\n",datetime);
  }
  #endif

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Stop("Ring");
  #endif

  // Return
  
  if (rc)
   ERROR_DisplayMessage(responseHandle);

  return rc;
 }

// ==================
// UNDERSAMPLING TOOL
// ==================

// -----------------------------------------------------------------------------
// FUNCTION      mediateRequestUsamp
// -----------------------------------------------------------------------------
// PURPOSE       Transfer the convolution options from the GUI to the engine
//
// RETURN        ERROR_ID_NO if no error found
// -----------------------------------------------------------------------------

RC mediateRequestUsamp(void *engineContext,mediate_usamp_t *pMediateUsamp,void *responseHandle)
 {
     // Declarations

  ENGINE_XSCONV_CONTEXT *pEngineContext = (ENGINE_XSCONV_CONTEXT*)engineContext;// pointer to the engine context
  SLIT *pSlitConv;                                                              // pointer to the convolution part of the engine context
  RC rc;

  // Initializations

  rc=ERROR_ID_NO;

  // General information

  pEngineContext->noComment=pMediateUsamp->noheader;
  pEngineContext->fraction=pMediateUsamp->shift;
  pEngineContext->analysisMethod=pMediateUsamp->methodType;

  strcpy(pEngineContext->path,pMediateUsamp->outputPhaseOneFile);               // output path
  strcpy(pEngineContext->path2,pMediateUsamp->outputPhaseTwoFile);              // output path

  strcpy(pEngineContext->calibrationFile,pMediateUsamp->calibrationFile);        // calibration file
  strcpy(pEngineContext->kuruczFile,pMediateUsamp->solarRefFile);                // Kurucz file used when I0 correction is applied

  // Description of the slit function

  setMediateSlit(&pEngineContext->slitConv,&pMediateUsamp->slit);

  if (!strlen(pEngineContext->calibrationFile))
   rc=ERROR_SetLast("mediateRequestUsamp",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Calibration","Calibration file name is missing");
  else
   {
       pSlitConv=&pEngineContext->slitConv;

       // Convolution slit function

       if (((pSlitConv->slitType==SLIT_TYPE_FILE) || pSlitConv->slitWveDptFlag) && !strlen(pSlitConv->slitFile))
     rc=ERROR_SetLast("mediateRequestUsamp",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Slit Function Type","Convolution slit function file is missing");

    else if ((pSlitConv->slitType==SLIT_TYPE_INVPOLY) &&
            ((pSlitConv->slitParam2<=(double)0.) ||
             (pSlitConv->slitParam2-floor(pSlitConv->slitParam2)!=(double)0.) ||
             (fmod(pSlitConv->slitParam2,(double)2.)!=(double)0.)))

     rc=ERROR_SetLast("mediateRequestUsamp",ERROR_TYPE_FATAL,ERROR_ID_MEDIATE,"Degree","Polynomial degree should be a positive integer and a multiple of 2");
   }
   
  if (rc)
   ERROR_DisplayMessage(responseHandle);

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION      UsampWriteHeader
// -----------------------------------------------------------------------------
// PURPOSE       Write options in the file header
//
// INPUT         pEngineContext : pointer to the current engine context
//               fp    : pointer to the output file
//               phase : phase of the undersampling
// -----------------------------------------------------------------------------

void UsampWriteHeader(ENGINE_XSCONV_CONTEXT *pEngineContext,FILE *fp,int phase)
 {
     // Declaration

  int slitType;

  // Initialization

  slitType=pEngineContext->slitConv.slitType;

  // Header

  fprintf(fp,";\n");
  fprintf(fp,"; UNDERSAMPLING CROSS SECTION (Phase %d)\n",phase);
  fprintf(fp,"; High resolution Kurucz file : %s\n",pEngineContext->kuruczFile);
  fprintf(fp,"; Calibration file : %s\n",pEngineContext->calibrationFile);

  fprintf(fp,"; Slit function type : %s %s\n",XSCONV_slitTypes[(slitType=pEngineContext->slitConv.slitType)],(pEngineContext->slitConv.slitWveDptFlag)?"wavelength dependent":"");

  if (slitType==SLIT_TYPE_FILE)
   fprintf(fp,"; Slit function file : %s\n",pEngineContext->slitConv.slitFile);
  else if (pEngineContext->slitConv.slitWveDptFlag)
   {
    if (strlen(pEngineContext->slitConv.slitFile))
     fprintf(fp,"; Slit function file : %s\n",pEngineContext->slitConv.slitFile);
    if (strlen(pEngineContext->slitConv.slitFile2))
     fprintf(fp,"; Slit function file 2 : %s\n",pEngineContext->slitConv.slitFile2);
    if (strlen(pEngineContext->slitConv.slitFile3))
     fprintf(fp,"; Slit function file 3 : %s\n",pEngineContext->slitConv.slitFile3);          // Super gaussian
   }
  else
   {
    if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_AGAUSS) || (slitType==SLIT_TYPE_SUPERGAUSS))
     fprintf(fp,"; Gaussian FWHM : %.3f\n",pEngineContext->slitConv.slitParam);
    if (slitType==SLIT_TYPE_ERF)
     fprintf(fp,"; Boxcar width : %.3f\n",pEngineContext->slitConv.slitParam2);
    if (slitType==SLIT_TYPE_AGAUSS)
     fprintf(fp,"; Asymmetry factor : %.3f\n",pEngineContext->slitConv.slitParam2);
    if (slitType==SLIT_TYPE_SUPERGAUSS)
     {
         fprintf(fp,"; Exponential term : %.3f\n",pEngineContext->slitConv.slitParam2);
      fprintf(fp,"; Asymmetry factor : %.3f\n",pEngineContext->slitConv.slitParam3);
     }

    if ((slitType== SLIT_TYPE_APOD) || (slitType== SLIT_TYPE_APODNBS))
     {
      fprintf(fp,"; Resolution : %.3lf\n",pEngineContext->slitConv.slitParam);
      fprintf(fp,"; Phase      : %.3lf\n",pEngineContext->slitConv.slitParam2);
     }

    if (slitType==SLIT_TYPE_VOIGT)
     {
      fprintf(fp,"; Gaussian FWHM : %.3f\n",pEngineContext->slitConv.slitParam);
      fprintf(fp,"; Lorentz/Gauss ratio : %.3f\n",pEngineContext->slitConv.slitParam2);
     }
   }

  if (slitType==SLIT_TYPE_INVPOLY)
   fprintf(fp,"; Polynomial degree : %d\n",(int)pEngineContext->slitConv.slitParam2);

  fprintf(fp,"; Analysis method : %s\n",mediateUsampAnalysisMethod[pEngineContext->analysisMethod]);
  fprintf(fp,"; Shift : %g\n",pEngineContext->fraction);

  fprintf(fp,";\n");
 }

// --------------------------------------------------------
// mediateUsampSave : Save the undersampling cross sections
// --------------------------------------------------------

RC mediateUsampSave(ENGINE_XSCONV_CONTEXT *pEngineContext,char *fileName,int phase,double *lambda,double *usampXS,int nSize,void *responseHandle)
 {
     // Declarations

     FILE *fp;                                                                     // pointer to the output file
     int i;                                                                        // browse data points of the cross section
     RC rc;                                                                        // return code

     // Initializations

     rc=ERROR_ID_NO;

     // Open file

  if ((fp=fopen(fileName,"w+t"))==NULL)
   rc=ERROR_SetLast("mediateUsampSave",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,fileName);
  else
   {
    if (!pEngineContext->noComment)
     UsampWriteHeader(pEngineContext,fp,phase);

    for (i=0;i<nSize;i++)
     fprintf(fp,"%.14le %.14le\n",lambda[i],usampXS[i]);

    fclose(fp);
   }

  // Return

  return rc;
 }

// ---------------------------------------------------------------------------
// mediateUsampCalculate : Main function to build undersampling cross sections
// ---------------------------------------------------------------------------

RC mediateUsampCalculate(void *engineContext,void *responseHandle)
{
  ENGINE_XSCONV_CONTEXT *pEngineContext=(ENGINE_XSCONV_CONTEXT*)engineContext;
  MATRIX_OBJECT calibrationMatrix,kuruczMatrix;
  double *phase1,*phase2;
  int hrN,fftSize,nSize;
  double *fftIn;
  INDEX i;
  RC rc;

  // Initializations

  memset(&calibrationMatrix,0,sizeof(MATRIX_OBJECT));
  memset(&kuruczMatrix,0,sizeof(MATRIX_OBJECT));
  memset(&usampFFT,0,sizeof(FFT));
  phase1=phase2=NULL;

  // Files read out

  if (!strlen(pEngineContext->calibrationFile) ) {
    return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_SPECIFIED,"calibration file");
  }
  if (!strlen(pEngineContext->kuruczFile) ) {
    return ERROR_SetLast(__func__,ERROR_TYPE_FATAL,ERROR_ID_FILE_NOT_SPECIFIED,"high resolution solar reference");
  }

  if (!(rc=MATRIX_Load(pEngineContext->calibrationFile,&calibrationMatrix,0,0,(double)0.,(double)0.,0,1,"mediateUsampCalculate (calibration file) ")) &&
      !(rc=MATRIX_Load(pEngineContext->kuruczFile,&kuruczMatrix,0,0,(double)calibrationMatrix.matrix[0][0]-7.,
       (double)calibrationMatrix.matrix[0][calibrationMatrix.nl-1]+7.,1,1,"mediateUsampCalculate (Kurucz) ")) &&
      !(rc=VECTOR_NormalizeVector(kuruczMatrix.matrix[1]-1,kuruczMatrix.nl,NULL,"mediateUsampCalculate ")))
   {
    hrN=usampFFT.oldSize=kuruczMatrix.nl;
    nSize=calibrationMatrix.nl;
    fftSize=usampFFT.fftSize=(int)((double)pow((double)2.,ceil(log((double)hrN)/log((double)2.))));

    if (((phase1=MEMORY_AllocDVector("mediateUsampCalculate ","phase 1",0,nSize-1))==NULL) ||
        ((phase2=MEMORY_AllocDVector("mediateUsampCalculate ","phase 2",0,nSize-1))==NULL) ||
        ((fftIn=usampFFT.fftIn=(double *)MEMORY_AllocDVector("mediateUsampCalculate ","fftIn",1,fftSize))==NULL) ||
        ((usampFFT.fftOut=(double *)MEMORY_AllocDVector("mediateUsampCalculate ","fftOut",1,fftSize))==NULL) ||
        ((usampFFT.invFftIn=(double *)MEMORY_AllocDVector("mediateUsampCalculate ","invFftIn",1,fftSize))==NULL) ||
        ((usampFFT.invFftOut=(double *)MEMORY_AllocDVector("mediateUsampCalculate ","invFftOut",1,fftSize))==NULL))

     rc=ERROR_ID_ALLOC;

    else
     {
      memcpy(fftIn+1,kuruczMatrix.matrix[1],sizeof(double)*hrN);

      for (i=hrN+1;i<=fftSize;i++)
       fftIn[i]=fftIn[2*hrN-i];

      realft(usampFFT.fftIn,usampFFT.fftOut,fftSize,1);

      memcpy(fftIn+1,kuruczMatrix.matrix[0],sizeof(double)*hrN);                // Reuse fftIn for high resolution wavelength safe keeping

      rc=USAMP_Build(phase1,                                                     // OUTPUT : phase 1 calculation
                    phase2,                                                     // OUTPUT : phase 2 calculation
                    calibrationMatrix.matrix[0],                                // GOME calibration
                    nSize,                                                      // size of GOME calibration
                   &kuruczMatrix,                                               // solar spectrum
                   &pEngineContext->slitConv,                                   // slit function
           (double) pEngineContext->fraction,                                   // tunes the phase
                    pEngineContext->analysisMethod);                            // analysis method
     }

    if (!rc)
     {
         // Local declarations

         plot_data_t spectrumData[2];
         char pageTitle[MAX_ITEM_TEXT_LEN];

         // Save

         if (!(rc=mediateUsampSave(pEngineContext,pEngineContext->path,1,calibrationMatrix.matrix[0],phase1,nSize,responseHandle)))
          rc=mediateUsampSave(pEngineContext,pEngineContext->path2,2,calibrationMatrix.matrix[0],phase2,nSize,responseHandle);

         // Plot

         sprintf(pageTitle,"Undersampling");

      mediateAllocateAndSetPlotData(&spectrumData[0],"Phase 1",calibrationMatrix.matrix[0],phase1,nSize,Line);
      mediateAllocateAndSetPlotData(&spectrumData[1],"Phase 2",calibrationMatrix.matrix[0],phase2,nSize,Line);
      mediateResponsePlotData(0,spectrumData,2,Spectrum,forceAutoScale,"Calculated undersampling cross sections","Wavelength (nm)","",responseHandle);
      mediateResponseLabelPage(0,pageTitle,"",responseHandle);
      mediateReleasePlotData(&spectrumData[1]);
      mediateReleasePlotData(&spectrumData[0]);
     }
   }

  // Buffers release

  if (phase1!=NULL)
   MEMORY_ReleaseDVector("mediateUsampCalculate ","phase1",phase1,0);
  if (phase2!=NULL)
   MEMORY_ReleaseDVector("mediateUsampCalculate ","phase2",phase2,0);

  if (usampFFT.fftIn!=NULL)
   MEMORY_ReleaseDVector("mediateUsampCalculate ","fftIn",usampFFT.fftIn,1);
  if (usampFFT.fftOut!=NULL)
   MEMORY_ReleaseDVector("mediateUsampCalculate ","fftOut",usampFFT.fftOut,1);
  if (usampFFT.invFftIn!=NULL)
   MEMORY_ReleaseDVector("mediateUsampCalculate ","invFftIn",usampFFT.invFftIn,1);
  if (usampFFT.invFftOut!=NULL)
   MEMORY_ReleaseDVector("mediateUsampCalculate ","invFftOut",usampFFT.invFftOut,1);

  MATRIX_Free(&calibrationMatrix,"mediateUsampCalculate");
  MATRIX_Free(&kuruczMatrix,"mediateUsampCalculate");
  
  if (rc)
   ERROR_DisplayMessage(responseHandle);

  return rc;
 }

// ============
// TOOL CONTEXT
// ============

// -----------------------------------------------------------------------------
// FUNCTION      mediateXsconvCreateContext
// -----------------------------------------------------------------------------
// PURPOSE       This function is called when one of the convolution tool is
//               started.  It creates a single context for safely accessing its
//               features through the mediator layer.  The engine context is
//               never destroyed before the user exits the program.
//
// RETURN        On success 0 is returned and the value of handleEngine is set,
//               otherwise -1 is retured and the value of handleEngine is undefined.
// -----------------------------------------------------------------------------

int mediateXsconvCreateContext(void **engineContext, void *responseHandle)
 {
     ENGINE_XSCONV_CONTEXT *pEngineContext;

     *engineContext=(void *)EngineXsconvCreateContext();
     pEngineContext=(ENGINE_XSCONV_CONTEXT *)*engineContext;

  if (pEngineContext==NULL)
   ERROR_DisplayMessage(responseHandle);

  return (pEngineContext!=NULL)?0:-1;
 }

// -----------------------------------------------------------------------------
// FUNCTION      mediateXsconvDestroyContext
// -----------------------------------------------------------------------------
// PURPOSE       Destroy the engine context when the user exits the program.
//
// RETURN        Zero is returned on success, -1 otherwise.
// -----------------------------------------------------------------------------

int mediateXsconvDestroyContext(void *engineContext, void *responseHandle)
 {
  return (!EngineXsconvDestroyContext((ENGINE_XSCONV_CONTEXT *)engineContext))?0:-1;
 }
