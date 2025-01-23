
/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

*/

#include <string.h>
#include <math.h>
#include <time.h>

#include "mediate_response.h"
#include "mediate_types.h"
#include "mediate_xsconv.h"
#include "mediate_common.h"
#include "mediate_xsconv_output_netcdf.h"

#include "engine_xsconv.h"
#include "xsconv.h"
#include "filter.h"
#include "vector.h"
#include "spline.h"
#include "raman.h"
#include "usamp.h"

// ================
// STATIC VARIABLES
// ================

const char *mediateConvolutionTypesStr[CONVOLUTION_TYPE_MAX]=
 {
  "Interpolation only",
  "Standard convolution",
  "Convolution with I0 correction"
 };

const char *mediateConvolutionFileExt[CONVOLUTION_TYPE_MAX]=
   {
    "_none",                                                                     // CONVOLUTION_TYPE_NONE
    "_std",                                                                       // CONVOLUTION_TYPE_STANDARD
    "_i0",                                                                        // CONVOLUTION_TYPE_I0_CORRECTION
  //  "_ring"                         // CONVOLUTION_TYPE_RING
   };

const char *mediateConvolutionFilterTypes[PRJCT_FILTER_TYPE_MAX]={"None","Kaiser","Boxcar","Gaussian","Triangular","Savitzky-Golay","Odd-even pixels correction","Binomial"};
const char *mediateUsampAnalysisMethod[PRJCT_ANLYS_METHOD_MAX]={"Optical density","Intensity fitting"};

FFT    usampFFT;

// ---------------------------------------------------------------
// mediateConvolutionSaveAscii : Save the convoluted cross section
// ----------------------------------------------------------------

RC mediateConvolutionSaveAscii(void *engineContext)
 {
  // Declarations

  ENGINE_XSCONV_CONTEXT *pEngineContext=(ENGINE_XSCONV_CONTEXT*)engineContext;
  char fileName[MAX_ITEM_TEXT_LEN];
  PRJCT_FILTER *pLFilter,*pHFilter;
  SZ_LEN fileNameLength;
  char *ptr,*ptr2;
  FILE *fp;
  INDEX i,j,slitType;
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

        fprintf(fp,"; Number of ground pixels : %d\n",pEngineContext->n_groundpixel_output);
        fprintf(fp,";\n; Columns description :\n");
        fprintf(fp,"; Column 1 : calibration;\n");
         
        if (pEngineContext->n_groundpixel_output==1)
          fprintf(fp,"; Column 2 : %s;\n",((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))?
                     "convoluted and filtered cross section" : "convoluted cross section");
        else
           fprintf(fp,"; Column 2-%d : %s;\n",pEngineContext->n_groundpixel_output+1,((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))?
                      "convoluted and filtered cross section" : "convoluted cross section");
 
        if ((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))  // !!! to test later for imagers !!!!
         {
          if (pEngineContext->n_groundpixel_output==1)
           fprintf(fp,"; Column 3 : convoluted only cross section.\n");  
          else
           fprintf(fp,"; Column %d-%d : convoluted only cross section.\n",pEngineContext->n_groundpixel_output+2,2*pEngineContext->n_groundpixel_output+1);
         }
        fprintf(fp,";\n");
       }

      // Case 1 : output only only one ground pixel

      if (pEngineContext->n_groundpixel_output==1)
       {   
        if ((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))   
          for (i=nextraPixels;i<nsize;i++)                                                          
           fprintf(fp,"%.14le %.14le %.14le\n",pXs->matrix[0][i],filterVector[i],pXs->matrix[1][i]); 
        else
         for (i=nextraPixels;i<nsize;i++)
          fprintf(fp,"%.14le %.14le\n",pXs->matrix[0][i],pXs->matrix[1][i]);
       }
       
      // Case 2 : duplicate the same cross section (the number of ground pixels)
      
      else if (pEngineContext->n_groundpixel_slit==1)
       {
        for (i=nextraPixels;i<nsize;i++)   
         {
          fprintf(fp,"%.14le ",pXs->matrix[0][i]);
          if ((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))   
           for (j=1;j<=pEngineContext->n_groundpixel_output;j++)
            fprintf(fp,"%.14le ",filterVector[i]);
          for (j=1;j<=pEngineContext->n_groundpixel_output;j++)
           fprintf(fp,"%.14le ",pXs->matrix[1][i]);
          fprintf(fp,"\n");
         }
       }  
         
      // case 3 : cross sections is convolved with a slit function for imagers   
               
      else
       {
        for (i=nextraPixels;i<nsize;i++)
         {
          fprintf(fp,"%.14le ",pXs->matrix[0][i]);
          if ((pLFilter->type!=PRJCT_FILTER_TYPE_NONE) || (pHFilter->type!=PRJCT_FILTER_TYPE_NONE))   
           for (j=1;j<=pEngineContext->n_groundpixel_output;j++)
            fprintf(fp,"%.14le ",filterVector[i]);                              // filterVector should be a matrix for imagers
          for (j=1;j<=pEngineContext->n_groundpixel_output;j++)
           fprintf(fp,"%.14le ",pXs->matrix[j][i]);
         } 
       }

      fclose(fp);
     }
    else
     rc=ERROR_SetLast("mediateConvolutionSaveAscii",ERROR_TYPE_FATAL,ERROR_ID_FILE_OPEN,fileName);
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
                XSCONV_xsnew,                                                   // convolved cross section
                XSCONV_kurucz;                                                  // kurucz

  PRJCT_FILTER *plFilter,*phFilter;                                             // pointers to the low pass and high pass filtering parts of the engine context
  SLIT *pSlitConv,*pSlitDConv;                                                  // pointers to the convolution and deconvolution slit function parts of the engine context

  char windowTitle[MAX_ITEM_TEXT_LEN],pageTitle[MAX_ITEM_TEXT_LEN];
  double lambdaMin,lambdaMax,slitParam[NSFP],slitParamD[NSFP],slitWidth,*tmpVector;

  int slitType,slitType2,deconvFlag,dispConv;
  int lowFilterType,highFilterType,nFilter,i,islit;
  RC rc;

  // Slit function

  pSlitDConv=&pEngineContext->slitDConv;
  pSlitConv=&pEngineContext->slitConv;

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

  memset(&XSCONV_xsnew,0,sizeof(MATRIX_OBJECT));
  memset(&pEngineContext->xsNew,0,sizeof(MATRIX_OBJECT));
  
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
  
  pEngineContext->n_groundpixel_slit=1;
  
  // Load calibration file and slit function

  if (((pEngineContext->convolutionType==CONVOLUTION_TYPE_NONE) ||
     (!(rc=XSCONV_LoadSlitFunction(XSCONV_slitMatrix,&pEngineContext->slitConv,&slitParam[0],&slitType)) &&
     (!deconvFlag || !(rc=XSCONV_LoadSlitFunction(XSCONV_slitDMatrix,pSlitDConv,&slitParamD[0],&slitType2))))) &&
      !(rc=XSCONV_LoadCalibrationFile(&XSCONV_xsnew,pEngineContext->calibrationFile,nFilter)) &&
     (((lowFilterType==PRJCT_FILTER_TYPE_NONE) && (highFilterType==PRJCT_FILTER_TYPE_NONE)) ||
     (((pEngineContext->filterVector=(double *)MEMORY_AllocDVector("mediateConvolutionCalculate","filterVector",0,XSCONV_xsnew.nl-1))!=NULL) &&
      ((tmpVector=(double *)MEMORY_AllocDVector("mediateConvolutionCalculate","tmpVector",0,XSCONV_xsnew.nl-1))!=NULL))) &&
      !(rc=MATRIX_Allocate(&pEngineContext->xsNew,XSCONV_xsnew.nl,pEngineContext->n_groundpixel_slit+1,0,0,1,"mediateConvolutionCalculate"))
      )
   {
    slitWidth=(double)3.*slitParam[0];
    
    pEngineContext->n_groundpixel_output=(pEngineContext->n_groundpixel_slit==1)?pEngineContext->n_groundpixel_general:pEngineContext->n_groundpixel_slit;
   
    // Window in wavelength

    if ((slitType!=SLIT_TYPE_FILE) || (pEngineContext->convolutionType==CONVOLUTION_TYPE_NONE))
     {
      lambdaMin=XSCONV_xsnew.matrix[0][0]-slitWidth-1.;                     // add 1 nm
      lambdaMax=XSCONV_xsnew.matrix[0][XSCONV_xsnew.nl-1]+slitWidth+1.;
     }
    else
     {
      lambdaMin=XSCONV_xsnew.matrix[0][0]+XSCONV_slitMatrix[0].matrix[0][0]-1.;                     // add 1 nm
      lambdaMax=XSCONV_xsnew.matrix[0][XSCONV_xsnew.nl-1]+XSCONV_slitMatrix[0].matrix[0][XSCONV_slitMatrix[0].nl-1]+1.;
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
                (XSCONV_xshr.nl!=XSCONV_xsnew.nl) ||
                !VECTOR_Equal(XSCONV_xshr.matrix[0],XSCONV_xsnew.matrix[0],XSCONV_xsnew.nl,(double)1.e-7))?1:0;
            
      memcpy(pEngineContext->xsNew.matrix[0],XSCONV_xsnew.matrix[0],sizeof(double)*XSCONV_xsnew.nl);
      
      for (islit=1;islit<=pEngineContext->n_groundpixel_slit;islit++)
       {
        for (i=0;i<XSCONV_xsnew.nl;i++)
         XSCONV_xsnew.matrix[1][i]=(double)0.;
            
        // -----------
        // Convolution
        // -----------

        switch(pEngineContext->convolutionType)
         {
       // ----------------------------------------------------------------------
          case CONVOLUTION_TYPE_NONE :
           rc=XSCONV_TypeNone(&XSCONV_xsnew,&XSCONV_xshr);
          break;
       // ----------------------------------------------------------------------
          case CONVOLUTION_TYPE_STANDARD :
           rc=XSCONV_TypeStandard(&XSCONV_xsnew,0,XSCONV_xsnew.nl,&XSCONV_xshr,&XSCONV_xshr,NULL,slitType,XSCONV_slitMatrix,slitParam,pSlitConv->slitWveDptFlag);
           break;
       // ----------------------------------------------------------------------
          case CONVOLUTION_TYPE_I0_CORRECTION :
            rc=XSCONV_TypeI0Correction(&XSCONV_xsnew,&XSCONV_xshr,&XSCONV_kurucz,pEngineContext->conc,slitType,XSCONV_slitMatrix,slitParam,pSlitConv->slitWveDptFlag);
          break;
       // ----------------------------------------------------------------------
         }
         
        memcpy(pEngineContext->xsNew.matrix[islit],XSCONV_xsnew.matrix[1],sizeof(double)*XSCONV_xsnew.nl);

        // ------------------------------------------
        // Save resulting cross section and plot data
        // ------------------------------------------

        if ((lowFilterType==PRJCT_FILTER_TYPE_NONE) && (highFilterType==PRJCT_FILTER_TYPE_NONE))
         sprintf(pageTitle,(pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)?"Spectrum after convolution":"Spectrum after interpolation");
        else
         sprintf(pageTitle,(pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)?"Spectrum after convolution and filtering":"Spectrum after interpolation and filtering");

        if (!rc && dispConv)
         {
          if (pEngineContext->n_groundpixel_slit==1)
           sprintf(windowTitle,(pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)?"Spectrum after convolution":"Spectrum after interpolation");
          else
           sprintf(windowTitle,(pEngineContext->convolutionType!=CONVOLUTION_TYPE_NONE)?"Spectrum after convolution (%d)":"Spectrum after interpolation (%d)",islit);

          MEDIATE_PLOT_CURVES(0,Spectrum,forceAutoScale,windowTitle,"Wavelength (nm)","",responseHandle,
                              CURVE(.name="High resolution spectrum", .x=XSCONV_xshr.matrix[0], .y=XSCONV_xshr.matrix[1], .length=XSCONV_xshr.nl),
                              CURVE(.name=windowTitle, .x=XSCONV_xsnew.matrix[0]+nFilter,.y=XSCONV_xsnew.matrix[1]+nFilter,.length=XSCONV_xsnew.nl-2*nFilter));
          mediateResponseLabelPage(0,pageTitle,"",responseHandle);
         }

        if (pEngineContext->filterVector!=NULL)
         memcpy(pEngineContext->filterVector,XSCONV_xsnew.matrix[1],XSCONV_xsnew.nl*sizeof(double));

        // ------------------
        // Low-Pass filtering
        // ------------------

        if ((lowFilterType!=PRJCT_FILTER_TYPE_NONE) && (pEngineContext->filterVector!=NULL) && !rc &&
          (((lowFilterType==PRJCT_FILTER_TYPE_ODDEVEN) && !(rc=FILTER_OddEvenCorrection(XSCONV_xsnew.matrix[0],XSCONV_xsnew.matrix[1],pEngineContext->filterVector,XSCONV_xsnew.nl))) ||
           ((lowFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) && !(rc=FILTER_Vector(plFilter,pEngineContext->filterVector,pEngineContext->filterVector,NULL,XSCONV_xsnew.nl,PRJCT_FILTER_OUTPUT_LOW)))) && dispConv)
         {
          if (pEngineContext->n_groundpixel_slit==1) 
           sprintf(windowTitle,"Spectrum after low-pass filtering");
          else
           sprintf(windowTitle,"Spectrum after low-pass filtering (%d)",islit);

          MEDIATE_PLOT_CURVES(0,Spectrum,forceAutoScale,windowTitle,"Wavelength (nm)","",responseHandle,
                              CURVE(.name="Convolved spectrum before low pass filtering", .x=XSCONV_xsnew.matrix[0]+nFilter, .y=XSCONV_xsnew.matrix[1]+nFilter, .length=XSCONV_xsnew.nl-2*nFilter),
                              CURVE(.name="Convolved spectrum after low pass filtering", .x=XSCONV_xsnew.matrix[0]+nFilter, .y=pEngineContext->filterVector+nFilter, .length=XSCONV_xsnew.nl-2*nFilter));
          mediateResponseLabelPage(0,pageTitle,"",responseHandle);
         }

        // -------------------
        // High-Pass filtering
        // -------------------

        if ((highFilterType!=PRJCT_FILTER_TYPE_NONE) && (highFilterType!=PRJCT_FILTER_TYPE_ODDEVEN) && (pEngineContext->filterVector!=NULL) && !rc &&
           !(rc=FILTER_Vector(phFilter,pEngineContext->filterVector,pEngineContext->filterVector,tmpVector,XSCONV_xsnew.nl,phFilter->filterAction)) && dispConv)
         {
          if (pEngineContext->n_groundpixel_slit==1)
           sprintf(windowTitle,"Spectrum after high-pass filtering");
          else
           sprintf(windowTitle,"Spectrum after high-pass filtering (%d)",islit);

          MEDIATE_PLOT_CURVES(0,Spectrum,forceAutoScale,windowTitle,"Wavelength (nm)","",responseHandle,
                              CURVE(.name=windowTitle, .x=XSCONV_xsnew.matrix[0]+nFilter, .y=XSCONV_xsnew.matrix[1]+nFilter, .length=XSCONV_xsnew.nl-2*nFilter),
                              CURVE(.name=windowTitle, .x=XSCONV_xsnew.matrix[0]+nFilter, .y=tmpVector+nFilter, .length=XSCONV_xsnew.nl-2*nFilter));
          mediateResponseLabelPage(0,pageTitle,"",responseHandle);

          if (pEngineContext->n_groundpixel_slit==1)
           sprintf(windowTitle,"Spectrum after %s",(phFilter->filterAction==PRJCT_FILTER_OUTPUT_HIGH_SUB)?"subtraction":"division");
          else
           sprintf(windowTitle,"Spectrum after %s (%d)",(phFilter->filterAction==PRJCT_FILTER_OUTPUT_HIGH_SUB)?"subtraction":"division",islit);

          MEDIATE_PLOT_CURVES(1,Spectrum,forceAutoScale,windowTitle,"Wavelength (nm)","",responseHandle,
                              CURVE(.name=windowTitle, .x=XSCONV_xsnew.matrix[0]+nFilter, .y=pEngineContext->filterVector+nFilter, .length=XSCONV_xsnew.nl-2*nFilter));
          mediateResponseLabelPage(1,pageTitle,"",responseHandle);
         }
       }

      // Result safe keeping
      
      if (!rc)
       rc=(pEngineContext->formatType==CONVOLUTION_FORMAT_NETCDF)?netcdf_save_convolution((void *)pEngineContext):mediateConvolutionSaveAscii((void *)pEngineContext);
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
  MATRIX_Free(&XSCONV_xsnew,"mediateConvolutionCalculate");
  MATRIX_Free(&pEngineContext->xsNew,"mediateConvolutionCalculate");

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
  pEngineContext->formatType=pMediateConvolution->general.formatType;
  pEngineContext->n_groundpixel_general=pMediateConvolution->general.n_groundpixel;
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

  pEngineContext->formatType=pMediateRing->formatType;
  pEngineContext->n_groundpixel_general=pMediateRing->n_groundpixel;

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
// FUNCTION      mediateRingSaveAscii
// -----------------------------------------------------------------------------
// PURPOSE       Save the ring cross section in an ASCII file
//
// INPUT         pEngineContext : pointer to the current engine context
// -----------------------------------------------------------------------------

int mediateRingSaveAscii(void *engineContext,double *raman,double *solar)
 {
  // Declarations

  ENGINE_XSCONV_CONTEXT *pEngineContext=(ENGINE_XSCONV_CONTEXT*)engineContext; 
  FILE *fp;  
  int   slitType;                                                 // type of the slit function
  int   i,islit;
  int   nring;
  
  nring=pEngineContext->xsNew.nl;
  
  if ((fp=fopen(pEngineContext->path,"w+t"))!=NULL)
   {
    if (!pEngineContext->noComment)
     {
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

      if (pEngineContext->saveRaman)
       {
        fprintf(fp,"; column 2 : ring (raman/solar spectrum)\n");
        fprintf(fp,"; column 3 : interpolated raman\n");
        fprintf(fp,"; column 4 : convoluted solar spectrum\n");
       }
      else if (pEngineContext->n_groundpixel_output==1)
       fprintf(fp,"; column 2 : ring (raman/solar spectrum)\n");
      else
       fprintf(fp,"; Column 2-%d : ring (raman/solar spectrum)\n",pEngineContext->n_groundpixel_output+1);

      fprintf(fp,";\n");
     }

    if (pEngineContext->saveRaman)
     for (i=0;i<nring;i++)
      fprintf(fp,"%.14le %.14le %.14le %.14le\n",pEngineContext->xsNew.matrix[0][i],pEngineContext->xsNew.matrix[1][i],raman[i],solar[i]);
    else if (pEngineContext->n_groundpixel_output==1)
     for (i=0;i<nring;i++)
      fprintf(fp,"%.14le %.14le\n",pEngineContext->xsNew.matrix[0][i],pEngineContext->xsNew.matrix[1][i]);
    else if (pEngineContext->n_groundpixel_slit==1)
     
     for (i=0;i<nring;i++)
      {
       fprintf(fp,"%.14le ",pEngineContext->xsNew.matrix[0][i]);
       for (islit=1;islit<=pEngineContext->n_groundpixel_output;islit++)
        fprintf(fp,"%.14le ",pEngineContext->xsNew.matrix[1][i]);
       fprintf(fp,"\n");
      }
      
    else
      
     for (i=0;i<nring;i++)
      {
       fprintf(fp,"%.14le ",pEngineContext->xsNew.matrix[0][i]);
       for (islit=1;islit<=pEngineContext->n_groundpixel_output;islit++)
        fprintf(fp,"%.14le ",pEngineContext->xsNew.matrix[islit][i]);
       fprintf(fp,"\n");
      }
   }  
          
  if (fp!=NULL)
   fclose(fp);        
  
  return 0;
 }

// -------------------------------------------------------------------
// mediateRingCalculate : Main function to create a ring cross section
// -------------------------------------------------------------------

RC mediateRingCalculate(void *engineContext,void *responseHandle)
 {
  // Declarations

  MATRIX_OBJECT slitTmp;
  ENGINE_XSCONV_CONTEXT *pEngineContext=(ENGINE_XSCONV_CONTEXT*)engineContext;
  char     pageTitle[MAX_ITEM_TEXT_LEN];
  double  slitWidth,                                                            // width of the slit function
         *raman,*raman2,*ramanint,                                              // output ring cross section
         *solarLambda,*solarVector,*solarDeriv2,                                // substitution vectors for solar spectrum
         *slitLambda,*slitVector,*slitDeriv2,                                   // substitution vectors for slit function
         *slitLambda2,*slitVector2,*slitDeriv22,                                // substitution vectors for slit function
         *ringLambda,*ringVector,                                               // substitution vectors for ring cross section
          temp,                                                                 // approximate average temperature in �K for scattering
          slitParam[NSFP];                                                      // gaussian full width at half maximum

  MATRIX_OBJECT  xsSolar,xsSolarConv,xsSlit[NSFP],xsRing,*pSlit,*pSlit2;// solar spectrum and slit function
  int     nsolar,nring,                                            // size of previous vectors
          wveDptFlag,
          slitType;                                                             // type of the slit function
  INDEX   i,islit;                                                            // indexes for loops and arrays
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
  memset(&pEngineContext->xsNew,0,sizeof(MATRIX_OBJECT));

  slitParam[0]=pEngineContext->slitConv.slitParam;
  slitParam[1]=pEngineContext->slitConv.slitParam2;
  slitParam[2]=pEngineContext->slitConv.slitParam3;

  slitLambda=slitVector=slitDeriv2=NULL;
  slitWidth=(double)RING_SLIT_WIDTH;                                            // NB : force slit width to 6 because of convolutions
  slitType=pEngineContext->slitConv.slitType;
  raman=raman2=ramanint=NULL;
  slitLambda2=slitVector2=slitDeriv22=NULL;
  temp=(double)pEngineContext->temperature;                                        // (double)250.;   May 2005/05/31
  
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

  pEngineContext->n_groundpixel_slit=1;

  if (!(rc=XSCONV_LoadCalibrationFile(&xsRing,pEngineContext->calibrationFile,0)) &&

  // Load slit function from file or pre-calculate the slit function

          ((slitType==SLIT_TYPE_NONE) || !(rc=XSCONV_LoadSlitFunction(xsSlit,&pEngineContext->slitConv,&slitParam[0],&slitType))) &&

  // Load solar spectrum in the range of wavelengths covered by the end wavelength scale corrected by the slit function

           !(rc=XSCONV_LoadCrossSectionFile(&xsSolar,pEngineContext->kuruczFile,(double)xsRing.matrix[0][0]-slitWidth-1.,
                                     (double)xsRing.matrix[0][xsRing.nl-1]+slitWidth+1.,(double)0.,CONVOLUTION_CONVERSION_NONE)) &&

           !(rc=MATRIX_Allocate(&xsSolarConv,xsSolar.nl,2,0,0,1,"mediateRingCalculate")) &&
           !(rc=MATRIX_Allocate(&pEngineContext->xsNew,xsRing.nl,pEngineContext->n_groundpixel_slit+1,0,0,1,"mediateRingCalculate")))
   {
    pEngineContext->n_groundpixel_output=(pEngineContext->n_groundpixel_slit==1)?pEngineContext->n_groundpixel_general:pEngineContext->n_groundpixel_slit;
    
    // Use substitution variables

    memcpy(xsSolarConv.matrix[0],xsSolar.matrix[0],sizeof(double)*xsSolar.nl);
    memcpy(pEngineContext->xsNew.matrix[0],xsRing.matrix[0],sizeof(double)*xsRing.nl);

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

        memcpy(slitTmp.matrix[0],(double *)pSlit->matrix[0]+1,sizeof(double)*(pSlit->nl-1));
       }

      if (wveDptFlag && ((slitType==SLIT_TYPE_FILE) || (slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_VOIGT) || (slitType==SLIT_TYPE_AGAUSS) || (slitType==SLIT_TYPE_SUPERGAUSS)))
       {
        slitLambda2=pSlit2->matrix[0];
        slitVector2=pSlit2->matrix[1];
        slitDeriv22=pSlit2->deriv2[1];
       }
      else
       slitLambda2=slitVector2=slitDeriv22=NULL;
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
       ((ramanint=MEMORY_AllocDVector("mediateRingCalculate","ramanint",0,nring-1))==NULL))
     rc=ERROR_ID_ALLOC;
    else
     {
      for (islit=1;islit<=pEngineContext->n_groundpixel_slit;islit++)
       {
        VECTOR_Init(raman,(double)0.,nsolar);
        VECTOR_Init(raman2,(double)0.,nsolar);
        
        // Start convolving the solar spectrum

        if (((slitType!=SLIT_TYPE_NONE) && ((rc=XSCONV_TypeStandard(&xsSolarConv,0,xsSolarConv.nl,&xsSolar,&xsSolar,NULL,slitType,xsSlit,slitParam,pEngineContext->slitConv.slitWveDptFlag))!=ERROR_ID_NO)) ||
            ((rc=SPLINE_Deriv2(solarLambda,solarVector,solarDeriv2,nsolar,"mediateRingCalculate1"))!=0) ||
            ((rc=raman_convolution(solarLambda,solarVector,solarDeriv2,raman,nsolar,temp,pEngineContext->normalizeFlag))!=0) ||
            
        // Interpolate the high resolution spectrum
        
            ((rc=SPLINE_Deriv2(solarLambda,solarVector,solarDeriv2,nsolar,"mediateRingCalculate2"))!=0) ||
            ((rc=SPLINE_Vector(solarLambda,solarVector,solarDeriv2,nsolar,ringLambda,ringVector,nring,SPLINE_CUBIC))!=0) || 

        // Interpolate the raman spectrum

            ((rc=SPLINE_Deriv2(solarLambda,raman,raman2,nsolar,"mediateRingCalculate3"))!=0) ||
            ((rc=SPLINE_Vector(solarLambda,raman,raman2,nsolar,ringLambda,ramanint,nring,SPLINE_CUBIC))!=0))
            
         goto EndRing;
        
       #if defined(__DEBUG_) && __DEBUG_
       DEBUG_PrintVar("High resolution vectors",solarLambda,0,nsolar-1,solarVector,0,nsolar-1,raman,0,nsolar-1,NULL);
       #endif
       
       // The final ring is the ratio between the interpolated raman and the interpolated convoluted solar spectrum
       
       for (i=0;i<nring;i++)
        pEngineContext->xsNew.matrix[islit][i]=((ramanint[i]>(double)0.) && (ringVector[i]>(double)0.))?
                                  (double)ramanint[i] /* ring effect source spectrum */ /ringVector[i] /* solar spectrum */:(double)0.;       
       }
       
      // Result safe keeping
      
      if (!rc)
       rc=(pEngineContext->formatType==CONVOLUTION_FORMAT_NETCDF)?netcdf_save_ring((void *)pEngineContext):mediateRingSaveAscii((void *)pEngineContext,ramanint,ringVector);

      strcpy(pageTitle,"Ring"); 
      MEDIATE_PLOT_CURVES(0,Spectrum,forceAutoScale,"Calculated ring cross section","Wavelength (nm)","",responseHandle,
                          CURVE(.name="Calculated ring cross section", .x=ringLambda, .y=pEngineContext->xsNew.matrix[1], .length=nring));
      mediateResponseLabelPage(0,pageTitle,"",responseHandle);
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
      // Save

      if (!(rc=mediateUsampSave(pEngineContext,pEngineContext->path,1,calibrationMatrix.matrix[0],phase1,nSize,responseHandle)))
       rc=mediateUsampSave(pEngineContext,pEngineContext->path2,2,calibrationMatrix.matrix[0],phase2,nSize,responseHandle);

      // Plot

      MEDIATE_PLOT_CURVES(0,Spectrum,forceAutoScale,"Calculated undersampling cross sections","Wavelength (nm)","",responseHandle,
                          CURVE(.name="Phase 1", .x=calibrationMatrix.matrix[0], .y=phase1, .length=nSize),
                          CURVE(.name="Phase 2", .x=calibrationMatrix.matrix[0], .y=phase2, .length=nSize));
      mediateResponseLabelPage(0, "Undersampling", "", responseHandle);
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
