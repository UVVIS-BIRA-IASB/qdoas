
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Functions of the mediator that are common to the
//                       QDOAS application and tools
//  Name of module    :  mediate_common.c
//  Creation date     :  07/05/2008
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

#include <string.h>

#include "mediate_common.h"
#include "engine_context.h"

// -----------------------------------------------------------------------------
// FUNCTION      setMediateSlit
// -----------------------------------------------------------------------------
// PURPOSE       Slit function parameterization
// -----------------------------------------------------------------------------

void setMediateSlit(SLIT *pEngineSlit,const mediate_slit_function_t *pMediateSlit)
 {
  // Fields

  pEngineSlit->slitType=pMediateSlit->type;

  pEngineSlit->slitParam=
  pEngineSlit->slitParam2=
  pEngineSlit->slitParam3=
  pEngineSlit->slitParam4=(double)0.;
  pEngineSlit->slitWveDptFlag=0;

  pEngineSlit->slitFile[0]=pEngineSlit->slitFile2[0]=pEngineSlit->slitFile3[0]='\0';

  switch(pEngineSlit->slitType)
   {
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_NONE :
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_GAUSS :                                                      // Gaussian line shape

     pEngineSlit->slitParam=pMediateSlit->gaussian.fwhm;
     pEngineSlit->slitWveDptFlag=pMediateSlit->gaussian.wveDptFlag;

     strcpy(pEngineSlit->slitFile,pMediateSlit->gaussian.filename);

    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_INVPOLY :
                                              // 2n-Lorentz (generalisation of the Lorentzian function
     pEngineSlit->slitParam=pMediateSlit->lorentz.width;
     pEngineSlit->slitParam2=2*pMediateSlit->lorentz.order;
     pEngineSlit->slitWveDptFlag=pMediateSlit->lorentz.wveDptFlag;

     strcpy(pEngineSlit->slitFile,pMediateSlit->lorentz.filename);

    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_VOIGT :                                                      // Voigt profile function

     pEngineSlit->slitParam=pMediateSlit->voigt.fwhmL;
     pEngineSlit->slitParam2=pMediateSlit->voigt.glRatioL;
     pEngineSlit->slitWveDptFlag=pMediateSlit->voigt.wveDptFlag;

     strcpy(pEngineSlit->slitFile,pMediateSlit->voigt.filename);
     strcpy(pEngineSlit->slitFile2,pMediateSlit->voigt.filename2);

    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_ERF :                                                        // error function (convolution of a Gaussian and a boxcar)

     pEngineSlit->slitParam=pMediateSlit->error.fwhm;
     pEngineSlit->slitParam2=pMediateSlit->error.width;
     pEngineSlit->slitWveDptFlag=pMediateSlit->error.wveDptFlag;

     strcpy(pEngineSlit->slitFile,pMediateSlit->error.filename);
     strcpy(pEngineSlit->slitFile2,pMediateSlit->error.filename2);

    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_AGAUSS :                                                     // Asymmetric Gaussian

     pEngineSlit->slitParam=pMediateSlit->agauss.fwhm;
     pEngineSlit->slitParam2=pMediateSlit->agauss.asym;

     pEngineSlit->slitWveDptFlag=pMediateSlit->agauss.wveDptFlag;

     strcpy(pEngineSlit->slitFile,pMediateSlit->agauss.filename);
     strcpy(pEngineSlit->slitFile2,pMediateSlit->agauss.filename2);

    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_SUPERGAUSS :                                                 // Super Gaussian

     pEngineSlit->slitParam=pMediateSlit->supergauss.fwhm;
     pEngineSlit->slitParam2=pMediateSlit->supergauss.exponential;
     pEngineSlit->slitParam3=pMediateSlit->supergauss.asym;

     pEngineSlit->slitWveDptFlag=pMediateSlit->supergauss.wveDptFlag;

     strcpy(pEngineSlit->slitFile,pMediateSlit->supergauss.filename);
     strcpy(pEngineSlit->slitFile2,pMediateSlit->supergauss.filename2);
     strcpy(pEngineSlit->slitFile3,pMediateSlit->supergauss.filename3);

    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_APOD :                                                       // apodisation function (used with FTS)
     pEngineSlit->slitParam=pMediateSlit->boxcarapod.resolution;
     pEngineSlit->slitParam2=pMediateSlit->boxcarapod.phase;
    break;
 // ----------------------------------------------------------------------------
    case SLIT_TYPE_APODNBS :                                                    // apodisation function (Norton Beer Strong function)
     pEngineSlit->slitParam=pMediateSlit->nbsapod.resolution;
     pEngineSlit->slitParam2=pMediateSlit->nbsapod.phase;
    break;
 // ----------------------------------------------------------------------------
 // NOT USED : commented on 01/02/2012    case SLIT_TYPE_GAUSS_FILE :
 // NOT USED : commented on 01/02/2012     strcpy(pEngineSlit->slitFile,pMediateSlit->gaussianfile.filename);
 // NOT USED : commented on 01/02/2012    break;
 // NOT USED : commented on 01/02/2012 // ----------------------------------------------------------------------------
 // NOT USED : commented on 01/02/2012    case SLIT_TYPE_INVPOLY_FILE :                                               // 2n-Lorentz line shape, wavelength dependent (file)
 // NOT USED : commented on 01/02/2012     strcpy(pEngineSlit->slitFile,pMediateSlit->lorentzfile.filename);
 // NOT USED : commented on 01/02/2012     pEngineSlit->slitParam2=pMediateSlit->lorentzfile.degree;
 // NOT USED : commented on 01/02/2012    break;
 // NOT USED : commented on 01/02/2012 // ----------------------------------------------------------------------------
 // NOT USED : commented on 01/02/2012    case SLIT_TYPE_ERF_FILE :                                                   // error function, wavelength dependent (file)
 // NOT USED : commented on 01/02/2012     strcpy(pEngineSlit->slitFile,pMediateSlit->errorfile.filename);
 // NOT USED : commented on 01/02/2012     strcpy(pEngineSlit->slitFile2,pMediateSlit->errorfile.filename2);
 // NOT USED : commented on 01/02/2012    break;
 // NOT USED : commented on 12/01/2012 // ----------------------------------------------------------------------------
 // NOT USED : commented on 12/01/2012    case SLIT_TYPE_GAUSS_T_FILE :
 // NOT USED : commented on 12/01/2012     strcpy(pEngineSlit->slitFile,pMediateSlit->gaussiantempfile.filename);
 // NOT USED : commented on 12/01/2012    break;
 // NOT USED : commented on 12/01/2012 // ----------------------------------------------------------------------------
 // NOT USED : commented on 12/01/2012    case SLIT_TYPE_ERF_T_FILE :
 // NOT USED : commented on 12/01/2012     strcpy(pEngineSlit->slitFile,pMediateSlit->errortempfile.filename);
 // NOT USED : commented on 12/01/2012    break;
 // NOT USED : commented on 12/01/2012 // ----------------------------------------------------------------------------
    default :
     pEngineSlit->slitWveDptFlag=pMediateSlit->file.wveDptFlag;
     strcpy(pEngineSlit->slitFile,pMediateSlit->file.filename);
     strcpy(pEngineSlit->slitFile2,pMediateSlit->file.filename2);
    break;
 // ----------------------------------------------------------------------------
   }
 }

// -----------------------------------------------------------------------------
// FUNCTION      setMediateFilter
// -----------------------------------------------------------------------------
// PURPOSE       Filtering configuration
// ----------------------------------------------------------------------------

void setMediateFilter(PRJCT_FILTER *pEngineFilter,const mediate_filter_t *pMediateFilter,int hpFilterFlag,int convoluteFlag)
 {
     // Initializations

     memset(pEngineFilter,0,sizeof(PRJCT_FILTER));

     pEngineFilter->type=pMediateFilter->mode;
     pEngineFilter->filterEffWidth=(double)0.;

     // Fill up the structure according to the filter type

     switch(pEngineFilter->type)
      {
 // ----------------------------------------------------------------------------
    case PRJCT_FILTER_TYPE_KAISER :                                             // kaiser filter

     pEngineFilter->kaiserCutoff=(float)pMediateFilter->kaiser.cutoffFrequency; // cutoff frequency for kaiser filter type
     pEngineFilter->kaiserPassBand=(float)pMediateFilter->kaiser.passband;      // pass band for kaiser filter type
     pEngineFilter->kaiserTolerance=(float)pMediateFilter->kaiser.tolerance;    // tolerance for kaiser filter type

     pEngineFilter->filterNTimes=pMediateFilter->kaiser.iterations;

     if (hpFilterFlag)
      {
       pEngineFilter->hpFilterCalib=pMediateFilter->kaiser.usage.calibrationFlag;
       pEngineFilter->hpFilterAnalysis=pMediateFilter->kaiser.usage.fittingFlag;
      }

     if (convoluteFlag)
      pEngineFilter->filterAction=(pMediateFilter->kaiser.usage.divide)?PRJCT_FILTER_OUTPUT_HIGH_DIV:PRJCT_FILTER_OUTPUT_HIGH_SUB;

    break;
 // ----------------------------------------------------------------------------
    case PRJCT_FILTER_TYPE_BOXCAR :                                             // boxcar filter

     pEngineFilter->filterWidth=pMediateFilter->boxcar.width;
     pEngineFilter->filterNTimes=pMediateFilter->boxcar.iterations;

     if (hpFilterFlag)
      {
       pEngineFilter->hpFilterCalib=pMediateFilter->boxcar.usage.calibrationFlag;
       pEngineFilter->hpFilterAnalysis=pMediateFilter->boxcar.usage.fittingFlag;
      }

     if (convoluteFlag)
      pEngineFilter->filterAction=(pMediateFilter->boxcar.usage.divide)?PRJCT_FILTER_OUTPUT_HIGH_DIV:PRJCT_FILTER_OUTPUT_HIGH_SUB;

    break;
 // ----------------------------------------------------------------------------
    case PRJCT_FILTER_TYPE_GAUSSIAN :                                           // gaussian filter

     pEngineFilter->fwhmWidth=(float)pMediateFilter->gaussian.fwhm;
     pEngineFilter->filterNTimes=pMediateFilter->gaussian.iterations;

     if (hpFilterFlag)
      {
       pEngineFilter->hpFilterCalib=pMediateFilter->gaussian.usage.calibrationFlag;
       pEngineFilter->hpFilterAnalysis=pMediateFilter->gaussian.usage.fittingFlag;
      }

     if (convoluteFlag)
      pEngineFilter->filterAction=(pMediateFilter->gaussian.usage.divide)?PRJCT_FILTER_OUTPUT_HIGH_DIV:PRJCT_FILTER_OUTPUT_HIGH_SUB;

    break;
 // ----------------------------------------------------------------------------
    case PRJCT_FILTER_TYPE_TRIANGLE :                                           // triangular filter

     pEngineFilter->filterWidth=pMediateFilter->triangular.width;
     pEngineFilter->filterNTimes=pMediateFilter->triangular.iterations;

     if (hpFilterFlag)
      {
       pEngineFilter->hpFilterCalib=pMediateFilter->triangular.usage.calibrationFlag;
       pEngineFilter->hpFilterAnalysis=pMediateFilter->triangular.usage.fittingFlag;
      }

     if (convoluteFlag)
      pEngineFilter->filterAction=(pMediateFilter->triangular.usage.divide)?PRJCT_FILTER_OUTPUT_HIGH_DIV:PRJCT_FILTER_OUTPUT_HIGH_SUB;

    break;
 // ----------------------------------------------------------------------------
    case PRJCT_FILTER_TYPE_SG :                                                 // savitzky-Golay filter

     pEngineFilter->filterWidth=pMediateFilter->savitzky.width;
     pEngineFilter->filterOrder=pMediateFilter->savitzky.order;
     pEngineFilter->filterNTimes=pMediateFilter->savitzky.iterations;

     if (hpFilterFlag)
      {
       pEngineFilter->hpFilterCalib=pMediateFilter->savitzky.usage.calibrationFlag;
       pEngineFilter->hpFilterAnalysis=pMediateFilter->savitzky.usage.fittingFlag;
      }

     if (convoluteFlag)
      pEngineFilter->filterAction=(pMediateFilter->savitzky.usage.divide)?PRJCT_FILTER_OUTPUT_HIGH_DIV:PRJCT_FILTER_OUTPUT_HIGH_SUB;

    break;
 // ----------------------------------------------------------------------------
    case PRJCT_FILTER_TYPE_BINOMIAL :                                           // binomial filter

     pEngineFilter->filterWidth=pMediateFilter->binomial.width;
     pEngineFilter->filterNTimes=pMediateFilter->binomial.iterations;

     if (hpFilterFlag)
      {
       pEngineFilter->hpFilterCalib=pMediateFilter->binomial.usage.calibrationFlag;
       pEngineFilter->hpFilterAnalysis=pMediateFilter->binomial.usage.fittingFlag;
      }

     if (convoluteFlag)
      pEngineFilter->filterAction=(pMediateFilter->binomial.usage.divide)?PRJCT_FILTER_OUTPUT_HIGH_DIV:PRJCT_FILTER_OUTPUT_HIGH_SUB;

    break;
 // ----------------------------------------------------------------------------
      }
 }
