
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  Ocean optics files read out
//  Name of module    :  OceanOptics.C
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
//  MODULE DESCRIPTION
//
//  This module contains the routines needed for reading the data from OCEAN
//  OPTICS
//
//  ----------------------------------------------------------------------------
//
//  FUNCTIONS
//
//  SetOceanOptics - set file pointers for Ocean optics files;
//  ReliOceanOptics - read Ocean Optics file;
//
//  ----------------------------------------------------------------------------

// =======
// INCLUDE
// =======

#include <string.h>
#include <math.h>

#include "doas.h"
#include "engine_context.h"
#include "vector.h"
#include "winthrd.h"
#include "zenithal.h"

const char *oomonth[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

// -----------------------------------------------------------------------------
// FUNCTION        SetOceanOptics
// -----------------------------------------------------------------------------
// PURPOSE         Initialize pointers
//
// INPUT           pEngineContext : information on the file to read
//                 specFp    : pointer to the ASCII file
//
// OUTPUT
//
// RETURN          ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//                 ERROR_ID_ALLOC if the allocation of a buffer failed;
//                 ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC SetOceanOptics(ENGINE_CONTEXT *pEngineContext,FILE *specFp)
 {
  // Declarations

  RC rc;                                                                        // return code

  // Initializations

  rc=ERROR_ID_NO;

  // Check the file pointer

  if (specFp==NULL)
   rc=ERROR_SetLast("SetOceanOptics",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else
   {
   	// Get the number of records in the file

    fseek(specFp,0L,SEEK_SET);
    pEngineContext->recordNumber=1;
   }

  // Return

  return rc;
 }

// -----------------------------------------------------------------------------
// FUNCTION        ReliOceanOptics
// -----------------------------------------------------------------------------
// PURPOSE         Read a record from the ASCII file
//
// INPUT           pEngineContext : information on the file to read
//                 recordNo  : the index of the record to read
//                 dateFlag  : 1 to search for a reference spectrum
//                 localDay  : if dateFlag is 1, the calendar day for the
//                             reference spectrum to search for
//                 specFp    : pointer to the ASCII file
//
// OUTPUT          information on the read out record
//
// RETURN          ERROR_ID_FILE_NOT_FOUND if the input file pointer is NULL;
//                 ERROR_ID_FILE_END if the end of the file is reached;
//                 ERROR_ID_FILE_EMPTY if the file is not large enough;
//                 ERROR_ID_ALLOC if the allocation of a buffer failed;
//                 ERROR_ID_FILE_RECORD if the record doesn't match the spectra selection criteria
//                 ERROR_ID_NO in case of success.
// -----------------------------------------------------------------------------

RC ReliOceanOptics(ENGINE_CONTEXT *pEngineContext,int recordNo,int dateFlag,int localDay,FILE *specFp)
 {
  // Declarations

  RECORD_INFO *pRecordInfo;                                                     // pointer to the record part of the engine context
  char fileLine[MAX_ITEM_TEXT_LEN],*str;
  double *spectrum,*lambda,                                                     // the spectrum and the wavelength calibration to read
          tmLocal;                                                              // the measurement time in seconds
  int day,mon,year,hour,minutes,sec;                                            // decomposition of the measurement date
  char tmp[100],weekday[100],month[100];
  INDEX i;                                                                      // browse items to read
  RC rc;                                                                        // return code
  int spectrumFound;
  int npixels;

  // Initializations
  const int n_wavel = NDET[0];
  pRecordInfo=&pEngineContext->recordInfo;
  spectrum=pEngineContext->buffers.spectrum;
  lambda=pEngineContext->buffers.lambda;
  spectrumFound=i=0;
  mon=0;
  rc=ERROR_ID_NO;

  memset(&pRecordInfo->present_datetime,0,sizeof(pRecordInfo->present_datetime));

  memset(month,0,100);

  VECTOR_Init(spectrum,(double)0.,n_wavel);
  VECTOR_Init(lambda,(double)0.,n_wavel);

  // Set file pointers

  if (specFp==NULL)
   rc=ERROR_SetLast("ReliOceanOptics",ERROR_TYPE_WARNING,ERROR_ID_FILE_NOT_FOUND,pEngineContext->fileInfo.fileName);
  else if ((recordNo<=0) || (recordNo>pEngineContext->recordNumber))
   rc=ERROR_ID_FILE_END;
  else
   {
    npixels=n_wavel;
   	while (!feof(specFp) && fgets(fileLine,MAX_ITEM_TEXT_LEN,specFp))
   	 {
   	 	if  (!spectrumFound)
   	 	 {
   	 	 	if ((str=strstr(fileLine,"Integration Time (usec)"))!=NULL)
   	 	 	 {
   	 	 	  sscanf(fileLine,"Integration Time (usec): %lf",&pRecordInfo->Tint);
   	 	 	  pRecordInfo->Tint*=(double)1.e-6;
   	 	 	 }
   	 	 	else if ((str=strstr(fileLine,"Integration Time (sec)"))!=NULL)
  	 	 	  sscanf(fileLine,"Integration Time (sec): %lf",&pRecordInfo->Tint);
   	 	 	else if ((str=strstr(fileLine,"Spectra Averaged: "))!=NULL)
  	 	 	  sscanf(fileLine,"Spectra Averaged: %d",&pRecordInfo->NSomme);
  	 	 	 else if ((str=strstr(fileLine,"Date: "))!=NULL)
  	 	 	  sscanf(fileLine,"Date: %[^' '] %[^' '] %d %d:%d:%d %[^' '] %d",weekday,month,&day,&hour,&minutes,&sec,tmp,&year);
   	 	 	else if ((str=strstr(fileLine,"Number of Pixels in Processed Spectrum: "))!=NULL)
   	 	 	 {
  	 	 	   sscanf(fileLine,"Number of Pixels in Processed Spectrum: %d",&npixels);
  	 	 	   if (npixels>n_wavel)
  	 	 	    npixels=n_wavel;
  	 	 	  }
   	 	 	else if ((str=strstr(fileLine,">>>>>Begin Processed Spectral Data<<<<<"))!=NULL)
  	 	 	  spectrumFound=1;
   	 	 }
   	 	else if (i<npixels)
   	 	 {
	 	 	   sscanf(fileLine,"%lf %lf",&lambda[i],&spectrum[i]);
	 	 	   i++;
	 	 	  }
   	 }

   	if (strlen(month))
   	 {
   	 	for (mon=0;mon<12;mon++)
   	 	 if (!strncmp(month,oomonth[mon],3))
   	 	  break;

   	 	if (mon<12)
   	 	 {
   	 	 	pRecordInfo->present_datetime.thedate.da_day=(char)day;
   	 	 	pRecordInfo->present_datetime.thedate.da_mon=(char)mon+1;
   	 	 	pRecordInfo->present_datetime.thedate.da_year=year;

   	 	 	pRecordInfo->present_datetime.thetime.ti_hour=(char)hour;
   	 	 	pRecordInfo->present_datetime.thetime.ti_min=(char)minutes;
   	 	 	pRecordInfo->present_datetime.thetime.ti_sec=(char)sec;

        pRecordInfo->Tm=(double)ZEN_NbSec(&pRecordInfo->present_datetime.thedate,&pRecordInfo->present_datetime.thetime,0);

        tmLocal=pRecordInfo->Tm+THRD_localShift*3600.;

        pRecordInfo->localCalDay=ZEN_FNCaljda(&tmLocal);
        pRecordInfo->localTimeDec=fmod(pRecordInfo->TimeDec+24.+THRD_localShift,(double)24.);
   	 	 }
   	 }

    if (!rc && (mon>0) && (mon<=12) && dateFlag && (pRecordInfo->localCalDay!=localDay))
     rc=ERROR_ID_FILE_RECORD;
   }

  // Return

  return rc;
 }

